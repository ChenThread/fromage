#include "common.h"
#include <orelei.h>

#include "../obj/soundbank.h"

void sound_init(void) {
	orelei_init_spu();
	orelei_sram_write_blocking(0x1000, soundbank_raw + 32, sizeof(soundbank_raw) - 32);
}

static int snote = 0;
static int32_t sound_rand = 1;

void sound_play(int id, int vol_left, int vol_right) {
	int base_freq = 1365;
	int pitch_diff = (RAND(sound_rand) & 0xFF) - 0x80;
	uint16_t *ids = (uint16_t*) soundbank_raw;
	orelei_play_note(snote, 0x1000 + (ids[id]<<3), 0x9FC083FF, vol_left, vol_right, base_freq + pitch_diff);
	orelei_commit_key_changes();
	snote = (snote + 1) & 15;
}

int sound_get_id(int32_t bid) {
	int a = RAND(sound_rand) & 3;
	int b = 0;
	switch (bid) {
		case 12: case 13: b = 1; break;
		case 1: case 4: case 7: case 14: case 15: case 16:
		case 41: case 42: case 43: case 44: case 45:
		case 48: case 49: b = 2; break;
		case 5: case 17: case 47: b = 3; break;
	}
	return (b<<2) + a;
}
