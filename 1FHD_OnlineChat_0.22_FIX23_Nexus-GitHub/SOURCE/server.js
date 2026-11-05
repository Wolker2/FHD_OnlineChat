export default {
  async scheduled(controller,env,ctx){
    // Runs only from Cloudflare cron, not from connected players.
    // Morning: HEAD metadata only; evening: verify actual ZIP and version.
    const mode=controller.cron==="0 5 * * *"?"light":"deep";
    ctx.waitUntil(env.CHAT.getByName("global").fetch(
      new Request("https://internal.fhd/_internal/fhd-updates/"+mode)
    ));
  },
  async fetch(req,env){
    const url=new URL(req.url);
    if(url.pathname==="/health")return env.CHAT.getByName("global").fetch(req);
    if(url.pathname!=="/ws")return new Response("FHD OnlineChat primary — WebSocket: /ws?name=Ranger",{headers:{"content-type":"text/plain;charset=utf-8"}});
    if(req.headers.get("Upgrade")?.toLowerCase()!=="websocket")return new Response("WebSocket required",{status:426});
    let name=(url.searchParams.get("name")||"").trim();
    if(!/^[\p{L}\p{N}_ .-]{1,24}$/u.test(name))return new Response("Invalid name",{status:400});
    const origin=req.headers.get("origin");
    if(origin){try{if(new URL(origin).host!==url.host)return new Response("Origin blocked",{status:403})}catch{return new Response("Origin blocked",{status:403})}}
    return env.CHAT.getByName("global").fetch(req);
  }
};
import {DurableObject} from "cloudflare:workers";
// One room calendar for all primary clients, using IANA DST rules.
const roomClock=new Intl.DateTimeFormat("en-GB",{timeZone:"Europe/Kyiv",year:"numeric",month:"2-digit",day:"2-digit"});
function roomDay(ms){const p=roomClock.formatToParts(ms);return ["year","month","day"].map(k=>p.find(v=>v.type===k).value).join("")}
function dayWindow(ms){
 const day=roomDay(ms);let lo=ms-30*3600000,hi=ms;
 while(lo<hi){const mid=Math.floor((lo+hi)/2);if(roomDay(mid)===day)hi=mid;else lo=mid+1}const start=lo;
 lo=ms+1;hi=ms+30*3600000;while(lo<hi){const mid=Math.floor((lo+hi)/2);if(roomDay(mid)===day)lo=mid+1;else hi=mid}
 return {day,start,end:lo};
}
// FHD update checker. All requests are initiated only by two daily cron triggers.
// No periodic polling on turns/messages, no Google Apps Script, no archive storage.
const UPDATE_FILE_ID = "1MmcxyeAnziTpLhv54isczUvIrns0ygKF";
const UPDATE_LINK = "https://drive.google.com/file/d/" + UPDATE_FILE_ID + "/view";
const UPDATE_DOWNLOAD = "https://drive.usercontent.google.com/download?export=download&id=" + UPDATE_FILE_ID + "&confirm=t";
const MAX_ZIP_BYTES = 8 * 1024 * 1024;
const LEGACY_CLIENT_VERSION = "0.21";
const SYSTEM_SENDER_ID = "00000000-0000-4000-8000-000000000000";

// Admin authorization is based exclusively on the *owner's* local UUID,
// supplied by the game client as ?pid=... . Never authorize by nickname.
// The SHA-256 digest of that UUID is configured as a Cloudflare Worker secret
// (FHD_ADMIN_PID_SHA256). Missing secret means commands fail CLOSED.
const ADMIN_COMMAND_CLEAR = "\\fhd clear chat";
const ADMIN_COMMAND_UPDATE = "\\fhd system chat";
const ADMIN_COMMAND_TEST_LINK = "\\fhd test link";
const ADMIN_COMMAND_AUTH = "\\fhd auth";
const FIRST_CLEAR_DAY = "20261104"; // Explicit owner request: 04 Nov 2026 in Europe/Kyiv
const FIRST_CLEAR_MARKER = "fhd_once_clear_20261104_v1";
async function isAdminPid(pid,configuredDigest){
  if(typeof configuredDigest!=="string"||!/^[a-f0-9]{64}$/i.test(configuredDigest))return false;
  if(typeof pid!=="string"||!FHD_UUID4.test(pid))return false;
  const digest=await sha256Hex(new TextEncoder().encode(pid.toLowerCase()));
  return digest===configuredDigest.toLowerCase();
}

