#!/usr/bin/env python3
"""Recreate exact ModuleInfo.txt from editable UTF-8 while preserving CRLF."""
from pathlib import Path
import hashlib
root=Path(__file__).resolve().parent
text=(root/'ModuleInfo.utf8.txt').read_bytes().decode('utf-8')
original=b'\xff\xfe'+text.encode('utf-16le')
EXPECTED='9c532c4946420f5dcd060cf82ed38885bbc15b1c2e1f97ff7f8c5b568e367de5'
assert len(original)==2282 and hashlib.sha256(original).hexdigest()==EXPECTED, 'ModuleInfo byte mismatch'
print('ModuleInfo.txt: 2282/2282 bytes rebuilt exactly')
