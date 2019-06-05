#include <stdarg.h>
#include <stdio.h>
#include <sawpads.h>
#include "common.h"

extern uint32_t block_lighting[6];

#define CHAR_WIDTH(c) ((font_raw[(uint8_t)(c) >> 1] >> (((uint8_t)(c) & 1) << 2)) & 0x0F)

static int get_text_width_buffer(const char *buffer)
{
	int len = 0;
	for (size_t i = 0; i < strlen(buffer); i++) {
		len += (CHAR_WIDTH(buffer[i])) + 1;
	}
	return len * VID_WIDTH_MULTIPLIER;
}

int get_text_width(const char *format, ...)
{
	char buffer[256];
	int len;

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);

	len = get_text_width_buffer(buffer);

	va_end(args);
	return len;
}

static void draw_text_buffer(int x, int y, int color, const char *buffer)
{
	x -= VID_WIDTH/2;
	y -= VID_HEIGHT/2;

	// Undo texpage
	DMA_PUSH(1, 1);
	dma_buffer[dma_pos++] = 0x00000000;
	DMA_PUSH(1, 1);
	dma_buffer[dma_pos++] = 0xE1000600;

	for (size_t i = 0; i < strlen(buffer); i++) {
		uint8_t c = buffer[i];
		int width = CHAR_WIDTH(c);
		int widthmul = width * VID_WIDTH_MULTIPLIER;
		int pos = (x & 0xFFFF) + ((y & 0xFFFF) << 16);
		int texcoord = (((c & 15) * VID_WIDTH_MULTIPLIER) << 3) | ((c >> 4) << 11);

		DMA_PUSH(4, 1);
		dma_buffer[dma_pos++] = 0x64000000 | color;
		dma_buffer[dma_pos++] = pos;
		dma_buffer[dma_pos++] = (384 << 22) | (0x35 << 16) | texcoord;
		dma_buffer[dma_pos++] = (8 << 16) | (widthmul & 0xFFFF);

		pos += 0x10001;

		DMA_PUSH(4, 1);
		dma_buffer[dma_pos++] = 0x66000000;
		dma_buffer[dma_pos++] = pos;
		dma_buffer[dma_pos++] = (384 << 22) | (0x35 << 16) | texcoord;
		dma_buffer[dma_pos++] = (8 << 16) | (widthmul & 0xFFFF);

		x += (width + 1) * VID_WIDTH_MULTIPLIER;
	}

	// Do texpage
	DMA_PUSH(1, 1);
	dma_buffer[dma_pos++] = 0x00000000;
	DMA_PUSH(1, 1);
	dma_buffer[dma_pos++] = 0xE100061E;

}

void draw_status_progress(int progress, int max)
{
	int width = VID_WIDTH / 2;

	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x6020F020;
	dma_buffer[dma_pos++] = (10 << 16) | (-(width/2) & 0xFFFF);
	dma_buffer[dma_pos++] = ((2*VID_HEIGHT_MULTIPLIER) << 16) | ((width * progress / max) & 0xFFFF);

	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x60181818;
	dma_buffer[dma_pos++] = (10 << 16) | (-(width/2) & 0xFFFF);
	dma_buffer[dma_pos++] = ((2*VID_HEIGHT_MULTIPLIER) << 16) | ((width) & 0xFFFF);
}

