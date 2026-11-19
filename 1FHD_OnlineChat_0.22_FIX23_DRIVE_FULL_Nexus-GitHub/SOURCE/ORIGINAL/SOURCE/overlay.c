/* FHD OnlineChat 0.17.2, game-session-only chat, gear and window lock.
 * Experimental XenoNativeLoader x86 plugin companion; no topmost HWND.
 * Hooks actual IDirect3DDevice9::EndScene and ::Present from one of two verified game profiles.
 * Refuses to hook on unknown EXE; does not patch any bytes in Rangers.exe.
 */
#include "font16.h"
typedef unsigned long DWORD; typedef unsigned int UINT;typedef unsigned short WORD,WCHAR;typedef unsigned char BYTE;
typedef long LONG;typedef int BOOL,HRESULT;typedef void *HMODULE,*HWND,*HANDLE,*LPVOID;
typedef const WCHAR *LPCWSTR;typedef int INT;typedef unsigned long SIZE_T;
#define WINAPI __stdcall
#define IMP __declspec(dllimport)
#define TRUE 1
#define WS_POPUP 0x80000000u
#define GWL_WNDPROC (-4)
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_MOUSEMOVE 0x0200
#define WM_MOUSEWHEEL 0x020A
#define VK_F12 0x7B
#define VK_ESCAPE 0x1B
#define VK_BACK 0x08
#define VK_RETURN 0x0D
#define PAGE_READWRITE 0x04
#define D3DFVF_FHD 0x144u
#define D3DPOOL_MANAGED 1
#define D3DFMT_A8R8G8B8 21
#define RENDER_LINES 9
#define HISTORY 30
#define PANEL_WIDTH g_panel_width
#define PANEL_HEIGHT g_panel_height
#define MAX_FONT_VERTS 18000
#define RGBA(a,r,g,b) (((DWORD)(a)<<24)|((DWORD)(r)<<16)|((DWORD)(g)<<8)|(DWORD)(b))

typedef LONG(WINAPI *WNDPROC)(HWND,UINT,SIZE_T,LONG);
typedef struct {LONG left,top,right,bottom;} RECT;
typedef struct {LONG x,y;} POINT;
typedef struct{UINT BackBufferWidth,BackBufferHeight,BackBufferFormat,BackBufferCount,MultiSampleType;DWORD MultiSampleQuality;UINT SwapEffect;HWND hDeviceWindow;BOOL Windowed,EnableAutoDepthStencil;UINT AutoDepthStencilFormat;DWORD Flags,FullScreen_RefreshRateInHz,PresentationInterval;} D3DPRESENT_PARAMETERS;
typedef struct{UINT AdapterOrdinal,DeviceType;HWND hFocusWindow;DWORD BehaviorFlags;} D3DCREATION_PARAMETERS;
typedef struct{DWORD X,Y,Width,Height;float MinZ,MaxZ;} D3DVIEWPORT9;
typedef struct{INT Pitch;void *pBits;} D3DLOCKED_RECT;
typedef struct{float x,y,z,rhw;DWORD color;float u,v;} VERT;
IMP HMODULE WINAPI GetModuleHandleW(LPCWSTR);
IMP HMODULE WINAPI LoadLibraryW(LPCWSTR);
IMP void * WINAPI GetProcAddress(HMODULE,const char*);
IMP HANDLE WINAPI GetCurrentProcess(void);
IMP BOOL WINAPI ReadProcessMemory(HANDLE,const void*,void*,SIZE_T,SIZE_T*);
IMP BOOL WINAPI VirtualProtect(LPVOID,SIZE_T,DWORD,DWORD*);
IMP HANDLE WINAPI CreateThread(LPVOID,SIZE_T,DWORD(WINAPI*)(LPVOID),LPVOID,DWORD,DWORD*);
IMP BOOL WINAPI CloseHandle(HANDLE);
IMP void WINAPI Sleep(DWORD);
IMP DWORD WINAPI GetCurrentProcessId(void);
IMP DWORD WINAPI GetWindowThreadProcessId(HWND,DWORD*);
IMP HWND WINAPI CreateWindowExW(DWORD,LPCWSTR,LPCWSTR,DWORD,int,int,int,int,HWND,void*,HMODULE,LPVOID);
IMP BOOL WINAPI DestroyWindow(HWND);
IMP LONG WINAPI SetWindowLongW(HWND,int,LONG);
IMP LONG WINAPI GetWindowLongW(HWND,int);
IMP LONG WINAPI CallWindowProcW(WNDPROC,HWND,UINT,SIZE_T,LONG);
IMP int WINAPI GetClientRect(HWND,RECT*);
IMP BOOL WINAPI ScreenToClient(HWND,POINT*);
IMP int WINAPI MultiByteToWideChar(UINT,DWORD,const char*,int,WCHAR*,int);
IMP DWORD WINAPI GetTickCount(void);
IMP void WINAPI OutputDebugStringA(const char*);
extern void overlay_log(const char*);
extern int WINAPI FHDChat_PollW(WCHAR*,int,WCHAR*,int,int*);
extern int WINAPI FHDChat_SendW(const WCHAR*);
extern int WINAPI FHDChat_StatusW(WCHAR*,int);
extern int WINAPI FHDChat_SetNickW(const WCHAR*);
extern int WINAPI FHDChat_GetNickW(WCHAR*,int);

int _fltused=0;
static void** g_hookvtable;static void* g_realdevice;static void* g_fontTexture;static void* g_fontOwner;
static HRESULT(WINAPI *g_original_end)(void*);
static HRESULT(WINAPI *g_original_present)(void*,const RECT*,const RECT*,HWND,const void*);static HWND g_game;static WNDPROC g_prevproc;
static int g_open=1,g_edit=0,g_scroll=0,g_count=0,g_hooked=0,g_panel_x=0,g_panel_y=300;
#include "chat_preferences.h"
extern int WINAPI FHDChat_LoadPreferences(FHDChatPreferences*);
extern int WINAPI FHDChat_SavePreferences(const FHDChatPreferences*);
IMP HWND WINAPI SetCapture(HWND);
IMP BOOL WINAPI ReleaseCapture(void);
static int g_panel_width=450,g_panel_height=270,g_opacity=70,g_origin_x,g_origin_y;
static FHDChatPreferences g_preferences,g_pending;
static int g_preferences_loaded,g_settings,g_name_edit,g_name_len,g_window_locked=1;
static int g_notifications,g_pending_notifications;
extern int WINAPI FHDChat_LoadNotifications(void);
extern int WINAPI FHDChat_SaveSettings(const FHDChatPreferences*,int);
extern int WINAPI FHDChat_LoadWindowLock(void);
extern int WINAPI FHDChat_SetWindowLock(int,const FHDChatPreferences*);
static WCHAR g_settings_notice[64];
static int g_dragging,g_drag_moved,g_drag_start_x,g_drag_start_y,g_drag_panel_x,g_drag_panel_y,g_mouse_owned;
static int game_session_active(void);
static void suspend_chat_for_menu(void);
static int g_session_was_active;
#ifdef FHD_TEST
static int g_session_override=-1;
#endif
static int chat_available(void){
 int active=game_session_active();
 if(active){g_session_was_active=1;return 1;}
 if(g_session_was_active){g_session_was_active=0;suspend_chat_for_menu();}return 0;
}
static WCHAR g_input[256];static int g_input_len=0;static int g_escape_cancel=0;
#include "chat_event.h"
extern int WINAPI FHDChat_PollEx(FHDChatEvent*);
extern int WINAPI FHDChat_GetOnline(void);
extern int WINAPI FHDChat_IsBackup(void);
extern int WINAPI FHDChat_CanPrivate(void);
extern int WINAPI FHDChat_SelectPeerW(const WCHAR*,const char*,char*,int);
extern int WINAPI FHDChat_SendToW(const WCHAR*,int,const char*);
extern DWORD WINAPI FHDChat_PrivateDay(void);
extern DWORD WINAPI FHDChat_RoomDay(void);
extern int WINAPI FHDChat_ParsePrivateW(const WCHAR*,char*,int,WCHAR*,int,int*);
#define CONVERSATIONS 128
#define LIST_WIDTH 128
static FHDChatEvent g_history[HISTORY];
static int g_count_public,g_scroll_public,g_tab;
typedef struct {char id[80];WCHAR name[36],draft[256];int len,count,scroll,unread;FHDChatEvent history[HISTORY];} CONVERSATION;
static CONVERSATION g_conversations[CONVERSATIONS];
static int g_conversation_count,g_peer_index=-1,g_list_scroll;
static WCHAR g_public_draft[256],g_unselected_draft[256];static int g_public_len,g_unselected_len;
/* Viewer-local colors persist for a connection after it leaves presence. */
static struct {char id[80];WCHAR name[36];DWORD color;int ordinal;} g_colors[512];
static int g_color_count;
static WCHAR g_peer[36],g_notice[64];static char g_peer_id[80];
static DWORD g_notice_until,g_private_day,g_day_checked;

static VERT g_verts[MAX_FONT_VERTS];static UINT g_vert_count;
static int g_view_w=0,g_view_h=0;
static BYTE g_owned_keys[256];
static HANDLE g_message_hook;
static DWORD g_input_thread;
static volatile LONG g_ready=0;static int g_frame_logged=0;
static volatile LONG g_endscene_calls=0,g_present_calls=0;
static DWORD g_last_stats=0;static int g_last_draw_error_logged=0;

