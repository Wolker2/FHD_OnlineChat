#!/usr/bin/env python3
"""Publish byte-exact editable materials from an original FHD install ZIP.
Never rewrites the install archive, compiles native code, or executes bundled scripts.
"""
import argparse
import hashlib
import json
import subprocess
import sys
import tempfile
import zipfile
from collections import defaultdict
from pathlib import Path, PurePosixPath

try:
    import dat_codec
except ImportError:
    dat_codec = None

def sha(data):
    return hashlib.sha256(data).hexdigest()

def put(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        assert path.read_bytes() == data, "Existing source differs: " + str(path)
    else:
        path.write_bytes(data)

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--archive",required=True)
    p.add_argument("--target",required=True)
    p.add_argument("--sha256",required=True)
    p.add_argument("--mod",required=True)
    a=p.parse_args()
    archive=Path(a.archive).resolve()
    target=Path(a.target).resolve()
    assert archive.is_file()
    assert sha(archive.read_bytes()) == a.sha256.lower(), "Original archive differs from expected SHA256"
    assert zipfile.is_zipfile(archive), "This source exporter requires ZIP format (regardless of extension)"
    prefix=f"Mods/FHD_MODS/{a.mod}/"
    allfiles, copied, original_text, dat_json, pngs = {}, [], [], [], []
    warnings=[]
    gi_groups=defaultdict(list)
    with zipfile.ZipFile(archive) as z, tempfile.TemporaryDirectory() as temp:
        assert z.testzip() is None, "Archive CRC error"
        info=[f for f in z.infolist() if not f.is_dir()]
        if not any(f.filename.startswith(prefix) for f in info):
            prefix=f"Mods/OtherMods/{a.mod}/"
        assert len({f.filename for f in info})==len(info), "Duplicate ZIP file names"
        assert info, "Empty archive"
        for f in info:
            n=f.filename
            norm=PurePosixPath(n)
            assert not norm.is_absolute() and ".." not in norm.parts and "\\" not in n, "Unsafe entry "+n
            data=z.read(f)
            allfiles[n]={"size":len(data),"sha256":sha(data)}
            if not n.startswith(prefix):
                continue
            rel=PurePosixPath(n[len(prefix):])
            suffix=rel.suffix.lower()
            if suffix in {".dll",".exe",".pyd",".so",".bin",".obj",".o",".a"}:
                continue
            dst=target/"SOURCE"/"ORIGINAL"/Path(*rel.parts)
            put(dst,data)
            copied.append(n)
            if suffix in {".c",".h",".cpp",".hpp",".py",".def",".s",".asm",".inc",".sh",".md",".txt",".json",".ini",".cfg"}:
                original_text.append(n)
            if suffix==".dat" and dat_codec is not None:
                fmt="cachedata" if rel.name.lower()=="cachedata.dat" else "blockpar"
                try:
                    decoded=dat_codec.decode(data,fmt)
                    if dat_codec.encode(decoded) != data:
                        raise ValueError("not byte-exact")
                    out=target/"SOURCE"/"DAT_JSON"/Path(*rel.parts)
                    out=out.with_name(out.name+".json")
                    put(out,(json.dumps(decoded,ensure_ascii=False,indent=2)+"\n").encode("utf-8"))
                    dat_json.append(str(rel))
                except Exception as e:
                    warnings.append("Cannot roundtrip DAT "+n+": "+str(e))
            if suffix==".gi":
                gi_groups[str(rel.parent)].append((rel.name,data))
            if rel.name.lower()=="moduleinfo.txt":
                for enc in ("utf-16","utf-8-sig","cp1251"):
                    try:
                        decoded=data.decode(enc)
                        if decoded.encode(enc)!=data:continue
                        out=target/"SOURCE"/"EDITABLE"/"ModuleInfo.utf8.txt"
                        put(out,decoded.encode("utf-8"))
                        break
                    except (UnicodeDecodeError,UnicodeEncodeError):
                        pass
        for group,values in gi_groups.items():
            inp=Path(temp)/("g"+str(len(pngs)))
            inp.mkdir()
            image_inputs=[]
            for filename,raw in values:
                f=inp/filename
                f.write_bytes(raw)
                image_inputs.append(str(f))
            out=target/"SOURCE"/"PNG"/group
            out.mkdir(parents=True,exist_ok=True)
            try:
                cmd=[sys.executable,"-m","srhd_modkit","convert","gi-png",*image_inputs,"--output",str(out)]
                proc=subprocess.run(cmd,capture_output=True,text=True,timeout=180)
                if proc.returncode:
                    raise RuntimeError(proc.stderr[-300:])
                for filename,raw in values:
                    pp=out/(Path(filename).stem+".png")
                    if pp.is_file() and pp.read_bytes().startswith(b"\x89PNG\r\n\x1a\n"):
                        pngs.append(str(pp.relative_to(target)))
                    else:
                        warnings.append("GI exported without valid PNG: "+filename)
            except Exception as e:
                warnings.append("GI-to-PNG conversion unavailable for "+group+": "+str(e)[:200])
    report={"mod":a.mod,"source_archive":archive.name,"archive_sha256":a.sha256.lower(),
            "archive_file_count":len(allfiles),"original_files_in_source":len(copied),
            "text_or_code_files":len(original_text),"byte_exact_DAT_to_JSON":len(dat_json),
            "png_files":len(pngs),"data_loss_in_original_archive":False,
            "dll_source_recovered_from_compiled_binary":False,
            "game_tested":False,"warnings":warnings,"files":allfiles}
    put(target/"VALIDATION_REPORT.json",(json.dumps(report,indent=2,ensure_ascii=False)+"\n").encode("utf-8"))
    print("PASS: original ZIP SHA256; files:",len(allfiles),"source:",len(copied),
          "text/code:",len(original_text),"DAT JSON:",len(dat_json),"PNG:",len(pngs))
    for warning in warnings:print("NOTICE:",warning)
if __name__=="__main__":
    main()
