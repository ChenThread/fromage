import sys, struct
from PIL import Image, ImageOps, ImageColor

im = Image.open(sys.argv[1])
fp = open(sys.argv[2], "wb")
clut = [None]
imgdata = [None] * (16*16)
imgwidth = 16

def draw_4bit(im, ix, iy, iw, ih, tx, ty):
	img = im.crop((ix, iy, ix+iw, iy+ih))
	imgp = img.convert(mode='P', palette=Image.ADAPTIVE, colors=15)
	imgpalr = imgp.getpalette()
	palette = [None] * 16
	for col in range(15):
		palette[col] = [imgpalr[col * 3], imgpalr[col * 3 + 1], imgpalr[col * 3 + 2], 255]
	palette[15] = [255, 255, 255, 255]
	for iry in range(ih):
		for irx in range(iw):
			impc = imgp.getpixel((irx, iry))
			impp = img.getpixel((irx, iry))
			if len(impp) > 3:
				palette[impc][3] = impp[3]
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

write_palette(fp, draw_4bit(im, 0, 0, 16, 16, 0, 0))

for iy in range(16):
	for ix in range(0, 16, 2):
		v = (imgdata[iy*imgwidth+ix+1] << 4) | (imgdata[iy*imgwidth+ix] << 0)
		fp.write(struct.pack("<B", v))

fp.close()