static void wcopy(WCHAR* dest,int cap,const WCHAR* s){int i=0;if(!dest||cap<=0)return;while(s&&s[i]&&i<cap-1){dest[i]=s[i];i++;}dest[i]=0;}
static int glyph_index(WCHAR c){if(c>=32&&c<=126)return c-32;if(c>=0x0400&&c<=0x045f)return 95+(c-0x0400);switch(c){case 0x0490:return 191;case 0x0491:return 192;case 0x2014:return 193;case 0x2013:return 194;case 0x2026:return 195;default:return '?' -32;}}
#define VT(obj,index,type) ((type)(*(void***)(obj))[index])
static HRESULT callEnd(void* dev){return g_original_end?g_original_end(dev):0;}
static void addrect(float x,float y,float w,float h,DWORD c,float u0,float v0,float u1,float v1){
 if(g_vert_count+6>MAX_FONT_VERTS)return;
 VERT p={x-0.5f,y-0.5f,0.0f,1.0f,c,u0,v0};
 VERT q={x+w-0.5f,y-0.5f,0.0f,1.0f,c,u1,v0};
 VERT r={x-0.5f,y+h-0.5f,0.0f,1.0f,c,u0,v1};
 VERT s={x+w-0.5f,y+h-0.5f,0.0f,1.0f,c,u1,v1};
 g_verts[g_vert_count++]=p;g_verts[g_vert_count++]=q;g_verts[g_vert_count++]=r;
 g_verts[g_vert_count++]=r;g_verts[g_vert_count++]=q;g_verts[g_vert_count++]=s;
}
static HRESULT drawverts(void* dev){HRESULT(WINAPI *f)(void*,UINT,UINT,const void*,UINT)=VT(dev,83,void*);if(!g_vert_count)return 0;HRESULT rv=f(dev,4,g_vert_count/3,g_verts,sizeof(VERT));g_vert_count=0;return rv;}
static void fonttext(float x,float y,const WCHAR* text,DWORD color,int maxChars){int charpos=0;if(!text)return;
 for(int i=0;text[i]&&charpos<maxChars;i++){if(text[i]=='\r'||text[i]=='\n')continue;int idx=glyph_index(text[i]);float u=(float)(idx%32)*16.0f/512.0f,v=(float)(idx/32)*20.0f/256.0f;
 addrect(x+charpos*8,y,16,20,color,u,v,u+16.0f/512.0f,v+20.0f/256.0f);charpos++;}}
static int init_font(void* dev){if(g_fontTexture&&g_fontOwner==dev)return 1;
 if(g_fontTexture && g_fontOwner!=dev){VT(g_fontTexture,2,DWORD(WINAPI*)(void*))(g_fontTexture);}
 g_fontTexture=0;g_fontOwner=dev;
 HRESULT(WINAPI *CreateTexture)(void*,UINT,UINT,UINT,DWORD,UINT,UINT,void**,HANDLE*)=VT(dev,23,void*);
 void* tex=0;if(CreateTexture(dev,512,256,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&tex,0)<0||!tex)return 0;
 HRESULT(WINAPI *LockRect)(void*,UINT,D3DLOCKED_RECT*,const RECT*,DWORD)=VT(tex,19,void*);
 HRESULT(WINAPI *UnlockRect)(void*,UINT)=VT(tex,20,void*);
 D3DLOCKED_RECT lr;if(LockRect(tex,0,&lr,0,0)<0){VT(tex,2,DWORD(WINAPI*)(void*)) (tex);return 0;}
 for(int y=0;y<256;y++){DWORD* pixels=(DWORD*)((BYTE*)lr.pBits+y*lr.Pitch);for(int x=0;x<512;x++)pixels[x]=0;}
 for(int i=0;i<GLYPH_COUNT;i++)for(int yy=0;yy<20;yy++)for(int xx=0;xx<GLYPH_ALPHA_WIDTH;xx++){
 BYTE alpha=glyph_alpha8[(i*20+yy)*GLYPH_ALPHA_WIDTH+xx];
 if(alpha){int x=(i%32)*16+xx,y=(i/32)*20+yy;DWORD* p=(DWORD*)((BYTE*)lr.pBits+y*lr.Pitch);p[x]=((DWORD)alpha<<24)|0x00ffffffu;}}
 UnlockRect(tex,0);g_fontTexture=tex;return 1;
}
static void setup_pipeline(void* dev,int textmode){
 HRESULT(WINAPI *SetRenderState)(void*,UINT,DWORD)=VT(dev,57,void*);
 HRESULT(WINAPI *SetTextureStageState)(void*,DWORD,UINT,DWORD)=VT(dev,67,void*);
 HRESULT(WINAPI *SetSamplerState)(void*,DWORD,UINT,DWORD)=VT(dev,69,void*);
 HRESULT(WINAPI *SetTexture)(void*,DWORD,void*)=VT(dev,65,void*);
 HRESULT(WINAPI *SetFVF)(void*,DWORD)=VT(dev,89,void*);
 HRESULT(WINAPI *SetVertexShader)(void*,void*)=VT(dev,92,void*);
 HRESULT(WINAPI *SetPixelShader)(void*,void*)=VT(dev,107,void*);
 SetVertexShader(dev,0);SetPixelShader(dev,0);SetFVF(dev,D3DFVF_FHD);
 SetRenderState(dev,7,0);SetRenderState(dev,14,0);SetRenderState(dev,22,1);SetRenderState(dev,27,1);SetRenderState(dev,19,5);SetRenderState(dev,20,6);SetRenderState(dev,137,0);SetRenderState(dev,174,0);
 SetTextureStageState(dev,0,1,textmode?4:2);SetTextureStageState(dev,0,2,textmode?2:0);SetTextureStageState(dev,0,3,0);
 SetTextureStageState(dev,0,4,textmode?4:2);SetTextureStageState(dev,0,5,textmode?2:0);SetTextureStageState(dev,0,6,0);
 SetTextureStageState(dev,1,1,1);SetTextureStageState(dev,1,4,1);
 SetSamplerState(dev,0,5,1);SetSamplerState(dev,0,6,1);SetSamplerState(dev,0,1,3);SetSamplerState(dev,0,2,3);
 SetTexture(dev,0,textmode?g_fontTexture:0);
}

