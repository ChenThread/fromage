import os.path
import sys


def main(): # type: () -> None
	in_fname = sys.argv[1]
	label_name = os.path.basename(in_fname).replace(".", "_")
	assert label_name != ""
	with open(in_fname, "rb") as infp:
		raw_data = infp.read()
	data = [ord(raw_data[i:i+1]) for i in range(len(raw_data))]
	outfp = sys.stdout
	outfp.write("uint8_t %s[] = {\n" % (label_name,))
	for i in range(0, len(data), 16):
		b = data[i:i+16]
		if i > 0:
			outfp.write(",")
		outfp.write(",".join("0x%02X" % (v,) for v in b) + "\n")
	outfp.write("};")


if __name__ == "__main__":
	main()