// Some shipped 0.22 DLL builds create a local UUID but DO NOT send it
// in the WebSocket URL. Keep optional pid compatibility for future clients.
// Never expose the raw UUID to peers:
// derive a stable public UUID from its SHA-256 instead. Legacy users keep random IDs.
const FHD_UUID4=/^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i;
async function publicIdFromPid(pid){
  if(typeof pid!=="string"||!FHD_UUID4.test(pid))return null;
  const canonical=pid.toLowerCase();
  if(canonical===SYSTEM_SENDER_ID)return null; // reserved SYSTEM identity
  const digest=new Uint8Array(await crypto.subtle.digest("SHA-256",
    new TextEncoder().encode("FHDOnlineChat/stable-public-id/v1:"+canonical)));
  const x=digest.slice(0,16);x[6]=(x[6]&15)|64;x[8]=(x[8]&63)|128;
  const h=Array.from(x,v=>v.toString(16).padStart(2,"0")).join("");
  return `${h.slice(0,8)}-${h.slice(8,12)}-${h.slice(12,16)}-${h.slice(16,20)}-${h.slice(20)}`;
}
async function sessionIdFromRequest(url,activeSockets){
  const proposed=await publicIdFromPid(url.searchParams.get("pid"));
  // If game folders were copied and two clients share a UUID, retain separate
  // live session IDs, rather than breaking private messaging / self coloring.
  if(proposed&&!activeSockets.some(ws=>ws.readyState===1&&
      ws.deserializeAttachment()?.id===proposed))return proposed;
  return crypto.randomUUID();
}

