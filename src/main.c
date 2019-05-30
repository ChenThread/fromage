#include <chenboot.h>
#include "common.h"

/*

colouur multipliers for the keep guessing Nitori:
[-0.6, -0.5, -0.8]
[+0.6, +1.0, +0.8]

bounding box for Nitori
X,Z: 0.6
height: 1.8
eyes: 1.7

- first, then + ##### ZXY
*/

int main(void);
void yield(void);

extern volatile uint8_t _BSS_START[];
extern volatile uint8_t _BSS_END[];

extern void aaa_nop_sled_cache_clearer(void);

const int16_t sintab[256] = {
0,101,201,301,401,501,601,700,799,897,995,1092,1189,1285,1380,1474,1567,1660,1751,1842,1931,2019,2106,2191,2276,2359,2440,2520,2598,2675,2751,2824,2896,2967,3035,3102,3166,3229,3290,3349,3406,3461,3513,3564,3612,3659,3703,3745,3784,3822,3857,3889,3920,3948,3973,3996,4017,4036,4052,4065,4076,4085,4091,4095,4096,4095,4091,4085,4076,4065,4052,4036,4017,3996,3973,3948,3920,3889,3857,3822,3784,3745,3703,3659,3612,3564,3513,3461,3406,3349,3290,3229,3166,3102,3035,2967,2896,2824,2751,2675,2598,2520,2440,2359,2276,2191,2106,2019,1931,1842,1751,1660,1567,1474,1380,1285,1189,1092,995,897,799,700,601,501,401,301,201,101,0,-101,-201,-301,-401,-501,-601,-700,-799,-897,-995,-1092,-1189,-1285,-1380,-1474,-1567,-1660,-1751,-1842,-1931,-2019,-2106,-2191,-2276,-2359,-2440,-2520,-2598,-2675,-2751,-2824,-2896,-2967,-3035,-3102,-3166,-3229,-3290,-3349,-3406,-3461,-3513,-3564,-3612,-3659,-3703,-3745,-3784,-3822,-3857,-3889,-3920,-3948,-3973,-3996,-4017,-4036,-4052,-4065,-4076,-4085,-4091,-4095,-4096,-4095,-4091,-4085,-4076,-4065,-4052,-4036,-4017,-3996,-3973,-3948,-3920,-3889,-3857,-3822,-3784,-3745,-3703,-3659,-3612,-3564,-3513,-3461,-3406,-3349,-3290,-3229,-3166,-3102,-3035,-2967,-2896,-2824,-2751,-2675,-2598,-2520,-2440,-2359,-2276,-2191,-2106,-2019,-1931,-1842,-1751,-1660,-1567,-1474,-1380,-1285,-1189,-1092,-995,-897,-799,-700,-601,-501,-401,-301,-201,-101,
};

uint8_t fsys_level[LEVEL_LY][LEVEL_LZ][LEVEL_LX];

// Top, Side, Bottom, (reserved)
// Texcoord, TexPage, CLUT, (reserved)
uint8_t block_side_index[6] = {
	1, 1, 1, 1, 2, 0,
};
uint32_t block_lighting[6] = {
	0x010101*((0x80*8+5)/10), // -Z
	0x010101*((0x80*8+5)/10), // +Z
	0x010101*((0x80*6+5)/10), // -X
	0x010101*((0x80*6+5)/10), // +X
	0x010101*((0x80*5+5)/10), // -Y
	0x010101*((0x80*10+5)/10), // +Y
};

#include "block_info.h"

#define FACE_N 0x0000
#define FACE_P 0x0100
const int16_t mesh_data_block[] = {
	// -Z
	FACE_N, FACE_N, FACE_N, 0x0F00,
	FACE_P, FACE_N, FACE_N, 0x0F0F,
	FACE_N, FACE_P, FACE_N, 0x0000,
	FACE_P, FACE_P, FACE_N, 0x000F,

	// +Z
	FACE_N, FACE_N, FACE_P, 0x0F0F,
	FACE_N, FACE_P, FACE_P, 0x000F,
	FACE_P, FACE_N, FACE_P, 0x0F00,
	FACE_P, FACE_P, FACE_P, 0x0000,

	// -X
	FACE_N, FACE_N, FACE_N, 0x0F0F,
	FACE_N, FACE_P, FACE_N, 0x000F,
	FACE_N, FACE_N, FACE_P, 0x0F00,
	FACE_N, FACE_P, FACE_P, 0x0000,

	// +X
	FACE_P, FACE_N, FACE_N, 0x0F00,
	FACE_P, FACE_N, FACE_P, 0x0F0F,
	FACE_P, FACE_P, FACE_N, 0x0000,
	FACE_P, FACE_P, FACE_P, 0x000F,

	// -Y
	FACE_N, FACE_N, FACE_N, 0x0000,
	FACE_N, FACE_N, FACE_P, 0x0F00,
	FACE_P, FACE_N, FACE_N, 0x000F,
	FACE_P, FACE_N, FACE_P, 0x0F0F,

	// +Y
	FACE_N, FACE_P, FACE_N, 0x0000,
	FACE_P, FACE_P, FACE_N, 0x000F,
	FACE_N, FACE_P, FACE_P, 0x0F00,
	FACE_P, FACE_P, FACE_P, 0x0F0F,
};

const int16_t mesh_data_plant[] = {
	// TODO
};

void yield(void)
{
	// TODO: halt
}

extern char _end[];
void *_cur_brk = (void *)_end;
void __attribute__((externally_visible)) *sbrk (intptr_t incr)
{
	void *ret = _cur_brk;
	_cur_brk += incr;
	return ret;
}

int32_t mat_rt11, mat_rt12, mat_rt13;
int32_t mat_rt21, mat_rt22, mat_rt23;
int32_t mat_rt31, mat_rt32, mat_rt33;
int32_t mat_tr_x, mat_tr_y, mat_tr_z;
int32_t mat_hr11, mat_hr13;
int32_t mat_hr31, mat_hr33;

int32_t gte_ofx = 0x0000;
int32_t gte_ofy = 0x0000;
int32_t gte_zsf3 = 1;
int32_t gte_zsf4 = 1;

// FORMULA:
// gte_h = 120/tan(fov_angle/2.0);

//int32_t gte_h = 289; // 45 deg
//int32_t gte_h = 208; // 60 deg
//int32_t gte_h = 156; // 75 deg
int32_t gte_h = 120; // 90 deg
//int32_t gte_h = 69; // 120 deg
//int32_t gte_h = 50; // 135 deg
//int32_t gte_h = 32; // 150 deg

