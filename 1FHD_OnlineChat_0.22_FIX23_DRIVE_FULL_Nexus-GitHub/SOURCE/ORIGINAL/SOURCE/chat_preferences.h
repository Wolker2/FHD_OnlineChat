#ifndef FHD_CHAT_PREFERENCES_H
#define FHD_CHAT_PREFERENCES_H
/* Internal x86 ABI: nickname UTF-16 + four signed settings, 80 bytes. */
typedef struct {WCHAR nickname[32];int size,opacity,x,y;} FHDChatPreferences;
#endif
