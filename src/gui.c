#include <stdarg.h>
#include <stdio.h>
#include "common.h"

extern uint32_t block_lighting[6];

#define CHAR_WIDTH(c) ((font_raw[(uint8_t)(c) >> 1] >> (((uint8_t)(c) & 1) << 2)) & 0x0F)

static int get_text_width_buffer(char *buffer)
{
	int len = 0;
	for (size_t i = 0; i < strlen(buffer); i++) {
		len += (CHAR_WIDTH(buffer[i])) + 1;
	}
	return len;
}

int get_text_width(char *format, ...)
{
	char buffer[256];
	int len;

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);

	len = get_text_width_buffer(buffer);

	va_end(args);
	return len * VID_WIDTH_MULTIPLIER;
}

static void draw_text_buffer(int x, int y, int color, char *buffer)
{
	x -= VID_WIDTH/2;
	y -= VID_HEIGHT/2;

	// TODO: Couldn't figure out how to set TexPage with E1
	// and have it work... maybe we can't just keep using one buffer

#if VID_WIDTH_MULTIPLIER == 1
	// Undo texpage
	DMA_PUSH(1, 1);
	dma_buffer[dma_pos++] = 0x00000000;
	DMA_PUSH(1, 1);
	dma_buffer[dma_pos++] = 0xE1000600;
#endif

	for (size_t i = 0; i < strlen(buffer); i++) {
		uint8_t c = buffer[i];
		int width = CHAR_WIDTH(c);
		int pos = (x & 0xFFFF) + ((y & 0xFFFF) << 16);
		int texcoord = (((c & 15) << 3) + 64) | ((c >> 4) << 11);

#if VID_WIDTH_MULTIPLIER == 1
		DMA_PUSH(4, 1);
		dma_buffer[dma_pos++] = 0x64000000 | color;
		dma_buffer[dma_pos++] = pos;
		dma_buffer[dma_pos++] = (384 << 22) | (0x35 << 16) | texcoord;
		dma_buffer[dma_pos++] = (8 << 16) | (width & 0xFFFF);
		//dma_buffer[dma_pos++] = (0x1D << 16) | (texcoord + width);

		pos += 0x10001;

		DMA_PUSH(4, 1);
		dma_buffer[dma_pos++] = 0x66000000;
		dma_buffer[dma_pos++] = pos;
		dma_buffer[dma_pos++] = (384 << 22) | (0x35 << 16) | texcoord;
		dma_buffer[dma_pos++] = (8 << 16) | (width & 0xFFFF);
		dma_buffer[dma_pos++] = (384 << 22) | (0x35 << 16) | texcoord;

#else
		DMA_PUSH(9, 1);
		dma_buffer[dma_pos++] = 0x2C000000 | color;
		dma_buffer[dma_pos++] = pos;
		dma_buffer[dma_pos++] = (384 << 22) | (0x35 << 16) | texcoord;
		dma_buffer[dma_pos++] = pos + width * VID_WIDTH_MULTIPLIER + 1;
		dma_buffer[dma_pos++] = (0x1D << 16) | (texcoord + width);
		dma_buffer[dma_pos++] = pos + 0x80000;
		dma_buffer[dma_pos++] = texcoord + 0x800;
		dma_buffer[dma_pos++] = pos + 0x80000 + width * VID_WIDTH_MULTIPLIER + 1;
		dma_buffer[dma_pos++] = texcoord + 0x800 + width;

		pos += 0x10001;

		DMA_PUSH(9, 1);
		dma_buffer[dma_pos++] = 0x2E000000;
		dma_buffer[dma_pos++] = pos;
		dma_buffer[dma_pos++] = (384 << 22) | (0x35 << 16) | texcoord;
		dma_buffer[dma_pos++] = pos + width * VID_WIDTH_MULTIPLIER + 1;
		dma_buffer[dma_pos++] = (0x1D << 16) | (texcoord + width);
		dma_buffer[dma_pos++] = pos + 0x80000;
		dma_buffer[dma_pos++] = texcoord + 0x800;
		dma_buffer[dma_pos++] = pos + 0x80000 + width * VID_WIDTH_MULTIPLIER + 1;
		dma_buffer[dma_pos++] = texcoord + 0x800 + width;
#endif

		x += (width + 1) * VID_WIDTH_MULTIPLIER;
	}

#if VID_WIDTH_MULTIPLIER == 1
	// Do texpage
	DMA_PUSH(1, 1);
	dma_buffer[dma_pos++] = 0x00000000;
	DMA_PUSH(1, 1);
	dma_buffer[dma_pos++] = 0xE100061D;
#endif

}