int32_t cam_ry = 0x0000;
int32_t cam_rx = 0x0000;
int32_t cam_x = 0x80 + (32<<8);
int32_t cam_y = 0x80 + (48<<8);
int32_t cam_z = 0x80 + (32<<8);
int32_t vel_x = 0;
int32_t vel_y = 0;
int32_t vel_z = 0;
bool use_dpad = true;

uint32_t ticks = 0;
uint32_t movement_ticks = 0;
uint32_t dma_pos = 0;
uint32_t dma_pos_start = 0;
uint32_t dma_start_ptr;

volatile uint32_t dma_next_start = 0;
uint32_t dma_buffer[256*512];
uint32_t dma_order_table[4][DMA_ORDER_MAX];
uint32_t dma_buffer_current = 0;
#define OT_WORLD 2

int joy_buttons_old = 0;
int current_block[HOTBAR_MAX] = {0, 1, 4, 45, 18, 4, 3, 20, 8};
int hotbar_pos = 0;

volatile uint32_t frame_x = 0;
volatile uint32_t frame_y = 0;
volatile uint32_t vis_frame_x = 0;
volatile uint32_t vis_frame_y = 0;
void frame_flip(void)
{
	frame_x = 320 - vis_frame_x;
}

// ISR handler

chenboot_exception_frame_t *isr_handler_c(chenboot_exception_frame_t *sp)
{
	// If it's not an interrupt, spin
	while((sp->cause & 0x3C) != 0x00) {
	}

	if((I_STAT & (1<<0)) != 0) {
		// VBLANK

		vblank_counter++;
		ticks++;

		//
		I_STAT = ~(1<<0);
		joy_has_ack++;
	}

	if((I_STAT & (1<<7)) != 0) {
		// JOY
		I_STAT = ~(1<<7);
		JOY_CTRL |= 0x0010; // ACK
		joy_has_ack++;
	}

	// Work around GTE bug
	if((sp->epc & 0xFE000000) == 0x4A000000) {
		sp->epc += 1; // skip op
	}

	return sp;
}

static inline bool is_face_lit(int32_t cx, int32_t cy, int32_t cz, uint32_t i) {
	switch (i) {
		case 0: return cz > 0 && world_get_top_opaque(cx, cz - 1) <= cy;
		case 2: return cx > 0 && world_get_top_opaque(cx - 1, cz) <= cy;
		case 1: return cz < LEVEL_LZ-1 && world_get_top_opaque(cx, cz + 1) <= cy;
		case 3: return cx < LEVEL_LX-1 && world_get_top_opaque(cx + 1, cz) <= cy;
		case 4: return world_get_top_opaque(cx, cz) < cy;
		case 5: return world_get_top_opaque(cx, cz) <= cy;
		default: return true;
	}
}

void draw_quads(int32_t cx, int32_t cy, int32_t cz, int di, const int16_t* mesh_data, const uint16_t bi[6][4], int face_count, uint32_t facemask, bool semitrans)
{
	int32_t ox = cx*0x0100;
	int32_t oy = cy*0x0100;
	int32_t oz = cz*0x0100;

	for (int i = 0; i < face_count; i++) {
		if(((1<<i)&facemask) == 0) {
			continue;
		}

		const uint16_t* block_data = bi[i];
		int mi = i << 4;

		int32_t x0 = ox+mesh_data[mi+0];
		int32_t y0 = oy+mesh_data[mi+1];
		int32_t z0 = oz+mesh_data[mi+2];
		int32_t t0 = mesh_data[mi+3]+block_data[0];
		int32_t cl = block_data[2];
		int32_t x1 = ox+mesh_data[mi+4];
		int32_t y1 = oy+mesh_data[mi+5];
		int32_t z1 = oz+mesh_data[mi+6];
		int32_t t1 = mesh_data[mi+7]+block_data[0];
		int32_t tp = block_data[1];
		int32_t x2 = ox+mesh_data[mi+8];
		int32_t y2 = oy+mesh_data[mi+9];
		int32_t z2 = oz+mesh_data[mi+10];
		int32_t t2 = mesh_data[mi+11]+block_data[0];
		int32_t x3 = ox+mesh_data[mi+12];
		int32_t y3 = oy+mesh_data[mi+13];
		int32_t z3 = oz+mesh_data[mi+14];
		int32_t t3 = mesh_data[mi+15]+block_data[0];

		int32_t xy0 = ((x0&0xFFFF)|(y0<<16));
		int32_t xy1 = ((x1&0xFFFF)|(y1<<16));
		int32_t xy2 = ((x2&0xFFFF)|(y2<<16));

		asm volatile ("mtc2 %0, $0\n" : "+r"(xy0) : : );
		asm volatile ("mtc2 %0, $1\n" : "+r"(z0) : : );
		asm volatile ("mtc2 %0, $2\n" : "+r"(xy1) : : );
		asm volatile ("mtc2 %0, $3\n" : "+r"(z1) : : );
		asm volatile ("mtc2 %0, $4\n" : "+r"(xy2) : : );
		asm volatile ("mtc2 %0, $5\n" : "+r"(z2) : : );

		// Apply transformation
		asm volatile ("cop2 0x00280030\n" :::); // RTPT

		uint32_t sxy0;
		uint32_t sxy1;
		uint32_t sxy2;
		asm volatile ("mfc2 %0, $12\nnop\n" : "=r"(sxy0) : : );
		asm volatile ("mfc2 %0, $13\nnop\n" : "=r"(sxy1) : : );
		asm volatile ("mfc2 %0, $14\nnop\n" : "=r"(sxy2) : : );

#if 0
		// Clamp oversize polys
		if(((int16_t)(sxy0>>16)) < -512) { continue; }
		if(((int16_t)(sxy0>>16)) > 512) { continue; }
		if(((int16_t)(sxy0&0xFFFF)) < -512) { continue; }
		if(((int16_t)(sxy0&0xFFFF)) > 512) { continue; }
		if(((int16_t)(sxy1>>16)) < -512) { continue; }
		if(((int16_t)(sxy1>>16)) > 512) { continue; }
		if(((int16_t)(sxy1&0xFFFF)) < -512) { continue; }
		if(((int16_t)(sxy1&0xFFFF)) > 512) { continue; }
		if(((int16_t)(sxy2>>16)) < -512) { continue; }
		if(((int16_t)(sxy2>>16)) > 512) { continue; }
		if(((int16_t)(sxy2&0xFFFF)) < -512) { continue; }
		if(((int16_t)(sxy2&0xFFFF)) > 512) { continue; }
#endif

#if 1
		// Back plane cull, flag edition
		int32_t gte_flags;
		asm volatile ("cfc2 %0, $31\nnop\n" : "=r"(gte_flags) : : );

		if((gte_flags & (1<<18)) != 0) {
			continue;
			//return;
		}
#endif

#if 0
		// Determine face
		asm volatile ("cop2 0x01400006\nnop\n" :::); // NCLIP
		int32_t backface_mac0;
		asm volatile ("mfc2 %0, $24\nnop\n" : "=r"(backface_mac0) : : );
		// Backface cull
		if(backface_mac0 < 2) { continue; }
#endif

		// Apply 4th point
		int32_t xy3 = ((x3&0xFFFF)|(y3<<16));
		asm volatile ("mtc2 %0, $0\n" : "+r"(xy3) : : );
		asm volatile ("mtc2 %0, $1\n" : "+r"(z3) : : );
		asm volatile ("nop\n");
		asm volatile ("cop2 0x00180001\nnop\n" :::); // RTPS
		asm volatile ("nop\n");
		asm volatile ("nop\n");
		asm volatile ("nop\n");
		asm volatile ("nop\n");
		asm volatile ("nop\n");
		uint32_t sxy3;
		asm volatile ("mfc2 %0, $14\nnop\n" : "=r"(sxy3) : : );

#if 0
		if(((int16_t)(sxy3>>16)) < -512) { continue; }
		if(((int16_t)(sxy3>>16)) > 512) { continue; }
		if(((int16_t)(sxy3&0xFFFF)) < -512) { continue; }
		if(((int16_t)(sxy3&0xFFFF)) > 512) { continue; }
#endif

#if 0
		// Back plane cull, Z edition
		int32_t average_z;
		//asm volatile ("cop2 0x0158002D\nnop\n" ::: ); // AVSZ3
		asm volatile ("cop2 0x0168002E\nnop\n" ::: ); // AVSZ4
		asm volatile ("mfc2 %0, $24\nnop\n" : "=r"(average_z) : : );

		if(average_z <= 4) {
			continue;
		}
#endif

		// Draw a quad
		DMA_PUSH(9, OT_WORLD + di);
		uint32_t lighting = block_lighting[i];
		if(!is_face_lit(cx, cy, cz, i)) {
			lighting >>= 1;
			lighting &= 0x7F7F7F;
		}
		if(semitrans) {
			dma_buffer[dma_pos++] = 0x2E000000 + (0x00FFFFFF&lighting);
		} else {
			dma_buffer[dma_pos++] = 0x2C000000 + (0x00FFFFFF&lighting);
		}
		dma_buffer[dma_pos++] = (sxy0);
		dma_buffer[dma_pos++] = (t0) | (cl<<16);
		dma_buffer[dma_pos++] = (sxy1);
		dma_buffer[dma_pos++] = (t1) | (tp<<16);
		dma_buffer[dma_pos++] = (sxy2);
		dma_buffer[dma_pos++] = (t2);
		dma_buffer[dma_pos++] = (sxy3);
		dma_buffer[dma_pos++] = (t3);
	}
}

