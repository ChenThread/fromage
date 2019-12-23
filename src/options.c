#include "common.h"

static const char *opt_renderdist_txt[] = {"Render distance: Tiny", "Render distance: Short", "Render distance: Normal", "Render distance: Extreme"};
static const char *opt_genmodes_txt[] = {"Generator: Default", "Generator: Flat"};

int gui_options_menu(options_t *options) {
	int last_option = 0;
	while (1) {
		last_option = gui_menu(
			6, last_option,
			options->pro_jumps ? "Movement: Quake Pro" : "Movement: Classic",
			options->move_dpad ? "Controls: D-Pad" : "Controls: Left Analog",
			opt_renderdist_txt[options->render_distance % 4],
			options->show_fps ? "Show FPS: On" : "Show FPS: Off",
			NULL,
			"Done"
		);
		switch (last_option) {
			case 0:
				options->pro_jumps = !options->pro_jumps;
				break;
			case 1:
				options->move_dpad = !options->move_dpad;
				break;
			case 2:
				options->render_distance = (options->render_distance + 1) % 4;
				break;
			case 3:
				options->show_fps = !options->show_fps;
				break;
			case 5:
				return 0;
			default:
				return -1;
		}
	}
}

int gui_worldgen_menu(void) {
	int last_option = 0;
	int wgen_mode = 0;
	while (1) {
		last_option = gui_menu(
			3, last_option,
			opt_genmodes_txt[wgen_mode],
			"Generate",
			"Return"
		);
		switch (last_option) {
			case 0:
				wgen_mode = (wgen_mode + 1) % 2;
				break;
			case 1:
				return wgen_mode;
			default:
				return -1;
		}
	}
}