function normalizedVersion(v){
  const m=/^(?:v)?(\d{1,3})\.(\d{1,3})(?:\.(\d{1,3}))?(?:[-+][\w.-]+)?$/i.exec(String(v||"").trim());
  return m?[+m[1],+m[2],+(m[3]||0)]:null;
}
function newerThan(a,b){
  const x=normalizedVersion(a),y=normalizedVersion(b);
  if(!x||!y)return false;
  for(let i=0;i<3;i++)if(x[i]!==y[i])return x[i]>y[i];
  return false;
}
function parseVersionFromModuleInfo(text){
  // ModuleInfo.txt from the official SRHD mod archive is UTF-16LE.
  const lines=text.split(/\r?\n/);
  const preferred=lines.filter(x=>/^SmallDescription(?:Eng)?=/.test(x));
  for(const line of [...preferred,...lines.filter(x=>/^FullDescription(?:Eng)?=/.test(x))]){
    const m=/FHD[ _]OnlineChat\s+v?(\d{1,3}\.\d{1,3}(?:\.\d{1,3})?)/i.exec(line);
    if(m&&normalizedVersion(m[1]))return m[1];
  }
  return null;
}
// Read the central directory rather than relying on ZIP local-entry ordering.
async function versionFromZip(raw){
  const b=raw instanceof Uint8Array?raw:new Uint8Array(raw);
  if(b.byteLength<22||b.byteLength>MAX_ZIP_BYTES)throw Error("ZIP size invalid");
  const dv=new DataView(b.buffer,b.byteOffset,b.byteLength);
  const u16=p=>dv.getUint16(p,true),u32=p=>dv.getUint32(p,true);
  let eocd=-1;
  for(let p=b.length-22;p>=Math.max(0,b.length-65557);p--){
    if(u32(p)===0x06054b50&&p+22+u16(p+20)===b.length){eocd=p;break;}
  }
  if(eocd<0)throw Error("ZIP end not found");
  let cursor=u32(eocd+16),count=u16(eocd+10);
  const dec=new TextDecoder("utf-8");
  for(let i=0;i<count&&i<1000;i++){
    if(cursor+46>b.length||u32(cursor)!==0x02014b50)throw Error("ZIP central directory invalid");
    const method=u16(cursor+10),packed=u32(cursor+20),unpacked=u32(cursor+24);
    const nameLen=u16(cursor+28),extraLen=u16(cursor+30),commentLen=u16(cursor+32),offset=u32(cursor+42);
    const end=cursor+46+nameLen+extraLen+commentLen;
    if(end>b.length)throw Error("ZIP central directory truncated");
    const name=dec.decode(b.subarray(cursor+46,cursor+46+nameLen));
    cursor=end;
    if(!/(^|\/)ModuleInfo\.txt$/i.test(name))continue;
    if(unpacked>64*1024||offset+30>b.length||u32(offset)!==0x04034b50)throw Error("ZIP entry invalid");
    const start=offset+30+u16(offset+26)+u16(offset+28);
    if(start+packed>b.length)throw Error("ZIP entry truncated");
    const compressed=b.subarray(start,start+packed);
    let data;
    if(method===0)data=compressed;
    else if(method===8){
      const stream=new Blob([compressed]).stream().pipeThrough(new DecompressionStream("deflate-raw"));
      data=new Uint8Array(await new Response(stream).arrayBuffer());
    }else throw Error("Unsupported ZIP compression");
    if(data.length!==unpacked)throw Error("ZIP entry size mismatch");
    const info=new TextDecoder(data[0]===0xff&&data[1]===0xfe?"utf-16le":"utf-8").decode(data);
    const version=parseVersionFromModuleInfo(info);
    if(!version)throw Error("ModuleInfo version absent");
    return version;
  }
  throw Error("ModuleInfo.txt not found");
}
// A tagged client build is read from the exact DLL shipped in the ZIP.
// The 0.22 game DLL already transmits ?build=... in its WebSocket URL.
// Only a deliberately stamped FHD1+16hex build ID is trusted.
async function stampedClientBuildFromZip(raw){
  const b=raw instanceof Uint8Array?raw:new Uint8Array(raw);
  const view=new DataView(b.buffer,b.byteOffset,b.byteLength);
  const u16=p=>view.getUint16(p,true),u32=p=>view.getUint32(p,true);
  let eocd=-1;
  for(let p=b.length-22;p>=Math.max(0,b.length-65557);--p){
    if(u32(p)===0x06054b50&&p+22+u16(p+20)===b.length){eocd=p;break;}
  }
  if(eocd<0)return null;
  let cursor=u32(eocd+16),count=u16(eocd+10);
  const decoder=new TextDecoder('utf-8');
  for(let i=0;i<count&&i<1000;i++){
    if(cursor+46>b.length||u32(cursor)!==0x02014b50)return null;
    const method=u16(cursor+10),packed=u32(cursor+20),unpacked=u32(cursor+24);
    const nameLen=u16(cursor+28),extraLen=u16(cursor+30),commentLen=u16(cursor+32),offset=u32(cursor+42);
    if(cursor+46+nameLen+extraLen+commentLen>b.length)return null;
    const name=decoder.decode(b.subarray(cursor+46,cursor+46+nameLen));
    cursor+=46+nameLen+extraLen+commentLen;
    if(!/(^|\/)Native\/FHDOnlineChat\.XenoPlugin\.dll$/i.test(name))continue;
    if(unpacked>2*1024*1024||offset+30>b.length||u32(offset)!==0x04034b50)return null;
    const start=offset+30+u16(offset+26)+u16(offset+28);
    if(start+packed>b.length)return null;
    const compressed=b.subarray(start,start+packed);
    let file;
    if(method===0)file=compressed;
    else if(method===8){
      const stream=new Blob([compressed]).stream().pipeThrough(new DecompressionStream('deflate-raw'));
      file=new Uint8Array(await new Response(stream).arrayBuffer());
    }else return null;
    if(file.length!==unpacked)return null;
    const pattern=new TextEncoder().encode('build=');
    // Literal is UTF-16LE. Search only the bounded native DLL for its build query.
    for(let k=0;k+2*(pattern.length+20)<=file.length;k+=1){
      let found=true;for(let j=0;j<pattern.length;j++){
        if(file[k+j*2]!==pattern[j]||file[k+j*2+1]!==0){found=false;break;}
      }
      if(!found)continue;
      let value='';for(let j=0;j<20;j++){
        const lo=file[k+(pattern.length+j)*2],hi=file[k+(pattern.length+j)*2+1];
        if(hi!==0||lo===0){value='';break;}value+=String.fromCharCode(lo);
      }
      if(/^FHD1[0-9a-f]{16}$/i.test(value))return value.toLowerCase();
    }
    return null;
  }
  return null;
}