static int get_model(int block) {
	return (block == 6) || (block >= 37 && block <= 40) ? 1 : 0;
}

void draw_block(int32_t cx, int32_t cy, int32_t cz, int di, int block, uint32_t facemask, bool transparent)
{
	if(block >= BLOCK_MAX || block < 0) {
		return;
	}

	if (get_model(block) == 1)
		draw_quads(cx, cy, cz, di, /* mesh_data_plant */mesh_data_block, block_info[block], 4, facemask, false);
	else
		draw_quads(cx, cy, cz, di, mesh_data_block, block_info[block], 6, facemask, transparent || ((block&(~1)) == 8));
}

static inline uint32_t should_render(int32_t b, int32_t nx, int32_t ny, int32_t nz) {
	int32_t nb = world_get_block_unsafe(nx, ny, nz);
	if (nb == 0) {
		return 1;
	} else if (b != nb) {
		return world_is_translucent(b) || world_is_translucent(nb);
	} else {
		return 0;
	}
}

static inline uint32_t calc_fmask(int32_t cx, int32_t cy, int32_t cz)
{
	int32_t b = world_get_block_unsafe(cx, cy, cz);
	if(b == 0) {
		return 0;
	}

	uint32_t fmask = 0;
	if(cz > 0 && should_render(b, cx, cy, cz-1)) {
		fmask |= 0x01;
	}
	if(cz < LEVEL_LZ-1 && should_render(b, cx, cy, cz+1)) {
		fmask |= 0x02;
	}
	if(cx > 0 && should_render(b, cx-1, cy, cz)) {
		fmask |= 0x04;
	}
	if(cx < LEVEL_LX-1 && should_render(b, cx+1, cy, cz)) {
		fmask |= 0x08;
	}
	if(cy > 0 && should_render(b, cx, cy-1, cz)) {
		fmask |= 0x10;
	}
	if(cy < LEVEL_LY-1 && should_render(b, cx, cy+1, cz)) {
		fmask |= 0x20;
	}

	return fmask;
}

inline void draw_block_in_level(int32_t cx, int32_t cy, int32_t cz, int32_t di, uint32_t nfmask)
{
	int block = world_get_block_unsafe(cx, cy, cz);
	if(block != 0) {
		uint32_t fmask = world_get_render_faces_unsafe(cx, cy, cz) & nfmask;
		if(fmask != 0) {
			draw_block(cx, cy, cz, di, block, fmask, false);
		}
	}
}

