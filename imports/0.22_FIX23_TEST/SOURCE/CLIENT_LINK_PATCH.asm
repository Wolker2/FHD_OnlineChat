.intel_syntax noprefix
.section .text
.global _start
.set SECTION_RVA, 0x628000
.set URL_RVA, 0x1e5d0
.set OPEN_RVA, 0x1e6a0
.set IAT_CLIP_OPEN, 0x1f374
.set IAT_CLIP_CLOSE, 0x1f358
.set IAT_CLIP_EMPTY, 0x1f35c
.set IAT_CLIP_SET, 0x1f384
.set IAT_GLOBAL_ALLOC, 0x1f30c
.set IAT_GLOBAL_FREE, 0x1f310
.set IAT_GLOBAL_LOCK, 0x1f314
.set IAT_GLOBAL_UNLOCK, 0x1f318
.set IAT_SHELL_EXECUTE, 0x1f398
.set GAME_HWND_RVA, 0x625684
.set RETURN_CONSUMED_RVA, 0x11441
_start:
handler_right:
    cmp dword ptr [esi+0x4e8],5
    je hit_right
    lea edx,[esi+0x48]
    call is_archive_url
    test eax,eax
    jne hit_right
miss_right:
    cmp dword ptr [esi+0x4e0],0
    ret

handler_left:
    cmp dword ptr [eax+0x4e8],5
    je hit_left
    push eax
    lea edx,[eax+0x48]
    call is_archive_url
    test eax,eax
    pop eax
    jne hit_left
miss_left:
    cmp dword ptr [eax+0x4e8],5
    ret

hit_right:
    call clipboard_and_open
    add esp,4  # drop patched call-site return; use native consumed-input epilogue
    .byte 0xe9
    .long RETURN_CONSUMED_RVA - (SECTION_RVA + . - _start + 4)

hit_left:
    call clipboard_and_open
    add esp,4
    .byte 0xe9
    .long RETURN_CONSUMED_RVA - (SECTION_RVA + . - _start + 4)

# Accept ONLY the one fixed archive link. Never hijack arbitrary chat URLs.
# Input edx points at FHDChatEvent.text (UTF-16LE), output eax is bool.
is_archive_url:
    push ebx
    push esi
    push edi
    push ecx
    call 2f
2:
    pop ebx
    sub ebx, SECTION_RVA + (2b - _start)
    mov esi,edx
    lea edi,[ebx + URL_RVA]
    mov ecx,71         # 70 characters plus U+0000
    cld
    repe cmpsw
    sete al
    movzx eax,al
    pop ecx
    pop edi
    pop esi
    pop ebx
    ret

clipboard_and_open:
    pushfd
    pushad
    call 1f
1:
    pop ebx
    sub ebx, SECTION_RVA + (1b - _start)
    mov eax, dword ptr [ebx + GAME_HWND_RVA]
    push eax
    call dword ptr [ebx + IAT_CLIP_OPEN]
    test eax, eax
    jz open_browser
    push 142
    push 2
    call dword ptr [ebx + IAT_GLOBAL_ALLOC]
    test eax, eax
    jz close_clipboard
    mov ebp,eax
    push ebp
    call dword ptr [ebx + IAT_GLOBAL_LOCK]
    test eax,eax
    jz free_allocation
    mov edi,eax
    lea esi,[ebx + URL_RVA]
    mov ecx,142
    cld
    rep movsb
    push ebp
    call dword ptr [ebx + IAT_GLOBAL_UNLOCK]
    call dword ptr [ebx + IAT_CLIP_EMPTY]
    test eax,eax
    jz free_allocation
    push ebp
    push 13
    call dword ptr [ebx + IAT_CLIP_SET]
    test eax,eax
    jnz close_clipboard
free_allocation:
    push ebp
    call dword ptr [ebx + IAT_GLOBAL_FREE]
close_clipboard:
    call dword ptr [ebx + IAT_CLIP_CLOSE]
open_browser:
    push 1
    push 0
    push 0
    lea eax, [ebx + URL_RVA]
    push eax
    lea eax, [ebx + OPEN_RVA]
    push eax
    push 0
    call dword ptr [ebx + IAT_SHELL_EXECUTE]
    popad
    popfd
    ret