function driveMetadataChanged(previous,current){
  if(!previous)return true;
  const keys=["etag","modified","length"];
  const comparable=keys.filter(k=>previous[k]&&current[k]);
  // Missing consistent validators: verify once rather than silently assuming
  // that a newly uploaded archive is unchanged.
  return comparable.length===0||comparable.some(k=>previous[k]!==current[k]);
}
function updateHeaders(response){
  return {
    etag:response.headers.get("etag")||null,
    modified:response.headers.get("last-modified")||null,
    length:response.headers.get("content-length")||null,
    contentType:response.headers.get("content-type")||null,
  };
}
async function getDriveHead(){
  const r=await fetch(UPDATE_DOWNLOAD,{method:"HEAD",redirect:"follow",signal:AbortSignal.timeout(15000)});
  if(!r.ok)throw Error("Drive HEAD HTTP "+r.status);
  const h=updateHeaders(r);
  // Google's HTML confirmation/permission page is NOT valid file metadata.
  if(/text\/html/i.test(h.contentType||""))throw Error("Drive HEAD returned HTML");
  return h;
}
async function getVerifiedDriveZip(){
  const r=await fetch(UPDATE_DOWNLOAD,{method:"GET",redirect:"follow",signal:AbortSignal.timeout(25000),headers:{"Accept":"application/zip,application/octet-stream"}});
  if(!r.ok)throw Error("Drive download HTTP "+r.status);
  if(/text\/html/i.test(r.headers.get("content-type")||""))throw Error("Drive returned HTML instead of ZIP");
  const size=Number(r.headers.get("content-length")||0);
  if(size>MAX_ZIP_BYTES)throw Error("Drive ZIP exceeds configured limit");
  const reader=r.body?.getReader();if(!reader)throw Error("Drive response is empty");
  const chunks=[];let used=0;
  try{
    for(;;){const {done,value}=await reader.read();if(done)break;used+=value.byteLength;
      if(used>MAX_ZIP_BYTES)throw Error("Drive ZIP exceeds configured limit");chunks.push(value);}
  }finally{reader.releaseLock()}
  const bytes=new Uint8Array(used);let pos=0;for(const chunk of chunks){bytes.set(chunk,pos);pos+=chunk.byteLength;}
  if(bytes[0]!==0x50||bytes[1]!==0x4b)throw Error("Drive returned non-ZIP data");
  return {bytes,headers:updateHeaders(r)};
}
async function sha256Hex(bytes){
  const raw=await crypto.subtle.digest("SHA-256",bytes);
  return Array.from(new Uint8Array(raw),n=>n.toString(16).padStart(2,"0")).join("");
}

