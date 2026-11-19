#ifndef FHD_COMPAT_PROFILES_H
#define FHD_COMPAT_PROFILES_H
/* Exact user-supplied PE32 builds. Validated once, before reading the device.
 * A SHA256 83300344af802bc51e64389c58f047e5afdf195c133048098be3881fae29ed98
 * B SHA256 e43a4b15503c705eebe2d8ce5778c2a2fb090db515a3fd86d319e5c5aa692ebf
 * No disk patch, filename assumption, broad scan or check on each frame. */
typedef struct {DWORD timestamp,image_size,device_rva,render_sites[3],player_slot,player_getter,player_vmt,player_class_name;const char*log;} FHDGameProfile;
static const FHDGameProfile g_profiles[]={
 {0x68ECCC46u,0x004D1000u,0x0047A798u,{0x000C530Du,0x000C5402u,0x000C560Du},0x0047C19Cu,0x0038C07Cu,0x0038BE5Cu,0x0038BF52u,"EXE_PROFILE_OK: A, SHA256 83300344, real D3D9 device verified"},
 {0x6AC53B97u,0x00461000u,0x0041302Cu,{0x003CAF19u,0x003CB00Eu,0x003CB1EFu},0x004100FCu,0x00347300u,0x00347118u,0x0034720Eu,"EXE_PROFILE_OK: B, SHA256 e43a4b15, real D3D9 device verified"}
};
#endif
