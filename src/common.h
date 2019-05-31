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

#define TV_PAL

typedef int32_t fixed;
typedef int64_t fixed64;
typedef fixed vec3[3];
typedef fixed vec4[4];
typedef vec4 mat4[4];

#define FM_PI ((fixed)0x00008000)

// Files
#define LEVEL_LX 64
#define LEVEL_LY 64
#define LEVEL_LZ 64
extern uint32_t atlas_raw[];
extern uint8_t fsys_level[LEVEL_LY][LEVEL_LZ][LEVEL_LX];

// block_info.h
#define BLOCK_MAX 50
#define QUAD_MAX 6
typedef struct block_info {
	uint16_t tc, tp, cl, pad0;
} block_info_t;

extern block_info_t block_info[BLOCK_MAX][QUAD_MAX];

// main.c
extern int32_t cam_x;
extern int32_t cam_y;
extern int32_t cam_z;

extern int32_t mat_rt11, mat_rt12, mat_rt13;
extern int32_t mat_rt21, mat_rt22, mat_rt23;
extern int32_t mat_rt31, mat_rt32, mat_rt33;

extern uint32_t dma_pos;
#define DMA_ORDER_MAX 64

extern uint32_t dma_buffer[256*512];
extern uint32_t dma_order_table[4][DMA_ORDER_MAX];
extern uint32_t dma_buffer_current;
#define OT_WORLD 2
#define DMA_PUSH(len, ot) \
	while(dma_pos*4 >= sizeof(dma_buffer)) {} \
	dma_buffer[dma_pos] = \
		(dma_order_table[dma_buffer_current][ot] & 0x00FFFFFF) \
		| ((len)<<24); \
	dma_order_table[dma_buffer_current][ot] = ((uint32_t)&dma_buffer[dma_pos])&0x00FFFFFF; \
	dma_pos++; \

extern int hotbar_pos;
#define HOTBAR_MAX 9
extern int current_block[HOTBAR_MAX];

void draw_block(int32_t cx, int32_t cy, int32_t cz, int di, int block, uint32_t facemask, bool transparent);

// gpu.c
extern volatile uint32_t vblank_counter;
void gp0_command(uint32_t v);
void gp0_data(uint32_t v);
void gp0_data_xy(uint32_t x, uint32_t y);
void gp1_command(uint32_t v);

// gui.c
void draw_current_block(void);
void draw_hotbar(void);
void draw_crosshair(void);
void draw_liquid_overlay(void);

// joy.c
extern volatile uint8_t joy_id;
extern volatile uint8_t joy_hid;
extern volatile uint16_t joy_buttons;
extern volatile uint8_t joy_axes[4];
extern volatile uint32_t joy_read_counter;
extern volatile uint8_t joy_has_ack;
extern volatile uint8_t joy_rumble0; // small
extern volatile uint8_t joy_rumble1; // large
void joy_unlock_dualshock(void);
void joy_do_read(void);

// world.c
uint8_t world_get_top_opaque(int32_t cx, int32_t cz);
uint32_t world_get_vis_blocks_unsafe(int32_t cx, int32_t cy, int32_t cz);
int32_t world_get_block_unsafe(int32_t cx, int32_t cy, int32_t cz);
int32_t world_get_render_faces_unsafe(int32_t cx, int32_t cy, int32_t cz);
int32_t world_get_block(int32_t cx, int32_t cy, int32_t cz);
void world_set_block(int32_t cx, int32_t cy, int32_t cz, uint8_t b, uint8_t flags);
bool world_cast_ray(int32_t px, int32_t py, int32_t pz, int32_t vx, int32_t vy, int32_t vz, int32_t *ocx, int32_t *ocy, int32_t *ocz, int32_t max_steps, bool use_block_before_hit);
uint32_t world_is_translucent(int32_t b);
uint32_t world_is_walkable(int32_t b);
int32_t world_is_colliding(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2);
int32_t world_is_colliding_fixed(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2);
void world_schedule_block_update(int32_t cx, int32_t cy, int32_t cz, uint32_t delay);
void world_init();
void world_update(uint32_t ticks);
