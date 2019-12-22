import sys, struct
import numpy as np
import scipy.cluster.vq as vq
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
	# generate palette
	img = im.crop((ix, iy, ix+iw, iy+ih)).convert("RGBA")
	img_data = np.zeros((iw * ih, 3))
	img_translucent = np.zeros((iw * ih))
	has_translucent = False
	for iry in range(ih):
		for irx in range(iw):
			img_pixel = img.getpixel((irx, iry))
			img_data[iry * iw + irx] = [
				int(img_pixel[0] * 31 / 255),
				int(img_pixel[1] * 31 / 255),
				int(img_pixel[2] * 31 / 255)
			]
			if img_pixel[3] <= 1:
				img_translucent[iry * iw + irx] = 1
				has_translucent = True
	centroids,_ = vq.kmeans(img_data, 15 if has_translucent else 16)
	palette = [0x0000] * 16
	for pl in range(len(centroids)):
		r = max(0, min(31, int(centroids[pl][0])))
		g = max(0, min(31, int(centroids[pl][1])))
		b = max(0, min(31, int(centroids[pl][2])))
		palette[pl] = 0x8000|(r<<0)|(g<<5)|(b<<10)
	# TODO: floyd-steinberg
	indexes,_ = vq.vq(img_data,centroids)
	for iry in range(ih):
		for irx in range(iw):
			iridx = iry * iw + irx
			impc = 15
			if img_translucent[iridx] == 0:
				impc = indexes[iridx]
			imgdata[(ty+iry)*imgwidth + tx+irx] = impc
	return palette

#	imgp = img.convert(mode='P', palette=Image.ADAPTIVE, colors=16)
#	imgpalr = imgp.getpalette()
#	palette = [None] * 16
#	for col in range(16):
#		palette[col] = [imgpalr[col * 3], imgpalr[col * 3 + 1], imgpalr[col * 3 + 2], 255]
#	for iry in range(ih):
#		for irx in range(iw):
#			impc = imgp.getpixel((irx, iry))
#			impp = img.getpixel((irx, iry))
#			if len(impp) > 3:
#				palette[impc][3] = impp[3]
#			imgdata[(ty+iry)*imgwidth + tx+irx] = impc
#	return palette

def write_palette(fp, palette):
	for ip in range(16):
		fp.write(struct.pack("<H", palette[ip]))

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
	for ix in range(0, imgwidth, 2):
		v = (imgdata[iy*imgwidth+ix+1] << 4) | (imgdata[iy*imgwidth+ix] << 0)
		fp.write(struct.pack("<B", v))
	write_palette(fp, clut[iy])

fp.close()
