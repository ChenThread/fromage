#include "common.h"

int gui_options_menu(options_t *options) {
	while (1) switch(gui_menu(
		4,
		options->pro_jumps ? "Movement: Quake Pro" : "Movement: Classic",
		options->move_dpad ? "Controls: D-pad" : "Controls: Left Analog",
		NULL,
		"Done"
	)) {
		case 0:
			options->pro_jumps = !options->pro_jumps;
			break;
		case 1:
			options->move_dpad = !options->move_dpad;
			break;
		case 3:
			return 0;
		default:
			return -1;
	}
}
