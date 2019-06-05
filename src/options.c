#include "common.h"

static char *opt_renderdist_txt[] = {"Render distance: Small", "Render distance: Normal", "Render distance: Large"};

int gui_options_menu(options_t *options) {
	int last_option = 0;
	while (1) {
		last_option = gui_menu(
			5, last_option,
			options->pro_jumps ? "Movement: Quake Pro" : "Movement: Classic",
			options->move_dpad ? "Controls: D-pad" : "Controls: Left Analog",
			opt_renderdist_txt[options->render_distance % 3],
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
				options->render_distance = (options->render_distance + 1) % 3;
				break;
			case 4:
				return 0;
			default:
				return -1;
		}
	}
}
