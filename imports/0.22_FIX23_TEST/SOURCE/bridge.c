/* FHD OnlineChat 0.7 — single in-game XenoNativeLoader plugin, x86, WinHTTP/Win32.
 * No C# compiler, external EXE, console, tray icon or overlay in other apps.
 * Requires Windows 8+ and borderless/windowed SRHD with XenoNativeLoader 0.6.5+.
 */
typedef unsigned long DWORD; typedef unsigned int UINT; typedef int BOOL; typedef long LONG;
typedef unsigned short WORD, WCHAR; typedef unsigned char BYTE; typedef void *HANDLE,*HMODULE,*HWND,*HINSTANCE,*HDC,*HBRUSH,*HGDIOBJ,*HICON,*HCURSOR,*HMENU,*HINTERNET,*LPVOID;
typedef const WCHAR *LPCWSTR; typedef WCHAR *LPWSTR; typedef unsigned int SIZE_T; typedef unsigned long COLORREF;
#define WINAPI __stdcall
#define IMP __declspec(dllimport)
#define EXP __declspec(dllexport)
#define TRUE 1
#define FALSE 0
#define WS_POPUP 0x80000000u
#define WS_CHILD 0x40000000u
#define WS_VISIBLE 0x10000000u
#define WS_BORDER 0x00800000u
#define ES_AUTOHSCROLL 0x80u
#define WS_EX_TOPMOST 0x8u
#define WS_EX_TOOLWINDOW 0x80u
#define WS_EX_LAYERED 0x80000u
#define WS_EX_NOACTIVATE 0x08000000u
#define SW_SHOW 5
#define SW_HIDE 0
#define WM_CREATE 1
#define WM_DESTROY 2
#define WM_PAINT 15
#define WM_COMMAND 273
#define WM_TIMER 275
#define WM_HOTKEY 786
#define WM_KEYDOWN 256
#define WM_MOUSEWHEEL 0x020A
#define VK_F12 0x7B
#define MAX_MESSAGES 30
#define VISIBLE_MESSAGES 11
#define VK_RETURN 13
#define WM_APP_NET 0x8500
#define WM_APP_CHAT 0x8501
#define WM_SETFONT 48
#define LWA_ALPHA 2
#define MOD_CONTROL 2
#define MOD_SHIFT 4
#define HOTKEY 1209
#define GWL_STYLE -16
#define CS_HREDRAW 2
#define CS_VREDRAW 1
#define TRANSPARENT 1
#define DT_SINGLELINE 0x20u
#define DT_VCENTER 0x4u
#define DT_END_ELLIPSIS 0x8000u
#define DT_NOPREFIX 0x800u
#define HWND_TOPMOST ((HWND)(-1))
#define HWND_NOTOPMOST ((HWND)(-2))
#define SWP_NOACTIVATE 0x10
#define SWP_NOZORDER 0x4
#define WINHTTP_FLAG_SECURE 0x800000u
#define WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET 114
#define WINHTTP_QUERY_STATUS_CODE 19
#define WINHTTP_QUERY_FLAG_NUMBER 0x20000000u
#define GENERIC_WRITE 0x40000000u
#define FILE_SHARE_READ 1u
#define OPEN_ALWAYS 4u
#define FILE_ATTRIBUTE_NORMAL 0x80u
#define FILE_END 2u
#define FILE_BEGIN 0u
#define FILE_ATTRIBUTE_DIRECTORY 0x10u
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#define CSIDL_PERSONAL 5
#define SWP_SHOWWINDOW 0x0040u
typedef struct {WORD year,month,dayOfWeek,day,hour,minute,second,milliseconds;} SYSTEMTIME;
typedef struct {DWORD low,high;} FILETIME;
typedef struct {DWORD attributes;FILETIME created,accessed,modified;DWORD sizeHigh,sizeLow,reserved0,reserved1;WCHAR fileName[260],altName[14];} WIN32_FIND_DATAW;
typedef BOOL (WINAPI *ENUMWINDOWSPROC)(HWND,LONG);
#define RGB2(r,g,b) ((COLORREF)((r)|((g)<<8)|((b)<<16)))

typedef struct {LONG left,top,right,bottom;} RECT;
typedef struct {HWND hwnd;UINT message;SIZE_T wParam;LONG lParam;DWORD time;LONG ptX,ptY;} MSG;
typedef struct {HDC hdc;BOOL erase;RECT paint;BOOL restore,incUpdate;BYTE reserve[32];} PAINTSTRUCT;
typedef LONG (WINAPI *WNDPROC)(HWND,UINT,SIZE_T,LONG);
typedef struct {UINT style;WNDPROC lpfnWndProc;int cbClsExtra,cbWndExtra;HINSTANCE inst;HICON icon;HCURSOR cursor;HBRUSH back;LPCWSTR menuName,className;} WNDCLASSW;