inline void draw_blocks_in_range(int32_t cam_cx, int32_t cam_cy, int32_t cam_cz, int32_t cd)
{
	int dymin = - cd;
	int dymax = + cd;
	int cymin = cam_cy + dymin;
	int cymax = cam_cy + dymax;
	if(cymin < 0) { dymin += -cymin; }
	if(cymax > LEVEL_LY) { dymax += LEVEL_LY-cymax; }
	for(int dy = dymin; dy <= dymax; dy++) {
		int cy = cam_cy + dy;
		//if(cy < 0 || cy >= LEVEL_LY) { continue; }
		int ady = (dy < 0 ? -dy : dy);
		uint32_t nfmask = 0;
		if(dy > 0) { nfmask |= 0x10; }
		if(dy < 0) { nfmask |= 0x20; }

		int dzmin = - (cd-ady);
		int dzmax = + (cd-ady);
		int czmin = cam_cz + dzmin;
		int czmax = cam_cz + dzmax;
		if(czmin < 0) { dzmin += -czmin; }
		if(czmax > LEVEL_LZ) { dzmax += LEVEL_LZ-1-czmax; }

	for(int dz = dzmin; dz <= dzmax; dz++) {
		int cz = cam_cz + dz;
		//if(cz < 0 || cz >= LEVEL_LZ) { continue; }
		int adz = (dz < 0 ? -dz : dz);
		nfmask &= ~0x03;
		if(dz > 0) { nfmask |= 0x01; }
		if(dz < 0) { nfmask |= 0x02; }

		int adx = cd-adz-ady;
		if(adx >= 0) {
			int cx1 = cam_cx - adx;
			int cx2 = cam_cx + adx;
			if(cx1 >= 0 && cx1 < LEVEL_LX) {
				draw_block_in_level(cx1, cy, cz, cd, nfmask|0x08);
			}
			if(cx2 != cx1 && cx2 >= 0 && cx2 < LEVEL_LX) {
				draw_block_in_level(cx2, cy, cz, cd, nfmask|0x04);
			}
		}
	}
	}
}

void draw_world(void)
{
	int cam_cx = cam_x >> 8;
	int cam_cy = cam_y >> 8;
	int cam_cz = cam_z >> 8;
	int cam_dist = 12;
	/*
	for(int cd = cam_dist; cd != 0; cd--) {
		draw_blocks_in_range(cam_cx, cam_cy, cam_cz, cd);
	}
	*/
	int cd = cam_dist;

	int cymin = cam_cy - cd;
	int cymax = cam_cy + cd;
	if(cymin < 0) { cymin = 0; }
	if(cymax >= LEVEL_LY) { cymax = LEVEL_LY-1; }

	int czmin = cam_cz - cd;
	int czmax = cam_cz + cd;
	if(czmin < 0) { czmin = 0; }
	if(czmax >= LEVEL_LZ) { czmax = LEVEL_LZ-1; }

	int cxmin = cam_cx - cd;
	int cxmax = cam_cx + cd;
	if(cxmin < 0) { cxmin = 0; }
	if(cxmax >= LEVEL_LX) { cxmax = LEVEL_LX-1; }

	cymin = (cymin+3)&~3;
	cymax = (cymax+0)&~3;
	czmin = (czmin+3)&~3;
	czmax = (czmax+0)&~3;
	cxmin = (cxmin+3)&~3;
	cxmax = (cxmax+0)&~3;

	int xrange = ((cxmax+4-cxmin)>>2);
	int yrange = ((cymax+4-cymin)>>2);
	int zrange = ((czmax+4-czmin)>>2);

	// Frustum cull planes
	int xmul = gte_h;
	int xdiv = 160;
	int ymul = gte_h;
	int ydiv = 120;
	int cull0xstep_raw = mat_rt31+mat_rt21*ymul/ydiv;
	int cull0ystep_raw = mat_rt32+mat_rt22*ymul/ydiv;
	int cull0zstep_raw = mat_rt33+mat_rt23*ymul/ydiv;
	int cull1xstep_raw = mat_rt31-mat_rt21*ymul/ydiv;
	int cull1ystep_raw = mat_rt32-mat_rt22*ymul/ydiv;
	int cull1zstep_raw = mat_rt33-mat_rt23*ymul/ydiv;
	int cull2xstep_raw = mat_rt31+mat_rt11*xmul/xdiv;
	int cull2ystep_raw = mat_rt32+mat_rt12*xmul/xdiv;
	int cull2zstep_raw = mat_rt33+mat_rt13*xmul/xdiv;
	int cull3xstep_raw = mat_rt31-mat_rt11*xmul/xdiv;
	int cull3ystep_raw = mat_rt32-mat_rt12*xmul/xdiv;
	int cull3zstep_raw = mat_rt33-mat_rt13*xmul/xdiv;

#define CULL_PLANE_SETUP(X) \
		int cull##X##xstep1 = cull##X##xstep_raw<<8; \
		int cull##X##ystep1 = cull##X##ystep_raw<<8; \
		int cull##X##zstep1 = cull##X##zstep_raw<<8; \
		int cull##X##xstep4 = cull##X##xstep1*4; \
		int cull##X##ystep4 = cull##X##ystep1*4; \
		int cull##X##zstep4 = cull##X##zstep1*4; \
		int cull##X##xmin = ((cxmin<<8)+0x200-cam_x)*cull##X##xstep_raw; \
		int cull##X##ymin = ((cymin<<8)+0x200-cam_y)*cull##X##ystep_raw; \
		int cull##X##zmin = ((czmin<<8)+0x200-cam_z)*cull##X##zstep_raw; \
 \
		int cull##X##yrange = yrange*cull##X##ystep4; \
		int bcull##X##yrange = 4*cull##X##ystep1; \
		cull##X##ystep4 -= zrange*cull##X##zstep4; \
		cull##X##zstep4 -= xrange*cull##X##xstep4; \
		cull##X##ystep1 -= 4*cull##X##zstep1; \
		cull##X##zstep1 -= 4*cull##X##xstep1; \
 \
		int cull##X = cull##X##ymin+cull##X##zmin+cull##X##xmin; \

	CULL_PLANE_SETUP(0)
	CULL_PLANE_SETUP(1)
	CULL_PLANE_SETUP(2)
	CULL_PLANE_SETUP(3)

	for(int iy = 0, cy = cymin, dy = cymin-cam_cy;
		iy < yrange; //cy < cymax+1;
		iy++, cy+=4, dy+=4
		,cull0 += cull0ystep4
		,cull1 += cull1ystep4
		,cull2 += cull2ystep4
		,cull3 += cull3ystep4
		) {
	for(int iz = 0, cz = czmin, dz = czmin-cam_cz;
		iz < zrange; //cz < czmax+1;
		iz++, cz+=4, dz+=4
		,cull0 += cull0zstep4
		,cull1 += cull1zstep4
		,cull2 += cull2zstep4
		,cull3 += cull3zstep4
		) {
	for(int ix = 0, cx = cxmin, dx = cxmin-cam_cx;
		ix < xrange; //cx < cxmax+1;
		ix++, cx+=4, dx+=4
		,cull0 += cull0xstep4
		,cull1 += cull1xstep4
		,cull2 += cull2xstep4
		,cull3 += cull3xstep4
		) {

		// Frustum culling
		if(cull0 < -(0x800<<12)) { continue; }
		if(cull1 < -(0x800<<12)) { continue; }
		if(cull2 < -(0x800<<12)) { continue; }
		if(cull3 < -(0x800<<12)) { continue; }

		uint32_t vismask = world_get_vis_blocks_unsafe(cx, cy, cz);
		if(vismask == 0) {
			continue;
		}

		int bcull0 = cull0;
		int bcull1 = cull1;
		int bcull2 = cull2;
		int bcull3 = cull3;

		for(int biy = 0, bcy = cy, bdy = dy;
			biy < 4;
			biy++, bcy++, bdy++
			,bcull0 += cull0ystep1
			,bcull1 += cull1ystep1
			,bcull2 += cull2ystep1
			,bcull3 += cull3ystep1
			) {

			uint32_t nfmask = 0;
			if(bdy > 0) { nfmask |= 0x10; }
			if(bdy < 0) { nfmask |= 0x20; }
			int ady = (bdy < 0 ? -bdy : bdy);
		for(int biz = 0, bcz = cz, bdz = dz;
			biz < 4;
			biz++, bcz++, bdz++
			, vismask>>=1
			,bcull0 += cull0zstep1
			,bcull1 += cull1zstep1
			,bcull2 += cull2zstep1
			,bcull3 += cull3zstep1
			) {

			if((vismask & 1) == 0) { continue; }
			nfmask &= ~0x03;
			if(bdz > 0) { nfmask |= 0x01; }
			if(bdz < 0) { nfmask |= 0x02; }
			int adz = (bdz < 0 ? -bdz : bdz);
		for(int bix = 0, bcx = cx, bdx = dx;
			bix < 4;
			bix++, bcx++, bdx++
			,bcull0 += cull0xstep1
			,bcull1 += cull1xstep1
			,bcull2 += cull2xstep1
			,bcull3 += cull3xstep1
			) {

			// Frustum culling
#if 0
			// CURRENTLY BROKEN.
			// Needs a special case for the (vismask&1)==0 case.
			// That is, add cullQstep4 to each bcullQ.
			// Do this in the vismask&1 check before the continue.
			if(bcull0 < -(0x200<<12)) { continue; }
			if(bcull1 < -(0x200<<12)) { continue; }
			if(bcull2 < -(0x200<<12)) { continue; }
			if(bcull3 < -(0x200<<12)) { continue; }
#endif
			nfmask &= ~0x0C;
			if(bdx > 0) { nfmask |= 0x04; }
			if(bdx < 0) { nfmask |= 0x08; }
			int adx = (bdx < 0 ? -bdx : bdx);
			draw_block_in_level(bcx, bcy, bcz, adx+ady+adz, nfmask);
		}
		}
		}
	}
	}
	}
}

