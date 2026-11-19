@echo off
setlocal
REM Run from "x86 Native Tools Command Prompt for VS" with Windows SDK available.
cd /d "%~dp0"
cl /nologo /c /O2 /GS- /Zl /TC bridge.c overlay.c || exit /b 1
link /nologo /DLL /MACHINE:X86 /NODEFAULTLIB /ENTRY:DllMain /DEF:plugin.def /OUT:..\Native\FHDOnlineChat.XenoPlugin.dll bridge.obj overlay.obj kernel32.lib user32.lib gdi32.lib shell32.lib winhttp.lib || exit /b 1
if %errorlevel% neq 0 exit /b 1
echo Native chat DLL built. Verify exports and in-game loading with XenoNative.log.
