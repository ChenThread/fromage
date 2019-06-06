#include "common.h"
#include "lz4.h"

uint8_t *lz4_alloc_and_unpack(uint8_t *buf, int cmpsize, int size) {
	uint8_t *out = malloc(size);
	if (out == NULL) return NULL;
	int outsize = LZ4_decompress_safe(buf, out, cmpsize, size);
	if (outsize <= 0) { free(out); return NULL; }
	return out;
}