static int ascii_equal(const char*a,const char*b){int i=0;while(a[i]&&b[i]&&a[i]==b[i])i++;return a[i]==b[i];}
static void ascii_copy(char*d,int cap,const char*s){int i=0;while(s&&s[i]&&i+1<cap){d[i]=s[i];i++;}d[i]=0;}
static void number_w(WCHAR* s,int cap,int n){WCHAR b[12];int count=0;if(n<0){wcopy(s,cap,L"—");return;}do{b[count++]=(WCHAR)('0'+n%10);n/=10;}while(n&&count<11);int i=0;while(count&&i+1<cap)s[i++]=b[--count];s[i]=0;}
static void append_w(WCHAR*d,int cap,const WCHAR*s){int n=0;while(d[n]&&n<cap-1)n++;wcopy(d+n,cap-n,s);}
static void notice(const WCHAR* text){wcopy(g_notice,64,text);g_notice_until=GetTickCount()+6000;}
static void save_draft(void);
static DWORD background_color(DWORD c){if(g_opacity==100)return c|0xff000000u;DWORD a=(c>>24)*(DWORD)g_opacity/70;if(a>255)a=255;return (c&0xffffffu)|(a<<24);}
static FHDChatPreferences *active_preferences(void){return g_settings?&g_pending:&g_preferences;}
static void preferences_defaults(FHDChatPreferences*p){p->nickname[0]=0;p->size=1;p->opacity=70;p->x=p->y=-1;}
static void apply_geometry(void){
 static const int widths[4]={400,450,600,750},heights[4]={240,270,360,450};FHDChatPreferences*p=active_preferences();
 g_panel_width=widths[p->size];g_panel_height=heights[p->size];g_opacity=p->opacity;
 if(g_view_w>0&&g_panel_width>g_view_w)g_panel_width=g_view_w;if(g_view_h>0&&g_panel_height>g_view_h)g_panel_height=g_view_h;
 int x=p->x,y=p->y;if(x<0||y<0){x=0;y=g_view_h-g_panel_height-30;}
 int xmax=g_view_w-g_panel_width,ymax=g_view_h-g_panel_height;if(xmax<0)xmax=0;if(ymax<0)ymax=0;
 if(x<0)x=0;if(y<0)y=0;if(x>xmax)x=xmax;if(y>ymax)y=ymax;
 g_panel_x=g_origin_x+x;g_panel_y=g_origin_y+y;
}
static void ensure_preferences(void){if(g_preferences_loaded)return;preferences_defaults(&g_preferences);FHDChat_LoadPreferences(&g_preferences);g_window_locked=FHDChat_LoadWindowLock();g_notifications=FHDChat_LoadNotifications();g_preferences_loaded=1;}
static void open_settings(void){save_draft();g_edit=0;g_pending=g_preferences;g_pending_notifications=g_notifications;g_name_len=0;while(g_pending.nickname[g_name_len])g_name_len++;g_settings=1;g_name_edit=0;g_settings_notice[0]=0;}
static void cancel_settings(void){g_settings=0;g_name_edit=0;g_settings_notice[0]=0;apply_geometry();}
static int commit_settings(void){
 int result=FHDChat_SaveSettings(&g_pending,g_pending_notifications);
 if(result!=1){wcopy(g_settings_notice,64,result<0?L"Ник: 1–24 буквы/цифры, пробел, _ . -":L"Не удалось сохранить. Проверьте доступ");return 0;}
 g_preferences=g_pending;g_notifications=g_pending_notifications;g_settings=0;g_name_edit=0;g_settings_notice[0]=0;apply_geometry();return 1;
}
static void begin_drag(HWND hwnd,int mx,int my){if(g_window_locked)return;g_dragging=1;g_drag_moved=0;g_drag_start_x=mx;g_drag_start_y=my;g_drag_panel_x=g_panel_x;g_drag_panel_y=g_panel_y;g_edit=0;g_name_edit=0;SetCapture(hwnd);}
static void update_drag(int mx,int my){
 if(g_window_locked)return;int dx=mx-g_drag_start_x,dy=my-g_drag_start_y;if(!g_drag_moved&&dx>=-2&&dx<=2&&dy>=-2&&dy<=2)return;
 g_drag_moved=1;FHDChatPreferences*p=active_preferences();p->x=g_drag_panel_x+dx-g_origin_x;p->y=g_drag_panel_y+dy-g_origin_y;
 /* Negative mouse positions clamp to the edge, rather than select auto anchor. */
 if(p->x<0)p->x=0;if(p->y<0)p->y=0;apply_geometry();p->x=g_panel_x-g_origin_x;p->y=g_panel_y-g_origin_y;
}
static void finish_drag(int release){if(!g_dragging)return;g_dragging=0;if(release)ReleaseCapture();if(g_drag_moved&&!g_settings&&FHDChat_SavePreferences(&g_preferences)!=1)notice(L"Положение не сохранено: нет доступа");g_drag_moved=0;}
/* The menu owns keyboard/mouse when no active campaign player exists. */
static void suspend_chat_for_menu(void){
 save_draft();g_edit=0;g_name_edit=0;finish_drag(1);
 if(g_settings)cancel_settings();g_mouse_owned=0;g_escape_cancel=0;
 for(int i=0;i<256;i++)g_owned_keys[i]=0;
}
static void toggle_window_lock(void){
 finish_drag(1);g_window_locked=!g_window_locked;int saved=FHDChat_SetWindowLock(g_window_locked,&g_preferences);
 const WCHAR*message=saved==1?(g_window_locked?L"Окно закреплено":L"Окно можно перемещать"):L"Фиксация изменена, но не сохранена";
 if(g_settings)wcopy(g_settings_notice,64,message);else notice(message);
}
#include "header_icons.h"
static void header_icon(float x,float y,const char mask[19][20],DWORD color){
 for(int yy=0;yy<19;yy++)for(int xx=0;xx<19;){if(mask[yy][xx]!='#'){xx++;continue;}int start=xx;while(xx<19&&mask[yy][xx]=='#')xx++;addrect(x+start,y+yy,xx-start,1,color,0,0,0,0);}
}
static void header_buttons(int x,int y){
 addrect(x+PANEL_WIDTH-60,y+2,26,22,background_color(g_window_locked?RGBA(220,24,65,57):RGBA(220,69,52,31)),0,0,0,0);
 addrect(x+PANEL_WIDTH-32,y+2,28,22,background_color(g_settings?RGBA(245,30,76,91):RGBA(210,24,49,64)),0,0,0,0);
 header_icon(x+PANEL_WIDTH-56,y+3,g_window_locked?icon_locked:icon_unlocked,g_window_locked?RGBA(255,117,230,166):RGBA(255,255,202,110));
 header_icon(x+PANEL_WIDTH-27,y+3,icon_gear,RGBA(255,151,220,235));
}
static void settings_shapes(int x,int y){
 addrect(x+12,y+52,PANEL_WIDTH-24,26,background_color(RGBA(235,24,49,64)),0,0,0,0);
 int bw=(PANEL_WIDTH-32)/4;for(int i=0;i<4;i++)addrect(x+12+i*bw,y+108,bw-4,26,background_color(i==g_pending.size?RGBA(255,30,100,109):RGBA(210,24,49,64)),0,0,0,0);
 addrect(x+PANEL_WIDTH-86,y+145,32,26,background_color(RGBA(240,30,76,91)),0,0,0,0);addrect(x+PANEL_WIDTH-46,y+145,32,26,background_color(RGBA(240,30,76,91)),0,0,0,0);
 addrect(x+PANEL_WIDTH-144,y+178,60,24,background_color(!g_pending_notifications?RGBA(255,30,100,109):RGBA(210,24,49,64)),0,0,0,0);
 addrect(x+PANEL_WIDTH-76,y+178,60,24,background_color(g_pending_notifications?RGBA(255,30,100,109):RGBA(210,24,49,64)),0,0,0,0);
 addrect(x+12,y+PANEL_HEIGHT-36,86,26,background_color(RGBA(240,35,58,72)),0,0,0,0);addrect(x+PANEL_WIDTH-240,y+PANEL_HEIGHT-36,104,26,background_color(RGBA(240,35,58,72)),0,0,0,0);addrect(x+PANEL_WIDTH-128,y+PANEL_HEIGHT-36,116,26,background_color(RGBA(255,30,91,78)),0,0,0,0);
}
static void settings_text(int x,int y){
 DWORD fg=RGBA(255,222,234,245);fonttext(x+16,y+4,L"Настройки чата",RGBA(255,117,230,255),28);
 fonttext(x+16,y+30,L"Ник (Enter — начать/закончить ввод)",fg,(PANEL_WIDTH-32)/8);
 fonttext(x+16,y+55,g_pending.nickname[0]?g_pending.nickname:L"Имя пилота (из игры)",g_pending.nickname[0]?RGBA(255,105,238,140):RGBA(255,112,148,169),(PANEL_WIDTH-32)/8);
 if(g_name_edit&&((GetTickCount()/420)%2)==0)fonttext(x+16+g_name_len*8,y+55,L"_",RGBA(255,105,238,140),1);
 fonttext(x+16,y+81,g_settings_notice[0]?g_settings_notice:L"Пустой ник — имя активного пилота",g_settings_notice[0]?RGBA(255,255,203,120):RGBA(255,151,195,215),(PANEL_WIDTH-32)/8);
 static const WCHAR*labels[4]={L"Меньше",L"Стандарт",L"Больше",L"Больше +"};int bw=(PANEL_WIDTH-32)/4;for(int i=0;i<4;i++)fonttext(x+18+i*bw,y+111,labels[i],fg,(bw-10)/8);
 WCHAR status[40],num[12];wcopy(status,40,L"Прозрачность: ");number_w(num,12,100-g_pending.opacity);append_w(status,40,num);append_w(status,40,L"%");fonttext(x+16,y+148,status,fg,28);
 fonttext(x+PANEL_WIDTH-76,y+148,L"-",fg,1);fonttext(x+PANEL_WIDTH-36,y+148,L"+",fg,1);
 fonttext(x+16,y+181,L"Уведомления",fg,20);
 fonttext(x+PANEL_WIDTH-136,y+181,L"Выкл",fg,5);fonttext(x+PANEL_WIDTH-64,y+181,L"Вкл",fg,4);
 fonttext(x+22,y+PANEL_HEIGHT-33,L"Сброс",fg,8);fonttext(x+PANEL_WIDTH-226,y+PANEL_HEIGHT-33,L"Отмена",fg,9);fonttext(x+PANEL_WIDTH-118,y+PANEL_HEIGHT-33,L"Применить",RGBA(255,130,245,179),12);
}
static int wide_equal(const WCHAR*a,const WCHAR*b){int i=0;while(a[i]&&b[i]&&a[i]==b[i])i++;return a[i]==b[i];}
static int color_referenced(const WCHAR*name,const char*id){
 for(int i=0;i<g_count_public;i++)if(ascii_equal(g_history[i].senderId,id)&&wide_equal(g_history[i].nick,name))return 1;
 for(int i=0;i<g_conversation_count;i++){
  CONVERSATION*c=&g_conversations[i];if(ascii_equal(c->id,id)&&wide_equal(c->name,name))return 1;
  for(int j=0;j<c->count;j++)if(ascii_equal(c->history[j].senderId,id)&&wide_equal(c->history[j].nick,name))return 1;
 }
 return 0;
}
static DWORD nickname_color(const WCHAR*name,const char*id,int own){
 /* Self color has priority over IDs, cached colors and nickname collisions. */
 if(own)return RGBA(255,105,238,140);
 if(!id||!id[0])return RGBA(255,116,196,255);
 for(int i=0;i<g_color_count;i++)if(ascii_equal(g_colors[i].id,id)&&wide_equal(g_colors[i].name,name))return g_colors[i].color;
 /* Retain every visible/history identity when reclaiming old connections. */
 if(g_color_count==512){int n=0;for(int i=0;i<g_color_count;i++)if(color_referenced(g_colors[i].name,g_colors[i].id))g_colors[n++]=g_colors[i];g_color_count=n;}
 BYTE used[512];for(int i=0;i<512;i++)used[i]=0;
 for(int i=0;i<g_color_count;i++)if(wide_equal(g_colors[i].name,name))used[g_colors[i].ordinal]=1;
 int ordinal=0;while(ordinal<511&&used[ordinal])ordinal++;
 int hue=(200+(ordinal%360)*137)%360,sector=hue/60,minimum=ordinal<360?85:125,range=255-minimum,f=(hue%60)*range/60;
 int r=minimum,g=minimum,b=minimum;
 switch(sector){case 0:r=255;g=minimum+f;break;case 1:r=255-f;g=255;break;case 2:g=255;b=minimum+f;break;case 3:g=255-f;b=255;break;case 4:r=minimum+f;b=255;break;default:r=255;b=255-f;break;}
 DWORD color=RGBA(255,r,g,b);
 if(g_color_count<512){int i=g_color_count++;ascii_copy(g_colors[i].id,80,id);wcopy(g_colors[i].name,36,name);g_colors[i].color=color;g_colors[i].ordinal=ordinal;}
 return color;
}
static int conversation_slot(const char*id,const WCHAR*name,int create){
 if(!id||!id[0])return -1;
 for(int i=0;i<g_conversation_count;i++)if(ascii_equal(g_conversations[i].id,id)){if(name&&name[0])wcopy(g_conversations[i].name,36,name);return i;}
 if(!create||g_conversation_count==CONVERSATIONS)return -1;
 int i=g_conversation_count++;ascii_copy(g_conversations[i].id,80,id);wcopy(g_conversations[i].name,36,name);return i;
}
static int unread_total(void){int n=0;for(int i=0;i<g_conversation_count;i++)n+=g_conversations[i].unread;return n;}
static int active_count(void){return g_tab?(g_peer_index>=0?g_conversations[g_peer_index].count:0):g_count_public;}
static int *active_scroll(void){return g_tab&&g_peer_index>=0?&g_conversations[g_peer_index].scroll:&g_scroll_public;}
static FHDChatEvent *active_history(void){return g_tab?(g_peer_index>=0?g_conversations[g_peer_index].history:0):g_history;}
static void save_draft(void){
 if(!g_tab){wcopy(g_public_draft,256,g_input);g_public_len=g_input_len;}
 else if(g_peer_index>=0){CONVERSATION *c=&g_conversations[g_peer_index];wcopy(c->draft,256,g_input);c->len=g_input_len;}
 else{wcopy(g_unselected_draft,256,g_input);g_unselected_len=g_input_len;}
}
static void load_draft(void){
 if(!g_tab){wcopy(g_input,256,g_public_draft);g_input_len=g_public_len;}
 else if(g_peer_index>=0){CONVERSATION*c=&g_conversations[g_peer_index];wcopy(g_input,256,c->draft);g_input_len=c->len;c->unread=0;}
 else{wcopy(g_input,256,g_unselected_draft);g_input_len=g_unselected_len;}
}
static void switch_tab(int tab){save_draft();g_tab=tab;g_edit=0;load_draft();}
static int choose_peer(const char*id,const WCHAR*name){
 int i=conversation_slot(id,name,1);if(i<0){notice(L"Список личных чатов заполнен");return 0;}
 save_draft();g_peer_index=i;wcopy(g_peer,36,g_conversations[i].name);ascii_copy(g_peer_id,80,id);g_tab=1;g_edit=0;load_draft();
 if(i<g_list_scroll)g_list_scroll=i;int lines=(PANEL_HEIGHT-102)/21;if(i>=g_list_scroll+lines)g_list_scroll=i-lines+1;
 g_notice[0]=0;return 1;
}
static int visible_lines(void){return (PANEL_HEIGHT-58-(g_tab?52:32))/20;}
static int list_lines(void){return (PANEL_HEIGHT-102)/21;}
static int messages_top(void){return g_tab?52:32;}
static int message_x(void){return g_tab?LIST_WIDTH+10:10;}
static int message_chars(void){return (PANEL_WIDTH-message_x()-24)/8;}
static int input_chars(void){return (PANEL_WIDTH-96)/8;}
static int nick_length(const FHDChatEvent*line){int n=0;while(n<(g_tab?12:16)&&line->nick[n])n++;return n;}
static int text_length(const WCHAR*s){int n=0;while(s[n])n++;return n;}
/* One shared layout for drawing, row counts, scrolling and nickname hit
 * testing. The first row reserves the nickname and gap; all continuation
 * rows start at the history left edge without repeating the nickname. */
