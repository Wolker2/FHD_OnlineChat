#!/usr/bin/env python3
"""Strict x86 PE patch against FHD 0.22 BuildID DLL only. No recompile.
Patch two existing mouse-message compare instructions with calls to code in an
extra read+execute PE section. The patch is fully reversible via the source ZIP.
"""
import struct,sys,hashlib,pathlib,subprocess
from zipfile import ZipFile

BASE_DLL='Mods/OtherMods/FHD_OnlineChat/Native/FHDOnlineChat.XenoPlugin.dll'
SOURCE=pathlib.Path('/mnt/data/FHD_OnlineChat_0.22_BUILDID_CLICK_TEST.zip')
EXPECTED_HASH='b9c10f02bfa4c85d25f223d8be40eb9d84940b568cedc034d524ca6a5e9b8cc5'
SECTION_RVA=0x628000
SOURCE_CODE=pathlib.Path('/mnt/data/link_action_work/patch_link.bin').read_bytes()
SEC_NAME=b'.fhdlnk\0'
OBJP=pathlib.Path('/mnt/data/link_action_work/patch_link.o')
NM=subprocess.check_output(['nm','-a',str(OBJP)],text=True)
HANDLER_LEFT_OFFSET=int(next(line.split()[0] for line in NM.splitlines() if line.split()[-1]=='handler_left'),16)
SRC_WIDE='https://drive.google.com/file/d/1MmcxyeAnziTpLhv54isczUvIrns0ygKF/view'.encode('utf-16le')+b'\0\0'


def align(v,n):return (v+n-1)//n*n

def patch(b):
    assert hashlib.sha256(b).hexdigest()==EXPECTED_HASH, 'Refusing to touch an unknown DLL build'
    result=bytearray(b)
    pe=struct.unpack_from('<I',result,0x3c)[0]
    assert result[pe:pe+4]==b'PE\0\0'
    coff=pe+4
    num=struct.unpack_from('<H',result,coff+2)[0]
    assert num==4
    oh=coff+20
    optional_size=struct.unpack_from('<H',result,coff+16)[0]
    assert struct.unpack_from('<H',result,oh)[0]==0x10b
    imagebase=struct.unpack_from('<I',result,oh+28)[0]
    assert imagebase==0x10000000
    sect_align,file_align=struct.unpack_from('<II',result,oh+32)
    assert sect_align==0x1000 and file_align==0x200
    sec_header=oh+optional_size
    assert result[sec_header:sec_header+8]==b'.text\0\0\0'
    text_size,text_rva,raw_len,raw_offset=struct.unpack_from('<IIII',result,sec_header+8)
    assert (text_size,text_rva,raw_len,raw_offset)==(0x13d88,0x1000,0x13e00,0x400)
    assert result[sec_header+3*40:sec_header+3*40+6]==b'.reloc'
    assert result[sec_header+4*40:sec_header+5*40]==bytes(40), 'No section header space'
    url_file_offset=0x14200+(0x1e5d0-0x15000)
    assert result[url_file_offset:url_file_offset+len(SRC_WIDE)]==SRC_WIDE
    assert len(SOURCE_CODE)<0x200
    assert len(result)==0x20400
    assert struct.unpack_from('<I',result,oh+56)[0]==0x628000

    # Site 1: RMB: original cmp [esi+4e0],0, flags needed on non-link path.
    # Site 2: LMB: original cmp [eax+4e8],5, flags needed on non-link path.
    hooks=[(0x11df4,b'\x83\xbe\xe0\x04\x00\x00\x00',SECTION_RVA),
           (0x11f4a,b'\x83\xb8\xe8\x04\x00\x00\x05',SECTION_RVA+HANDLER_LEFT_OFFSET)]
    for site,prior,dest in hooks:
        off=raw_offset+site-text_rva
        assert result[off:off+len(prior)]==prior,(hex(site),result[off:off+7].hex())
        call=b'\xe8'+struct.pack('<i',dest-(site+5))
        result[off:off+7]=call+b'\x90\x90'

    # Append fifth section (no changes to original sections' addresses or bytes).
    countoff=coff+2
    struct.pack_into('<H',result,countoff,5)
    section_size=align(len(SOURCE_CODE),file_align)
    new_hdr=sec_header+num*40
    struct.pack_into('<8sIIIIIIHHI',result,new_hdr,SEC_NAME,len(SOURCE_CODE),SECTION_RVA,section_size,len(b),0,0,0,0,0x60000020)
    struct.pack_into('<I',result,oh+56,align(SECTION_RVA+len(SOURCE_CODE),sect_align))
    code_size=struct.unpack_from('<I',result,oh+4)[0]
    struct.pack_into('<I',result,oh+4,code_size+section_size)
    struct.pack_into('<I',result,oh+64,0)  # clear PE checksum, not signed
    result.extend(SOURCE_CODE + bytes(section_size-len(SOURCE_CODE)))
    # Postconditions and side-effect budget: all original bytes unchanged except
    # two 7-byte hook sites, section-count, header, SizeOfCode/Image and checksum.
    assert result[url_file_offset:url_file_offset+len(SRC_WIDE)]==SRC_WIDE
    assert struct.unpack_from('<I',result,oh+56)[0]==0x629000
    assert result[new_hdr:new_hdr+8]==SEC_NAME
    assert result[0x400+0x11df4-0x1000]==0xe8
    assert result[0x400+0x11f4a-0x1000]==0xe8
    return bytes(result)

if __name__=='__main__':
    with ZipFile(SOURCE) as z: before=z.read(BASE_DLL)
    after=patch(before)
    out=pathlib.Path('/mnt/data/link_action_work/test_patched.dll')
    out.write_bytes(after)
    print('source sha256',hashlib.sha256(before).hexdigest())
    print('patched sha256',hashlib.sha256(after).hexdigest())
    print('sizes',len(before),len(after),'new code',len(SOURCE_CODE))
    print('patched DLL',out)