void draw_text(int x, int y, int color, char *format, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);

	draw_text_buffer(x, y, color, buffer);

	va_end(args);
}

static void draw_block_icon_flat(int bx, int by, int bw, int bh, block_info_t *bi1)
{
	int bwh = bw/2;
	int bhh = bh/2;

	// Flat texture
	DMA_PUSH(9, 1);
	dma_buffer[dma_pos++] = 0x2C808080;
	dma_buffer[dma_pos++] = ((by-bhh) << 16) | ((bx-bwh) & 0xFFFF);
	dma_buffer[dma_pos++] = (bi1->cl << 16) | (bi1->tc + (0x0000));
	dma_buffer[dma_pos++] = ((by-bhh) << 16) | ((bx+bwh) & 0xFFFF);
	dma_buffer[dma_pos++] = (bi1->tp << 16) | (bi1->tc + (0x000F));
	dma_buffer[dma_pos++] = ((by+bhh) << 16) | ((bx-bwh) & 0xFFFF);
	dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (0x0F00));
	dma_buffer[dma_pos++] = ((by+bhh) << 16) | ((bx+bwh) & 0xFFFF);
	dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (0x0F0F));
}

void draw_block_icon(int bx, int by, int bw, int bh, int block_id)
{
	block_info_t *bi0 = &block_info[block_id][5];
	block_info_t *bi1 = &block_info[block_id][2];

	if (block_id == 6 || (block_id >= 37 && block_id <= 40)) {
		draw_block_icon_flat(bx, by, bw, bh, bi1);
	} else if (block_id > 0 && block_id < BLOCK_MAX) {
		int bh8 = 8 * bh / 32;
		int bw14 = 14 * bw / 32;
		int bh16 = 16 * bh / 32;

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
		uint32_t lu = ((by-bh8) << 16) | ((bx-bw14) & 0xFFFF);
		uint32_t ld = ((by+bh8) << 16) | ((bx-bw14) & 0xFFFF);
		uint32_t cu = ((by-bh16) << 16) | ((bx) & 0xFFFF);
		uint32_t cc = ((by+0) << 16) | ((bx) & 0xFFFF);
		uint32_t cd = ((by+bh16) << 16) | ((bx) & 0xFFFF);
		uint32_t ru = ((by-bh8) << 16) | ((bx+bw14) & 0xFFFF);
		uint32_t rd = ((by+bh8) << 16) | ((bx+bw14) & 0xFFFF);
		DMA_PUSH(9, 1);
		dma_buffer[dma_pos++] = 0x2C000000 | block_lighting[5];
		dma_buffer[dma_pos++] = lu;
		dma_buffer[dma_pos++] = (bi0->cl << 16) | (bi0->tc + (0x0000));
		dma_buffer[dma_pos++] = cu;
		dma_buffer[dma_pos++] = (bi0->tp << 16) | (bi0->tc + (0x000F));
		dma_buffer[dma_pos++] = cc;
		dma_buffer[dma_pos++] = (0 << 16) | (bi0->tc + (0x0F00));
		dma_buffer[dma_pos++] = ru;
		dma_buffer[dma_pos++] = (0 << 16) | (bi0->tc + (0x0F0F));

		DMA_PUSH(9, 1);
		dma_buffer[dma_pos++] = 0x2C000000 | block_lighting[0];
		dma_buffer[dma_pos++] = lu;
		dma_buffer[dma_pos++] = (bi1->cl << 16) | (bi1->tc + (0x0000));
		dma_buffer[dma_pos++] = cc;
		dma_buffer[dma_pos++] = (bi1->tp << 16) | (bi1->tc + (0x000F));
		dma_buffer[dma_pos++] = ld;
		dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (0x0F00));
		dma_buffer[dma_pos++] = cd;
		dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (0x0F0F));

		DMA_PUSH(9, 1);
		dma_buffer[dma_pos++] = 0x2C000000 | block_lighting[2];
		dma_buffer[dma_pos++] = cc;
		dma_buffer[dma_pos++] = (bi1->cl << 16) | (bi1->tc + (0x0000));
		dma_buffer[dma_pos++] = ru;
		dma_buffer[dma_pos++] = (bi1->tp << 16) | (bi1->tc + (0x000F));
		dma_buffer[dma_pos++] = cd;
		dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (0x0F00));
		dma_buffer[dma_pos++] = rd;
		dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (0x0F0F));
	}
}