static int message_row(const FHDChatEvent*line,int start,int first,int*length){
 const WCHAR*text=line->text;int left=message_x()+(first?12+nick_length(line)*8:0);
 /* Glyph quads are 16px wide with an 8px advance. Leave a 16px right inset. */
 int capacity=(PANEL_WIDTH-left-24)/8;if(capacity<1)capacity=1;
 int n=0;while(n<capacity&&text[start+n]&&text[start+n]!='\r'&&text[start+n]!='\n')n++;
 int next=start+n;
 if(n==capacity&&text[next]&&text[next]!='\r'&&text[next]!='\n'){
  int split=n;while(split>0&&text[start+split]!=' '&&text[start+split]!='\t')split--;
  if(split>0){n=split;next=start+split;while(text[next]==' '||text[next]=='\t')next++;}
 }
 if(text[next]=='\r')next++;if(text[next]=='\n')next++;
 *length=n;return next;
}
static int event_rows(const FHDChatEvent*line){
 int offset=0,rows=0;do{int length;offset=message_row(line,offset,rows==0,&length);rows++;}while(line->text[offset]);return rows;
}
static int active_rows(void){int n=0;FHDChatEvent*h=active_history();for(int i=0;i<active_count();i++)n+=event_rows(&h[i]);return n;}
static FHDChatEvent *hit_message_row(int row){int start=active_rows()-visible_lines()-*active_scroll();if(start<0)start=0;row+=start;FHDChatEvent*h=active_history();for(int i=0;i<active_count();i++){int n=event_rows(&h[i]);if(row==0)return &h[i];if(row<n)return 0;row-=n;}return 0;}
static void check_private_day(void){
 DWORD now=GetTickCount(),room=FHDChat_RoomDay();if(g_private_day&&now-g_day_checked<30000&&(!room||room==g_private_day))return;g_day_checked=now;
 DWORD day=room?room:FHDChat_PrivateDay();if(!g_private_day){g_private_day=day;return;}
 if(day!=g_private_day){g_private_day=day;g_count_public=0;g_scroll_public=0;g_conversation_count=0;g_peer_index=-1;g_list_scroll=0;g_peer[0]=0;g_peer_id[0]=0;g_unselected_draft[0]=0;g_unselected_len=0;
  for(int i=0;i<CONVERSATIONS;i++){g_conversations[i].id[0]=0;g_conversations[i].draft[0]=0;g_conversations[i].len=0;g_conversations[i].count=0;g_conversations[i].scroll=0;g_conversations[i].unread=0;}
  if(g_tab){g_input[0]=0;g_input_len=0;g_edit=0;}
 }
}
static int send_current(void){
 char id[80];WCHAR name[36];int offset=0,command=g_input[0]=='@'||g_input[0]=='"';
 if(command){
  int result=FHDChat_ParsePrivateW(g_input,id,80,name,36,&offset);
  if(result!=1){notice(result<0?(result==-1?L"Одинаковые ники: выберите через ПКМ":L"После ника нужен текст сообщения"):L"Ник не найден. Пример: @Ranger текст");return 0;}
  if(!FHDChat_CanPrivate()){notice(L"Личный чат временно недоступен");return 0;}
  if(conversation_slot(id,name,0)<0&&g_conversation_count==CONVERSATIONS){notice(L"Список личных чатов заполнен");return 0;}
  if(!FHDChat_SendToW(g_input+offset,1,id)){notice(L"Получатель или соединение недоступны");return 0;}
  /* Remove only this command; preserve the target's existing draft. */
  g_input[0]=0;g_input_len=0;choose_peer(id,name);g_edit=0;return 2;
 }
 if(g_tab){if(!g_peer_id[0]){notice(L"ПКМ по нику или @ник текст");return 0;}if(!FHDChat_CanPrivate()){notice(L"Личный чат временно недоступен");return 0;}}
 int ok=FHDChat_SendToW(g_input,g_tab,g_tab?g_peer_id:0);if(!ok)notice(g_tab?L"Получатель или соединение недоступны":L"Нет соединения. Текст сохранён");return ok;
}
static void finish_send(int result){if(result==1){g_input_len=0;g_input[0]=0;save_draft();}g_edit=0;*active_scroll()=0;}
static void incoming(void){
 check_private_day();FHDChatEvent e;
 for(int i=0;i<96&&FHDChat_PollEx(&e);i++){
  if(e.day&&e.day!=g_private_day){g_day_checked=GetTickCount()-30000;check_private_day();if(e.day!=g_private_day)continue;}
  if(e.event==2){g_day_checked=GetTickCount()-30000;check_private_day();continue;}
  if(e.event==1){if(!e.channel){g_count_public=0;g_scroll_public=0;}continue;}
  nickname_color(e.nick,e.senderId,e.own);
  FHDChatEvent *history=g_history;int *count=&g_count_public;
  if(e.channel){
   int index=conversation_slot(e.peerId,e.peer,1);if(index<0){notice(e.peerId[0]?L"Список личных чатов заполнен":e.text);continue;}
   CONVERSATION*c=&g_conversations[index];history=c->history;count=&c->count;
   if(!e.own&&!(game_session_active()&&g_tab&&g_open&&index==g_peer_index)&&c->unread<999)c->unread++;
  }
  if(*count==HISTORY){for(int j=1;j<HISTORY;j++)history[j-1]=history[j];(*count)--;}
  history[(*count)++]=e;
  /* Replay, service events and own echoes never open the panel. Only open
   * the chat panel inside an active campaign; never focus a Windows window
   * or start text entry. Preserve the selected conversation and drafts. */
  if(g_notifications&&!g_open&&!e.own&&e.event==0&&game_session_active()){
   if(g_settings)cancel_settings();g_open=1;g_edit=0;g_name_edit=0;
  }
 }
}
static void draw_chat(void* dev){ensure_preferences();int available=chat_available();incoming();if(!available)return;D3DVIEWPORT9 vp;HRESULT(WINAPI *GetViewport)(void*,D3DVIEWPORT9*)=VT(dev,48,void*);
 if(GetViewport(dev,&vp)<0||vp.Width<500||vp.Height<320){if(!g_last_draw_error_logged){g_last_draw_error_logged=1;overlay_log("DRAW_SKIP: viewport is unavailable or too small");}return;}
 g_view_w=(int)vp.Width;g_view_h=(int)vp.Height;
 g_origin_x=(int)vp.X;g_origin_y=(int)vp.Y;apply_geometry();
 int maxlist=g_conversation_count-list_lines();if(maxlist<0)maxlist=0;if(g_list_scroll>maxlist)g_list_scroll=maxlist;
 int maxscroll=active_rows()-visible_lines();if(maxscroll<0)maxscroll=0;if(*active_scroll()>maxscroll)*active_scroll()=maxscroll;
 if(!g_open)return;
 if(!init_font(dev)){if(!g_last_draw_error_logged){g_last_draw_error_logged=1;overlay_log("DRAW_SKIP: font texture initialization failed");}return;}
 void* st=0;HRESULT(WINAPI *CreateStateBlock)(void*,UINT,void**)=VT(dev,59,void*);
 if(CreateStateBlock(dev,1,&st)<0||!st){if(!g_last_draw_error_logged){g_last_draw_error_logged=1;overlay_log("DRAW_SKIP: CreateStateBlock failed");}return;}
 VT(st,4,HRESULT(WINAPI*)(void*))(st);
 int x=g_panel_x,y=g_panel_y;
 setup_pipeline(dev,0);g_vert_count=0;
 addrect(x,y,PANEL_WIDTH,PANEL_HEIGHT,background_color(RGBA(178,9,18,32)),0,0,0,0);
 addrect(x,y,PANEL_WIDTH,26,background_color(RGBA(225,20,38,62)),0,0,0,0);
 if(g_settings)settings_shapes(x,y);else{
 addrect(x+3+(g_tab?112:0),y+2,106,22,background_color(RGBA(235,30,76,91)),0,0,0,0);
 addrect(x+8,y+PANEL_HEIGHT-39,PANEL_WIDTH-16,30,background_color(g_edit?RGBA(240,24,66,65):RGBA(210,24,39,56)),0,0,0,0);
 if(g_tab){
  addrect(x+LIST_WIDTH,y+27,1,PANEL_HEIGHT-69,RGBA(160,74,145,183),0,0,0,0);
  for(int i=0;i<list_lines()&&i+g_list_scroll<g_conversation_count;i++)if(i+g_list_scroll==g_peer_index)addrect(x+3,y+31+i*21,LIST_WIDTH-6,21,background_color(RGBA(200,30,76,91)),0,0,0,0);
 }
 addrect(x+1,y+PANEL_HEIGHT-1,PANEL_WIDTH-2,1,RGBA(235,74,145,183),0,0,0,0);
 }
 header_buttons(x,y);
 drawverts(dev);
 setup_pipeline(dev,1);g_vert_count=0;

 if(g_settings)settings_text(x,y);else{
 fonttext(x+16,y+4,L"Общий",g_tab?RGBA(255,153,181,196):RGBA(255,117,230,255),12);
 WCHAR label[40];wcopy(label,40,L"Личный");if(unread_total()){WCHAR num[12];number_w(num,12,unread_total());append_w(label,40,L" (");append_w(label,40,num);append_w(label,40,L")");}
 fonttext(x+128,y+4,label,g_tab?RGBA(255,117,230,255):RGBA(255,153,181,196),18);
 WCHAR status[40],num[12];wcopy(status,40,L"Онлайн: ");number_w(num,12,FHDChat_GetOnline());append_w(status,40,num);
 fonttext(x+PANEL_WIDTH-162,y+4,FHDChat_IsBackup()?L"Резерв":status,FHDChat_IsBackup()?RGBA(255,255,203,120):RGBA(255,159,220,198),12);
 if(g_tab){
  for(int i=0;i<list_lines()&&i+g_list_scroll<g_conversation_count;i++){
   CONVERSATION*c=&g_conversations[i+g_list_scroll];WCHAR label[48];wcopy(label,48,c->name);
   if(c->unread){WCHAR num[12];number_w(num,12,c->unread);int len=0;while(label[len])len++;if(len>8)label[8]=0;append_w(label,48,L" (");append_w(label,48,num);append_w(label,48,L")");}
   fonttext(x+8,y+32+i*21,label,nickname_color(c->name,c->id,0),14);
  }
  WCHAR target[60];wcopy(target,60,L"Кому: ");append_w(target,60,g_peer[0]?g_peer:L"@ник текст");fonttext(x+message_x(),y+29,target,RGBA(255,151,195,215),message_chars());
 }
 int visible=visible_lines(),start=active_rows()-visible-*active_scroll();if(start<0)start=0;FHDChatEvent*history=active_history();int virtualrow=0;
 for(int j=0;j<active_count();j++){
  FHDChatEvent *line=&history[j];int k=nick_length(line),offset=0,r=0;
  do{
   int length,next=message_row(line,offset,r==0,&length),screenrow=virtualrow-start;
   if(screenrow>=0&&screenrow<visible){
    if(!r)fonttext(x+message_x(),y+messages_top()+screenrow*20,line->nick,nickname_color(line->nick,line->senderId,line->own),g_tab?12:16);
    fonttext(x+message_x()+(r?0:12+k*8),y+messages_top()+screenrow*20,line->text+offset,RGBA(255,222,234,245),length);
   }
   offset=next;r++;virtualrow++;
  }while(line->text[offset]);
 }
 if(g_notice[0]&&(LONG)(g_notice_until-GetTickCount())>0)fonttext(x+10,y+PANEL_HEIGHT-58,g_notice,RGBA(255,255,203,120),(PANEL_WIDTH-30)/8);
 if(g_input_len){
  /* Keep draft text clear of the send button. */
  int limit=input_chars(),off=(g_input_len>limit)?g_input_len-limit:0;
  fonttext(x+12,y+PANEL_HEIGHT-34,g_input+off,RGBA(255,241,247,255),input_chars());
 }else{
  fonttext(x+12,y+PANEL_HEIGHT-34,g_edit?L"Введите сообщение...":(FHDChat_IsBackup()?L"Резерв — без истории":L"ENTER - написать сообщение"),
           g_edit?RGBA(255,165,202,215):RGBA(255,112,148,169),input_chars());
 }
 if(g_edit&&((GetTickCount()/420)%2)==0){
  int n=g_input_len;if(n>input_chars())n=input_chars();
  fonttext(x+12+n*8,y+PANEL_HEIGHT-34,L"_",RGBA(255,105,238,140),1);
 }
 fonttext(x+PANEL_WIDTH-56,y+PANEL_HEIGHT-34,L">>",RGBA(255,106,234,142),2);
 }
 drawverts(dev);
 VT(st,5,HRESULT(WINAPI*)(void*))(st);
 VT(st,2,DWORD(WINAPI*)(void*))(st);
}
/* GR_WinMessage dispatches a MSG and then separately invokes the game UI
 * callback with the same MSG. WndProc alone CANNOT consume that second path.
 * A WH_GETMESSAGE hook on the game's own thread rewrites consumed messages to
 * WM_NULL before either path runs. No global keyboard hook or external process.
 */
