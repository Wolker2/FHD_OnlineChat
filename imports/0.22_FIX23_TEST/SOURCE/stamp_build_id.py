#!/usr/bin/env python3
"""FHD OnlineChat: stamp an x86 client build tag without rebuilding its DLL.
Run only when packaging a release; never from the installed game.
Usage: python stamp_release.py INPUT.zip OUTPUT.zip [server.js]
"""
from pathlib import Path
from zipfile import ZipFile
from hashlib import sha256
import sys,re
DLL_NAME='Mods/OtherMods/FHD_OnlineChat/Native/FHDOnlineChat.XenoPlugin.dll'
SERVER_NAME='Mods/OtherMods/FHD_OnlineChat/SOURCE/server.js'
WIDE_PREFIX='build='.encode('utf-16le')
TOKEN_REGEX=re.compile(r'^(?:0\.22-fixed-native021|FHD1[0-9a-fA-F]{16})$')

def make_tagged_archive(input_zip,output_zip,server_js=None):
    with ZipFile(input_zip) as z:
        items={x.filename:z.read(x.filename) for x in z.infolist() if not x.is_dir()}
        meta={x.filename:x for x in z.infolist() if not x.is_dir()}
    assert DLL_NAME in items and SERVER_NAME in items
    data=items[DLL_NAME]
    positions=[m.start() for m in re.finditer(re.escape(WIDE_PREFIX),data)]
    assert len(positions)==1, 'must have exactly one Unicode build marker in DLL'
    pos=positions[0]+len(WIDE_PREFIX)
    original=data[pos:pos+40].decode('utf-16le')
    assert TOKEN_REGEX.fullmatch(original),f'unknown client marker: {original!r}'
    normalized=data[:pos]+('0'*20).encode('utf-16le')+data[pos+40:]
    h=sha256()
    # Tag represents the installed mod, not the server source or documentation.
    for name in sorted(items):
        if not name.startswith('Mods/OtherMods/FHD_OnlineChat/') or '/SOURCE/' in name:
            continue
        if not any(name.endswith(x) for x in ('ModuleInfo.txt','.dll','.ini','.gi','.png','.dat','.cfg','.pkg')):
            continue
        payload=normalized if name==DLL_NAME else items[name]
        h.update(name.encode());h.update(len(payload).to_bytes(8,'little'));h.update(payload)
    tag='FHD1'+h.hexdigest()[:16]
    changed=data[:pos]+tag.encode('utf-16le')+data[pos+40:]
    assert len(changed)==len(data)
    assert changed[:pos]==data[:pos] and changed[pos+40:]==data[pos+40:]
    assert sum(a!=b for a,b in zip(changed,data))<=40
    items[DLL_NAME]=changed
    if server_js is not None:
        items[SERVER_NAME]=Path(server_js).read_bytes()
    info=('FHD OnlineChat Build ID\n'
          f'BuildID={tag}\n'
          'The only change to the client DLL is the 20-character UTF-16LE\n'
          'build= string used in its WebSocket connection URL.\n'
          'Original executable code, UI, editor and networking are unchanged.\n'
          'This file is a packaging receipt, not a runtime dependency.\n')
    items['BUILD_ID_INFO.txt']=info.encode('utf-8')
    with ZipFile(output_zip,'w') as out:
        for name,d in items.items():
            if name in meta:out.writestr(meta[name],d)
            else:out.writestr(name,d)
    with ZipFile(output_zip) as o:
        assert o.testzip() is None
        new=o.read(DLL_NAME)
        assert new[pos:pos+40]==tag.encode('utf-16le')
    return original,tag, sha256(data).hexdigest(),sha256(changed).hexdigest()

if __name__=='__main__':
    if len(sys.argv)<3:raise SystemExit(__doc__)
    old,tag,before,after=make_tagged_archive(sys.argv[1],sys.argv[2],sys.argv[3] if len(sys.argv)>3 else None)
    print('STAMPED',tag,'from',old)
    print('DLL hashes:',before[:12],'->',after[:12])
