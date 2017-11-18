import sys, struct
#import high_quality_rips

indat = open(sys.argv[1], "rb").read()[18:][:256*256*4]
fp = open(sys.argv[2], "wb")
for i in xrange(256*256):
	p = [ord(c) for c in indat[i*4+0:i*4+4]]
	if p[3] >= 2:   # >= 128:
		# Opaque
		r = max(0, min(31, p[2]>>3))
		g = max(0, min(31, p[1]>>3))
		b = max(0, min(31, p[0]>>3))
		v = 0x8000|(r<<0)|(g<<5)|(b<<10)
	else:
		# Transparent
		v = 0x0000
	fp.write(struct.pack("<H", v))
	#print p

fp.close()