typedef struct {HWND hwnd;UINT message;SIZE_T wParam;LONG lParam;DWORD time;POINT pt;DWORD lPrivate;} CHATMSG;
typedef LONG(WINAPI *MSGHOOKPROC)(int,SIZE_T,LONG);
IMP HANDLE WINAPI SetWindowsHookExW(int,MSGHOOKPROC,HMODULE,DWORD);
IMP LONG WINAPI CallNextHookEx(HANDLE,int,SIZE_T,LONG);
IMP BOOL WINAPI GetModuleHandleExW(DWORD,LPCWSTR,HMODULE*);
IMP BOOL WINAPI TranslateMessage(const CHATMSG*);

static void map_mouse(HWND hwnd,int *x,int *y){RECT r;
 if(g_view_w>0&&g_view_h>0&&GetClientRect(hwnd,&r)&&r.right>0&&r.bottom>0){*x=*x*g_view_w/r.right;*y=*y*g_view_h/r.bottom;}
}
static int inside(int x,int y){return game_session_active()&&g_open&&x>=g_panel_x&&x<g_panel_x+PANEL_WIDTH&&y>=g_panel_y&&y<g_panel_y+PANEL_HEIGHT;}
/* Return 1 iff this message belongs to chat. Called by both queue and WndProc;
 * a consumed queue message is nulled, so it is never processed twice. */