static inline bool try_move(int32_t dx, int32_t dy, int32_t dz, bool do_move) {
	// camera pos is eye height
	int32_t ncx = cam_x + dx;
	int32_t ncy = cam_y + dy;
	int32_t ncz = cam_z + dz;

	if (!world_is_colliding_fixed(ncx - 77, ncy - 435, ncz - 77, ncx + 77, ncy + 26, ncz + 77)) {
		if(do_move) {
			cam_x = ncx;
			cam_y = ncy;
			cam_z = ncz;
		}
		return true;
	} else {
		return false;
	}
}

static void setup_matrix(bool with_y) {
	int yang = (cam_ry+0x80)>>8;
	int xang = with_y ? (cam_rx+0x80)>>8 : 0;
	int32_t rxc = sintab[(xang+0x40)&0xFF];
	int32_t rxs = sintab[(xang+0x00)&0xFF];
	int32_t ryc = sintab[(yang+0x40)&0xFF];
	int32_t rys = sintab[(yang+0x00)&0xFF];

	mat_rt11 =  ryc;
	mat_rt12 =  0x0000;
	mat_rt13 = -rys;
	mat_rt21 =  -((rys*rxs+0x800)>>12);
	mat_rt22 =  -(rxc);
	mat_rt23 =  -((ryc*rxs+0x800)>>12);
	mat_rt31 =  (rys*rxc+0x800)>>12;
	mat_rt32 = -rxs;
	mat_rt33 =  (ryc*rxc+0x800)>>12;

	mat_hr11 =  ryc;
	mat_hr13 = -rys;
	mat_hr31 =  rys;
	mat_hr33 =  ryc;
}

bool block_is_in_liquid(int cx, int cy, int cz)
{
	int32_t cb = world_get_block(cx, cy, cz);
	if (cb >= 8 && cb <= 11) {
		return true;
	} else {
		return false;
	}
}

bool player_is_in_liquid(void)
{
	int32_t cam_cx = cam_x >> 8;
	int32_t cam_cy = cam_y >> 8;
	int32_t cam_cz = cam_z >> 8;

	if(block_is_in_liquid(cam_cx, cam_cy-1, cam_cz)) {
		return true;
	} else if(block_is_in_liquid(cam_cx, cam_cy, cam_cz)) {
		return true;
	} else {
		return false;
	}
}