IMP HMODULE WINAPI GetModuleHandleW(LPCWSTR);
IMP DWORD WINAPI GetModuleFileNameW(HMODULE,LPWSTR,DWORD);
IMP DWORD WINAPI GetCurrentProcessId(void);
IMP DWORD WINAPI GetLastError(void);
IMP DWORD WINAPI GetTickCount(void);
IMP void WINAPI Sleep(DWORD);
IMP void WINAPI GetSystemTime(SYSTEMTIME*);
IMP HANDLE WINAPI CreateThread(LPVOID,SIZE_T,DWORD(WINAPI*)(LPVOID),LPVOID,DWORD,DWORD*);
IMP BOOL WINAPI CloseHandle(HANDLE);
IMP BOOL WINAPI DisableThreadLibraryCalls(HMODULE);
IMP HANDLE WINAPI CreateFileW(LPCWSTR,DWORD,DWORD,LPVOID,DWORD,DWORD,HANDLE);
IMP BOOL WINAPI WriteFile(HANDLE,const void*,DWORD,DWORD*,LPVOID);
IMP BOOL WINAPI ReadFile(HANDLE,LPVOID,DWORD,DWORD*,LPVOID);
IMP HANDLE WINAPI FindFirstFileW(LPCWSTR,WIN32_FIND_DATAW*);
IMP BOOL WINAPI FindNextFileW(HANDLE,WIN32_FIND_DATAW*);
IMP BOOL WINAPI FindClose(HANDLE);
IMP int WINAPI CompareFileTime(const FILETIME*,const FILETIME*);
IMP long WINAPI SHGetFolderPathW(HWND,int,HANDLE,DWORD,LPWSTR);
IMP DWORD WINAPI SetFilePointer(HANDLE,LONG,LONG*,DWORD);
IMP BOOL WINAPI GetStringTypeW(DWORD,LPCWSTR,int,WORD*);
IMP UINT WINAPI GetPrivateProfileIntW(LPCWSTR,LPCWSTR,int,LPCWSTR);
IMP BOOL WINAPI FlushFileBuffers(HANDLE);
IMP BOOL WINAPI MoveFileExW(LPCWSTR,LPCWSTR,DWORD);
IMP BOOL WINAPI DeleteFileW(LPCWSTR);
IMP DWORD WINAPI GetPrivateProfileStringW(LPCWSTR,LPCWSTR,LPCWSTR,LPWSTR,DWORD,LPCWSTR);
IMP LONG WINAPI InterlockedCompareExchange(volatile LONG*,LONG,LONG);
IMP LONG WINAPI InterlockedExchange(volatile LONG*,LONG);
IMP int WINAPI WideCharToMultiByte(UINT,DWORD,LPCWSTR,int,char*,int,const char*,BOOL*);
IMP int WINAPI MultiByteToWideChar(UINT,DWORD,const char*,int,LPWSTR,int);
IMP HWND WINAPI GetForegroundWindow(void);
IMP short WINAPI GetAsyncKeyState(int);
IMP BOOL WINAPI ClientToScreen(HWND,LPVOID);
IMP BOOL WINAPI IsIconic(HWND);
IMP BOOL WINAPI EnumWindows(ENUMWINDOWSPROC,LONG);
IMP LONG WINAPI GetWindowLongW(HWND,int);
IMP DWORD WINAPI GetWindowThreadProcessId(HWND,DWORD*);
IMP BOOL WINAPI GetWindowRect(HWND,RECT*);
IMP WORD WINAPI RegisterClassW(const WNDCLASSW*);
IMP HWND WINAPI CreateWindowExW(DWORD,LPCWSTR,LPCWSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,LPVOID);
IMP LONG WINAPI DefWindowProcW(HWND,UINT,SIZE_T,LONG);
IMP BOOL WINAPI DestroyWindow(HWND);
IMP BOOL WINAPI ShowWindow(HWND,int);
IMP BOOL WINAPI SetWindowPos(HWND,HWND,int,int,int,int,UINT);
IMP BOOL WINAPI SetLayeredWindowAttributes(HWND,COLORREF,BYTE,DWORD);
IMP UINT WINAPI SetTimer(HWND,UINT,UINT,LPVOID);
IMP BOOL WINAPI KillTimer(HWND,UINT);
IMP BOOL WINAPI GetMessageW(MSG*,HWND,UINT,UINT);
IMP BOOL WINAPI TranslateMessage(const MSG*);
IMP LONG WINAPI DispatchMessageW(const MSG*);
IMP BOOL WINAPI PostMessageW(HWND,UINT,SIZE_T,LONG);
IMP BOOL WINAPI RegisterHotKey(HWND,int,UINT,UINT);
IMP BOOL WINAPI UnregisterHotKey(HWND,int);
IMP BOOL WINAPI InvalidateRect(HWND,const RECT*,BOOL);
IMP HDC WINAPI BeginPaint(HWND,PAINTSTRUCT*);
IMP BOOL WINAPI EndPaint(HWND,const PAINTSTRUCT*);
IMP BOOL WINAPI GetClientRect(HWND,RECT*);
IMP int WINAPI DrawTextW(HDC,LPCWSTR,int,RECT*,UINT);
IMP int WINAPI FillRect(HDC,const RECT*,HBRUSH);
IMP DWORD WINAPI SendMessageW(HWND,UINT,SIZE_T,LONG);
IMP int WINAPI GetWindowTextW(HWND,LPWSTR,int);
IMP BOOL WINAPI SetWindowTextW(HWND,LPCWSTR);
IMP HWND WINAPI SetFocus(HWND);
IMP LONG WINAPI SetWindowLongW(HWND,int,LONG);
IMP LONG WINAPI CallWindowProcW(WNDPROC,HWND,UINT,SIZE_T,LONG);
IMP BOOL WINAPI IsWindowVisible(HWND);
IMP int WINAPI SetBkMode(HDC,int);
IMP HGDIOBJ WINAPI GetStockObject(int);
IMP COLORREF WINAPI SetTextColor(HDC,COLORREF);
IMP HGDIOBJ WINAPI SelectObject(HDC,HGDIOBJ);
IMP HBRUSH WINAPI CreateSolidBrush(COLORREF);
IMP BOOL WINAPI DeleteObject(HGDIOBJ);
IMP HINTERNET WINAPI WinHttpOpen(LPCWSTR,DWORD,LPCWSTR,LPCWSTR,DWORD);
IMP HINTERNET WINAPI WinHttpConnect(HINTERNET,LPCWSTR,WORD,DWORD);
IMP HINTERNET WINAPI WinHttpOpenRequest(HINTERNET,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR*,DWORD);
IMP BOOL WINAPI WinHttpSetOption(HINTERNET,DWORD,LPVOID,DWORD);
IMP BOOL WINAPI WinHttpSetTimeouts(HINTERNET,int,int,int,int);
IMP BOOL WINAPI WinHttpSendRequest(HINTERNET,LPCWSTR,DWORD,LPVOID,DWORD,DWORD,SIZE_T);
IMP BOOL WINAPI WinHttpReceiveResponse(HINTERNET,LPVOID);
IMP BOOL WINAPI WinHttpQueryHeaders(HINTERNET,DWORD,LPCWSTR,LPVOID,DWORD*,DWORD*);
IMP HINTERNET WINAPI WinHttpWebSocketCompleteUpgrade(HINTERNET,SIZE_T);
IMP DWORD WINAPI WinHttpWebSocketReceive(HINTERNET,LPVOID,DWORD,DWORD*,DWORD*);
IMP DWORD WINAPI WinHttpWebSocketSend(HINTERNET,DWORD,LPVOID,DWORD);
IMP DWORD WINAPI WinHttpWebSocketClose(HINTERNET,WORD,LPVOID,DWORD);
IMP BOOL WINAPI WinHttpCloseHandle(HINTERNET);

typedef void (WINAPI *HostLog)(LPCWSTR,LPCWSTR);
typedef struct {DWORD size,requiredHostApi;WCHAR id[64],version[32],description[160];unsigned long long exclusiveCapabilities;} XenoPluginInfoV1;
typedef struct {DWORD size,apiVersion;HMODULE gameModule;LPCWSTR gameRoot,executablePath,pluginRoot,configPath;HostLog log;LPVOID getBool,getInt,getString;} XenoPluginHostV1;

static HMODULE g_dll;
static HWND g_window,g_edit,g_send,g_hide,g_game;
static int g_window_positioned=0;
static DWORD g_game_window_area=0;
static volatile LONG g_forced_reconnect=0;
static DWORD g_last_game_window_scan=0;
static DWORD g_last_raise=0;
static WNDPROC g_editproc;
static DWORD g_pid;
static int g_show=1,g_started=0,g_was_f12_down=0,g_scroll=0,g_netstatus=0,g_online=-1;
static WCHAR g_nick[32]=L"Ranger",g_status[128]=L"Подключение к Cloudflare...",g_ini[512],g_cfg[512];
static int g_auto_nick=1,g_auto_found=0;
static volatile LONG g_lock;
static HINTERNET g_ws;
static int g_backup=0,g_failed_day=0;
static unsigned g_counter=0;
static DWORD g_last_heartbeat=0,g_last_receive,g_backup_started;
static int g_primary_failures,g_backup_cooldown,g_closing;
static volatile LONG g_wire_lock,g_socket_failed;
#define FHD_BUSY 170u
#define FHD_PRIMARY_HEARTBEAT 10000u
#define FHD_PRIMARY_STALE 35000u
#define FHD_PRIMARY_RETRY 60000u
static int utcday(void){SYSTEMTIME st;GetSystemTime(&st);return (int)st.year*10000+(int)st.month*100+(int)st.day;}
#include "chat_event.h"
#include "chat_preferences.h"
#define EVENT_QUEUE 96
static int g_nmessages[2],g_queue_head[2],g_queue_turn;
static FHDChatEvent g_messages[2][EVENT_QUEUE];
typedef struct {char id[80];WCHAR nick[36];} CHATUSER;
static CHATUSER g_users[128];
static int g_user_count,g_last_send_channel;
static DWORD g_room_day;
IMP void WINAPI GetLocalTime(SYSTEMTIME*);
static char g_our_id[80],g_self_ids[64][80];static int g_self_count,g_self_next;

