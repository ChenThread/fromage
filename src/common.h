#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <psxdefs.h>

#define ASSERT(x) if(!(x)) { for(;;) {} }
#define ABS(x) ((x)>=0 ? (x) : -(x))
#define READ16LE(secbuf,i) ((secbuf)[(i)] | ((secbuf)[(i)+1]<<8))
#define READ32LE(secbuf,i) ((secbuf)[(i)] | ((secbuf)[(i)+1]<<8) | ((secbuf)[(i)+2]<<16) | ((secbuf)[(i)+3]<<24))

#define FASTMEM __attribute__((section(".fastmem")))

#include "config.h"

typedef int32_t fixed;
typedef int64_t fixed64;
typedef fixed vec3[3];
typedef fixed vec4[4];
typedef vec4 mat4[4];

#define FM_PI ((fixed)0x00008000)
#define FACE_ZN 0
#define FACE_ZP 1
#define FACE_XN 2
#define FACE_XP 3
#define FACE_YN 4
#define FACE_YP 5

#define RAND(seed) ((seed) = ((seed) * 16843009) + 826366247)
typedef void save_progress_callback(int progress, int max);

typedef struct {
	bool pro_jumps;
	bool move_dpad;
	uint8_t render_distance;
	uint8_t show_fps;
	uint8_t debug_mode;
	uint8_t sound_on;
	uint8_t music_on;
	uint8_t fog_on;
	uint8_t fov_mode;
} options_t;

// Files
extern uint8_t font_raw[];
extern uint8_t fsys_level[LEVEL_LY][LEVEL_LZ][LEVEL_LX];

// block_info.h
#define BLOCK_MAX 50
#define QUAD_MAX 6
typedef struct block_info {
	uint16_t tc, tp, cl, pad0;
} block_info_t;

extern block_info_t block_info[BLOCK_MAX][QUAD_MAX];

// cdrom.c
typedef struct {
	int32_t lba, size;
	uint8_t flags;
	char filename[16];
} file_record_t;

void cdrom_tick_vblank(void);
void cdrom_tick_song_player(int vblanks, int music_on);
file_record_t *cdrom_get_file(const char *name);
int cdrom_read_record(file_record_t *record, uint8_t *buffer);
void cdrom_isr(void);
int cdrom_has_songs(void);
void cdrom_init(save_progress_callback *pc);

// main.c
extern int32_t cam_x;
extern int32_t cam_y;
extern int32_t cam_z;

extern int32_t mat_rt11, mat_rt12, mat_rt13;
extern int32_t mat_rt21, mat_rt22, mat_rt23;
extern int32_t mat_rt31, mat_rt32, mat_rt33;

#define OT_WORLD 2

extern int hotbar_pos;
#define HOTBAR_MAX 9
extern int current_block[HOTBAR_MAX];

void draw_block(int32_t cx, int32_t cy, int32_t cz, int di, int block, uint32_t facemask, bool transparent);

// joy.c
extern int32_t joy_pressed;
void joy_update(int ticks, int autorepeat_divisor);

// gpu.c
extern volatile uint32_t vblank_counter;

void frame_start(void);
void frame_flip(void);
void frame_flip_nosync(void);
void wait_for_next_vblank(void);
void wait_for_vblanks(uint32_t count);
void gp0_command(uint32_t v);
void gp0_data(uint32_t v);
void gp0_data_xy(uint32_t x, uint32_t y);
void gp1_command(uint32_t v);

// gpu_dma.c
extern uint32_t dma_pos;
#define DMA_BUFFER_COUNT 4
#define DMA_ORDER_MAX 128
#define DMA_BUFFER_SIZE (256*384)

extern uint32_t dma_buffer[DMA_BUFFER_SIZE];
extern uint32_t dma_order_table[DMA_BUFFER_COUNT][DMA_ORDER_MAX];
extern uint32_t dma_buffer_current;
#define DMA_PUSH(len, ot) \
	if(dma_pos >= ((sizeof(dma_buffer)-32)/sizeof(int32_t))) { \
		dma_pos = 0; \
	} \
	dma_buffer[dma_pos] = \
		(dma_order_table[dma_buffer_current][ot] & 0x00FFFFFF) \
		| ((len)<<24); \
	dma_order_table[dma_buffer_current][ot] = ((uint32_t)&dma_buffer[dma_pos])&0x00FFFFFF; \
	dma_pos++; \