void draw_everything(void)
{
	// Set up GTE parameters
	asm volatile ("ctc2 %0, $24\nnop\n" : "+r"(gte_ofx) : : );
	asm volatile ("ctc2 %0, $25\nnop\n" : "+r"(gte_ofy) : : );
	asm volatile ("ctc2 %0, $26\nnop\n" : "+r"(gte_h) : : );
	asm volatile ("ctc2 %0, $29\nnop\n" : "+r"(gte_zsf3) : : );
	asm volatile ("ctc2 %0, $30\nnop\n" : "+r"(gte_zsf4) : : );

	// Set up GTE matrix
	setup_matrix(true);

	mat_tr_x = -(cam_x*mat_rt11 + cam_y*mat_rt12 + cam_z*mat_rt13 + 0x800)>>12;
	mat_tr_y = -(cam_x*mat_rt21 + cam_y*mat_rt22 + cam_z*mat_rt23 + 0x800)>>12;
	mat_tr_z = -(cam_x*mat_rt31 + cam_y*mat_rt32 + cam_z*mat_rt33 + 0x800)>>12;

	uint32_t mat_rtp1 = (mat_rt11&0xFFFF)|(mat_rt12<<16);
	uint32_t mat_rtp2 = (mat_rt13&0xFFFF)|(mat_rt21<<16);
	uint32_t mat_rtp3 = (mat_rt22&0xFFFF)|(mat_rt23<<16);
	uint32_t mat_rtp4 = (mat_rt31&0xFFFF)|(mat_rt32<<16);
	uint32_t mat_rtp5 = (mat_rt33);

	asm volatile ("ctc2 %0, $0\n" : "+r"(mat_rtp1) : : );
	asm volatile ("ctc2 %0, $1\n" : "+r"(mat_rtp2) : : );
	asm volatile ("ctc2 %0, $2\n" : "+r"(mat_rtp3) : : );
	asm volatile ("ctc2 %0, $3\n" : "+r"(mat_rtp4) : : );
	asm volatile ("ctc2 %0, $4\n" : "+r"(mat_rtp5) : : );
	asm volatile ("ctc2 %0, $5\n" : "+r"(mat_tr_x) : : );
	asm volatile ("ctc2 %0, $6\n" : "+r"(mat_tr_y) : : );
	asm volatile ("ctc2 %0, $7\n" : "+r"(mat_tr_z) : : );

	// Set up DMA buffer

	if(dma_pos_start >= sizeof(dma_buffer)*2/sizeof(dma_buffer[0])/3) {
		dma_pos = 0;
	}
	dma_pos_start = dma_pos;
	dma_buffer_current += 1;
	dma_buffer_current &= 3;
	dma_start_ptr = 0x00FFFFFF&(uint32_t)&dma_order_table[dma_buffer_current][DMA_ORDER_MAX-1];
	for(int i = 0; i < DMA_ORDER_MAX; i++) {
		dma_order_table[dma_buffer_current][i] = (i == 0
			? 0x00FFFFFF
			: ((uint32_t)&dma_order_table[dma_buffer_current][i-1])&0xFFFFFF);
	}

	// Clear screen
#if 1
	// Sky gradient
	DMA_PUSH(8, DMA_ORDER_MAX-1);
	dma_buffer[dma_pos++] = 0x38FFCB7F;
	dma_buffer[dma_pos++] = ((-160)&0xFFFF)|((-120)<<16);
	dma_buffer[dma_pos++] = 0x00FFCB7F;
	dma_buffer[dma_pos++] = ((+160)&0xFFFF)|((-120)<<16);
	dma_buffer[dma_pos++] = 0x00FFF0E1;
	dma_buffer[dma_pos++] = ((-160)&0xFFFF)|((+120)<<16);
	dma_buffer[dma_pos++] = 0x00FFF0E1;
	dma_buffer[dma_pos++] = ((+160)&0xFFFF)|((+120)<<16);
#else
	// Solid colour
	DMA_PUSH(4, DMA_ORDER_MAX-1);
	dma_buffer[dma_pos++] = 0x01000000;
	//dma_buffer[dma_pos++] = 0x02FFCF9F;
	dma_buffer[dma_pos++] = 0x02FFCB7F;
	dma_buffer[dma_pos++] = ((frame_x+0)&0xFFFF)|((frame_y+0)<<16); // X/Y
	dma_buffer[dma_pos++] = ((320)&0xFFFF)|((240)<<16); // W/H
#endif

	// Send VERY FIRST COMMANDS
	DMA_PUSH(3, DMA_ORDER_MAX-1);
	dma_buffer[dma_pos++] = 0xE3000000 | ((frame_x+0)<<0) | ((frame_y+0)<<10); // XY1 draw range
	dma_buffer[dma_pos++] = 0xE4000000 | ((frame_x+320-1)<<0) | ((frame_y+240-1)<<10); // XY2 draw range
	dma_buffer[dma_pos++] = 0xE5000000 | ((frame_x+320/2)<<0) | ((frame_y+240/2)<<11); // Draw offset

	// Load mesh data into GTE
	draw_world();

	int32_t cam_cx = cam_x >> 8;
	int32_t cam_cy = cam_y >> 8;
	int32_t cam_cz = cam_z >> 8;

	// Draw other things
	draw_current_block();
	draw_hotbar();
	draw_crosshair();
	draw_liquid_overlay();

	DMA_PUSH(1, 0);
	dma_buffer[dma_pos++] = 0x00000000;

	/*
	gp0_command(0x720000FF);
	gp0_data_xy(rx1-8/2, ry1-8/2);
	gp0_command(0x72FFFFFF);
	gp0_data_xy(rx0-8/2, ry0-8/2);
	*/

	// DMA the data
	if(dma_pos != dma_pos_start) {
		while((DMA_n_CHCR(2) & (1<<24)) != 0) {
			//
		}
		DMA_n_CHCR(2) = 0x00000401;
		DMA_DICR = 0;
		DMA_DPCR = 0x07654321;
		DMA_n_MADR(2) = dma_start_ptr;
		DMA_n_BCR(2)  = 0;
		DMA_n_CHCR(2) |= 0x01000000;
		DMA_DPCR |= (0x8<<(4*2)); // Enable DMA
	}
}