static int weq(const WCHAR *a,const WCHAR *b){int i=0;while(a[i]&&b[i]&&a[i]==b[i])i++;return a[i]==b[i];}
static int slen(const WCHAR* x){int i=0;while(x&&x[i])i++;return i;}
static int alen(const char* x){int i=0;while(x&&x[i])i++;return i;}
static void wcopy(WCHAR* d,int cap,const WCHAR* s){int k=0;if(!d||cap<=0)return;while(s&&s[k]&&k+1<cap){d[k]=s[k];k++;}d[k]=0;}
static int seq(const char* a,const char* b){int i=0;while(a[i]&&b[i]&&a[i]==b[i])i++;return a[i]==b[i];}
static void copyascii(char*d,int cap,const char*s){int i=0;while(i+1<cap&&s&&s[i]){d[i]=s[i];i++;}d[i]=0;}
static void utf8_to_w(const char*s,WCHAR* out,int cap){if(!s||cap<2)return;int n=MultiByteToWideChar(65001,0,s,-1,out,cap);if(n<=0)out[0]=0;out[cap-1]=0;}
static void w_to_utf8(const WCHAR*s,char*out,int cap){if(!s||cap<2)return;int n=WideCharToMultiByte(65001,0,s,-1,out,cap,0,0);if(n<=0)out[0]=0;out[cap-1]=0;}
static void lock(void){while(InterlockedCompareExchange(&g_lock,1,0)!=0)Sleep(1);}
static void unlock(void){InterlockedExchange(&g_lock,0);}
static void log_line(const char *line){HANDLE f=CreateFileW(g_ini,GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);if(f==(HANDLE)-1)return;SetFilePointer(f,0,0,FILE_END);DWORD wrote=0;WriteFile(f,line,(DWORD)alen(line),&wrote,0);WriteFile(f,"\r\n",2,&wrote,0);CloseHandle(f);}
/* One send at a time is required by WinHTTP. UI sends fail fast if a
 * heartbeat/close is using the socket; they retain the user's draft. */