static int chat_message(HWND hwnd,UINT m,SIZE_T w,LONG l){
 if(!chat_available())return 0;
 if(m==0x0008||(m==0x001c&&!w)){ /* WM_KILLFOCUS / inactive WM_ACTIVATEAPP */
  g_edit=0;g_name_edit=0;g_escape_cancel=0;finish_drag(1);g_mouse_owned=0;for(int i=0;i<256;i++)g_owned_keys[i]=0;return 0;
 }
 if(m==0x0215||m==0x001f){finish_drag(0);g_mouse_owned=0;}
 if(m==WM_KEYUP||m==0x0105){
  if(w<256&&g_owned_keys[w]){g_owned_keys[w]=0;return 1;}
  if((g_edit||g_settings)&&g_open)return 1;
 }
 if((m==WM_KEYDOWN||m==0x0104)&&w==VK_F12){
  g_owned_keys[VK_F12]=1;
  if(!(l&0x40000000)){g_open=!g_open;g_edit=0;g_name_edit=0;finish_drag(1);}
  return 1;
 }
 if(g_open&&g_settings&&m>=0x0100&&m<=0x0109){
  if(m==WM_KEYDOWN||m==0x0104){if(w<256)g_owned_keys[w]=1;if(!(l&0x40000000)){if(w==VK_ESCAPE){g_escape_cancel=1;if(g_name_edit)g_name_edit=0;else cancel_settings();}else if(w==VK_RETURN)g_name_edit=!g_name_edit;}return 1;}
  if((m==WM_CHAR||m==0x0106)&&g_name_edit){WCHAR c=(WCHAR)w;if(c==8){if(g_name_len)g_pending.nickname[--g_name_len]=0;}else if(c>=32&&c!=127&&g_name_len<24){g_pending.nickname[g_name_len++]=c;g_pending.nickname[g_name_len]=0;}}
  return 1;
 }
 if(g_open&&m==WM_MOUSEMOVE){int mx=(short)(l&0xffff),my=(short)((l>>16)&0xffff);map_mouse(hwnd,&mx,&my);if(g_dragging){update_drag(mx,my);return 1;}if(g_settings||inside(mx,my))return 1;}
 if(m==WM_LBUTTONUP&&(g_dragging||g_mouse_owned)){finish_drag(1);g_mouse_owned=0;return 1;}
 if(g_open&&m==WM_LBUTTONDOWN){
  int mx=(short)(l&0xffff),my=(short)((l>>16)&0xffff);map_mouse(hwnd,&mx,&my);
  if(g_settings||inside(mx,my))g_mouse_owned=1;
  if(inside(mx,my)&&my<g_panel_y+26){
   if(mx>=g_panel_x+PANEL_WIDTH-32){if(g_settings)cancel_settings();else open_settings();return 1;}
   if(mx>=g_panel_x+PANEL_WIDTH-60){toggle_window_lock();return 1;}
   if(g_settings||mx>=g_panel_x+240){begin_drag(hwnd,mx,my);return 1;}
  }
  if(g_settings){
   int x=mx-g_panel_x,y=my-g_panel_y;g_settings_notice[0]=0;
   if(y>=108&&y<134&&x>=12&&x<12+4*((PANEL_WIDTH-32)/4)){g_pending.size=(x-12)/((PANEL_WIDTH-32)/4);g_name_edit=0;apply_geometry();}
   else if(y>=145&&y<171){if(x>=PANEL_WIDTH-86&&x<PANEL_WIDTH-54&&g_pending.opacity<100)g_pending.opacity+=10;else if(x>=PANEL_WIDTH-46&&x<PANEL_WIDTH-14&&g_pending.opacity>10)g_pending.opacity-=10;if(g_pending.opacity>100)g_pending.opacity=100;if(g_pending.opacity<10)g_pending.opacity=10;apply_geometry();}
   else if(y>=178&&y<202&&x>=PANEL_WIDTH-144&&x<PANEL_WIDTH-16){if(x<PANEL_WIDTH-84)g_pending_notifications=0;else if(x>=PANEL_WIDTH-76)g_pending_notifications=1;g_name_edit=0;}
   else if(y>=PANEL_HEIGHT-36&&y<PANEL_HEIGHT-10){if(x>=12&&x<98){preferences_defaults(&g_pending);g_pending_notifications=0;g_name_len=0;g_name_edit=0;apply_geometry();}else if(x>=PANEL_WIDTH-240&&x<PANEL_WIDTH-136)cancel_settings();else if(x>=PANEL_WIDTH-128&&x<PANEL_WIDTH-12)commit_settings();}
   else if(!(y>=52&&y<78))g_name_edit=0;
   return 1;
  }
 }
 if(g_open&&g_settings&&m>=0x0200&&m<=0x020e)return 1;
 if(g_open&&m==0x0203){int mx=(short)(l&0xffff),my=(short)((l>>16)&0xffff);map_mouse(hwnd,&mx,&my);if(inside(mx,my))return 1;}
 if(g_open&&(m==WM_KEYDOWN||m==0x0104)&&w==VK_RETURN){
  g_owned_keys[VK_RETURN]=1;
  if(!(l&0x40000000)){
   if(!g_edit){g_edit=1;overlay_log("CHAT_INPUT_OPENED: Enter");}
   else if(g_input_len){
    int sent=send_current();if(sent){finish_send(sent);overlay_log("CHAT_INPUT_SENT: Enter");}
    else overlay_log("CHAT_SEND_FAILED: message retained");
   }else g_edit=0;
  }
  return 1;
 }
 if(g_open&&m==WM_CHAR&&w==VK_RETURN)return 1;
 if(g_escape_cancel&&m==WM_CHAR&&w==VK_ESCAPE){g_escape_cancel=0;return 1;}
 if(g_open&&g_edit){
  if(m==WM_CHAR||m==0x0106){ /* WM_CHAR / WM_SYSCHAR, Unicode layout translation */
   WCHAR c=(WCHAR)w;
   if(c==8){if(g_input_len)g_input[--g_input_len]=0;return 1;}
   if(c>=32&&c!=127&&g_input_len<180){g_input[g_input_len++]=c;g_input[g_input_len]=0;}
   return 1;
  }
  if(m==WM_KEYDOWN||m==0x0104){
   if(w<256)g_owned_keys[w]=1;
   if(w==VK_ESCAPE){g_edit=0;g_escape_cancel=1;overlay_log("CHAT_INPUT_CANCELLED: Escape");}
   return 1;
  }
  if(m>=0x0100&&m<=0x0109)return 1;
 }

 if((m==0x0204||m==0x0205||m==0x0206)&&g_open){ /* RMB picks an actual live sender */
  int mx=(short)(l&0xffff),my=(short)((l>>16)&0xffff);map_mouse(hwnd,&mx,&my);
  if(inside(mx,my)){
   if(m==0x0204){int row=(my-g_panel_y-messages_top())/20;
    if(my>=g_panel_y+messages_top()&&row>=0&&row<visible_lines()){
     FHDChatEvent *line=hit_message_row(row);
     if(line){int n=nick_length(line);
      if(!line->own&&mx>=g_panel_x+message_x()&&mx<g_panel_x+message_x()+8+n*8){
       char id[80];int resolved=FHDChat_SelectPeerW(line->nick,line->peerId,id,80);
       if(resolved==1)choose_peer(id,line->nick);
       else notice(resolved<0?L"Выберите свежее сообщение игрока":L"Игрок сейчас не в сети");
      }
     }
    }
   }
   return 1;
  }
 }
 if(m==WM_LBUTTONDOWN&&g_open){
  int mx=(short)(l&0xffff),my=(short)((l>>16)&0xffff);map_mouse(hwnd,&mx,&my);
  if(inside(mx,my)){
   if(my<g_panel_y+26){if(mx<g_panel_x+115)switch_tab(0);else if(mx<g_panel_x+240)switch_tab(1);return 1;}
   if(g_tab&&mx<g_panel_x+LIST_WIDTH&&my>=g_panel_y+32&&my<g_panel_y+32+list_lines()*21){int index=g_list_scroll+(my-g_panel_y-32)/21;if(index<g_conversation_count)choose_peer(g_conversations[index].id,g_conversations[index].name);return 1;}
   /* Mouse never enables typing. Only Enter can set g_edit to 1. */
   if(g_edit&&my>g_panel_y+PANEL_HEIGHT-44&&mx>g_panel_x+PANEL_WIDTH-65){
    if(g_input_len){int sent=send_current();if(sent)finish_send(sent);}
   }
   return 1;
  }
  g_edit=0;
 }
 if(m==WM_LBUTTONUP&&g_open){int mx=(short)(l&0xffff),my=(short)((l>>16)&0xffff);map_mouse(hwnd,&mx,&my);if(inside(mx,my))return 1;}
 if(m==WM_MOUSEWHEEL&&g_open){POINT pos={(short)(l&0xffff),(short)((l>>16)&0xffff)};
  if(ScreenToClient(hwnd,&pos)){int mx=pos.x,my=pos.y;map_mouse(hwnd,&mx,&my);
   if(inside(mx,my)){
    int delta=((short)((w>>16)&0xffff))>0?1:-1;
    if(g_tab&&mx<g_panel_x+LIST_WIDTH){int max=g_conversation_count>list_lines()?g_conversation_count-list_lines():0;g_list_scroll-=delta;if(g_list_scroll<0)g_list_scroll=0;if(g_list_scroll>max)g_list_scroll=max;}
    else{int max=active_rows()>visible_lines()?active_rows()-visible_lines():0;int*scroll=active_scroll();*scroll+=delta;if(*scroll<0)*scroll=0;if(*scroll>max)*scroll=max;}
    return 1;
   }
  }
 }
 return 0;
}
static LONG WINAPI game_proc(HWND hwnd,UINT m,SIZE_T w,LONG l){
 if(chat_message(hwnd,m,w,l))return 0;
 return g_prevproc?CallWindowProcW(g_prevproc,hwnd,m,w,l):0;
}
static LONG WINAPI chat_queue_hook(int code,SIZE_T removed,LONG param){
 if(code>=0&&removed==1&&param){ /* Only PM_REMOVE: never mutate peeked messages. */
  CHATMSG *msg=(CHATMSG*)(SIZE_T)param;
  if(msg->hwnd==g_game){
   int translate=g_open&&(g_settings?g_name_edit:g_edit)&&(msg->message==WM_KEYDOWN||msg->message==0x0104)&&msg->wParam!=VK_RETURN&&msg->wParam!=VK_ESCAPE&&msg->wParam!=VK_F12;
   if(chat_message(msg->hwnd,msg->message,msg->wParam,msg->lParam)){
    /* The game's TranslateMessage will see WM_NULL. Translate here once to
     * obtain WM_CHAR in the real keyboard layout before suppressing KEYDOWN. */
    if(translate)TranslateMessage(msg);
    msg->hwnd=0;msg->message=0;msg->wParam=0;msg->lParam=0;
   }
  }
 }
 return CallNextHookEx(g_message_hook,code,removed,param);
}
static void ensure_queue_hook(void){
 if(g_message_hook||!g_game)return;
 DWORD pid=0;DWORD tid=GetWindowThreadProcessId(g_game,&pid);
 if(!tid||pid!=GetCurrentProcessId())return;
 HMODULE module=0;
 if(!GetModuleHandleExW(6,(LPCWSTR)(SIZE_T)chat_queue_hook,&module))return;
 g_message_hook=SetWindowsHookExW(3,chat_queue_hook,module,tid);
 if(g_message_hook){g_input_thread=tid;overlay_log("GAME_QUEUE_INPUT_HOOKED: WH_GETMESSAGE, Enter-only typing, game keys suppressed");}
 else overlay_log("ERROR: game-thread message hook installation failed");
}
#include "compat_profiles.h"
static DWORD exe_base,g_device_global_rva;
static int g_profile_index=-1;
static volatile LONG g_pilot_ready;
static DWORD g_session_player;
/* Each site is a full EndScene + four zero arguments + Present pair.
 * All three paths must use the same device global. Absolute operands are
 * compared against the actual load address, so PE relocations are supported. */