export class ChatRoom extends DurableObject{
 constructor(ctx,env){super(ctx,env);
 ctx.setWebSocketAutoResponse(new WebSocketRequestResponsePair('{"type":"ping"}','{"type":"pong"}'));ctx.storage.sql.exec("CREATE TABLE IF NOT EXISTS messages(id INTEGER PRIMARY KEY AUTOINCREMENT,at INTEGER NOT NULL,nick TEXT NOT NULL,body TEXT NOT NULL)");
 const columns=new Set([...ctx.storage.sql.exec("PRAGMA table_info(messages)")].map(c=>c.name));
 if(!columns.has("sender_id"))ctx.storage.sql.exec("ALTER TABLE messages ADD COLUMN sender_id TEXT");
 this.nickColumns=["nick","sender"].filter(n=>columns.has(n));
 if(!this.nickColumns.length)throw new Error("Unsupported history schema");
 this.nameExpr=this.nickColumns.length===2?"COALESCE(nick,sender)":this.nickColumns[0];
 ctx.storage.sql.exec("CREATE TABLE IF NOT EXISTS fhd_chat_day(id INTEGER PRIMARY KEY,day TEXT NOT NULL)");
 this.ready=ctx.blockConcurrencyWhile(async()=>{
   this.rollDay();
   // Explicit, date-scoped one-time deletion requested by the owner.
   // Idempotent in durable storage. Never repeat on a future day/restart.
   if(this.day===FIRST_CLEAR_DAY&&!await ctx.storage.get(FIRST_CLEAR_MARKER)){
     const removed=this.deleteTodayMessages();
     await ctx.storage.put(FIRST_CLEAR_MARKER,{removed,at:Date.now(),day:this.day});
     this.resetClientHistories();
   }
   if(await ctx.storage.getAlarm()===null)await ctx.storage.setAlarm(this.dayEnd)});
 }
 deleteTodayMessages(){
  // Only the current Kyiv calendar day, not other dates or private messages.
  const row=[...this.ctx.storage.sql.exec("SELECT COUNT(*) AS total FROM messages WHERE at >= ? AND at < ?",this.dayStart,this.dayEnd)][0];
  this.ctx.storage.sql.exec("DELETE FROM messages WHERE at >= ? AND at < ?",this.dayStart,this.dayEnd);
  return Number(row?.total||0);
 }
 resetClientHistories(){
  // 0.21/0.22 native clients do NOT clear history upon a same-day 'reset'.
  // An ordinary 'hello' containing history:[] invokes their existing
  // parse_history() -> clear_history() path without touching any DLL.
  for(const ws of this.ctx.getWebSockets()){
    if(ws.readyState!==1)continue;
    const a=ws.deserializeAttachment();if(!a?.id)continue;
    this.send(ws,{type:"hello",id:a.id,name:a.name,version:"0.20",day:this.day,history:[]});
  }
  this.presence();
 }
 async handleAdminAuth(ws,me,rawText){
  // Legacy clients cannot transmit ?pid. Accept an admin code through the
  // EXISTING chat protocol; intercept before persistence/broadcast/logging.
  const found=/^\\fhd\s+auth\s+(\S+)$/i.exec(rawText.trim());
  if(!found){this.send(ws,{type:"error",message:"Формат: \\fhd auth <код>"});return}
  const now=Date.now();
  const since=(me.authWindowSince&&now-me.authWindowSince<300000)?me.authWindowSince:now;
  const attempts=(since===me.authWindowSince?(me.authAttempts||0):0)+1;
  const changed={...me,authWindowSince:since,authAttempts:attempts};
  if(attempts>5){ws.serializeAttachment(changed);this.send(ws,{type:"error",message:"Слишком много попыток. Повтори через 5 минут"});return}
  const expected=this.env.FHD_ADMIN_LOGIN_SHA256;
  const actual=await sha256Hex(new TextEncoder().encode(found[1]));
  let diff=0;
  if(typeof expected!=="string"||!/^[0-9a-f]{64}$/i.test(expected))diff=1;
  else for(let i=0;i<64;i++)diff|=actual.charCodeAt(i)^expected.toLowerCase().charCodeAt(i);
  if(diff!==0){ws.serializeAttachment(changed);this.send(ws,{type:"error",message:"Неверный код администратора"});return}
  ws.serializeAttachment({...changed,admin:true,authAttempts:0});
  this.send(ws,{type:"error",message:"Права администратора активированы для этого подключения"});
 }
 async handleAdminCommand(ws,me,command){
  if(!me.admin){this.send(ws,{type:"error",message:"Требуется вход администратора: \\fhd auth <код>"});return}
  await this.checkDay();
  if(command===ADMIN_COMMAND_CLEAR){
    const removed=this.deleteTodayMessages();
    this.resetClientHistories();
    this.send(ws,{type:"error",message:`Чат очищен: ${removed} сообщений за ${this.day} (Киев)`});
    return;
  }
  if(command===ADMIN_COMMAND_TEST_LINK){
    const release=(await this.ctx.storage.get("fhd_update_state"))?.release;
    if(!release){this.send(ws,{type:"error",message:"Нет подтверждённого ZIP на Google Диске для теста ссылки"});return}
    this.send(ws,{type:"error",message:"ТЕСТ: нажмите СКАЧАТЬ в выделенном уведомлении (ЛКМ/ПКМ — копировать и открыть браузер)."});
    // The native highlighted update alone is interactive, including its wrapped line.
    this.send(ws,{type:"update",version:"FIX",build:release.digest||release.version,kind:"test"});
    return;
  }
  if(command===ADMIN_COMMAND_UPDATE){
    const now=Date.now();
    if(this.adminLastUpdateCheckAt&&now-this.adminLastUpdateCheckAt<30000){
      this.send(ws,{type:"error",message:"Проверка обновлений уже выполнялась менее 30 секунд назад"});return;
    }
    this.adminLastUpdateCheckAt=now;
    const result=await this.checkUpdate("deep",true);
    if(!result.ok){this.send(ws,{type:"error",message:`Проверка обновлений: ${result.error}`});return}
    const release=(await this.ctx.storage.get("fhd_update_state"))?.release;
    const currentAtt=ws.deserializeAttachment()||me;
    let installed="точно не определена";
    if(release){
      if(newerThan(release.version,currentAtt.version||LEGACY_CLIENT_VERSION))installed="устарела (номер версии)";
      else if(release.buildId&&String(currentAtt.build||"").toLowerCase()===release.buildId.toLowerCase())installed="актуальна (Build ID совпадает)";
      else if(release.digest&&String(currentAtt.build||"").toLowerCase()===release.digest.toLowerCase())installed="актуальна (SHA-256 совпадает)";
    }
    this.send(ws,{type:"error",message:`Проверка завершена. Версия на Диске: ${result.version||"не определена"}; ZIP изменился с предыдущей проверки: ${result.digestChanged?"да":"нет"}; ваша сборка: ${installed}; уведомлений отправлено: ${result.notified||0}.`});
  }
 }
 async checkUpdate(mode,forceNotify=false){
  // A morning HEAD can reveal a changed upload but not its version.
  // If metadata changed (or HEAD is denied), do one full verification then
  // immediately notify outdated clients. Otherwise morning uses HEAD only.
  const previous=await this.ctx.storage.get("fhd_update_state")||{};
  const next={...previous,lastAttempt:new Date().toISOString(),lastMode:mode};
  let notified=0,verified=false,changed=false,digestChanged=false;
  const verifyZip=async()=>{
    const {bytes,headers}=await getVerifiedDriveZip();
    const [version,digest,readBuildId]=await Promise.all([
      versionFromZip(bytes),sha256Hex(bytes),stampedClientBuildFromZip(bytes)
    ]);
    digestChanged=previous.release?.digest!==digest;
    // If a new ZIP reuses an old build ID, it is stale: never treat clients
    // with that ID as verified-current after the archive has changed.
    const buildId=readBuildId&&(
      !previous.release?.buildId||previous.release.buildId!==readBuildId||!digestChanged
    )?readBuildId:null;
    const current={version,digest,buildId,url:UPDATE_LINK,verifiedAt:next.lastAttempt};
    next.metadata=headers;
    next.deepCheckedAt=next.lastAttempt;
    next.lastVerified={version,digest};
    next.release=current; // Exactly what Drive currently distributes.
    verified=true;
    // Any different ZIP SHA-256 is a new release, even when ModuleInfo
    // still says exactly the same version (many owner fixes use 0.22).
    const versionAdvanced=!previous.release||newerThan(version,previous.release.version);
    if(forceNotify||digestChanged||versionAdvanced){
      for(const w of this.ctx.getWebSockets())if(this.sendUpdateIfNeeded(w,current,forceNotify))notified++;
    }
  };
  try{
    if(mode==="light"){
      let shouldVerify=false;
      try{
        const head=await getDriveHead();
        changed=driveMetadataChanged(previous.metadata,head);
        next.metadata=head;
        next.lightCheckedAt=next.lastAttempt;
        shouldVerify=changed||!previous.release;
        next.headError=null;
      }catch(e){
        // HEAD is blocked or returns a Drive HTML confirmation page.
        // Try one verified GET, never issue an unverified update notice.
        next.headError=String(e?.message||e).slice(0,180);
        next.lightCheckedAt=next.lastAttempt;
        shouldVerify=true;
      }
      if(shouldVerify)await verifyZip();
    }else if(mode==="deep")await verifyZip();
    else throw Error("Unknown updater mode");
    next.lastError=null;
  }catch(e){
    next.lastError=String(e?.message||e).slice(0,180);
    // Preserve last verified release on failure; never send false notice.
    next.release=previous.release;
  }
  await this.ctx.storage.put("fhd_update_state",next);
  return {ok:!next.lastError,mode,version:next.release?.version||null,
    verified,changed,digestChanged,notified,error:next.lastError};
 }
 sendUpdateIfNeeded(ws,release,forceNotify=false){
  if(!release||ws.readyState!==1)return false;
  const a=ws.deserializeAttachment();if(!a?.id)return false;
  const clientVersion=a.version||LEGACY_CLIENT_VERSION;
  // Never prompt a client running a numerically newer release.
  if(newerThan(clientVersion,release.version))return false;
  const digest=typeof release.digest==='string'&&/^[0-9a-f]{64}$/i.test(release.digest)
    ?release.digest.toLowerCase():null;
  const clientBuild=String(a.build||'').toLowerCase();
  const versionAdvanced=newerThan(release.version,clientVersion);
  const sameClientBuild=Boolean(release.buildId&&clientBuild===release.buildId.toLowerCase());
  const exactFingerprint=Boolean(digest&&clientBuild===digest);
  // A stamped DLL's build ID matches the DLL read from the current ZIP.
  // A legacy DLL has a fixed '0.22-fixed-native021' label; it cannot prove
  // whether that DLL came from the latest archive. Treat it as unverified.
  if(!versionAdvanced&&(sameClientBuild||exactFingerprint))return false;
  if(!versionAdvanced&&!digest)return false;
  const noticeKey='zip-sha256-v2:'+(digest||release.version);
  if(!forceNotify&&a.updateNotified===noticeKey)return false;
  const target=normalizedVersion(clientVersion);
  const supportsNativeUpdate=target&&(target[0]>0||(target[0]===0&&target[1]>=22));
  if(supportsNativeUpdate){
    // The native 0.22 client renders a highlighted clickable message from
    // type=update: "Вышла новая версия FHD_OnlineChat " + version + " ! СКАЧАТЬ".
    // For an unchanged ModuleInfo version we pass the display-only token FIX;
    // keep actual release.version in storage/version comparisons unchanged.
    // This produces ONE highlighted message, not a duplicated SYSTEM chat line.
    this.send(ws,{type:'update',version:versionAdvanced?release.version:'FIX',
      build:digest||release.version,kind:versionAdvanced?'version':'fix'});
    // No separate URL message: the native highlighted СКАЧАТЬ is actionable.
  }else{
    // Legacy 0.21 clients cannot render clickable native update notices.
    const displayVersion=versionAdvanced?release.version:'FIX';
    const text=`Вышла новая версия FHD_OnlineChat ${displayVersion} ! СКАЧАТЬ: ${UPDATE_LINK}`;
    this.send(ws,{type:'chat',name:'SYSTEM',text,fromId:SYSTEM_SENDER_ID,day:this.day});
  }
  ws.serializeAttachment({...a,updateNotified:noticeKey});
  return true;
 }
 rollDay(){
  const now=Date.now(),day=roomDay(now);if(this.day===day)return false;
  const previous=[...this.ctx.storage.sql.exec("SELECT day FROM fhd_chat_day WHERE id=1")][0]?.day;
  const window=dayWindow(now);
  this.ctx.storage.sql.exec("DELETE FROM messages WHERE at < ?",window.start);
  this.ctx.storage.sql.exec("INSERT INTO fhd_chat_day(id,day) VALUES(1,?) ON CONFLICT(id) DO UPDATE SET day=excluded.day",day);
  this.day=day;this.dayStart=window.start;this.dayEnd=window.end;
  if(previous!==day)this.broadcast({type:"reset",day});return true;
 }
 async checkDay(){await this.ready;if(this.rollDay())await this.ctx.storage.setAlarm(this.dayEnd)}
 async alarm(){await this.ready;this.rollDay();await this.ctx.storage.setAlarm(this.dayEnd)}
 send(ws,obj){try{ws.send(JSON.stringify(obj))}catch{}}
 users(){return this.ctx.getWebSockets().filter(w=>w.readyState===1).map(w=>w.deserializeAttachment()).filter(v=>v?.id)}
 broadcast(v){for(const w of this.ctx.getWebSockets())this.send(w,v)}
  presence(){
    const sockets=this.ctx.getWebSockets().filter(w=>w.readyState===1);
    const users=sockets.map(w=>w.deserializeAttachment()).filter(u=>u?.id);
    const groups=new Map();
    for(const u of users){const a=groups.get(u.name)||[];a.push(u);groups.set(u.name,a)}
    const ordinals=new Map();
    for(const group of groups.values()){
      if(group.length<2){ordinals.set(group[0].id,0);continue}
      group.sort((a,b)=>(a.joined||0)-(b.joined||0)||a.id.localeCompare(b.id));
      for(let i=0;i<group.length;i++)ordinals.set(group[i].id,i+1);
    }
    // Names remain unchanged: older 0.21 clients and private messages still
    // resolve by the same session IDs. 0.22 UI uses ordinal only for duplicates.
    this.broadcast({type:"presence",online:users.length,
      users:users.map(u=>({id:u.id,name:u.name,ordinal:ordinals.get(u.id)||0}))});
  }
 async fetch(req){
 await this.checkDay();
 const internal=new URL(req.url).pathname;
 if(internal==="/_internal/fhd-updates/light"||internal==="/_internal/fhd-updates/deep"){
   return Response.json(await this.checkUpdate(internal.endsWith("light")?"light":"deep"));
 }
 if(new URL(req.url).pathname==="/health"){
  this.ctx.storage.sql.exec(`SELECT ${this.nameExpr} AS name,body FROM messages ORDER BY id DESC LIMIT 1`);
  return Response.json({ok:true,service:"FHD_OnlineChat",role:"primary",version:"0.20",storage_ready:true,day:this.day,time_zone:"Europe/Kyiv",next_reset:this.dayEnd});
 }
 if(this.ctx.getWebSockets().length>=120)return new Response("Room full",{status:503});
 const url=new URL(req.url);const name=url.searchParams.get("name");
 const id=await sessionIdFromRequest(url,this.ctx.getWebSockets());
 const admin=await isAdminPid(url.searchParams.get("pid"),this.env.FHD_ADMIN_PID_SHA256);
 const pair=new WebSocketPair();const a=pair[0],b=pair[1];this.ctx.acceptWebSocket(b);
 const supplied=new URL(req.url).searchParams.get("v");
 const version=normalizedVersion(supplied)?supplied:LEGACY_CLIENT_VERSION;
 const declaredBuild=url.searchParams.get('build')||'';
 const build=/^[a-zA-Z0-9._-]{1,80}$/.test(declaredBuild)?declaredBuild:'';
 b.serializeAttachment({id,name,last:0,joined:Date.now(),version,build,updateNotified:null,admin});
 const history=[...this.ctx.storage.sql.exec(`SELECT id,at,${this.nameExpr} AS name,body AS text,sender_id AS fromId FROM messages WHERE at >= ? ORDER BY id DESC LIMIT 30`,this.dayStart)].reverse().map(m=>({...m,day:this.day}));
 this.send(b,{type:"hello",id,name,version:"0.20",day:this.day,history});
 const update=await this.ctx.storage.get("fhd_update_state");
 this.sendUpdateIfNeeded(b,update?.release);
 this.presence();return new Response(null,{status:101,webSocket:a})}
 async webSocketMessage(ws,raw){await this.ready;const me=ws.deserializeAttachment();if(!me?.id)return;let data;try{data=JSON.parse(raw)}catch{return}
 if(data?.type==="ping"){this.send(ws,{type:"pong"});return}
 if(typeof data?.text!=="string"||data.text.trim().length===0||data.text.length>500||!["chat","private"].includes(data.type))return;
 // Intercept in the NORMAL public-chat packet before persistence/rate limiting.
 // Backslash commands never appear as public messages (even if unauthorized).
 if(data.type==="chat"){
   const command=data.text.trim().replace(/\s+/g," ").toLowerCase();
   if(command===ADMIN_COMMAND_AUTH||command.startsWith(ADMIN_COMMAND_AUTH+" ")){
     await this.handleAdminAuth(ws,me,data.text);return;
   }
   if(command===ADMIN_COMMAND_CLEAR||command===ADMIN_COMMAND_UPDATE||command===ADMIN_COMMAND_TEST_LINK){
     await this.handleAdminCommand(ws,me,command);return;
   }
   if(command.startsWith("\\fhd ")){
     this.send(ws,{type:"error",message:"Неизвестная команда FHD"});return;
   }
 }
 await this.checkDay();const now=Date.now();if(now-me.last<1200){this.send(ws,{type:"error",message:"Rate limit"});return}
 me.last=now;ws.serializeAttachment(me);
 const msg={type:data.type,id:crypto.randomUUID(),at:now,name:me.name,fromId:me.id,day:this.day,text:data.text.trim()};
 if(data.type==="private"){const peer=this.ctx.getWebSockets().find(w=>w.readyState===1&&w.deserializeAttachment()?.id===data.toId);if(!peer){this.send(ws,{type:"error",message:"Recipient offline"});return}const direct={...msg,toId:data.toId,toName:peer.deserializeAttachment().name};this.send(peer,direct);if(peer!==ws)this.send(ws,{...direct,sent:true});return}
 this.ctx.storage.sql.exec(`INSERT INTO messages(at,${this.nickColumns.join(",")},body,sender_id) VALUES(${Array(this.nickColumns.length+3).fill("?").join(",")})`,now,...this.nickColumns.map(()=>me.name),msg.text,me.id);
 this.ctx.storage.sql.exec("DELETE FROM messages WHERE id NOT IN(SELECT id FROM messages ORDER BY id DESC LIMIT 30)");this.broadcast(msg)
 }
 webSocketClose(ws){try{ws.close()}catch{}this.presence()}
 webSocketError(ws){try{ws.close(1011,"Socket error")}catch{}this.presence()}
}