void gpu_dma_init(void);
int gpu_dma_finish(void);

void gpu_dma_load(uint32_t *buffer, int x, int y, int width, int height, int use_lz4);

// gui.c
#define FONT_CHARS 128
int get_text_width(const char *format, ...);
void draw_text(int x, int y, int color, const char *format, ...);
void draw_status_progress(int progress, int max);
void draw_block_icon(int bx, int by, int bw, int bh, int bid);
void draw_block_background(block_info_t *bi);
void draw_dirt_background(void);
void draw_status_window(int style, const char *format, ...);
void draw_block_sel_menu(int pos, uint8_t *slots, int slotcount);
void draw_current_block(void);
void draw_hotbar(void);
void draw_crosshair(void);
void draw_liquid_overlay(void);
int gui_menu(int optcount, int optstartpos, ...);
void gui_terrible_text_viewer(const char* text);

// options.c
int gui_options_menu(options_t *options);
int gui_worldgen_menu(void);

// save.c
#define SAVE_ERROR_COMPRESSION -1
#define SAVE_ERROR_OUT_OF_SPACE -2
#define SAVE_ERROR_NOT_FOUND -3
#define SAVE_ERROR_INVALID_ARGUMENTS -4
#define SAVE_ERROR_CARD -5
#define SAVE_ERROR_CARD_FATAL -6
#define SAVE_ERROR_CORRUPT_DATA -7
#define SAVE_ERROR_MAP_TOO_LARGE -8
#define SAVE_ERROR_OUT_OF_MEMORY -9
#define SAVE_ERROR_UNSUPPORTED_DATA -10

typedef struct {
	int16_t xsize, ysize, zsize;
	int32_t cam_x, cam_y, cam_z, cam_rx, cam_ry;
	uint8_t hotbar_blocks[HOTBAR_MAX];
	uint8_t hotbar_pos;
	options_t options;
} level_info;

const char *save_get_error_string(int value);
int load_level(int save_id, level_info *info, uint8_t *target, int32_t target_size, save_progress_callback *pc);
int save_level(int save_id, level_info *info, const uint8_t *data, save_progress_callback *pc);

// sound.c
void sound_init(void);
void sound_play(int id, int vol_left, int vol_right);
int sound_get_id(int32_t block_id);

// util.c
uint8_t *lz4_alloc_and_unpack(uint8_t *buf, int cmpsize, int size);

// world.c
uint8_t world_get_top_opaque(int32_t cx, int32_t cz);
uint32_t world_get_vis_blocks_unsafe(int32_t cx, int32_t cy, int32_t cz);
int32_t world_get_block_unsafe(int32_t cx, int32_t cy, int32_t cz);
int32_t world_get_render_faces_unsafe(int32_t cx, int32_t cy, int32_t cz);
int32_t world_get_block(int32_t cx, int32_t cy, int32_t cz);
void world_set_block(int32_t cx, int32_t cy, int32_t cz, uint8_t b, uint8_t flags);
int32_t world_cast_ray(int32_t px, int32_t py, int32_t pz, int32_t vx, int32_t vy, int32_t vz, int32_t *ocx, int32_t *ocy, int32_t *ocz, int32_t max_steps, bool use_block_before_hit);
uint32_t world_is_translucent(int32_t b);
uint32_t world_is_walkable(int32_t b);
int32_t world_is_colliding(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2);
int32_t world_is_colliding_fixed(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2);
void world_schedule_block_update(int32_t cx, int32_t cy, int32_t cz, uint32_t delay);
void world_init();
void world_update(uint32_t ticks, uint32_t *vblank_counter);

// worldgen.c
typedef void worldgen_stage_callback(const char *message);
void world_generate(int mode, uint8_t *map, int32_t lx, int32_t ly, int32_t lz, uint32_t seed, worldgen_stage_callback *wc, save_progress_callback *pc);
