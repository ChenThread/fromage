#include "common.h"
#include <orelei.h>

#define SOUND_ID_COUNT 16

static uint16_t ids[SOUND_ID_COUNT];

void sound_init(void) {
	file_record_t *file = cdrom_get_file("SOUNDS.LZ4");
	uint8_t *cmpbuf = malloc(file->size);
	cdrom_read_record(file, cmpbuf);

	memcpy(ids, cmpbuf, SOUND_ID_COUNT * sizeof(uint16_t));
	int cmpbufsize = file->size - SOUND_ID_COUNT*2 - 4;
	int bufsize = READ32LE(cmpbuf, SOUND_ID_COUNT*2);
	uint8_t *buf = lz4_alloc_and_unpack(cmpbuf + SOUND_ID_COUNT*2 + 4, cmpbufsize, bufsize);

	orelei_init_spu();
	orelei_sram_write_blocking(0x1000, buf, bufsize);

	free(buf);
	free(cmpbuf);
}

static int snote = 0;
static int32_t sound_rand = 1;

void sound_play(int id, int vol_left, int vol_right) {
	int base_freq = 2048;
	int pitch_diff = (RAND(sound_rand) & 0xFF) - 0x80;
	orelei_play_note(snote, 0x1000 + (ids[id]<<3), 0x9FC083FF, vol_left, vol_right, base_freq + pitch_diff);
	orelei_commit_key_changes();
	snote = (snote + 1) & 15;
}

#define SOUND_GRASS 0
#define SOUND_GRAVEL 1
#define SOUND_STONE 2
#define SOUND_WOOD 3

static uint8_t sound_ids[] = {
	SOUND_GRASS,
	SOUND_STONE, SOUND_GRASS, SOUND_GRASS, SOUND_STONE, SOUND_WOOD, // 1-5
	SOUND_GRASS, SOUND_STONE, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, // 6-10
	SOUND_GRASS, SOUND_GRAVEL, SOUND_GRAVEL, SOUND_STONE, SOUND_STONE, // 11-15
	SOUND_STONE, SOUND_WOOD, SOUND_GRASS, SOUND_STONE, SOUND_STONE, // 16-20
	SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, // 21-25
	SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, // 26-30
	SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, // 31-35
	SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, SOUND_GRASS, // 36-40
	SOUND_STONE, SOUND_STONE, SOUND_STONE, SOUND_STONE, SOUND_STONE, // 41-45
	SOUND_STONE, SOUND_WOOD, SOUND_STONE, SOUND_STONE // 46-49
};

int sound_get_id(int32_t bid) {
	int a = RAND(sound_rand) & 3;
	int b = sound_ids[bid] & 3;
	return (b<<2) + a;
}
