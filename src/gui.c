#include "common.h"

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
	for (int i = 0; i < HOTBAR_MAX; i++) {
		int bx = 8 + (-10*HOTBAR_MAX) + (i*20);
		int by = 108;
		if (current_block[i] > 0 && current_block[i] < BLOCK_MAX) {
			DMA_PUSH(3, 1);
			uint16_t* bi = block_info[current_block[i]][2];
			dma_buffer[dma_pos++] = 0x7D000000;
			dma_buffer[dma_pos++] = ((by-8) << 16) | ((bx-8) & 0xFFFF);
			dma_buffer[dma_pos++] = (bi[2] << 16) | bi[0];
		}
		if (i == hotbar_pos) {
			DMA_PUSH(3, 1);
			dma_buffer[dma_pos++] = 0x60080808;
			dma_buffer[dma_pos++] = ((by-9) << 16) | ((bx-9) & 0xFFFF);
			dma_buffer[dma_pos++] = (18 << 16) | (18 << 0);
			DMA_PUSH(3, 1);
			dma_buffer[dma_pos++] = 0x60C0C0C0;
			dma_buffer[dma_pos++] = ((by-11) << 16) | ((bx-11) & 0xFFFF);
			dma_buffer[dma_pos++] = (22 << 16) | (22 << 0);
		}
	}

	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x60202020;
	dma_buffer[dma_pos++] = (98 << 16) | ((0 - 10*HOTBAR_MAX) & 0xFFFF);
	dma_buffer[dma_pos++] = (20 << 16) | ((20*HOTBAR_MAX) << 0);
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