static int render_signature_ok(const BYTE*p,DWORD global){
 static const BYTE tail[]={0x50,0x8b,0x00,0xff,0x90,0xa8,0,0,0,0x6a,0,0x6a,0,0x6a,0,0x6a,0,0xa1};
 if(p[0]!=0xa1||*(const DWORD*)(p+1)!=global)return 0;
 for(int i=0;i<18;i++)if(p[5+i]!=tail[i])return 0;
 if(*(const DWORD*)(p+23)!=global)return 0;
 static const BYTE present[]={0x50,0x8b,0x00,0xff,0x50,0x44};
 for(int i=0;i<6;i++)if(p[27+i]!=present[i])return 0;
 return 1;
}
static int pilot_signature_ok(const BYTE*b,const FHDGameProfile*p){
 const BYTE*g=b+p->player_getter;DWORD global=(DWORD)(SIZE_T)b+p->player_slot,vmt=(DWORD)(SIZE_T)b+p->player_vmt;
 if(g[0]!=0xa1||*(const DWORD*)(g+1)!=global||g[5]!=0x35||*(const DWORD*)(g+6)!=0xb1cd15d3u)return 0;
 if(*(const DWORD*)(b+p->player_vmt-76)!=vmt||*(const DWORD*)(b+p->player_vmt-40)!=0xdf0)return 0;
 if(*(const DWORD*)(b+p->player_vmt-44)!=(DWORD)(SIZE_T)b+p->player_class_name)return 0;
 static const BYTE name[]={7,'T','P','l','a','y','e','r'};for(int i=0;i<8;i++)if(b[p->player_class_name+i]!=name[i])return 0;
 return 1;
}
static int is_supported_exe(void){
 g_session_player=0;g_pilot_ready=0;exe_base=0;g_device_global_rva=0;g_profile_index=-1;
 BYTE*b=(BYTE*)GetModuleHandleW(0);if(!b)return 0;
 if(b[0]!='M'||b[1]!='Z')return 0;
 DWORD nt=*(DWORD*)(b+0x3c);if(nt>0x1000||nt<0x40)return 0;
 BYTE*h=b+nt;if(h[0]!='P'||h[1]!='E'||h[2]||h[3])return 0;
 if(*(WORD*)(h+4)!=0x14c||*(WORD*)(h+20)<96||*(WORD*)(h+24)!=0x10b)return 0;
 DWORD stamp=*(DWORD*)(h+8),size=*(DWORD*)(h+24+56);
 for(int i=0;i<(int)(sizeof(g_profiles)/sizeof(g_profiles[0]));i++){
  const FHDGameProfile*p=&g_profiles[i];if(stamp!=p->timestamp||size!=p->image_size)continue;
  DWORD global=(DWORD)(SIZE_T)b+p->device_rva;
  for(int j=0;j<3;j++){if(p->render_sites[j]>size-33||!render_signature_ok(b+p->render_sites[j],global))return 0;}
  exe_base=(DWORD)(SIZE_T)b;g_device_global_rva=p->device_rva;g_profile_index=i;g_pilot_ready=pilot_signature_ok(b,p);return 1;
 }
 return 0;
}
static void* real_device(void){return g_device_global_rva?*(void**)(exe_base+g_device_global_rva):0;}
/* Read only this process, never call a Delphi method from a worker thread.
 * Bad/transient pointers fail through ReadProcessMemory, not direct dereference. */
static int pilot_read(DWORD address,void*out,DWORD count){
 SIZE_T got=0;if(address<0x10000u||count>128u||address>0xffffffffu-count)return 0;
 return ReadProcessMemory(GetCurrentProcess(),(const void*)(SIZE_T)address,out,count,&got)&&got==count;
}
/* The verified module global is permanent; its nil value is updated by the
 * game's SetPlayer during startup, campaign destruction and replacement.
 * Only a changed non-nil object needs one bounded class-pointer read. */
