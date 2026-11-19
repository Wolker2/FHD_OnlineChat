#!/usr/bin/env python3
"""Point patch: make wrapped native update event (event==5) clickable.
Applies only to the verified FHD_OnlineChat 0.22 LINK_COPY_BROWSER_TEST binary.
No recompilation, no changes to editing/UI/network code. Reversible by restoring ZIP.
"""
import hashlib,struct,sys
from pathlib import Path

# The expected hash is injected during verification against the user's ZIP.
EXPECTED_SHA256='22cec940a3ffdb6443d160c74693618d88849d1bb4f9cf2de1a7b447e6edb898'
IMAGEBASE=0x10000000
HOOK_RVA=0x628120
JLE_RVA=0x145dc
CAVE_RVA=0x14619
ORIGINAL_ZERO_RVA=0x14604
RETURN_POINTER_RVA=0x1460b

def make_jmp(from_rva,to_rva):
    return b'\xe9'+struct.pack('<i',to_rva-(from_rva+5))

def patch(data):
    assert hashlib.sha256(data).hexdigest()==EXPECTED_SHA256, 'Unknown DLL: refused binary modification'
    result=bytearray(data)
    pe=struct.unpack_from('<I',result,0x3c)[0]
    assert result[pe:pe+4]==b'PE\0\0'
    coff=pe+4
    nsects=struct.unpack_from('<H',result,coff+2)[0]
    assert nsects==5
    oh=coff+20
    nh=oh+struct.unpack_from('<H',result,coff+16)[0]
    sections={}
    for k in range(nsects):
        off=nh+k*40
        name,vsize,rva,rawsize,rawptr=struct.unpack_from('<8sIIII',result,off)
        sections[name.rstrip(b'\0').decode('ascii')]=(off,vsize,rva,rawsize,rawptr)
    assert '.fhdlnk' in sections and '.text' in sections
    sectOff,size,rva,rawsz,rawptr=sections['.fhdlnk']
    assert rva==0x628000 and size in (281,0x119) and rawsz==0x200
    txoff,_,textrva,_,textraw=sections['.text']
    def fileoff(va):
        if textrva<=va<textrva+0x13e00:return textraw+va-textrva
        if rva<=va<rva+rawsz:return rawptr+va-rva
        raise ValueError(hex(va))
    jle=fileoff(JLE_RVA);cave=fileoff(CAVE_RVA);hook=fileoff(HOOK_RVA)
    assert result[jle:jle+2]==b'\x7e\x26',('Original short jump changed',result[jle:jle+2].hex())
    assert result[cave:cave+7]==b'\xcc'*7, 'Function gap unexpectedly occupied'
    assert result[hook:hook+32]==b'\0'*32, 'FHD link section no longer has reserved padding'
    assert result[fileoff(HOOK_RVA-1)]==0
    # Preserve original for every non-update message. For an update event
    # only, return the same event pointer for its continuation row, which
    # contains the underlined blue word 'СКАЧАТЬ'.
    code=(
        b'\x8b\x04\x24' +             # mov eax,[esp]    ; current FHDChatEvent*
        b'\x85\xc0' +                 # test eax,eax
        b'\x74\x0e' +                 # je non_update
        b'\x83\xb8\xe8\x04\x00\x00\x05' + # cmp dword ptr [eax+0x4e8],5
        b'\x75\x05' +                 # jne non_update
        make_jmp(HOOK_RVA+0x10, RETURN_POINTER_RVA)+
        make_jmp(HOOK_RVA+0x15, ORIGINAL_ZERO_RVA)
    )
    assert len(code)==26 and HOOK_RVA+len(code)<=rva+rawsz
    # Redirect only the branch handling a continuation row (JLE).
    result[jle:jle+2]=b'\x7e'+bytes([CAVE_RVA-(JLE_RVA+2)])
    result[cave:cave+5]=make_jmp(CAVE_RVA,HOOK_RVA)
    result[hook:hook+len(code)]=code
    struct.pack_into('<I',result,sectOff+8, HOOK_RVA-rva+len(code)) # VirtualSize
    struct.pack_into('<I',result,oh+64,0) # PE checksum
    assert result[jle:jle+2]==b'\x7e\x3b'
    assert result[cave:cave+5]==make_jmp(CAVE_RVA,HOOK_RVA)
    assert result[hook:hook+len(code)]==code
    return bytes(result)

if __name__=='__main__':
    if len(sys.argv)!=3: raise SystemExit('usage: patch_update_second_row.py input.dll output.dll')
    before=Path(sys.argv[1]).read_bytes()
    after=patch(before)
    Path(sys.argv[2]).write_bytes(after)
    print('INPUT SHA-256:',hashlib.sha256(before).hexdigest())
    print('OUTPUT SHA-256:',hashlib.sha256(after).hexdigest())
    print('BYTE CHANGES:',sum(a!=b for a,b in zip(before,after)))
