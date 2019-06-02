# font file format: 256 nibbles of fontwidth, 8KB VRAM data

import sys, struct
from PIL import Image, ImageOps, ImageColor

im = Image.open(sys.argv[1]).convert("RGBA")
fp = open(sys.argv[2], "wb")

def find_width(fch, offx, offy):
	if fch == 32:
		return 6
	for ix in range(7, 0, -1):
		for iy in range(8):
			if im.getpixel((offx+ix, offy+iy))[3] > 0:
				return ix+1
	return 1

for fch in range(0, 128, 2):
	offx = (fch & 15) * 8
	offy = (fch >> 4) * 8
	fp.write(struct.pack("<B", find_width(fch, offx, offy) | (find_width(fch + 1, offx + 8, offy) << 4)))

has_alpha = False
for iy in range(64):
	for ix in range(128):
		impc = im.getpixel((ix, iy))
		if impc[3] == 0:
			has_alpha = True
			break

alpha_idx = (3 if has_alpha else 0)

for iy in range(64):
	for ix in range(0, 128, 2):
		v = 0
		impc = im.getpixel((ix, iy))
		if (impc[alpha_idx] > 0):
			v |= 1
		impc = im.getpixel((ix+1, iy))
		if (impc[alpha_idx] > 0):
			v |= 16
#		fp.write(struct.pack("<B", v))
		fp.write(struct.pack("<B", (v & 0x1) * 17))
		fp.write(struct.pack("<B", (v >> 4) * 17))

fp.close()