static int game_session_active(void){
#ifdef FHD_TEST
 if(g_session_override>=0)return g_session_override;
#endif
 if(!exe_base||g_profile_index<0)return 0;
 const FHDGameProfile*p=&g_profiles[g_profile_index];DWORD encoded=*(volatile DWORD*)(exe_base+p->player_slot),player=encoded^0xb1cd15d3u;
 if(!player){g_session_player=0;return 0;}
 if(player==g_session_player)return 1;
 DWORD vmt=0;if((player&3)||!pilot_read(player,&vmt,4)||vmt!=exe_base+p->player_vmt)return 0;
 g_session_player=player;return 1;
}
int WINAPI FHDGame_ReadPilotNameW(WCHAR*out,int cap){
 if(!out||cap<2)return 0;out[0]=0;if(!g_pilot_ready||g_profile_index<0)return -1;
 const FHDGameProfile*p=&g_profiles[g_profile_index];DWORD base=exe_base,encoded,again,head[3],bytes,check_name,check_bytes;WCHAR name[25];
 if(!pilot_read(base+p->player_slot,&encoded,4))return 0;
 DWORD player=encoded^0xb1cd15d3u;if(!player||(player&3)||!pilot_read(player,head,12)||head[0]!=base+p->player_vmt)return 0;
 DWORD str=head[2];if((str&1)||!pilot_read(str-4,&bytes,4)||bytes<2||bytes>48||(bytes&1)||bytes/2>=(DWORD)cap)return 0;
 if(!pilot_read(str,name,bytes+2)||name[bytes/2]!=0)return 0;
 for(DWORD i=0;i<bytes/2;i++)if(name[i]<32||name[i]==127)return 0;
 if(!pilot_read(base+p->player_slot,&again,4)||again!=encoded||!pilot_read(player+8,&check_name,4)||check_name!=str||!pilot_read(str-4,&check_bytes,4)||check_bytes!=bytes)return 0;
 wcopy(out,cap,name);return 1;
}
static void diagnostics(void){
 DWORD now=GetTickCount();if(now-g_last_stats<10000)return;
 g_last_stats=now;
 char line[140];
 /* No CRT sprintf: diagnostic only needs ASCII decimal counters. */
 char *p=line;const char *head="FRAME_STATS: EndScene=";
 while(*head)*p++=*head++;
 DWORD x=(DWORD)g_endscene_calls;char num[16];int n=0;
 do{num[n++]=(char)('0'+x%10);x/=10;}while(x&&n<15);
 while(n--)*p++=num[n];
 head=" Present=";while(*head)*p++=*head++;
 x=(DWORD)g_present_calls;n=0;do{num[n++]=(char)('0'+x%10);x/=10;}while(x&&n<15);
 while(n--)*p++=num[n];*p=0;overlay_log(line);
}
static HRESULT WINAPI hook_end_scene(void* dev){
 g_endscene_calls++;
 if(g_ready && dev==g_realdevice){
  if(!g_frame_logged){g_frame_logged=1;overlay_log("GAME_FRAME_CONFIRMED: actual Rangers.exe EndScene intercepted");}
  draw_chat(dev);
 }
 return callEnd(dev);
}
static HRESULT WINAPI hook_present(void* dev,const RECT* src,const RECT* dest,HWND override,const void* dirty){
 g_present_calls++;
 return g_original_present?g_original_present(dev,src,dest,override,dirty):0;
}
static int patch_ptr(void** addr,void* fn,void** original){
 DWORD oldProtect=0,unused=0;
 if(!VirtualProtect(addr,sizeof(void*),PAGE_READWRITE,&oldProtect))return 0;
 /* x86 aligned 32-bit writes are atomic; bridge uses InterlockedExchange. */
 *original=*addr;
 *addr=fn;
 VirtualProtect(addr,sizeof(void*),oldProtect,&unused);
 return 1;
}
static int hook_device(void* dev){
 void **vt=*(void***)dev;
 if(!vt)return 0;
 if(vt==g_hookvtable){g_realdevice=dev;return 1;}
 /* Do not patch unknown COM methods or hook a second vtable if not needed. */
 if(vt[42]==(void*)hook_end_scene&&vt[17]==(void*)hook_present){g_realdevice=dev;g_ready=1;return 1;}
 if(!vt[42]||!vt[17])return 0;
 /* Set old targets before patch. Hook Present first, EndScene last.
  * Do not release g_realdevice: the game owns it. */
 g_original_end=(HRESULT(WINAPI*)(void*))vt[42];
 g_original_present=(HRESULT(WINAPI*)(void*,const RECT*,const RECT*,HWND,const void*))vt[17];
 void* prev;
 if(!patch_ptr(&vt[17],(void*)hook_present,&prev))return 0;
 if(!patch_ptr(&vt[42],(void*)hook_end_scene,&prev)){
  void* unused;patch_ptr(&vt[17],(void*)g_original_present,&unused);return 0;
 }
 g_hookvtable=vt;g_realdevice=dev;g_ready=1;
 overlay_log("REAL_DEVICE_HOOK_INSTALLED: Rangers.exe global, EndScene[42], Present[17]");
 return 1;
}
static DWORD WINAPI hook_thread(LPVOID param){(void)param;
 if(!is_supported_exe()){
  overlay_log("ERROR_UNSUPPORTED_EXE: no matching verified game profile; chat hook disabled");return 0;
 }
 overlay_log(g_profiles[g_profile_index].log);
 if(!g_pilot_ready)overlay_log("PILOT_READER_UNAVAILABLE: using manual/current chat nickname; graphics stays enabled");
 DWORD lastWait=0;int installed=0;
 for(;;){
  void* dev=real_device();
  if(dev && (!g_ready||dev!=g_realdevice)){
   if(hook_device(dev)){
    installed=1;g_frame_logged=0;
    /* Ask the native game window for messages: D3D9 focus window is canonical. */
    if(!g_game){D3DCREATION_PARAMETERS p;HRESULT(WINAPI *GetParams)(void*,D3DCREATION_PARAMETERS*)=VT(dev,9,void*);
     if(GetParams(dev,&p)>=0&&p.hFocusWindow){DWORD pid=0;GetWindowThreadProcessId(p.hFocusWindow,&pid);
      if(pid==GetCurrentProcessId()){
       g_game=p.hFocusWindow;g_prevproc=(WNDPROC)(SIZE_T)GetWindowLongW(g_game,GWL_WNDPROC);
       if(g_prevproc){LONG previous=SetWindowLongW(g_game,GWL_WNDPROC,(LONG)(SIZE_T)game_proc);
        if(!previous){g_game=0;g_prevproc=0;overlay_log("WARN: game keyboard hook not installed");}
        else overlay_log("GAME_WINDOW_INPUT_HOOKED: F12, Enter, mouse");
       }
      }
     }
    }
   }else if(GetTickCount()-lastWait>5000){lastWait=GetTickCount();overlay_log("WAIT: found real Direct3DDevice but unable to patch vtable");}
  }else if(!dev&&GetTickCount()-lastWait>5000){lastWait=GetTickCount();overlay_log("WAIT: Direct3DDevice not created yet");}
  if(installed){ensure_queue_hook();diagnostics();}
  Sleep(250);
 }
}
int overlay_bootstrap(void){HANDLE t=CreateThread(0,0,hook_thread,0,0,0);if(!t)return 0;CloseHandle(t);return 1;}

void *memset(void*ptr,int val,SIZE_T count){unsigned char* p=(unsigned char*)ptr;for(SIZE_T i=0;i<count;i++)p[i]=(unsigned char)val;return ptr;}

#ifdef FHD_TEST
__declspec(dllexport) void WINAPI Test_SessionOverride(int value){g_session_override=value;}
__declspec(dllexport) int WINAPI Test_SessionActive(void){return game_session_active();}
__declspec(dllexport) int WINAPI Test_ReadPilot(WCHAR*out,int cap){return FHDGame_ReadPilotNameW(out,cap);}
__declspec(dllexport) int WINAPI Test_PilotReady(void){return g_pilot_ready;}
__declspec(dllexport) int WINAPI Test_SupportedExe(void){return is_supported_exe();}
__declspec(dllexport) DWORD WINAPI Test_DeviceRva(void){return g_device_global_rva;}
__declspec(dllexport) int WINAPI Test_Profile(void){return g_profile_index+1;}
__declspec(dllexport) void* WINAPI Test_RealDevice(void){return real_device();}
__declspec(dllexport) int WINAPI Test_HookRealDevice(void){void*d=real_device();return d?hook_device(d):0;}
__declspec(dllexport) void WINAPI Test_Draw(void* dev){draw_chat(dev);}
__declspec(dllexport) int WINAPI Test_Message(HWND h,UINT m,SIZE_T w,LONG l){return chat_message(h,m,w,l);}
__declspec(dllexport) LONG WINAPI Test_Queue(int c,SIZE_T r,CHATMSG* m){return chat_queue_hook(c,r,(LONG)(SIZE_T)m);}
__declspec(dllexport) void WINAPI Test_SetGame(HWND h){g_game=h;}
__declspec(dllexport) int WINAPI Test_Editing(void){return g_edit;}
__declspec(dllexport) int WINAPI Test_Open(void){return g_open;}
__declspec(dllexport) int WINAPI Test_Length(void){return g_input_len;}
__declspec(dllexport) const WCHAR* WINAPI Test_Text(void){return g_input;}
__declspec(dllexport) void WINAPI Test_Bounds(int* b){b[0]=g_panel_x;b[1]=g_panel_y;b[2]=PANEL_WIDTH;b[3]=PANEL_HEIGHT;}
__declspec(dllexport) void WINAPI Test_InstallInput(void){ensure_queue_hook();}
__declspec(dllexport) int WINAPI Test_Tab(void){return g_tab;}
__declspec(dllexport) int WINAPI Test_HistoryCount(int tab){if(!tab)return g_count_public;int n=0;for(int i=0;i<g_conversation_count;i++)n+=g_conversations[i].count;return n;}
__declspec(dllexport) const char* WINAPI Test_Target(void){return g_peer_id;}
__declspec(dllexport) void WINAPI Test_Pump(void){incoming();}
__declspec(dllexport) void WINAPI Test_CheckDay(void){g_day_checked=GetTickCount()-30000;check_private_day();}
__declspec(dllexport) int WINAPI Test_ConversationCount(void){return g_conversation_count;}
__declspec(dllexport) int WINAPI Test_ConversationMessages(const char*id){int i=conversation_slot(id,0,0);return i<0?0:g_conversations[i].count;}
__declspec(dllexport) int WINAPI Test_ConversationUnread(const char*id){int i=conversation_slot(id,0,0);return i<0?0:g_conversations[i].unread;}
__declspec(dllexport) const WCHAR* WINAPI Test_ConversationText(const char*id,int row){int i=conversation_slot(id,0,0);return i<0||row<0||row>=g_conversations[i].count?L"":g_conversations[i].history[row].text;}
__declspec(dllexport) DWORD WINAPI Test_Color(const WCHAR*name,const char*id){return nickname_color(name,id,0);}
__declspec(dllexport) int WINAPI Test_Choose(const char*id,const WCHAR*name){return choose_peer(id,name);}
__declspec(dllexport) int WINAPI Test_ActiveCount(void){return active_count();}
__declspec(dllexport) int WINAPI Test_ActiveRows(void){return active_rows();}
__declspec(dllexport) const WCHAR* WINAPI Test_Notice(void){return g_notice;}
__declspec(dllexport) int WINAPI Test_Notifications(void){return g_notifications;}
__declspec(dllexport) int WINAPI Test_PendingNotifications(void){return g_pending_notifications;}
__declspec(dllexport) int WINAPI Test_WindowLocked(void){return g_window_locked;}
__declspec(dllexport) int WINAPI Test_Settings(void){return g_settings;}
__declspec(dllexport) int WINAPI Test_NameEditing(void){return g_name_edit;}
__declspec(dllexport) int WINAPI Test_Opacity(void){return g_opacity;}
__declspec(dllexport) void WINAPI Test_ReloadPreferences(void){g_preferences_loaded=0;ensure_preferences();g_settings=0;apply_geometry();}
__declspec(dllexport) const WCHAR* WINAPI Test_SettingsNotice(void){return g_settings_notice;}

#endif
