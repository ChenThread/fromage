#include <stdbool.h>
#include <stdio.h>
#include <orelei.h>
#include <seedy.h>
#include "common.h"

void cdrom_isr(void) {
	seedy_isr_cdrom();
}

#define READ16LE(secbuf,i) ((secbuf)[(i)] | ((secbuf)[(i)+1]<<8))
#define READ32LE(secbuf,i) ((secbuf)[(i)] | ((secbuf)[(i)+1]<<8) | ((secbuf)[(i)+2]<<16) | ((secbuf)[(i)+3]<<24))

#define FILE_RECORD_MAX 16

static file_record_t files[FILE_RECORD_MAX];
static int files_count = 0;

static uint32_t rand_seed = 1;
static uint16_t song_lengths[4];
static int32_t song_count = 0;
static int32_t song_vblanks = 0;
static int32_t song_stop_req = 0;

void cdrom_tick_vblank(void) {
	if (song_vblanks > 0) {
		song_vblanks--;
		if (song_vblanks <= 0) {
				song_stop_req = 1;
		}
	}
}

void cdrom_tick_song_player(int vbls) {
	if (song_count < 0) return;

	if (song_stop_req > 0) {
		orelei_close_cd_audio();
		seedy_stop_xa();
		song_stop_req = 0;
		song_vblanks = 0;
	}

	if (song_vblanks <= 0) {
		int it_time = 0;
		for (int i = 0; i < vbls; i++) {
			if ((RAND(rand_seed) & 0x3FF) == 0x121) it_time++;
		}

		if (it_time > 0) {
			file_record_t *record = cdrom_get_file("MUSIC.XA");
			if (record != NULL) {
				int val = ((rand_seed >> 16) % song_count);
				orelei_open_cd_audio(0x3FFF, 0x3FFF);
				seedy_read_xa(record->lba,
					SEEDY_PLAY_XA_37800
					| SEEDY_PLAY_XA_STEREO
					| SEEDY_READ_SINGLE_SPEED,
					1, val
				);
				song_vblanks = (song_lengths[val] + 3) * VBLANKS_PER_SEC;
			}
		}
	} else {
	}
}

file_record_t *cdrom_get_file(const char *name) {
	char fname[16];
	snprintf(fname, 16, "%s;1", name);

	for (int i = 0; i < files_count; i++) {
		if (strcmp(fname, files[i].filename) == 0) {
			return &files[i];
		}
	}

	return NULL;
}

#define CDROM_INIT_STEPS 5

void cdrom_init(save_progress_callback *pc) {
	// this is NOT how you write an ISO9660 parser
	// EVER

	uint8_t *buffer;

	buffer = malloc(2048);
	if (pc != NULL) pc(0, CDROM_INIT_STEPS);

	seedy_init_cdrom();
	if (pc != NULL) pc(1, CDROM_INIT_STEPS);

	// read pvd
	seedy_read_data_sync(16, 0, buffer, 2048);
	if (pc != NULL) pc(2, CDROM_INIT_STEPS);

	// parse pvd
	int record_lba = READ32LE(buffer, 0x9C + 0x02);
	int record_size = READ32LE(buffer, 0x9C + 0x0A);

	// read root directory records
	buffer = realloc(buffer, record_size);
	seedy_read_data_sync(record_lba, 0, buffer, record_size);
	if (pc != NULL) pc(3, CDROM_INIT_STEPS);

	// parse root directory records
	files_count = 0;
	uint8_t *ptr = buffer;

	int j = 0;
	while (*ptr >= 33 && files_count < FILE_RECORD_MAX) {
		int i = files_count;
		int fnsize = *(ptr + 0x20);
		if (fnsize > 15) fnsize = 15;

		files[i].lba = READ32LE(ptr, 0x02);
		files[i].size = READ32LE(ptr, 0x0A);
		files[i].flags = *(ptr + 0x19);

		memcpy(files[i].filename, ptr + 0x21, fnsize);
		files[i].filename[fnsize] = 0;

		ptr += (*ptr);
		files_count++;
	}
	if (pc != NULL) pc(4, CDROM_INIT_STEPS);

	{
		file_record_t *record = cdrom_get_file("MUSIC.HDR");
		if (record != NULL) {
			song_count = record->size >> 1;
			seedy_read_data_sync(record->lba, 0, buffer, record->size);
			for (int i = 0; i < song_count; i++) {
				song_lengths[i] = READ16LE(buffer, (i << 1));
			}
		}
	}

	if (pc != NULL) pc(5, CDROM_INIT_STEPS);
	free(buffer);
}

