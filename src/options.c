#include "common.h"

int gui_options_menu(options_t *options) {
	while (1) switch(gui_menu(
		2,
		options->pro_jumps ? "Movement: Quake Pro" : "Movement: Classic",
		"Done"
	)) {
		case 0:
			options->pro_jumps = !options->pro_jumps;
			break;
		case 1:
			return 0;
		default:
			return -1;
	}
}
