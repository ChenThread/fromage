import os, sys, struct

fp = open(sys.argv[1], "wb")
file_count = len(sys.argv) - 3
pos = [None] * (file_count)
sector_size = 2352
sectors_per_second = 75/4.0

fp_volume = open(sys.argv[2], "r")
fp.write(struct.pack("<H", int(fp_volume.readline(), 16)))
fp_volume.close()

j = 0
for i in range(file_count):
	sectors = os.path.getsize(sys.argv[3+i]) / sector_size
	pos[i] = int((sectors / sectors_per_second) + 2)

for i in range(len(pos)):
	fp.write(struct.pack("<H", pos[i]))

fp.close()
