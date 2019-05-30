#include "common.h"

extern uint32_t block_lighting[6];

void draw_current_block(void)
{
	int32_t cam_cx = cam_x >> 8;
	int32_t cam_cy = cam_y >> 8;
	int32_t cam_cz = cam_z >> 8;

	if (current_block[hotbar_pos] > 0)
	{
		int32_t sel_cx = -1;
		int32_t sel_cy = -1;
		int32_t sel_cz = -1;
		bool sel_valid = world_cast_ray(
			cam_x, cam_y, cam_z,
			mat_rt31, mat_rt32, mat_rt33,
			&sel_cx, &sel_cy, &sel_cz,
			10, true);

		if(sel_valid) {
			int32_t fmask = 0x00;
			fmask |= (cam_cx < sel_cx ? 0x04 : cam_cx > sel_cx ? 0x08 : 0);
			fmask |= (cam_cy < sel_cy ? 0x10 : cam_cy > sel_cy ? 0x20 : 0);
			fmask |= (cam_cz < sel_cz ? 0x01 : cam_cz > sel_cz ? 0x02 : 0);
			int di = ABS(sel_cx - cam_cx) + ABS(sel_cy - cam_cy) + ABS(sel_cz - cam_cz);
			draw_block(sel_cx, sel_cy, sel_cz, di, current_block[hotbar_pos], fmask, true);
		}
	}
}

void draw_hotbar(void)
{
	const int bw = 30;
	const int bh = 30;
	const int by = 103;

	for (int i = 0; i < HOTBAR_MAX; i++) {
		int bx = -((bw*(HOTBAR_MAX-1))/2) + (i*bw);
		uint16_t *bi0 = block_info[current_block[i]][0];
		uint16_t *bi1 = block_info[current_block[i]][2];

		if (current_block[i] == 6 || (current_block[i] >= 37 && current_block[i] <= 40)) {
			// Flat texture
			DMA_PUSH(9, 1);
			dma_buffer[dma_pos++] = 0x2C808080;
			dma_buffer[dma_pos++] = ((by-16) << 16) | ((bx-16) & 0xFFFF);
			dma_buffer[dma_pos++] = (bi0[2] << 16) | (bi0[0] + (0x0000));
			dma_buffer[dma_pos++] = ((by-16) << 16) | ((bx+16) & 0xFFFF);
			dma_buffer[dma_pos++] = (bi0[1] << 16) | (bi0[0] + (0x000F));
			dma_buffer[dma_pos++] = ((by+16) << 16) | ((bx-16) & 0xFFFF);
			dma_buffer[dma_pos++] = (0 << 16) | (bi0[0] + (0x0F00));
			dma_buffer[dma_pos++] = ((by+16) << 16) | ((bx+16) & 0xFFFF);
			dma_buffer[dma_pos++] = (0 << 16) | (bi0[0] + (0x0F0F));
		} else if (current_block[i] > 0 && current_block[i] < BLOCK_MAX) {
			// Isometric cube
			// Positions:
			//
			// left, centre, right
			// up, down
			//     cu
			//  lu    ru
			//     cc
			//  ld    rd
			//     cd
			uint32_t lu = ((by-8) << 16) | ((bx-14) & 0xFFFF);
			uint32_t ld = ((by+8) << 16) | ((bx-14) & 0xFFFF);
			uint32_t cu = ((by-16) << 16) | ((bx) & 0xFFFF);
			uint32_t cc = ((by+0) << 16) | ((bx) & 0xFFFF);
			uint32_t cd = ((by+16) << 16) | ((bx) & 0xFFFF);
			uint32_t ru = ((by-8) << 16) | ((bx+14) & 0xFFFF);
			uint32_t rd = ((by+8) << 16) | ((bx+14) & 0xFFFF);
			DMA_PUSH(9, 1);
			dma_buffer[dma_pos++] = 0x2C000000 | block_lighting[5];
			dma_buffer[dma_pos++] = lu;
			dma_buffer[dma_pos++] = (bi0[2] << 16) | (bi0[0] + (0x0000));
			dma_buffer[dma_pos++] = cu;
			dma_buffer[dma_pos++] = (bi0[1] << 16) | (bi0[0] + (0x000F));
			dma_buffer[dma_pos++] = cc;
			dma_buffer[dma_pos++] = (0 << 16) | (bi0[0] + (0x0F00));
			dma_buffer[dma_pos++] = ru;
			dma_buffer[dma_pos++] = (0 << 16) | (bi0[0] + (0x0F0F));

			DMA_PUSH(9, 1);
			dma_buffer[dma_pos++] = 0x2C000000 | block_lighting[0];
			dma_buffer[dma_pos++] = lu;
			dma_buffer[dma_pos++] = (bi1[2] << 16) | (bi1[0] + (0x0000));
			dma_buffer[dma_pos++] = cc;
			dma_buffer[dma_pos++] = (bi1[1] << 16) | (bi1[0] + (0x000F));
			dma_buffer[dma_pos++] = ld;
			dma_buffer[dma_pos++] = (0 << 16) | (bi0[0] + (0x0F00));
			dma_buffer[dma_pos++] = cd;
			dma_buffer[dma_pos++] = (0 << 16) | (bi0[0] + (0x0F0F));

			DMA_PUSH(9, 1);
			dma_buffer[dma_pos++] = 0x2C000000 | block_lighting[2];
			dma_buffer[dma_pos++] = cc;
			dma_buffer[dma_pos++] = (bi1[2] << 16) | (bi1[0] + (0x0000));
			dma_buffer[dma_pos++] = ru;
			dma_buffer[dma_pos++] = (bi1[1] << 16) | (bi1[0] + (0x000F));
			dma_buffer[dma_pos++] = cd;
			dma_buffer[dma_pos++] = (0 << 16) | (bi0[0] + (0x0F00));
			dma_buffer[dma_pos++] = rd;
			dma_buffer[dma_pos++] = (0 << 16) | (bi0[0] + (0x0F0F));
		}
	}

	// Box around current hotbar item
	{
		int bx = -((bw*(HOTBAR_MAX-1))/2) + (hotbar_pos*bw);
		DMA_PUSH(3, 1);
		dma_buffer[dma_pos++] = 0x60080808;
		dma_buffer[dma_pos++] = ((by-(bh/2)+1) << 16) | ((bx-(bw/2)+1) & 0xFFFF);
		dma_buffer[dma_pos++] = ((bh-2) << 16) | ((bw-2) & 0xFFFF);
		DMA_PUSH(3, 1);
		dma_buffer[dma_pos++] = 0x60C0C0C0;
		dma_buffer[dma_pos++] = ((by-(bh/2)-1) << 16) | ((bx-(bw/2)-1) & 0xFFFF);
		dma_buffer[dma_pos++] = ((bh+2) << 16) | ((bw+2) & 0xFFFF);
	}

	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x60202020;
	dma_buffer[dma_pos++] = ((by-bh/2) << 16) | ((0 - (bw*HOTBAR_MAX)/2) & 0xFFFF);
	dma_buffer[dma_pos++] = (bh << 16) | ((bw*HOTBAR_MAX) & 0xFFFF);
}

