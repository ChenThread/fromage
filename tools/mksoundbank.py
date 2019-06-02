import sys, struct

fp = open(sys.argv[1], "wb")
file_count = len(sys.argv) - 2
buffers = [None] * file_count
pos = [None] * (file_count)

j = 0
for i in range(file_count):
	with open(sys.argv[2+i], "rb") as tf:
		pos[i] = j
		buffers[i] = tf.read()
		j += int(len(buffers[i]) / 8)

for i in range(len(pos)):
	fp.write(struct.pack("<H", pos[i]))

for i in range(file_count):
	fp.write(buffers[i])

fp.close()