void player_update(int mmul)
{
	int jx0 = (int)(int8_t)(joy_axes[2]);
	int jy0 = (int)(int8_t)(joy_axes[3]);
	int jx1 = (int)(int8_t)(joy_axes[0]);
	int jy1 = (int)(int8_t)(joy_axes[1]);
	if (use_dpad == true) {
		jx0 = jy0 = 0;
		if ((joy_buttons & PAD_UP) == 0) jy0 = -0x7F;
		if ((joy_buttons & PAD_DOWN) == 0) jy0 = 0x7F;
		if ((joy_buttons & PAD_LEFT) == 0) jx0 = -0x7F;
		if ((joy_buttons & PAD_RIGHT) == 0) jx0 = 0x7F;
	}

	cam_ry += (jx1<<3) * mmul;
	cam_rx += (jy1<<3) * mmul;
	if (cam_ry < -0x8000) cam_ry += 0x10000;
	if (cam_ry > 0x8000) cam_ry -= 0x10000;
	if (cam_rx < -0x4000) cam_rx = -0x4000;
	if (cam_rx > 0x4000) cam_rx = 0x4000;

	int joy_pressed = (~joy_buttons_old) & ~joy_buttons;
	joy_buttons_old = ~joy_buttons;

	if ((joy_pressed & PAD_SELECT) != 0)
		use_dpad = !use_dpad;

	if ((joy_pressed & PAD_L2) != 0) {
		int32_t sel_cx = -1;
		int32_t sel_cy = -1;
		int32_t sel_cz = -1;
		bool sel_valid = world_cast_ray(
			cam_x, cam_y, cam_z,
			mat_rt31, mat_rt32, mat_rt33,
			&sel_cx, &sel_cy, &sel_cz,
			10, false);

		if (sel_valid && world_get_block(sel_cx, sel_cy, sel_cz) != 0) {
			int32_t bl = world_get_block(sel_cx, sel_cy, sel_cz);
			if (bl == 46) {
				for (int32_t cdx = -2; cdx <= 2; cdx++)
				for (int32_t cdy = -2; cdy <= 2; cdy++)
				for (int32_t cdz = -2; cdz <= 2; cdz++)
					world_set_block(sel_cx+cdx, sel_cy+cdy, sel_cz+cdz, 0, 1);
			} else {
				world_set_block(sel_cx, sel_cy, sel_cz, 0, 1);
			}
		}
	}

	if ((joy_pressed & PAD_O) != 0) {
		int32_t sel_cx = -1;
		int32_t sel_cy = -1;
		int32_t sel_cz = -1;
		bool sel_valid = world_cast_ray(
			cam_x, cam_y, cam_z,
			mat_rt31, mat_rt32, mat_rt33,
			&sel_cx, &sel_cy, &sel_cz,
			10, false);

		if (sel_valid) {
			int32_t sel_block = world_get_block(sel_cx, sel_cy, sel_cz);
			for (int i = 0; i <= HOTBAR_MAX; i++) {
				if (i == HOTBAR_MAX)
					current_block[hotbar_pos] = world_get_block(sel_cx, sel_cy, sel_cz);
				else if (current_block[i] == sel_block) {
					hotbar_pos = i;
					break;
				}

			}
		}
	}

	if ((joy_pressed & PAD_R2) != 0) {
		int32_t sel_cx = -1;
		int32_t sel_cy = -1;
		int32_t sel_cz = -1;
		bool sel_valid = world_cast_ray(
			cam_x, cam_y, cam_z,
			mat_rt31, mat_rt32, mat_rt33,
			&sel_cx, &sel_cy, &sel_cz,
			10, true);

		if (sel_valid && try_move(0, 0, 0, false)) {
			world_set_block(sel_cx, sel_cy, sel_cz, current_block[hotbar_pos], 1);
		}
	}

	if ((joy_pressed & PAD_T) != 0) current_block[hotbar_pos] = current_block[hotbar_pos] == 0 ? (BLOCK_MAX-1) : (current_block[hotbar_pos] - 1);
	if ((joy_pressed & PAD_S) != 0) current_block[hotbar_pos] = (current_block[hotbar_pos] + 1) % BLOCK_MAX;

	if ((joy_pressed & PAD_L1) != 0) { hotbar_pos--; if (hotbar_pos < 0) hotbar_pos = HOTBAR_MAX-1; }
	if ((joy_pressed & PAD_R1) != 0) hotbar_pos = (hotbar_pos + 1) % HOTBAR_MAX;

	setup_matrix(false);
	int32_t lvx = jx0 >> 1;
	int32_t lvy = 0;
	int32_t lvz = -(jy0 >> 1);

	if ((joy_buttons & PAD_R3) == 0) { } else { lvx >>= 1; lvz >>= 1; }

#if 0
	int32_t gvx = 0;
	int32_t gvy = 0;
	int32_t gvz = 0;
	gvx = (lvx*mat_rt11 + lvy*mat_rt21 + lvz*mat_rt31 + 0x800)>>12;
	gvy = (lvx*mat_rt12 + lvy*mat_rt22 + lvz*mat_rt32 + 0x800)>>12;
	gvz = (lvx*mat_rt13 + lvy*mat_rt23 + lvz*mat_rt33 + 0x800)>>12;
	cam_x += gvx;
	cam_y += gvy;
	cam_z += gvz;
#else
	int32_t gvx = 0;
	int32_t gvz = 0;
	gvx = (lvx*mat_hr11 + lvz*mat_hr31 + 0x800)>>12;
	gvz = (lvx*mat_hr13 + lvz*mat_hr33 + 0x800)>>12;
#if 1
	int acc_x = gvx;
	int acc_z = gvz;

	// Get normalised accel vector
	int nacc_x = acc_x;
	int nacc_z = acc_z;
	int nacc_len = 1;
	while(nacc_len*nacc_len < nacc_x*nacc_x + nacc_z*nacc_z) {
		nacc_len <<= 1;
	}
	nacc_len >>= 1;
	while(nacc_len*nacc_len < nacc_x*nacc_x + nacc_z*nacc_z) {
		nacc_len++;
	}
	nacc_len--;
	nacc_x = (nacc_x<<12) / nacc_len;
	nacc_z = (nacc_z<<12) / nacc_len;

	// Cap acceleration
	if(nacc_len > (0x1000>>6)) {
		acc_x = (nacc_x+(1<<5))>>6;
		acc_z = (nacc_z+(1<<5))>>6;
	}

	// Apply friction
	if (!try_move(0, -16, 0, false)) {
		vel_x -= ((vel_x+1)>>1) + (vel_x>>31);
		vel_z -= ((vel_z+1)>>1) + (vel_z>>31);
	}

	// Get speed cap
	int speed_cap_dot = vel_x*nacc_x + vel_z*nacc_z;

	int speed_cap = 0x100;
	if(speed_cap_dot < speed_cap*speed_cap) {
		vel_x += acc_x;
		vel_z += acc_z;
	}
#else
	vel_x = (vel_x + gvx) / 2;
	vel_z = (vel_z + gvz) / 2;
#endif

	bool in_liquid = player_is_in_liquid();

	for (int i = 0; i < mmul; i++) {
		if ((joy_buttons & PAD_X) == 0) {
			if (in_liquid || !try_move(0, -16, 0, false))
				vel_y = in_liquid ? 48 : 96;
		}
		if (try_move(0, vel_y, 0, true)) {
			if (vel_y > -192) vel_y -= ((ABS(vel_y) >> 4) + 4) >> (in_liquid ? 2 : 0);
			if (vel_y < -192) vel_y = -192;
		} else if (vel_y > 0) {
			vel_y = 0;
		} else {
			vel_y /= 2;
			while (!try_move(0, vel_y, 0, true)) {
				if(vel_y == 0) {
					while (vel_y < 128 && !try_move(0, vel_y, 0, false)) {
						vel_y += 4;
					}
					cam_y += (vel_y>>1)+1;
					vel_y = 0;
					break;
				}
				vel_y /= 2;
			}
		}
		while (vel_x != 0 && !try_move(vel_x, 0, 0, true)) { vel_x /= 2; }
		while (vel_z != 0 && !try_move(0, 0, vel_z, true)) { vel_z /= 2; }
	}
#endif
}

