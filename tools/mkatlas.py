import sys, struct
from PIL import Image, ImageOps, ImageColor

im = Image.open(sys.argv[1])
imwater = None
imlava = None
if len(sys.argv) >= 5:
	imwater = Image.open(sys.argv[3])
	imlava = Image.open(sys.argv[4])
fp = open(sys.argv[2], "wb")
clut = [None] * 256
imgdata = [None] * (256*256)
imgwidth = 256

def draw_4bit(im, ix, iy, iw, ih, tx, ty):
	img = im.crop((ix, iy, ix+iw, iy+ih))
	imgp = img.convert(mode='P', palette=Image.ADAPTIVE, colors=16)
	imgpalr = imgp.getpalette()
	palette = [None] * 16
	for col in range(16):
		palette[col] = [imgpalr[col * 3], imgpalr[col * 3 + 1], imgpalr[col * 3 + 2], 255]
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

def add_texture(im, ix, iy, i):
	tx = (i & 15) << 4
	ty = (i & 240)
	clut[i] = draw_4bit(im, ix, iy, 16, 16, tx, ty)

for i in range(256):
	tx = (i & 15) << 4
	ty = (i & 240)
	add_texture(im, tx, ty, i)

if imlava != None:
	for i in range(20):
		add_texture(imlava, 0, i*16, 80+i)
	for i in range(32):
		add_texture(imwater, 0, i*16, 100+i)

for iy in range(256):
	for ix in range(0, 256, 2):
		v = (imgdata[iy*256+ix+1] << 4) | (imgdata[iy*256+ix] << 0)
		fp.write(struct.pack("<B", v))
	write_palette(fp, clut[iy])

fp.close()