void draw_crosshair(void)
{
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x60FFFFFF;
	dma_buffer[dma_pos++] = (((-3) & 0xFFFF) << 16) | ((0) & 0xFFFF);
	dma_buffer[dma_pos++] = (7 << 16) | 1;
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x60FFFFFF;
	dma_buffer[dma_pos++] = (((0) & 0xFFFF) << 16) | ((-3) & 0xFFFF);
	dma_buffer[dma_pos++] = (1 << 16) | 7;
}

void draw_liquid_overlay(void)
{
	int32_t cam_cx = cam_x >> 8;
	int32_t cam_cy = cam_y >> 8;
	int32_t cam_cz = cam_z >> 8;

	int32_t cam_cb = world_get_block(cam_cx, cam_cy, cam_cz);

	if ((cam_cb & (~1)) == 8) {
		DMA_PUSH(6, 1);
		for (int i = 0; i < 2; i++) {
			dma_buffer[dma_pos++] = 0x62501000;
			dma_buffer[dma_pos++] = (((-120)&0xFFFF) << 16) | ((-160)&0xFFFF);
			dma_buffer[dma_pos++] = (240 << 16) | (320 << 0);
		}
	} else if ((cam_cb & (~1)) == 10) {
		DMA_PUSH(9, 1);
		for (int i = 0; i < 3; i++) {
			dma_buffer[dma_pos++] = 0x62081CB0;
			dma_buffer[dma_pos++] = (((-120)&0xFFFF) << 16) | ((-160)&0xFFFF);
			dma_buffer[dma_pos++] = (240 << 16) | (320 << 0);
		}
	}
}