int main(void)
{
	int i;
	int x, y, xc, yc;

	// Disable DMA
	DMA_DPCR &= ~0x08888888;

	// Configure ISR
	chenboot_isr_disable();
	I_MASK = 0;
	I_STAT = 0;
	chenboot_isr_install(isr_handler_c);
	chenboot_isr_enable();

	// Reset GPU
	gp1_command(0x00000000); // Reset
	//gp1_command(0x04000001); // DMA mode: FIFO (1)
	gp1_command(0x04000002); // DMA mode: DMA to GPU (2)
	gp1_command(0x05000000 | ((0)<<0) | ((0)<<10)); // Display start (x,y)
	gp1_command(0x06000000 | ((0x260)<<0) | ((0x260+320*8)<<12)); // X screen range
#ifdef TV_PAL
	gp1_command(0x07000000 | ((0xA3-240/2)<<0) | ((0xA3+240/2)<<10)); // Y screen range
#else
	gp1_command(0x07000000 | ((0x88-240/2)<<0) | ((0x88+240/2)<<10)); // Y screen range
#endif

	// Set display mode
	gp1_command(0x08000000
		| (1<<0) | (0<<6) // X resolution (320)
		| (0<<2) | (0<<5) // Y resolution / interlace (240/OFF)
#ifdef TV_PAL
		| (1<<3) // TV standard (PAL)
#else
		| (0<<3) // TV standard (NTSC)
#endif
		| (0<<4) // Colour depth (15bpp)
	);

	// Set up drawing modes
	gp0_command(0xE1000600); // Texpage
	gp0_command(0xE2000000); // Texwindow
	gp0_command(0xE3000000 | ((0)<<0) | ((0)<<10)); // XY1 draw range
	gp0_command(0xE4000000 | ((320-1)<<0) | ((240-1)<<10)); // XY2 draw range
	gp0_command(0xE5000000 | ((320/2)<<0) | ((240/2)<<11)); // Draw offset
	//gp0_command(0xE5000000 | ((0)<<0) | ((0)<<10)); // Draw offset
	gp0_command(0xE6000000); // Mask bit setting

	// Clear screen
	gp0_command(0x01000000);
	gp0_command(0x02000000);
	gp0_data(((0)<<0) | ((0)<<16)); // X/Y
	gp0_data(((320)<<0) | ((240)<<16)); // W/H

	// Draw a rectangle
	gp0_command(0x60FFFFFF);
	gp0_data(((0)<<0) | ((0)<<16));
	gp0_data(((5)<<0) | ((5)<<16));

	// Enable display
	gp1_command(0x03000000); // Display enable: ON (1)

	// Enable VBLANK interrupt
	I_MASK |= (1<<0);

	// Prepare joypad
	JOY_CTRL = 0x0010;
	JOY_MODE = 0x000D;
	JOY_BAUD = 0x0088;

	// Enable joypad ISR
	I_STAT = ~(1<<7);
	I_MASK |= (1<<7);

	joy_unlock_dualshock();

	// Generate a texture
	for(int y = 0; y < 32; y++) {
		for(int x = 0; x < 32; x++) {
			uint32_t v = (x^y);
			v *= 0x421;
			v += 0x8000;
			//v = 0x7FFF;
			((uint16_t *)dma_buffer)[y*32+x] = v;
		}
	}

	// DMA a texture
#if 0
	// TODO: Actually use DMA
	gp1_command(0x04000001); // DMA mode: FIFO (1)
	gp0_command(0xA0000000);
	gp0_data_xy(0,256);
	gp0_data_xy(256/1,256);
	for(int i = 0; i < 256*256/2; i++) {
		gp0_data(atlas_raw[i]);
	}
	gp0_command(0x01000000);
	gp1_command(0x04000002); // DMA mode: DMA to GPU (2)
#else
	DMA_n_CHCR(2) = 1;
	DMA_DICR = 0;
	DMA_DPCR = 0x07654321;
	DMA_n_MADR(2) = ((uint32_t)atlas_raw)&0x00FFFFFF;
	DMA_n_BCR(2)  = ((320*(256/8))<<13)|0x08;
	gp0_command(0xA0000000);
	gp0_data_xy(0,256);
	gp0_data_xy(320/4,256);
	DMA_DPCR |= (0x8<<(4*2)); // Enable DMA
	DMA_n_CHCR(2) = 0x01000201;
	while((DMA_n_CHCR(2) & (1<<24)) != 0) {
		//
	}
	//DMA_n_CHCR(2) = 1;
	//gp0_command(0x01000000);
#endif

        // Generate a world
	for(int y = 0; y < LEVEL_LY/2; y++) {
		uint8_t bid = 1;
		if (y == (LEVEL_LY/2) - 1) bid = 2;
		else if (y >= (LEVEL_LY/2) - 4) bid = 3;
		for (int z = 0; z < LEVEL_LZ; z++)
		for (int x = 0; x < LEVEL_LX; x++) {
			fsys_level[y][z][x] = bid;
		}
	}

	world_init();

	ticks = movement_ticks = 0;
	vblank_counter = 0;
	for(;;) {
		yield();

		if(vblank_counter > 0) {
			uint32_t frames_to_run = vblank_counter;
			//vblank_counter -= frames_to_run;

			draw_everything();

			// count the vblanks and reset counter
			vblank_counter = 0;
			int mmul = ticks - movement_ticks;
			movement_ticks = ticks;

			player_update(mmul);

			world_update(ticks);
			// FIXME: if vsync is disabled,
			// joypad reads occasionally glitch
			vis_frame_x = frame_x;
			vis_frame_y = frame_y;
			frame_flip();
			while((DMA_n_CHCR(2) & (1<<24)) != 0) {
				//
			}
			while(vblank_counter == 0) {}
			gp1_command(0x05000000 | ((vis_frame_x)<<0) | ((vis_frame_y)<<10)); // Display start (x,y)
			joy_do_read();
		}
	}

	return 0;
}