static DWORD send_packet(HINTERNET socket,const void*data,DWORD size){
 if(InterlockedCompareExchange(&g_wire_lock,1,0))return FHD_BUSY;
 DWORD result=12019u;
 if(socket&&socket==g_ws&&!g_closing){result=WinHttpWebSocketSend(socket,2,(LPVOID)data,size);
  if(result){InterlockedExchange(&g_socket_failed,1);g_online=-1;g_netstatus=0;}}
 InterlockedExchange(&g_wire_lock,0);return result;
}
static void reconnect_socket(HINTERNET socket){
 if(InterlockedCompareExchange(&g_wire_lock,1,0)){InterlockedExchange(&g_socket_failed,1);return;}
 if(socket&&socket==g_ws&&!g_closing){g_closing=1;g_online=-1;g_netstatus=0;WinHttpWebSocketClose(socket,1000,0,0);}
 InterlockedExchange(&g_wire_lock,0);
}
static int use_backup(void){
 if(g_failed_day==utcday())return 1;
 if(g_backup_cooldown&&GetTickCount()-g_backup_started<FHD_PRIMARY_RETRY)return 1;
 g_backup_cooldown=0;return 0;
}
static void connection_failed(int backup,DWORD http_status,int forced){
 if(forced){g_primary_failures=0;g_backup_cooldown=0;return;}
 if(backup)return;
 if(http_status==429){g_failed_day=utcday();g_primary_failures=0;return;}
 if(++g_primary_failures>=3){g_primary_failures=0;g_backup_cooldown=1;g_backup_started=GetTickCount();}
}
static void log_connection(int backup,DWORD http,DWORD error){
 if(!g_ini[0])return;char text[128];const char*label=backup?"Reserve closed: HTTP=":"Primary closed: HTTP=";int n=0;
 while(*label)text[n++]=*label++;DWORD values[2]={http,error};
 for(int k=0;k<2;k++){if(k){const char*p=" WINHTTP=";while(*p)text[n++]=*p++;}char digits[12];int j=0;do{digits[j++]=(char)('0'+values[k]%10);values[k]/=10;}while(values[k]);while(j)text[n++]=digits[--j];}text[n]=0;log_line(text);
}
static int valid_nick(const WCHAR *name){int n=slen(name);if(n<1||n>24)return 0;
 for(int i=0;i<n;i++){WCHAR c=name[i];if(c=='_'||c==' '||c=='.'||c=='-')continue;
  if(c<128){if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')))return 0;}
  else{WORD type=0;if((c>=0xD800&&c<=0xDFFF)||!GetStringTypeW(1,&c,1,&type)||!(type&(0x100|4)))return 0;}
 }return 1;
}
static void concat(WCHAR *dst,int cap,const WCHAR *tail){int n=slen(dst),i=0;while(tail[i]&&n+1<cap)dst[n++]=tail[i++];dst[n]=0;}
static void ensure_preferences_path(void){
 if(g_cfg[0])return;WCHAR path[512];if(!GetModuleFileNameW(g_dll,path,480))return;
 int k=slen(path);while(k>0&&path[k-1]!=92)path[--k]=0;
 concat(path,512,L"FHDOnlineChat_User.ini");wcopy(g_cfg,512,path);
}
/* Kept outside the existing 80-byte preferences ABI. */
static int g_window_locked=1,g_window_lock_loaded;
static int g_notifications,g_notifications_loaded;
int WINAPI FHDChat_LoadNotifications(void){
 ensure_preferences_path();g_notifications=GetPrivateProfileIntW(L"Chat",L"Notifications",0,g_cfg)==1;g_notifications_loaded=1;return g_notifications;
}
int WINAPI FHDChat_LoadWindowLock(void){
 ensure_preferences_path();int value=(int)GetPrivateProfileIntW(L"Chat",L"Locked",1,g_cfg);
 g_window_locked=value==0?0:1;g_window_lock_loaded=1;return g_window_locked;
}
EXP int WINAPI FHDChat_LoadPreferences(FHDChatPreferences*p){
 if(!p)return 0;ensure_preferences_path();p->nickname[0]=0;p->size=1;p->opacity=70;p->x=p->y=-1;
 if(!g_cfg[0])return 0;
 GetPrivateProfileStringW(L"Chat",L"Nickname",L"",p->nickname,32,g_cfg);
 if(p->nickname[0]&&!valid_nick(p->nickname))p->nickname[0]=0;
 p->size=(int)GetPrivateProfileIntW(L"Chat",L"Size",1,g_cfg);if(p->size<0||p->size>3)p->size=1;
 p->opacity=(int)GetPrivateProfileIntW(L"Chat",L"Opacity",70,g_cfg);if(p->opacity<10||p->opacity>100)p->opacity=70;
 p->x=(int)GetPrivateProfileIntW(L"Chat",L"X",-1,g_cfg);p->y=(int)GetPrivateProfileIntW(L"Chat",L"Y",-1,g_cfg);
 if(p->x<0||p->y<0||p->x>100000||p->y>100000)p->x=p->y=-1;return 1;
}
extern int WINAPI FHDGame_ReadPilotNameW(WCHAR*,int);
static volatile LONG g_pilot_refresh_lock;
static DWORD g_pilot_last_check;static int g_pilot_checked,g_pilot_samples;
static WCHAR g_pilot_candidate[32];
/* Two equal valid snapshots, at most once per second. Manual naming wins if
 * applied while the snapshot is being read. No save-file searches or hooks. */
static int refresh_nickname_from_pilot(int force){
 if(!g_auto_nick||InterlockedCompareExchange(&g_pilot_refresh_lock,1,0))return 0;
 int changed=0;DWORD now=GetTickCount();WCHAR name[32];
 if(!g_auto_nick)goto done;
 if(!force&&g_pilot_checked&&now-g_pilot_last_check<1000)goto done;
 g_pilot_checked=1;g_pilot_last_check=now;
 if(FHDGame_ReadPilotNameW(name,32)!=1||!valid_nick(name)){g_pilot_samples=0;g_pilot_candidate[0]=0;goto done;}
 if(!weq(name,g_pilot_candidate)){wcopy(g_pilot_candidate,32,name);g_pilot_samples=1;goto done;}
 if(g_pilot_samples<2)g_pilot_samples++;
 lock();if(g_auto_nick&&!weq(g_nick,name)){wcopy(g_nick,32,name);g_auto_found=1;changed=1;}unlock();
 if(changed){
  if(g_ini[0])log_line("Nickname updated from active game pilot");
  if(g_window)PostMessageW(g_window,WM_APP_NET,0,0);
  if(g_ws){InterlockedExchange(&g_forced_reconnect,1);reconnect_socket(g_ws);}
 }
 done:InterlockedExchange(&g_pilot_refresh_lock,0);return changed;
}
static void pref_number(WCHAR*dst,int cap,int n){WCHAR num[16];int k=0,i=0;if(n<0){concat(dst,cap,L"-");n=-n;}do{num[k++]=(WCHAR)('0'+n%10);n/=10;}while(n&&k<15);WCHAR ordered[16];while(k)ordered[i++]=num[--k];ordered[i]=0;concat(dst,cap,ordered);}
EXP int WINAPI FHDChat_SetNickW(const WCHAR*);
EXP int WINAPI FHDChat_SavePreferences(const FHDChatPreferences*p){
 if(!p||p->size<0||p->size>3||p->opacity<10||p->opacity>100)return -1;
 if(p->x<-1||p->y<-1||p->x>100000||p->y>100000)return -1;
 WCHAR nick[32];wcopy(nick,32,p->nickname);int first=0;while(nick[first]==' ')first++;int n=slen(nick);while(n>first&&nick[n-1]==' ')n--;nick[n]=0;
 if(first){for(int i=0;i<=n-first;i++)nick[i]=nick[i+first];}
 if(nick[0]&&!valid_nick(nick))return -1;
 ensure_preferences_path();if(!g_cfg[0])return 0;if(!g_window_lock_loaded)FHDChat_LoadWindowLock();if(!g_notifications_loaded)FHDChat_LoadNotifications();
 WCHAR text[512],tmp[540];text[0]=0xfeff;text[1]=0;
 concat(text,512,L"[Chat]\r\nNickname=");concat(text,512,nick);concat(text,512,L"\r\nSize=");pref_number(text,512,p->size);
 concat(text,512,L"\r\nOpacity=");pref_number(text,512,p->opacity);concat(text,512,L"\r\nX=");pref_number(text,512,p->x);
 concat(text,512,L"\r\nY=");pref_number(text,512,p->y);concat(text,512,L"\r\nLocked=");pref_number(text,512,g_window_locked);concat(text,512,L"\r\nNotifications=");pref_number(text,512,g_notifications);concat(text,512,L"\r\n");
 wcopy(tmp,540,g_cfg);concat(tmp,540,L".tmp");HANDLE f=CreateFileW(tmp,GENERIC_WRITE,0,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return 0;
 DWORD wrote=0,bytes=(DWORD)slen(text)*2;int ok=WriteFile(f,text,bytes,&wrote,0)&&wrote==bytes;if(ok)ok=FlushFileBuffers(f);CloseHandle(f);
 if(!ok||!MoveFileExW(tmp,g_cfg,9)){DeleteFileW(tmp);return 0;}
 lock();int was_auto=g_auto_nick;g_auto_nick=!nick[0];unlock();
 if(nick[0]){g_failed_day=0;FHDChat_SetNickW(nick);}else if(!was_auto)refresh_nickname_from_pilot(1);
 return 1;
}
/* The notification toggle is committed with the rest of the settings in one
 * atomic INI replace. Failed Apply keeps the previous runtime value. */
int WINAPI FHDChat_SaveSettings(const FHDChatPreferences*p,int notifications){
 if(notifications!=0&&notifications!=1)return -1;
 if(!g_notifications_loaded)FHDChat_LoadNotifications();int previous=g_notifications;g_notifications=notifications;
 int result=FHDChat_SavePreferences(p);if(result!=1)g_notifications=previous;return result;
}
int WINAPI FHDChat_SetWindowLock(int value,const FHDChatPreferences*p){
 if(value!=0&&value!=1)return -1;g_window_locked=value;g_window_lock_loaded=1;return FHDChat_SavePreferences(p);
}
static BOOL WINAPI enum_game_window(HWND h,LONG unused){(void)unused;DWORD pid=0;RECT r;
 if(h==g_window||!IsWindowVisible(h)||GetWindowThreadProcessId(h,&pid)==0||pid!=g_pid)return 1;
 if(GetWindowLongW(h,GWL_STYLE)&WS_CHILD)return 1;
 if(!GetWindowRect(h,&r))return 1;
 int w=r.right-r.left,hh=r.bottom-r.top;
 if(w<640||hh<480)return 1;
 DWORD area=(DWORD)w*(DWORD)hh;
 if(area>g_game_window_area){g_game_window_area=area;g_game=h;}
 return 1;
}
static void update_game_window(void){g_game_window_area=0;EnumWindows(enum_game_window,0);}
static void setstatus(const WCHAR* text,int stat){g_netstatus=stat;wcopy(g_status,128,text);if(g_window)PostMessageW(g_window,WM_APP_NET,0,0);}

DWORD WINAPI FHDChat_RoomDay(void){return g_ws&&!g_backup&&g_netstatus==2?g_room_day:0;}
EXP DWORD WINAPI FHDChat_PrivateDay(void){DWORD room=FHDChat_RoomDay();if(room)return room;SYSTEMTIME t;GetLocalTime(&t);return (DWORD)t.year*10000+(DWORD)t.month*100+(DWORD)t.day;}
static int valid_id(const char *s){int n=alen(s);if(n!=36)return 0;for(int i=0;i<36;i++){if(i==8||i==13||i==18||i==23){if(s[i]!='-')return 0;}else if(!((s[i]>='0'&&s[i]<='9')||(s[i]>='a'&&s[i]<='f')||(s[i]>='A'&&s[i]<='F')))return 0;}return 1;}
static int own_id(const char*id){if(!id||!id[0])return 0;for(int i=0;i<g_self_count;i++)if(seq(id,g_self_ids[i]))return 1;return seq(id,g_our_id);}
static void remember_self(const char*id){if(!valid_id(id)||own_id(id))return;copyascii(g_self_ids[g_self_next],80,id);g_self_next=(g_self_next+1)%64;if(g_self_count<64)g_self_count++;}
static void add_event(const WCHAR* nick,const WCHAR* txt,int own,int channel,const char* id,const WCHAR* peer,int event){
 FHDChatEvent e;wcopy(e.nick,36,nick);wcopy(e.text,512,txt);wcopy(e.peer,36,peer);copyascii(e.peerId,80,id);e.own=own;e.channel=channel;e.event=event;e.day=FHDChat_PrivateDay();copyascii(e.senderId,80,channel&&own?g_our_id:id);
 lock();int c=channel?1:0;if(g_nmessages[c]==EVENT_QUEUE){g_queue_head[c]=(g_queue_head[c]+1)%EVENT_QUEUE;g_nmessages[c]--;}
 g_messages[c][(g_queue_head[c]+g_nmessages[c])%EVENT_QUEUE]=e;g_nmessages[c]++;unlock();
}
static void addmsg(const WCHAR* n,const WCHAR* t,int own){add_event(n,t,own,0,"",L"",0);}
static void clear_history(void){add_event(L"",L"",0,0,"",L"",1);}
int WINAPI FHDChat_IsBackup(void){return g_ws&&g_backup&&g_netstatus==3;}
EXP int WINAPI FHDChat_GetOnline(void){return g_netstatus>=2?g_online:-1;}
EXP int WINAPI FHDChat_CanPrivate(void){return g_ws&&!g_backup&&g_netstatus==2&&valid_id(g_our_id);}
/* Prefer the exact live connection ID. Historical names are resolved only
 * when exactly one other live connection has that name. Never guess duplicates. */
EXP int WINAPI FHDChat_SelectPeerW(const WCHAR *name,const char *hint,char *out,int cap){
 if(!name||!out||cap<37)return 0;out[0]=0;int found=-1,count=0;
 lock();
 for(int i=0;i<g_user_count;i++){
  if(seq(g_users[i].id,g_our_id)||!weq(g_users[i].nick,name))continue;
  if(hint&&hint[0]&&seq(g_users[i].id,hint)){copyascii(out,cap,g_users[i].id);unlock();return 1;}
  found=i;count++;
 }
 if(hint&&hint[0]){unlock();return 0;}
 if(count==1)copyascii(out,cap,g_users[found].id);unlock();return count==1?1:count>1?-1:0;
}
/* Longest exact online name, with spaces allowed; ambiguous names never send. */
EXP int WINAPI FHDChat_ParsePrivateW(const WCHAR *input,char *out,int cap,WCHAR *name,int namecap,int *body){
 if(!input||!out||cap<37||!name||namecap<2||!body)return 0;
 out[0]=0;name[0]=0;*body=0;if(input[0]!='@'&&input[0]!='"')return 0;
 int length=slen(input),best=0,count=0,chosen=-1,offset=0;
 lock();
 for(int i=0;i<g_user_count;i++){
  if(seq(g_users[i].id,g_our_id))continue;
  int n=slen(g_users[i].nick);if(!n||n+1>length)continue;
  int k=0;while(k<n&&input[k+1]==g_users[i].nick[k])k++;if(k!=n)continue;
  int end=1+n;
  if(input[0]=='"'&&input[end]=='"')end++;
  if(input[end]&&input[end]!=' ')continue;
  while(input[end]==' ')end++;
  if(n>best){best=n;count=0;chosen=i;offset=end;}
  if(n==best)count++;
 }
 if(count==1){copyascii(out,cap,g_users[chosen].id);wcopy(name,namecap,g_users[chosen].nick);*body=offset;}
 unlock();if(count>1)return -1;if(!count)return 0;return input[offset]?1:-2;
}
static void peer_name(const char* id,WCHAR *name,int cap){name[0]=0;lock();for(int i=0;i<g_user_count;i++)if(seq(id,g_users[i].id)){wcopy(name,cap,g_users[i].nick);break;}unlock();}
static int getjson(const char*,const char*,char*,int);
static DWORD packet_day(const char*);
/* Parse bounded history array returned by Cloudflare on connection (up to 30 items). */
static void parse_history(const char *json){const char *p=json;const char key[]="\"history\"";int k=0;
 while(*p){int n=0;while(key[n]&&p[n]==key[n])n++;if(!key[n])break;p++;}
 if(!*p)return;p+=9;while(*p&&*p!='[')p++;if(!*p)return;p++;
 clear_history();
 for(int count=0;*p&&*p!=']'&&count<MAX_MESSAGES;){
   if(*p!='{'){p++;continue;}const char *start=p;int depth=0,quoted=0,escaped=0;
   do{char ch=*p++;if(quoted){if(escaped)escaped=0;else if(ch=='\\')escaped=1;else if(ch=='"')quoted=0;}
     else{if(ch=='"')quoted=1;else if(ch=='{')depth++;else if(ch=='}')depth--;}}
   while(*p&&depth>0&&p-start<8000);
   if(depth!=0||p-start>=8000)break;
   char item[8001],who[160],body[2200];int size=(int)(p-start);
   for(int i=0;i<size;i++)item[i]=start[i];item[size]=0;
   DWORD day=packet_day(item);if(day&&day!=FHDChat_PrivateDay())continue;
   getjson(item,"name",who,sizeof(who));getjson(item,"text",body,sizeof(body));
   if(who[0]&&body[0]){WCHAR wname[64],wbody[512];utf8_to_w(who,wname,64);utf8_to_w(body,wbody,512);char id[80];getjson(item,"fromId",id,80);add_event(wname,wbody,valid_id(id)?own_id(id):(!g_our_id[0]&&weq(wname,g_nick)),0,id,L"",FHD_EVENT_HISTORY);count++;}
 }
}

/* Read a named JSON string field with quoted-string escape decoding (UTF-8 output).
 * Protocol messages from the Worker are bounded, validated by the server. */
static int getjson(const char* js,const char* field,char*out,int max){int i=0,j=0;char key[64];int k=0;key[k++]='"';for(;field&&*field&&k<59;field++)key[k++]=*field;key[k++]='"';key[k]=0;
 while(js&&js[i]){int z=0;while(key[z]&&js[i+z]==key[z])z++;if(!key[z])break;i++;}if(!js||!js[i]){out[0]=0;return 0;}i+=k;
 while(js[i]==' '||js[i]==':'||js[i]=='\t')i++;if(js[i]!='"'){out[0]=0;return 0;}i++;
 while(js[i]&&js[i]!='"'&&j+5<max){char c=js[i++];if(c=='\\'){c=js[i++];if(c=='n'||c=='r'||c=='t')c=' ';else if(c=='u'){unsigned v=0;int q;for(q=0;q<4&&js[i];q++){char h=js[i++];v=(v<<4)+(h>='0'&&h<='9'?h-'0':h>='a'&&h<='f'?h-'a'+10:h>='A'&&h<='F'?h-'A'+10:0);}if(v<128){c=(char)v;}else if(v<2048){out[j++]=(char)(0xc0|(v>>6));c=(char)(0x80|(v&63));}else{out[j++]=(char)(0xe0|(v>>12));out[j++]=(char)(0x80|((v>>6)&63));c=(char)(0x80|(v&63));}}}if(c=='\r'||c=='\n')c=' ';out[j++]=c;}out[j]=0;return j;}
static DWORD packet_day(const char*json){char text[16];getjson(json,"day",text,16);if(alen(text)!=8)return 0;DWORD day=0;for(int i=0;i<8;i++){if(text[i]<'0'||text[i]>'9')return 0;day=day*10+text[i]-'0';}int month=(day/100)%100,date=day%100;return day>=20000101u&&month>=1&&month<=12&&date>=1&&date<=31?day:0;}
static int adopt_room_day(DWORD day){if(!day||day<=g_room_day)return 0;g_room_day=day;add_event(L"",L"",0,0,"",L"",2);return 1;}
static int json_escape(const char*in,char*out,int cap){int j=0,i;for(i=0;in[i]&&j+8<cap;i++){unsigned char c=(unsigned char)in[i];if(c=='"'||c=='\\'){out[j++]='\\';out[j++]=(char)c;}else if(c<32){out[j++]=' ';}else out[j++]=(char)c;}out[j]=0;return j;}

static void read_presence(const char *json){
 int count=-1;const char *q=json;
 while(*q){if(q[0]=='"'&&q[1]=='o'&&q[2]=='n'&&q[3]=='l'&&q[4]=='i'&&q[5]=='n'&&q[6]=='e'&&q[7]=='"'){q+=8;while(*q&&(*q<'0'||*q>'9'))q++;count=0;while(*q>='0'&&*q<='9'&&count<10000)count=count*10+(*q++-'0');break;}q++;}
 CHATUSER next[128];int nusers=0;
 q=json;const char key[]="\"users\"";while(*q){int k=0;while(key[k]&&q[k]==key[k])k++;if(!key[k])break;q++;}
 if(!*q)return;while(*q&&*q!='[')q++;if(!*q)return;q++;
 while(*q&&*q!=']'){
  if(*q!='{'){q++;continue;}
  const char *start=q;int depth=0,quoted=0,escape=0;
  do{char c=*q++;if(quoted){if(escape)escape=0;else if(c=='\\')escape=1;else if(c=='"')quoted=0;}else if(c=='"')quoted=1;else if(c=='{')depth++;else if(c=='}')depth--;}while(*q&&depth&&q-start<2000);
  if(depth||q-start>=2000)break;
  char item[2001],id[80],name[160];int n=(int)(q-start);for(int i=0;i<n;i++)item[i]=start[i];item[n]=0;
  getjson(item,"id",id,80);getjson(item,"name",name,160);if(!valid_id(id)||!name[0])continue;
  WCHAR wn[36];utf8_to_w(name,wn,36);if(nusers<128){copyascii(next[nusers].id,80,id);wcopy(next[nusers].nick,36,wn);nusers++;}
 }
 lock();for(int i=0;i<nusers;i++)g_users[i]=next[i];g_user_count=nusers;if(count>=0)g_online=count;unlock();
}
static void receive_message(const char* json){
 char typ[40],name[160],body[2200],from[80],to[80],target[160];WCHAR wn[36],wt[512],wp[36];getjson(json,"type",typ,40);if(!typ[0])getjson(json,"event",typ,40);
 if(seq(typ,"pong"))return;
 if(seq(typ,"phx_reply")){char status[32],ref[16];getjson(json,"status",status,32);getjson(json,"ref",ref,16);if(seq(status,"ok")&&seq(ref,"1"))setstatus(L"Резерв подключён",3);return;}
 if(seq(typ,"reset")){if(!g_backup)adopt_room_day(packet_day(json));return;}
 if(seq(typ,"hello")){DWORD day=packet_day(json);if(day&&g_room_day&&day<g_room_day)return;if(!g_backup)adopt_room_day(day);g_primary_failures=0;getjson(json,"id",from,80);remember_self(from);copyascii(g_our_id,80,from);lock();g_user_count=0;unlock();g_online=-1;parse_history(json);setstatus(L"Подключён",2);return;}
 if(seq(typ,"presence")){read_presence(json);return;}
 if(seq(typ,"chat")||seq(typ,"private")||seq(typ,"broadcast")){
  if(!g_backup){DWORD day=packet_day(json);if(day&&g_room_day&&day<g_room_day)return;adopt_room_day(day);}
  getjson(json,"name",name,160);getjson(json,"text",body,2200);getjson(json,"fromId",from,80);if(!name[0]||!body[0])return;
  utf8_to_w(name,wn,36);utf8_to_w(body,wt,512);
  int own=own_id(from)||(!g_our_id[0]&&weq(wn,g_nick));
  if(seq(typ,"private")){
   if(own){getjson(json,"toId",to,80);getjson(json,"toName",target,160);if(target[0])utf8_to_w(target,wp,36);else peer_name(to,wp,36);if(!valid_id(to))return;add_event(wn,wt,1,1,to,wp,0);}
   else{if(!valid_id(from))return;add_event(wn,wt,0,1,from,wn,0);}return;
  }
  add_event(wn,wt,own,0,from,L"",0);return;
 }
 if(seq(typ,"error")){getjson(json,"message",body,2200);utf8_to_w(body,wt,512);add_event(L"Сервер",wt,0,g_last_send_channel,"",L"",FHD_EVENT_SERVICE);}
}
EXP int WINAPI FHDChat_SendToW(const WCHAR* input,int channel,const char* recipient){if(!input)return 0;WCHAR line[512];char utf[1600],escaped[3200],packet[3400];int i=0,j=0;wcopy(line,512,input);if(!line[0]||slen(line)>500)return 0;w_to_utf8(line,utf,1600);json_escape(utf,escaped,3200);
 const char*p1="{\"type\":\"chat\",\"text\":\"";const char*p2="\"}";while(p1[i]&&j<3300)packet[j++]=p1[i++];i=0;while(escaped[i]&&j<3300)packet[j++]=escaped[i++];i=0;while(p2[i]&&j<3398)packet[j++]=p2[i++];packet[j]=0;
 if(!g_ws||g_netstatus<2||g_closing)return 0;
 g_last_send_channel=channel?1:0;
 if(channel){
  if(!FHDChat_CanPrivate()||!recipient||!valid_id(recipient)||seq(recipient,g_our_id))return 0;
  WCHAR peer[36];peer_name(recipient,peer,36);if(!peer[0])return 0;
  char privatePacket[3500];const char *a="{\"type\":\"private\",\"toId\":\"",*b="\",\"text\":\"",*c="\"}";int pos=0;
  for(int k=0;a[k];k++)privatePacket[pos++]=a[k];for(int k=0;recipient[k];k++)privatePacket[pos++]=recipient[k];for(int k=0;b[k];k++)privatePacket[pos++]=b[k];for(int k=0;escaped[k];k++)privatePacket[pos++]=escaped[k];for(int k=0;c[k];k++)privatePacket[pos++]=c[k];privatePacket[pos]=0;
  return send_packet(g_ws,privatePacket,(DWORD)pos)==0;
 }
 if(g_backup){
  char nick_utf[128],name_escaped[256],packet2[4500];int k=0,q=0;const char *a="{\"topic\":\"realtime:public:fhd-onlinechat-global\",\"event\":\"broadcast\",\"join_ref\":\"1\",\"ref\":\"2\",\"payload\":{\"type\":\"broadcast\",\"event\":\"chat\",\"payload\":{\"type\":\"chat\",\"id\":\"fhd-native-00000000\",\"name\":\"";
  const char *b="\",\"text\":\"";const char *c="\"}}}";
  w_to_utf8(g_nick,nick_utf,128);json_escape(nick_utf,name_escaped,256);
  while(a[k]&&q<4300)packet2[q++]=a[k++];
  /* Replace eight zeroes in message id with unique tick/counter hex digits. */
  unsigned uid=(unsigned)GetTickCount()+(++g_counter);char hex[]="0123456789abcdef";
  for(int pos=0;pos+8<q;pos++)if(packet2[pos]=='0'&&packet2[pos+1]=='0'&&packet2[pos+2]=='0'&&packet2[pos+3]=='0'&&packet2[pos+4]=='0'&&packet2[pos+5]=='0'&&packet2[pos+6]=='0'&&packet2[pos+7]=='0'){for(int bit=0;bit<8;bit++)packet2[pos+bit]=hex[(uid>>(28-4*bit))&15];break;}
  k=0;while(name_escaped[k]&&q<4300)packet2[q++]=name_escaped[k++];k=0;while(b[k]&&q<4300)packet2[q++]=b[k++];k=0;while(escaped[k]&&q<4300)packet2[q++]=escaped[k++];k=0;while(c[k]&&q<4400)packet2[q++]=c[k++];packet2[q]=0;
  return send_packet(g_ws,packet2,(DWORD)q)==0;
 }
 return send_packet(g_ws,packet,(DWORD)j)==0;
}
EXP int WINAPI FHDChat_SendW(const WCHAR* input){return FHDChat_SendToW(input,0,0);}
static DWORD WINAPI netthread(LPVOID arg){(void)arg;for(;;){HINTERNET ss=0,cc=0,rr=0,ws=0;WCHAR path[350]=L"/ws?name=",nick[64];char utf[128],urlseg[256];int n=0,j=0;DWORD status=0,size=sizeof(status),network_error=0;int backup=use_backup();
 WCHAR cloudhost[]=L"fhd-onlinechat.fhd-onlinechat-wolker.workers.dev";
 WCHAR backuphost[]=L"stnagaovqsuxfftvunpw.supabase.co";
 WCHAR backuppath[]=L"/realtime/v1/websocket?vsn=1.0.0&apikey=sb_publishable_1Q265Sef-yB_Bsg0kud3Hw_eYFlNWFL";
 lock();wcopy(nick,64,g_nick);unlock();w_to_utf8(nick,utf,128);
 for(int i=0;utf[i]&&j<200;i++){unsigned char c=(unsigned char)utf[i];if((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.')urlseg[j++]=c;else{const char h[]="0123456789ABCDEF";urlseg[j++]='%';urlseg[j++]=h[c>>4];urlseg[j++]=h[c&15];}}urlseg[j]=0;
 for(int i=0;urlseg[i]&&n<200;i++)path[9+n++]=(WCHAR)(unsigned char)urlseg[i];path[9+n]=0;
 setstatus(backup?L"Резерв: подключение к Supabase...":L"Подключение к Cloudflare...",1);
 ss=WinHttpOpen(L"FHDOnlineChat/0.20",0,0,0,0);if(!ss)goto failed;
 WinHttpSetTimeouts(ss,8000,8000,12000,15000);
 cc=WinHttpConnect(ss,backup?backuphost:cloudhost,443,0);if(!cc)goto failed;
 rr=WinHttpOpenRequest(cc,L"GET",backup?backuppath:path,0,0,0,WINHTTP_FLAG_SECURE);if(!rr)goto failed;
 if(!WinHttpSetOption(rr,WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,0,0))goto failed;
 if(!WinHttpSendRequest(rr,0,0,0,0,0,0))goto failed;
 if(!WinHttpReceiveResponse(rr,0))goto failed;
 if(!WinHttpQueryHeaders(rr,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,0,&status,&size,0)||status!=101)goto failed;
 ws=WinHttpWebSocketCompleteUpgrade(rr,0);if(!ws)goto failed;WinHttpCloseHandle(rr);rr=0;
 while(InterlockedCompareExchange(&g_wire_lock,1,0))Sleep(1);
 g_ws=ws;g_backup=backup;g_closing=0;g_socket_failed=0;g_last_receive=g_last_heartbeat=GetTickCount();InterlockedExchange(&g_wire_lock,0);
 if(backup){
  const char join[]="{\"topic\":\"realtime:public:fhd-onlinechat-global\",\"event\":\"phx_join\",\"ref\":\"1\",\"join_ref\":\"1\",\"payload\":{\"config\":{\"broadcast\":{\"ack\":false,\"self\":true},\"presence\":{\"enabled\":false},\"private\":false}}}";
  if(send_packet(ws,join,(DWORD)alen(join))!=0)goto failed;
  setstatus(L"Резерв: ожидание Supabase...",1);
 }else setstatus(L"Cloudflare подключён",2);
 {static char buffer[65000];char piece[4096];DWORD sz=0,kind=0;int total=0;
  for(;;){DWORD res=WinHttpWebSocketReceive(ws,piece,4096,&sz,&kind);if(res||kind==4){network_error=res;break;}g_last_receive=GetTickCount();if(total+sz>=64990){total=0;break;}for(DWORD t=0;t<sz;t++)buffer[total++]=piece[t];if(kind==2||kind==0){buffer[total]=0;receive_message(buffer);total=0;}
    if(backup&&!use_backup())break;
  }
 }
 failed:
 if(!network_error&&status!=101)network_error=GetLastError();log_connection(backup,status,network_error);
 while(InterlockedCompareExchange(&g_wire_lock,1,0))Sleep(1);
 if(g_ws==ws)g_ws=0;g_closing=0;g_socket_failed=0;if(ws)WinHttpCloseHandle(ws);InterlockedExchange(&g_wire_lock,0);
 g_room_day=0;g_online=-1;lock();g_user_count=0;unlock();g_our_id[0]=0;
 if(rr)WinHttpCloseHandle(rr);if(cc)WinHttpCloseHandle(cc);if(ss)WinHttpCloseHandle(ss);
 connection_failed(backup,status,(int)InterlockedExchange(&g_forced_reconnect,0));
 setstatus(L"Отключено. Повторное подключение...",0);
 Sleep(5000);
 }}

static void heartbeat_step(void){
 refresh_nickname_from_pilot(0);HINTERNET socket=g_ws;if(!socket||g_closing)return;DWORD now=GetTickCount();
 if(g_socket_failed){reconnect_socket(socket);return;}
 if(g_backup){
  if(!use_backup()){InterlockedExchange(&g_forced_reconnect,1);reconnect_socket(socket);return;}
  if(now-g_last_heartbeat>=20000){const char heartbeat[]="{\"topic\":\"phoenix\",\"event\":\"heartbeat\",\"ref\":\"heartbeat-fhd08\",\"payload\":{}}";
   DWORD result=send_packet(socket,heartbeat,(DWORD)alen(heartbeat));if(result!=FHD_BUSY)g_last_heartbeat=now;if(result&&result!=FHD_BUSY)reconnect_socket(socket);}
  return;
 }
 if(now-g_last_receive>=FHD_PRIMARY_STALE){if(g_ini[0])log_line("Primary watchdog: no reply for 35 seconds");reconnect_socket(socket);return;}
 if(now-g_last_heartbeat>=FHD_PRIMARY_HEARTBEAT){const char heartbeat[]="{\"type\":\"ping\"}";
  DWORD result=send_packet(socket,heartbeat,(DWORD)alen(heartbeat));if(result!=FHD_BUSY)g_last_heartbeat=now;if(result&&result!=FHD_BUSY)reconnect_socket(socket);}
}
static DWORD WINAPI heartbeat_thread(LPVOID unused){(void)unused;for(;;){Sleep(1000);heartbeat_step();}}
EXP int WINAPI FHDChat_PollEx(FHDChatEvent *event){if(!event)return 0;lock();int c=g_queue_turn;if(!g_nmessages[c])c=1-c;if(!g_nmessages[c]){unlock();return 0;}*event=g_messages[c][g_queue_head[c]];g_queue_head[c]=(g_queue_head[c]+1)%EVENT_QUEUE;g_nmessages[c]--;g_queue_turn=1-c;unlock();return 1;}
EXP int WINAPI FHDChat_PollW(WCHAR* nick,int maxNick,WCHAR* body,int maxBody,int* own){FHDChatEvent e;if(!nick||!body||maxNick<2||maxBody<2)return 0;while(FHDChat_PollEx(&e)){if(e.event==1||e.event==2||e.channel)continue;wcopy(nick,maxNick,e.nick);wcopy(body,maxBody,e.text);if(own)*own=e.own;return 1;}return 0;}
EXP int WINAPI FHDChat_SetNickW(const WCHAR* nick){
 if(!nick||!valid_nick(nick))return 0;
 if(weq(nick,g_nick))return 1;
 lock();wcopy(g_nick,32,nick);unlock();
 if(g_ws){InterlockedExchange(&g_forced_reconnect,1);reconnect_socket(g_ws);}
 return 1;
}
EXP int WINAPI FHDChat_StatusW(WCHAR* dst,int len){
 if(!dst||len<2)return 0;wcopy(dst,len,g_status);return g_netstatus;
}
EXP BOOL WINAPI XenoPlugin_Query(XenoPluginInfoV1* info){
 if(!info||info->size<520)return FALSE;
 info->requiredHostApi=1;wcopy(info->id,64,L"FHDOnlineChatNativeBridge");
 wcopy(info->version,32,L"0.22-fixed-native021");
 wcopy(info->description,160,L"Direct3D9 in-frame chat UI + network transport");
 if(info->size>=sizeof(XenoPluginInfoV1))info->exclusiveCapabilities=0;
 return TRUE;
}
static int start_bridge(void){
 if(InterlockedCompareExchange((volatile LONG*)&g_started,1,0)!=0)return 1;
 WCHAR path[512];GetModuleFileNameW(g_dll,path,512);
 int k=slen(path);while(k>0&&path[k-1]!=92)path[--k]=0;
 concat(path,512,L"FHDOnlineChat_Network.log");wcopy(g_ini,512,path);
 log_line("FHD OnlineChat 0.22 FIXED NATIVE021 started");
 FHDChatPreferences preferences;FHDChat_LoadPreferences(&preferences);
 if(preferences.nickname[0]){g_auto_nick=0;wcopy(g_nick,32,preferences.nickname);}
 /* Automatic pilot identity becomes available after the game profile is ready. */
 HANDLE t=CreateThread(0,0,netthread,0,0,0);
 if(!t){g_started=0;log_line("Cannot start network thread");return 0;}
 CloseHandle(t);
 HANDLE hb=CreateThread(0,0,heartbeat_thread,0,0,0);if(hb)CloseHandle(hb);
 return 1;
}
EXP int WINAPI FHDChat_Start(void){return start_bridge();}
void overlay_log(const char* line){if(g_ini[0])log_line(line);}
EXP int WINAPI FHDChat_GetNickW(WCHAR* dst,int cap){if(!dst||cap<2)return 0;wcopy(dst,cap,g_nick);return 1;}
extern int overlay_bootstrap(void);
EXP DWORD WINAPI XenoPlugin_Initialize(const XenoPluginHostV1* host){
 if(!host||host->apiVersion<1||host->size<32)return 1;
 if(!start_bridge())return 2;
 if(!overlay_bootstrap())log_line("D3D9 bootstrap thread failed");
 if(host->log)host->log(L"FHDOnlineChat",L"Real Direct3D9 game device discovery requested");
 return 0;
}
BOOL WINAPI DllMain(HMODULE m,DWORD why,LPVOID arg){(void)arg;if(why==1){g_dll=m;DisableThreadLibraryCalls(m);}return TRUE;}
void *memcpy(void *to,const void *from,SIZE_T n){BYTE *d=(BYTE*)to;const BYTE *s=(const BYTE*)from;for(SIZE_T i=0;i<n;i++)d[i]=s[i];return to;}
void *memmove(void *to,const void *from,SIZE_T n){BYTE *d=(BYTE*)to;const BYTE *s=(const BYTE*)from;if(d<s){for(SIZE_T i=0;i<n;i++)d[i]=s[i];}else if(d>s){for(SIZE_T i=n;i>0;i--)d[i-1]=s[i-1];}return to;}

#ifdef FHD_TEST
EXP void WINAPI Test_NetworkReset(void){g_ws=0;g_backup=0;g_netstatus=0;g_online=-1;g_failed_day=0;g_primary_failures=0;g_backup_cooldown=0;g_closing=0;g_socket_failed=0;g_wire_lock=0;g_auto_nick=0;g_ini[0]=0;g_forced_reconnect=0;}
EXP void WINAPI Test_NetworkRun(void){netthread(0);}
EXP void WINAPI Test_HeartbeatStep(void){heartbeat_step();}
EXP int WINAPI Test_UseBackup(void){return use_backup();}
EXP void WINAPI Test_ConnectionFailed(int backup,DWORD http,int forced){connection_failed(backup,http,forced);}
EXP void WINAPI Test_HeartbeatStart(int backup){g_ws=(HINTERNET)123;g_backup=backup;g_netstatus=backup?3:2;g_closing=0;g_socket_failed=0;g_last_receive=g_last_heartbeat=GetTickCount();}
EXP void WINAPI Test_ReceiveActivity(void){g_last_receive=GetTickCount();}
EXP void WINAPI Test_WireBusy(int busy){g_wire_lock=busy;}
EXP int WINAPI Test_Closing(void){return g_closing;}
EXP void WINAPI Test_ConfigPath(const WCHAR*p){wcopy(g_cfg,512,p);}
EXP int WINAPI Test_RefreshPilot(int force){return refresh_nickname_from_pilot(force);}
EXP int WINAPI Test_AutoNick(void){return g_auto_nick;}
EXP void WINAPI Test_Receive(const char *json){receive_message(json);}
EXP void WINAPI Test_Transport(int backup){g_ws=(HINTERNET)123;g_backup=backup;g_netstatus=backup?3:2;if(backup){g_room_day=0;g_online=-1;g_user_count=0;}}
#endif


/* Fixed 0.22 exported helpers & allocating x86 _chkstk implementation */
EXP int WINAPI FHDChat_OurIdentityW(WCHAR* dst, int cap);
EXP int WINAPI FHDChat_OpenUpdateLink(void);
EXP int WINAPI FHDChat_NameCollisionW(const WCHAR* name);
EXP int WINAPI FHDChat_PeerOrdinal(const char* id);

#if defined(__i386__) || defined(_M_IX86)
/* CRITICAL FIX (0.22 FIXED): _chkstk on x86 MUST subtract EAX from ESP
 * after probing guard pages; a probe-only stub causes immediate stack
 * corruption / Access Violation in netthread (0x87A4) and FHDChat_SendToW (0x3720). */
__asm__(
    ".global __chkstk\n"
    "__chkstk:\n"
    "  pushl %ecx\n"
    "  cmpl $0x1000, %eax\n"
    "  leal 8(%esp), %ecx\n"
    "  jb 2f\n"
    "1:\n"
    "  subl $0x1000, %ecx\n"
    "  testl %ecx, (%ecx)\n"
    "  subl $0x1000, %eax\n"
    "  cmpl $0x1000, %eax\n"
    "  ja 1b\n"
    "2:\n"
    "  subl %eax, %ecx\n"
    "  testl %ecx, (%ecx)\n"
    "  leal 4(%esp), %eax\n"
    "  movl %ecx, %esp\n"
    "  movl -4(%eax), %ecx\n"
    "  pushl (%eax)\n"
    "  subl %esp, %eax\n"
    "  ret\n"
);
#endif
