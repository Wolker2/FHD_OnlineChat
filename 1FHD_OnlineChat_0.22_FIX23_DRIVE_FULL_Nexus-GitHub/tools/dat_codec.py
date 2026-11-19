#!/usr/bin/env python3
"""Lossless SRHD BlockPar/CacheData DAT <-> editable JSON (Python stdlib only).

Reimplementation based on the publicly documented rangers-utils format;
Sergei Osminin's MIT-licensed rangers-utils is credited in docs.
No third-party dependencies. Decode->encode is byte-identical for the
three supplied original DevKit v18 DAT files on CPython 3.13/zlib 1.2.13.
"""
import argparse
import json
import struct
import zlib
from pathlib import Path

KEYS = {'blockpar':bytes.fromhex('89 c6 e8 b1'), 'cachedata':bytes.fromhex('37 3f 8f ea')}


def cipher(b:bytes,seed:int)->bytes:
    # Park-Miller minimal-standard PRNG; same symmetric cipher as game format
    seed &= 0xffffffff
    if seed >= 0x80000000: seed-=0x100000000
    out=bytearray(b)
    for i in range(len(out)):
        hi, lo=divmod(seed,0x1f31d)
        seed=lo*0x41a7-hi*0xb14
        if seed < 1: seed += 0x7fffffff
        out[i] ^= (seed-1)&255
    return bytes(out)

class Reader:
    def __init__(self,data): self.data=data;self.p=0
    def get(self,n):
        if n<0 or self.p+n>len(self.data):raise ValueError('Unexpected binary EOF')
        a=self.data[self.p:self.p+n];self.p+=n;return a
    def u8(self):return self.get(1)[0]
    def u32(self):return struct.unpack('<I',self.get(4))[0]
    def wide(self):
        start=self.p
        while self.p+1<len(self.data):
            x=self.get(2)
            if x==b'\x00\x00':return self.data[start:self.p-2].decode('utf-16le')
        raise ValueError('Unterminated UTF16')
    def block(self,fmt,depth=0):
        if depth>128:raise ValueError('Too much nesting')
        ordered= fmt=='blockpar'
        sorted_flag=bool(self.u8()) if ordered else None
        count=self.u32()
        if count>200000:raise ValueError('Unreasonable entry count')
        entries=[]
        for _ in range(count):
            meta=[self.u32(),self.u32()] if ordered and sorted_flag else None
            kind=self.u8()
            if kind not in (1,2):raise ValueError('Unknown record type: '+str(kind))
            name=self.wide()
            if kind==1: item={'kind':'parameter','name':name,'value':self.wide()}
            else: item={'kind':'block','name':name,'value':self.block(fmt,depth+1)}
            if meta is not None:item['sort_meta']=meta
            entries.append(item)
        b={'entries':entries}
        if ordered:b['sorted']=sorted_flag
        return b


def decode(data:bytes,fmt:str):
    key=KEYS[fmt]
    if len(data)<16:raise ValueError('Short DAT')
    digest=struct.unpack_from('<I',data)[0]
    seed=int.from_bytes(bytes(a^b for a,b in zip(data[4:8],key)),'little',signed=True)
    zipped=cipher(data[8:],seed)
    if zlib.crc32(zipped)!=digest:raise ValueError('DAT CRC mismatch / wrong format')
    if zipped[:4]!=b'ZL01':raise ValueError('Unsupported DAT compression')
    uncompressed_length=struct.unpack_from('<I',zipped,4)[0]
    plain=zlib.decompress(zipped[8:])
    if len(plain)!=uncompressed_length:raise ValueError('DAT decompressed length mismatch')
    reader=Reader(plain)
    root=reader.block(fmt)
    if reader.p!=len(plain):raise ValueError('DAT unconsumed data: '+str(len(plain)-reader.p))
    return {'format':fmt,'seed':seed,'root':root}


def writewide(out:bytearray,s:str):
    if '\x00' in s:raise ValueError('Null character in source string')
    out.extend(s.encode('utf-16le'));out.extend(b'\x00\x00')


def writeblock(out:bytearray,block:dict,fmt:str):
    sorted_flag=bool(block.get('sorted')) if fmt=='blockpar' else False
    if fmt=='blockpar':out.append(int(sorted_flag))
    entries=block['entries']
    out.extend(struct.pack('<I',len(entries)))
    for entry in entries:
        if sorted_flag:
            if 'sort_meta' in entry:
                index,count=entry['sort_meta']
            else:
                # For inserted entries; unchanged exports preserve original metadata.
                names=[e['name'] for e in entries]
                i=entries.index(entry)
                index=0
                while i-index>0 and names[i-index-1]==entry['name']:index+=1
                count=names.count(entry['name']) if index==0 else 0
            out.extend(struct.pack('<II',index,count))
        kind=entry['kind']
        out.append({'parameter':1,'block':2}[kind]);writewide(out,entry['name'])
        if kind=='parameter':writewide(out,entry['value'])
        else:writeblock(out,entry['value'],fmt)


def encode(project:dict)->bytes:
    fmt=project['format'];seed=project['seed'];key=KEYS[fmt]
    plain=bytearray();writeblock(plain,project['root'],fmt)
    zipped=b'ZL01'+struct.pack('<I',len(plain))+zlib.compress(plain,9)
    digest=struct.pack('<I',zlib.crc32(zipped))
    seedbits=(int(seed)&0xffffffff).to_bytes(4,'little')
    seed_key=bytes(a^b for a,b in zip(seedbits,key))
    return digest+seed_key+cipher(zipped,seed)


def pretty_block(b,depth=0):
    out=[];space='    '*depth
    for entry in b['entries']:
        if entry['kind']=='block':
            flag=' ^' if entry['value'].get('sorted') else ' ~' if 'sorted' in entry['value'] else ''
            out.append(f"{space}{entry['name']}{flag}{{")
            out.extend(pretty_block(entry['value'],depth+1));out.append(space+'}')
        else:
            value=entry['value']
            if '\n' in value or '\r' in value:
                out.append(f"{space}{entry['name']}=<<<")
                out.extend(space+line for line in value.splitlines())
                out.append(space+'>>>')
            else:out.append(f"{space}{entry['name']}={value}")
    return out


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('command',choices=['decode','encode','verify'])
    p.add_argument('src',type=Path)
    p.add_argument('dst',nargs='?',type=Path)
    p.add_argument('--format',choices=KEYS)
    a=p.parse_args()
    if a.command=='decode':
        if not a.format or not a.dst:p.error('decode requires --format and dst')
        project=decode(a.src.read_bytes(),a.format)
        a.dst.parent.mkdir(parents=True,exist_ok=True)
        a.dst.write_text(json.dumps(project,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
        print('Decoded',a.src,'->',a.dst,'entries',len(project['root']['entries']))
    else:
        project=json.loads(a.src.read_text(encoding='utf-8'))
        result=encode(project)
        if a.command=='verify':
            if not a.dst:p.error('verify requires comparison DAT dst')
            ref=a.dst.read_bytes()
            print('MATCH' if result==ref else 'DIFFERENT',len(result),len(ref))
            if result!=ref:raise SystemExit(1)
        else:
            if not a.dst:p.error('encode requires output DAT dst')
            a.dst.parent.mkdir(parents=True,exist_ok=True);a.dst.write_bytes(result)
            print('Encoded',a.src,'->',a.dst,len(result))

if __name__=='__main__':main()
