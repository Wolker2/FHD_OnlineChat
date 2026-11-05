#ifndef FHD_CHAT_EVENT_H
#define FHD_CHAT_EVENT_H
/* Internal x86 ABI, shared by the network bridge and in-frame UI.
 * Event 0: live message; 1: history reset; 2: day reset;
 * 3: replayed history; 4: service error (neither triggers auto-open). */
#define FHD_EVENT_HISTORY 3
#define FHD_EVENT_SERVICE 4
typedef struct {
 WCHAR nick[36],text[512],peer[36];
 char peerId[80];
 int own,channel,event;
 DWORD day;
 char senderId[80];
} FHDChatEvent;
#endif
