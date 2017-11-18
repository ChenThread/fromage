import sys, struct
from PIL import Image, ImageOps, ImageColor

im = Image.open(sys.argv[1])
fp = open(sys.argv[2], "wb")
clut = []
imgdata = [None] * (256*256)

for i in xrange(256):
	ix = (i & 15) << 4
	iy = (i & 240)
	ibox = (ix, iy, ix+16, iy+16)
	imc = im.crop(ibox)
	imr = imc.convert(mode='P', palette=Image.ADAPTIVE, colors=16)
	im.paste(imr, ibox)
	impal = imr.getpalette()[:64]
	for iry in xrange(16):
		for irx in xrange(16):
			impc = imr.getpixel((irx, iry))
			impal[48 + impc] = imc.getpixel((irx, iry))[3]
			imgdata[(iy+iry)*256 + ix+irx] = impc
	clut.append(impal)

for iy in xrange(256):
	for ix in xrange(0, 256, 2):
		v = (imgdata[iy*256+ix+1] << 4) | (imgdata[iy*256+ix] << 0)
		fp.write(struct.pack("<B", v))
	for ip in xrange(16):
		if clut[iy][48 + ip] >= 2:   # >= 128:
			# Opaque
			r = max(0, min(31, clut[iy][ip * 3 + 0]>>3))
			g = max(0, min(31, clut[iy][ip * 3 + 1]>>3))
			b = max(0, min(31, clut[iy][ip * 3 + 2]>>3))
			v = 0x8000|(r<<0)|(g<<5)|(b<<10)
		else:
			# Transparent
			v = 0x0000
		fp.write(struct.pack("<H", v))

fp.close()
