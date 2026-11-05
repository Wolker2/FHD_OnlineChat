"""Experimental fixed-cell antialiased atlas. Pillow; DejaVu Sans Mono 13px.
No font file is loaded by the game; the generated header is compiled into DLL.
"""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
import argparse
p=argparse.ArgumentParser()
p.add_argument('--font',default='/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf')
a=p.parse_args()
font=ImageFont.truetype(a.font,13)
chars=[chr(i) for i in range(32,127)]+[chr(i) for i in range(0x400,0x460)]+[chr(i) for i in [0x0490,0x0491,0x2014,0x2013,0x2026]]
assert len(chars)==196
values=[]
for ch in chars:
 # Draw away from the left border to retain negative bearings.
 source=Image.new('L',(32,20));ImageDraw.Draw(source).text((8,1),ch,fill=255,font=font)
 cell=Image.new('L',(8,20));bbox=source.getbbox()
 if bbox:
  ink=source.crop(bbox);width=min(7,ink.width)
  if ink.width>7:ink=ink.resize((width,ink.height),Image.Resampling.LANCZOS)
  # A guaranteed transparent last column separates every pair of letters.
  cell.paste(ink,((7-width)//2,bbox[1]))
 assert not cell.crop((7,0,8,20)).getbbox()
 values.extend(cell.tobytes())
out=Path(__file__).with_name('font16.h')
text='/* EXPERIMENT: DejaVu Sans Mono 13px, antialiased 8x20 cell; column 7 blank. */\n'
text+='static const unsigned char glyph_alpha8[] = {\n'
text+=',\n'.join(' '+','.join(str(v) for v in values[i:i+32]) for i in range(0,len(values),32))
text+='\n};\n#define GLYPH_COUNT 196\n#define GLYPH_ALPHA_WIDTH 8\n'
out.write_text(text)
print('Generated',len(values),'alpha samples')
