/*
lz4pack - a tiny packer for the lz4 block format (as opposed to the frame format)
used for certain fromage assets
Copyright (c) 2019 asie
*/

#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "lz4.h"

int main(int argc, char** argv) {
	if (argc < 3) {
		fprintf(stderr, "Must specify input and output filenames!\n");
		return 1;
	}

	int offset = 0;
	int prepend_size = 0;

	int c;
	while ((c = getopt(argc, argv, "po:")) != -1) {
		switch (c) {
			case 'p': prepend_size = 1; break;
			case 'o': offset = atoi(optarg); break;
		}
	}

	FILE *fin = fopen(argv[optind + 0], "rb");
	if (fin == NULL) {
		fprintf(stderr, "Could not open input file!\n");
		return 1;
	}

	FILE *fout = fopen(argv[optind + 1], "wb");
	if (fout == NULL) {
		fprintf(stderr, "Could not open output file!\n");
		return 1;
	}

	// read whole file in
	fseek(fin, 0, SEEK_END);
	int size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	int read = 0;

	uint8_t *buf = malloc(size);
	if (buf == NULL) { return 1; }

	while (read < size && !feof(fin)) {
		read += fread(buf + read, 1, size - read, fin);
	}
	fclose(fin);

	if (offset > 0) {
		fwrite(buf, 1, offset, fout);
	}

	if (prepend_size != 0) {
		int s = size - offset;
		fputc(s & 0xFF, fout);
		fputc((s >> 8) & 0xFF, fout);
		fputc((s >> 16) & 0xFF, fout);
		fputc((s >> 24) & 0xFF, fout);
	}

	int cmp_size = LZ4_compressBound(size - offset);
	char *cmp_data = malloc(cmp_size);
	if (cmp_data == NULL) return 1;

	cmp_size = LZ4_compress_default(buf + offset, cmp_data, size - offset, cmp_size);
	if (cmp_size <= 0) return 1;

	cmp_data = realloc(cmp_data, cmp_size);
	if (cmp_data == NULL) return 1;

	fwrite(cmp_data, 1, cmp_size, fout);
	fclose(fout);
	return 0;
}