void draw_status_window(char *format, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);

	// Draw text
	int width = get_text_width_buffer(buffer);
	draw_text_buffer((VID_WIDTH - width) / 2, (VID_HEIGHT - 8) / 2, 0xFFFFFF, buffer);

	block_info_t *bi = &block_info[3][5];

	// Draw fancy dirt background
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x62000000;
	dma_buffer[dma_pos++] = (-(VID_HEIGHT/2) << 16) | (-(VID_WIDTH/2) & 0xFFFF);
	dma_buffer[dma_pos++] = ((VID_HEIGHT) << 16) | ((VID_WIDTH) & 0xFFFF);
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x62000000;
	dma_buffer[dma_pos++] = (-(VID_HEIGHT/2) << 16) | (-(VID_WIDTH/2) & 0xFFFF);
	dma_buffer[dma_pos++] = ((VID_HEIGHT) << 16) | ((VID_WIDTH) & 0xFFFF);

	for (int y = 0; y < VID_HEIGHT; y += 16) {
		for (int x = 0; x < VID_WIDTH; x += 16 * VID_WIDTH_MULTIPLIER) {
			draw_block_icon_flat(
				x - (VID_WIDTH/2) + 8 * VID_WIDTH_MULTIPLIER,
				y - (VID_HEIGHT/2) + 8,
				16 * VID_WIDTH_MULTIPLIER,
				16,
				bi);
		}
	}

	va_end(args);
}

