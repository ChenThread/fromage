/*
lz4pack - a tiny packer for the lz4 block format (as opposed to the frame format)
used for certain fromage assets
Copyright (c) 2019 asie
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "lz4.h"

int main(int argc, char** argv) {
	if (argc < 3) {
		fprintf(stderr, "Must specify input and output filenames!\n");
		return 1;
	}

	FILE *fin = fopen(argv[1], "rb");
	if (fin == NULL) {
		fprintf(stderr, "Could not open input file!\n");
		return 1;
	}

	FILE *fout = fopen(argv[2], "wb");
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

	int cmp_size = LZ4_compressBound(size);
	char *cmp_data = malloc(cmp_size);
	if (cmp_data == NULL) return 1;

	cmp_size = LZ4_compress_default(buf, cmp_data, size, cmp_size);
	if (cmp_size <= 0) return 1;

	cmp_data = realloc(cmp_data, cmp_size);
	if (cmp_data == NULL) return 1;

	fwrite(cmp_data, 1, cmp_size, fout);
	fclose(fout);
	return 0;
}
