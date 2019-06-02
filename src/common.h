#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <psxregs.h>
#include <psxdefs/joy.h>
#include "psx.h"

#define ASSERT(x) if(!(x)) { for(;;) {} }
#define ABS(x) ((x)>=0 ? (x) : -(x))

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

// Files
#define LEVEL_LX 64
#define LEVEL_LY 64
#define LEVEL_LZ 64
extern uint32_t atlas_raw[];
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
void cdrom_isr(void);
void cdrom_init(void);

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
int update_joy_pressed(void);

// gpu.c
extern volatile uint32_t vblank_counter;

void frame_start(void);
void frame_flip(void);
void wait_for_next_vblank(void);
void wait_for_vblanks(uint32_t count);
void gpu_dma_load(uint32_t *buffer, int x, int y, int width, int height);
void gp0_command(uint32_t v);
void gp0_data(uint32_t v);
void gp0_data_xy(uint32_t x, uint32_t y);
void gp1_command(uint32_t v);

// gpu_dma.c
extern uint32_t dma_pos;
#define DMA_ORDER_MAX 64

extern uint32_t dma_buffer[256*512];
extern uint32_t dma_order_table[4][DMA_ORDER_MAX];
extern uint32_t dma_buffer_current;
#define DMA_PUSH(len, ot) \
	while(dma_pos >= (sizeof(dma_buffer)/sizeof(int32_t))) {} \
	dma_buffer[dma_pos] = \
		(dma_order_table[dma_buffer_current][ot] & 0x00FFFFFF) \
		| ((len)<<24); \
	dma_order_table[dma_buffer_current][ot] = ((uint32_t)&dma_buffer[dma_pos])&0x00FFFFFF; \
	dma_pos++; \

void gpu_dma_init(void);
void gpu_dma_finish(void);

// gui.c
#define FONT_CHARS 128
int get_text_width(const char *format, ...);
void draw_text(int x, int y, int color, const char *format, ...);
void draw_status_progress(int progress, int max);
void draw_block_icon(int bx, int by, int bw, int bh, int bid);
void draw_block_background(block_info_t *bi);
void draw_dirt_background(void);
void draw_status_window(const char *format, ...);
void draw_block_sel_menu(int selected_block);
void draw_current_block(void);
void draw_hotbar(void);
void draw_crosshair(void);
void draw_liquid_overlay(void);
int gui_menu(int optcount, ...);
void gui_terrible_text_viewer(const char* text);

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

typedef void save_progress_callback(int progress, int max);
typedef struct {
	int16_t xsize, ysize, zsize;
	int32_t cam_x, cam_y, cam_z, cam_rx, cam_ry;
	uint8_t hotbar_blocks[HOTBAR_MAX];
	uint8_t hotbar_pos;
} level_info;

const char *save_get_error_string(int value);
int load_level(int save_id, level_info *info, uint8_t *target, int32_t target_size, save_progress_callback *pc);
int save_level(int save_id, level_info *info, const uint8_t *data, save_progress_callback *pc);

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
void world_update(uint32_t ticks);

// worldgen.c
typedef void worldgen_stage_callback(const char *message);
void world_generate(uint8_t *map, int32_t lx, int32_t ly, int32_t lz, uint32_t seed, worldgen_stage_callback *wc, save_progress_callback *pc);
