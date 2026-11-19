@echo off
setlocal
cd /d "%~dp0"
REM Verified: Zig 0.13.0, Windows x86.
zig cc -target x86-windows-gnu -shared -O2 -fno-builtin -s bridge.c overlay.c plugin_zig.def -luser32 -lgdi32 -lwinhttp -lshell32 -o ..\Native\FHDOnlineChat.XenoPlugin.dll
if errorlevel 1 exit /b 1
echo Built FHD OnlineChat 0.21-font-preview1 (EXPERIMENT).