void draw_text(int x, int y, int color, const char *format, ...)
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
		uint16_t tc0 = 0x0000;
		uint16_t tc1 = 0x000F;
		uint16_t tc2 = 0x0F00;
		uint16_t tc3 = 0x0F0F;
		uint32_t cmd = ((block_id&(~1)) == 8) ? 0x2E000000 : 0x2C000000;

		if (block_id == 44) {
			lu += bh8<<16;
			cu += bh8<<16;
			ru += bh8<<16;
			cc += bh8<<16;
			tc0 |= 0x0008;
			tc2 |= 0x0008;
		}
		DMA_PUSH(9, 1);
		dma_buffer[dma_pos++] = cmd | block_lighting[5];
		dma_buffer[dma_pos++] = lu;
		dma_buffer[dma_pos++] = (bi0->cl << 16) | (bi0->tc + (0x0000));
		dma_buffer[dma_pos++] = cu;
		dma_buffer[dma_pos++] = (bi0->tp << 16) | (bi0->tc + (0x000F));
		dma_buffer[dma_pos++] = cc;
		dma_buffer[dma_pos++] = (0 << 16) | (bi0->tc + (0x0F00));
		dma_buffer[dma_pos++] = ru;
		dma_buffer[dma_pos++] = (0 << 16) | (bi0->tc + (0x0F0F));

		DMA_PUSH(9, 1);
		dma_buffer[dma_pos++] = cmd | block_lighting[0];
		dma_buffer[dma_pos++] = lu;
		dma_buffer[dma_pos++] = (bi1->cl << 16) | (bi1->tc + (tc0));
		dma_buffer[dma_pos++] = cc;
		dma_buffer[dma_pos++] = (bi1->tp << 16) | (bi1->tc + (tc1));
		dma_buffer[dma_pos++] = ld;
		dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (tc2));
		dma_buffer[dma_pos++] = cd;
		dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (tc3));

		DMA_PUSH(9, 1);
		dma_buffer[dma_pos++] = cmd | block_lighting[2];
		dma_buffer[dma_pos++] = cc;
		dma_buffer[dma_pos++] = (bi1->cl << 16) | (bi1->tc + (tc0));
		dma_buffer[dma_pos++] = ru;
		dma_buffer[dma_pos++] = (bi1->tp << 16) | (bi1->tc + (tc1));
		dma_buffer[dma_pos++] = cd;
		dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (tc2));
		dma_buffer[dma_pos++] = rd;
		dma_buffer[dma_pos++] = (0 << 16) | (bi1->tc + (tc3));
	}
}

void draw_block_background(block_info_t *bi)
{
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x62000000;
	dma_buffer[dma_pos++] = (-(VID_HEIGHT/2) << 16) | (-(VID_WIDTH/2) & 0xFFFF);
	dma_buffer[dma_pos++] = ((VID_HEIGHT) << 16) | ((VID_WIDTH) & 0xFFFF);
	DMA_PUSH(3, 1);
	dma_buffer[dma_pos++] = 0x62000000;
	dma_buffer[dma_pos++] = (-(VID_HEIGHT/2) << 16) | (-(VID_WIDTH/2) & 0xFFFF);
	dma_buffer[dma_pos++] = ((VID_HEIGHT) << 16) | ((VID_WIDTH) & 0xFFFF);

	for (int y = 0; y < VID_HEIGHT; y += 16 * VID_HEIGHT_MULTIPLIER) {
		for (int x = 0; x < VID_WIDTH; x += 16 * VID_WIDTH_MULTIPLIER) {
			draw_block_icon_flat(
				x - (VID_WIDTH/2) + 8 * VID_WIDTH_MULTIPLIER,
				y - (VID_HEIGHT/2) + 8,
				16 * VID_WIDTH_MULTIPLIER,
				16 * VID_HEIGHT_MULTIPLIER,
				bi);
		}
	}
}

void draw_dirt_background(void)
{
	draw_block_background(&block_info[3][5]);
}

void draw_status_window(const char *format, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);

	// Draw text
	int width = get_text_width_buffer(buffer);
	draw_text_buffer((VID_WIDTH - width) / 2, (VID_HEIGHT - 8) / 2, 0xFFFFFF, buffer);

	draw_dirt_background();

	va_end(args);
}

