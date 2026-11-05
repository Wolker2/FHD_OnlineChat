#!/usr/bin/env python3
"""Verify byte-identical FHD OnlineChat 0.22 FIX23 source snapshot."""
from pathlib import Path
import hashlib
import sys

root = Path(__file__).resolve().parent
lines = (root / "SHA256SUMS.txt").read_text(encoding="utf-8").splitlines()
errors = []
for line in lines:
    checksum, relative = line.split(None, 1)
    path = root / relative.strip()
    if not path.is_file():
        errors.append(f"Missing: {relative}")
        continue
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    if digest != checksum:
        errors.append(f"Changed: {relative}")
print(f"OnlineChat 0.22 FIX23: {len(lines)-len(errors)}/{len(lines)} source SHA-256 OK")
for item in errors:
    print(item, file=sys.stderr)
raise SystemExit(1 if errors else 0)
