# font file format: 256 nibbles of fontwidth, 8KB VRAM data

import sys, struct
from PIL import Image, ImageOps, ImageColor

im = Image.open(sys.argv[1])
fp = open(sys.argv[2], "wb")

def find_width(fch, offx, offy):
	if fch == 32:
		return 6
	for ix in xrange(7, 1, -1):
		for iy in xrange(8):
			if im.getpixel((offx+ix, offy+iy))[3] > 0:
				return ix+1
	return 1

for fch in xrange(0, 128, 2):
	offx = (fch & 15) * 8
	offy = (fch >> 4) * 8
	fp.write(struct.pack("<B", find_width(fch, offx, offy) | (find_width(fch + 1, offx + 8, offy) << 4)))

for iy in xrange(64):
	for ix in xrange(0, 128, 2):
		v = 0
		impc = im.getpixel((ix, iy))
		if (impc[3] > 0):
			v |= 1
		impc = im.getpixel((ix+1, iy))
		if (impc[3] > 0):
			v |= 16
		fp.write(struct.pack("<B", v))

fp.close()