void draw_block_sel_menu(int pos, uint8_t *slots, int slotcount)
{
	// 34x34 slots
	int bg_w = ((25 * 9) + 18 + 4) * VID_WIDTH_MULTIPLIER;
	int bg_h = ((25 * ((slotcount+8)/9))) + 18 + 15;

	int y = 0;
	int x = 0;

	// blocks
	for (int id = 0; id < slotcount; id++) {
		int x_center = (-bg_w/2) + 2 + 9;
		int y_center = (-bg_h/2) + 12 + 9;
		int bid = slots[id];

		x_center += (25 * VID_WIDTH_MULTIPLIER * x) + 16 * VID_WIDTH_MULTIPLIER;
		y_center += (25 * y) + 11;

		if (id == pos) {
			draw_block_icon(x_center, y_center, 32 * VID_WIDTH_MULTIPLIER, 32, bid);

			// background
			DMA_PUSH(3, 1);
			dma_buffer[dma_pos++] = 0x62FFFFFF;
			dma_buffer[dma_pos++] = (
				(y_center - 16) << 16) |
				((x_center - 16 * VID_WIDTH_MULTIPLIER) & 0xFFFF);
			dma_buffer[dma_pos++] = ((32) << 16) | ((32 * VID_WIDTH_MULTIPLIER) & 0xFFFF);
		} else {
			draw_block_icon(x_center, y_center, 16 * VID_WIDTH_MULTIPLIER, 16, bid);
		}

		if ((++x) == 9) { y++; x = 0; }
	}

	// text
	int text_w = get_text_width("Select block");
	draw_text((VID_WIDTH - text_w) / 2, ((VID_HEIGHT - bg_h) / 2) + 4, 0xFFFFFF, "Select block");

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
		int32_t sel_valid = world_cast_ray(
			cam_x, cam_y, cam_z,
			mat_rt31, mat_rt32, mat_rt33,
			&sel_cx, &sel_cy, &sel_cz,
			10, true);

		if(sel_valid >= 0) {
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
		for (int i = 0; i < 2; i++) {
			DMA_PUSH(3, 1);
			dma_buffer[dma_pos++] = 0x62501000;
			dma_buffer[dma_pos++] = (((-(VID_HEIGHT/2))&0xFFFF) << 16) | ((-(VID_WIDTH/2))&0xFFFF);
			dma_buffer[dma_pos++] = (VID_HEIGHT << 16) | (VID_WIDTH << 0);
		}
	} else if ((cam_cb & (~1)) == 10) {
		for (int i = 0; i < 3; i++) {
			DMA_PUSH(3, 1);
			dma_buffer[dma_pos++] = 0x62081CB0;
			dma_buffer[dma_pos++] = (((-(VID_HEIGHT/2))&0xFFFF) << 16) | ((-(VID_WIDTH/2))&0xFFFF);
			dma_buffer[dma_pos++] = (VID_HEIGHT << 16) | (VID_WIDTH << 0);
		}
	}
}

int gui_menu(int optcount, ...)
{
	char *strs[optcount];
	int curr_opt = 0;

	va_list args;
	va_start(args, optcount);
	for (int i = 0; i < optcount; i++)
		strs[i] = va_arg(args, char*);
	va_end(args);

	while (1) {
		gpu_dma_init();
		frame_start();

		int opt_height = 20;
		int opt_width = VID_WIDTH * 3 / 4;
		int opt_text_y = (opt_height - 8) / 2;
		int opt_distance = opt_height + 8;
		int opt_allheight = (opt_distance * (optcount - 1)) + opt_height;
		int opt_x = (VID_WIDTH - opt_width) / 2;
		int opt_y = (VID_HEIGHT - opt_allheight) / 2;

		for (int i = 0; i < optcount; i++)
		{
			if (strs[i] == NULL) continue;

			int tw = get_text_width_buffer(strs[i]);
			int ty = opt_y + (opt_distance * i);
			draw_text((VID_WIDTH - tw) / 2, ty + opt_text_y, 0xFFFFFF, strs[i]);

			DMA_PUSH(3, 1);
			dma_buffer[dma_pos++] = i == curr_opt ? 0x62C0C0C0 : 0x62000000;
			dma_buffer[dma_pos++] = ((ty - (VID_HEIGHT/2)) << 16) | ((opt_x - (VID_WIDTH/2)) & 0xFFFF);
			dma_buffer[dma_pos++] = ((opt_height) << 16) | ((opt_width) & 0xFFFF);
		}

		draw_dirt_background();
		gpu_dma_finish();
		frame_flip();
		sawpads_do_read();

		int joy_pressed = update_joy_pressed();
		if ((joy_pressed & PAD_DOWN) != 0) {
			curr_opt = (curr_opt + 1) % optcount;
			while (strs[curr_opt] == NULL)
				curr_opt = (curr_opt + 1) % optcount;
		}
		if ((joy_pressed & PAD_UP) != 0) {
			curr_opt = (curr_opt - 1);
			if (curr_opt < 0) curr_opt = optcount - 1;
			while (strs[curr_opt] == NULL) {
				curr_opt = (curr_opt - 1);
				if (curr_opt < 0) curr_opt = optcount - 1;
			}
		}
		if ((joy_pressed & (PAD_START | PAD_T | PAD_O)) != 0) { curr_opt = -1; break; }
		if ((joy_pressed & PAD_X) != 0) break;
	}

	update_joy_pressed();
	return curr_opt;
}

