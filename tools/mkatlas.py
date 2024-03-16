import sys, struct
import numpy as np
import scipy.cluster.vq as vq
from PIL import Image, ImageOps, ImageColor

im = Image.open(sys.argv[1])
fp = open(sys.argv[2], "wb")
clut = [None] * 256
single_pixel_colors = [None] * 256
imgdata = [None] * (256*256)
imgwidth = 256
mipmap_levels = 4

def draw_4bit(im, ix, iy, iw, ih, tx, ty):
	# generate palette
	img = [None] * (mipmap_levels+1)
	img_data = [None] * (mipmap_levels+1)
	img_translucent = [None] * (mipmap_levels+1)

	img[0] = im.crop((ix, iy, ix+iw, iy+ih)).convert("RGBA")
	img_data[0] = np.zeros((iw * ih, 3))
	img_translucent[0] = np.zeros((iw * ih))
	for irm in range(1,mipmap_levels+1):
		img[irm] = img[0].resize((iw>>irm, ih>>irm), Image.LANCZOS)
		img_data[irm] = np.zeros(((iw>>irm) * (ih>>irm), 3))
		img_translucent[irm] = np.zeros(((iw>>irm) * (ih>>irm)))
	has_translucent = False
	for irm in range(0,mipmap_levels+1):
		for iry in range(ih>>irm):
			for irx in range(iw>>irm):
				img_pixel = img[irm].getpixel((irx, iry))
				img_data[irm][iry * (iw>>irm) + irx] = [
					int(img_pixel[0] * 31 / 255.0),
					int(img_pixel[1] * 31 / 255.0),
					int(img_pixel[2] * 31 / 255.0)
				]
				if img_pixel[3] <= 1:
					img_translucent[irm][iry * (iw>>irm) + irx] = 1
					has_translucent = True
	centroids,_ = vq.kmeans(img_data[0], 15 if has_translucent else 16)
	palette = [0x0000] * 16
	for pl in range(len(centroids)):
		r = max(0, min(31, int(centroids[pl][0] + 0.5)))
		g = max(0, min(31, int(centroids[pl][1] + 0.5)))
		b = max(0, min(31, int(centroids[pl][2] + 0.5)))
		palette[pl] = 0x8000|(r<<0)|(g<<5)|(b<<10)
	# TODO: floyd-steinberg
	for irm in range(0,mipmap_levels+1):
		indexes,_ = vq.vq(img_data[irm],centroids)
		for iry in range(ih>>irm):
			for irx in range(iw>>irm):
				iridx = iry * (iw>>irm) + irx
				impc = 15
				if img_translucent[irm][iridx] == 0:
					impc = indexes[iridx]
				imgdata[((ty>>irm)+iry)*imgwidth + ((tx>>irm)+irx)] = impc
	single_pixel = img[mipmap_levels].getpixel((0, 0))
	single_pixel_color = 0
	single_pixel_color |= int(single_pixel[2]) << 16
	single_pixel_color |= int(single_pixel[1]) << 8
	single_pixel_color |= int(single_pixel[0])
	return palette, single_pixel_color

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
	clut[i], single_pixel_colors[i] = draw_4bit(im, ix, iy, 16, 16, tx, ty)

for i in range(256):
	tx = (i & 15) << 4
	ty = (i & 240)
	add_texture(im, tx, ty, i)

for iy in range(128):
	fp.write(struct.pack("<I", single_pixel_colors[iy + 128]))

for iy in range(256):
	for ix in range(0, imgwidth, 2):
		v = (imgdata[iy*imgwidth+ix+1] << 4) | (imgdata[iy*imgwidth+ix] << 0)
		fp.write(struct.pack("<B", v))
	write_palette(fp, clut[iy])

fp.close()