void draw_block_sel_menu(int selected_block)
{
	// 34x34 slots
	int bg_w = ((25 * 9) + 18 + 4) * VID_WIDTH_MULTIPLIER;
	int bg_h = 141 + 18 + 13;

	int y = 0;
	int x = 0;

	// blocks
	for (int id = 1; id <= 49; id++) {
		int x_center = (-bg_w/2) + 2 + 9;
		int y_center = (-bg_h/2) + 12 + 9;

		x_center += (25 * VID_WIDTH_MULTIPLIER * x) + 16 * VID_WIDTH_MULTIPLIER;
		y_center += (25 * y) + 8;

		if (id == selected_block) {
			draw_block_icon(x_center, y_center, 32 * VID_WIDTH_MULTIPLIER, 32, id);

			// background
			DMA_PUSH(3, 1);
			dma_buffer[dma_pos++] = 0x62FFFFFF;
			dma_buffer[dma_pos++] = (
				(y_center - 16) << 16) |
				((x_center - 16 * VID_WIDTH_MULTIPLIER) & 0xFFFF);
			dma_buffer[dma_pos++] = ((32) << 16) | ((32 * VID_WIDTH_MULTIPLIER) & 0xFFFF);
		} else {
			draw_block_icon(x_center, y_center, 16 * VID_WIDTH_MULTIPLIER, 16, id);
		}

		if ((++x) == 9) { y++; x = 0; }
	}

	// text
	int text_w = get_text_width("Select block");
	draw_text((VID_WIDTH - text_w) / 2, ((VID_HEIGHT - bg_h) / 2) + 2, 0xFFFFFF, "Select block");

	// background
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x62080808;
	dma_buffer[dma_pos++] = ((-bg_h/2) << 16) | ((-bg_w/2) & 0xFFFF);
	dma_buffer[dma_pos++] = ((bg_h) << 16) | ((bg_w) & 0xFFFF);
}

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
	const int bw = 16 * VID_WIDTH_MULTIPLIER;
	const int bh = 16;
	const int by = 103 + 8;

	{
		int bx = -((bw*(HOTBAR_MAX-1))/2) + (hotbar_pos*bw);
		DMA_PUSH(3, 1);
		dma_buffer[dma_pos++] = 0x60C0C0C0;
		dma_buffer[dma_pos++] = ((by-(bh/2)-1) << 16) | ((bx-(bw/2)-1) & 0xFFFF);
		dma_buffer[dma_pos++] = ((2) << 16) | ((bw+2) & 0xFFFF);
		DMA_PUSH(3, 1);
		dma_buffer[dma_pos++] = 0x60C0C0C0;
		dma_buffer[dma_pos++] = ((by+(bh/2)-1) << 16) | ((bx-(bw/2)-1) & 0xFFFF);
		dma_buffer[dma_pos++] = ((2) << 16) | ((bw+2) & 0xFFFF);
		DMA_PUSH(3, 1);
		dma_buffer[dma_pos++] = 0x60C0C0C0;
		dma_buffer[dma_pos++] = ((by-(bh/2)-1) << 16) | ((bx-(bw/2)-2) & 0xFFFF);
		dma_buffer[dma_pos++] = ((bh+2) << 16) | ((4) & 0xFFFF);
		DMA_PUSH(3, 1);
		dma_buffer[dma_pos++] = 0x60C0C0C0;
		dma_buffer[dma_pos++] = ((by-(bh/2)-1) << 16) | ((bx+(bw/2)-2) & 0xFFFF);
		dma_buffer[dma_pos++] = ((bh+2) << 16) | ((4) & 0xFFFF);
	}

	for (int i = 0; i < HOTBAR_MAX; i++) {
		int bx = -((bw*(HOTBAR_MAX-1))/2) + (i*bw);
		draw_block_icon(bx, by, bw, bh, current_block[i]);
	}

	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x62000000;
	dma_buffer[dma_pos++] = ((by-bh/2) << 16) | ((0 - (bw*HOTBAR_MAX)/2) & 0xFFFF);
	dma_buffer[dma_pos++] = (bh << 16) | ((bw*HOTBAR_MAX) & 0xFFFF);
}

void draw_crosshair(void)
{
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x60FFFFFF;
	dma_buffer[dma_pos++] = (((-3) & 0xFFFF) << 16) | ((0) & 0xFFFF);
	dma_buffer[dma_pos++] = (7 << 16) | (1 * VID_WIDTH_MULTIPLIER);
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x60FFFFFF;
	dma_buffer[dma_pos++] = (((0) & 0xFFFF) << 16) | ((-3 * VID_WIDTH_MULTIPLIER) & 0xFFFF);
	dma_buffer[dma_pos++] = (1 << 16) | (7 * VID_WIDTH_MULTIPLIER);
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
			dma_buffer[dma_pos++] = (((-(VID_HEIGHT/2))&0xFFFF) << 16) | ((-(VID_WIDTH/2))&0xFFFF);
			dma_buffer[dma_pos++] = (VID_HEIGHT << 16) | (VID_WIDTH << 0);
		}
	} else if ((cam_cb & (~1)) == 10) {
		DMA_PUSH(9, 1);
		for (int i = 0; i < 3; i++) {
			dma_buffer[dma_pos++] = 0x62081CB0;
			dma_buffer[dma_pos++] = (((-(VID_HEIGHT/2))&0xFFFF) << 16) | ((-(VID_WIDTH/2))&0xFFFF);
			dma_buffer[dma_pos++] = (VID_HEIGHT << 16) | (VID_WIDTH << 0);
		}
	}
}

