import sys, struct
from PIL import Image, ImageOps, ImageColor

im = Image.open(sys.argv[1])
fp = open(sys.argv[2], "wb")
clut = []
imgdata = [None] * (256*256)
imgwidth = 256

def draw_4bit(im, ix, iy, iw, ih, tx, ty):
	img = im.crop((ix, iy, ix+iw, iy+ih))
	imgp = img.convert(mode='P', palette=Image.ADAPTIVE, colors=16)
	imgpalr = imgp.getpalette()
	palette = [None] * 16
	for col in range(16):
		palette[col] = [imgpalr[col * 3], imgpalr[col * 3 + 1], imgpalr[col * 3 + 2], 0]
	for iry in range(ih):
		for irx in range(iw):
			impc = imgp.getpixel((irx, iry))
			palette[impc][3] = img.getpixel((irx, iry))[3]
			imgdata[(ty+iry)*imgwidth + tx+irx] = impc
	return palette

def write_palette(fp, palette):
	for ip in range(16):
		v = 0x0000
		if palette[ip][3] >= 2:
			# Opaque
			r = max(0, min(31, palette[ip][0]>>3))
			g = max(0, min(31, palette[ip][1]>>3))
			b = max(0, min(31, palette[ip][2]>>3))
			v = 0x8000|(r<<0)|(g<<5)|(b<<10)
		fp.write(struct.pack("<H", v))

for i in range(256):
	ix = (i & 15) << 4
	iy = (i & 240)
	clut.append(draw_4bit(im, ix, iy, 16, 16, ix, iy))

for iy in range(256):
	for ix in range(0, 256, 2):
		v = (imgdata[iy*256+ix+1] << 4) | (imgdata[iy*256+ix] << 0)
		fp.write(struct.pack("<B", v))
	write_palette(fp, clut[iy])

fp.close()