void gui_terrible_text_viewer(const char* text)
{
	// Leave this alone. It can be slow and terrible.
	// It's for fulfilling licensing obligations and nothing else.
	// Shoo!
	int text_pos = 0;
	int last_tp = -1;
	int line_height = 9;
	int border_width = VID_WIDTH * 7 / 8;
	int border_height = VID_HEIGHT * 7 / 8;
	int window_width = border_width - 8*VID_WIDTH_MULTIPLIER;
	int window_height = (border_height - 8);
	window_height = window_height - (window_height % line_height);
	int window_x = ((VID_WIDTH - window_width) / 2);
	int window_y = ((VID_HEIGHT - window_height) / 2);
	int border_x = ((-border_width) / 2);
	int border_y = ((-border_height) / 2);
	char buffer[512];

	window_height -= 16;

	while (1) {
	if (last_tp != text_pos) {
		const char *tpos = text;
		int newlines = 0;
		while (newlines < text_pos && (*tpos)!=0) {
			if (*(tpos++) == 10) newlines++;
		}
		if ((*tpos) == 0) {
			text_pos--;
			continue;
		}

		gpu_dma_init();
		frame_start();

		draw_text_buffer(window_x, window_y + window_height + 8, 0x404040, "[press o to exit]");

		int iy = 0;
		int ix = 0;
		int ib = 0;
		while (iy < window_height) {
			switch (*tpos) {
				case 13: continue;
				case 10:
				case 32: {
					buffer[ib] = 0;
					int w = get_text_width_buffer(buffer);
					if (w > window_width) {
						char *last_word = strrchr(buffer, ' ');
						last_word[0] = '\0';
						draw_text_buffer(window_x + ix, window_y + iy, 0xFFFFFF, buffer);
						last_word[0] = ' ';
						int lwl = strlen(last_word + 1);
						memmove(buffer, last_word + 1, lwl);
						ib = lwl;
						buffer[ib++] = 32;
						iy += line_height;
					} else if ((*tpos) == 10) {
						draw_text_buffer(window_x + ix, window_y + iy, 0xFFFFFF, buffer);
						ib = 0;
						iy += line_height;
					} else {
						buffer[ib++] = 32;
					}
				} break;
				case 0:
					iy = window_height;
					break;
				default:
					buffer[ib++] = *tpos;
					break;
			}
			tpos++;
		}


		DMA_PUSH(3, 1);
		dma_buffer[dma_pos++] = 0x62000000;
		dma_buffer[dma_pos++] = (border_y << 16) | (border_x & 0xFFFF);
		dma_buffer[dma_pos++] = (border_height << 16) | (border_width & 0xFFFF);

		draw_dirt_background();
		gpu_dma_finish();
		frame_flip();

		last_tp = text_pos;
	} else {
		wait_for_next_vblank();
		sawpads_do_read();
	}

		int joy_pressed = update_joy_pressed();
		if ((joy_pressed & PAD_DOWN) != 0) text_pos++;
		if ((joy_pressed & PAD_UP) != 0) text_pos--;
		if ((joy_pressed & (PAD_T | PAD_O)) != 0) break;
	}
}
