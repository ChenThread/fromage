#include "common.h"

static const char *opt_renderdist_txt[] = {"Render distance: Short", "Render distance: Normal", "Render distance: Far", "Render distance: Extreme"};
static const char *opt_genmodes_txt[] = {"Generator: Default", "Generator: Flat"};
static const char *opt_fovmodes_txt[] = {"FOV: Narrow", "FOV: Normal", "FOV: Wide"};

int gui_options_menu(options_t *options) {
	int last_option = 0;
	while (1) {
#ifdef STANDALONE_EXE
		last_option = gui_menu(
			8, last_option,
			options->pro_jumps ? "Movement: Quake Pro" : "Movement: Classic",
			options->move_dpad ? "Controls: D-Pad" : "Controls: Left Analog",
			opt_renderdist_txt[options->render_distance % 4],
			options->show_fps ? "Show FPS: On" : "Show FPS: Off",
			options->fog_on ? "Fog: On" : "Fog: Off",
			opt_fovmodes_txt[options->fov_mode % 3],
			NULL,
			"Done"
		);
#else
		last_option = gui_menu(
			10, last_option,
			options->pro_jumps ? "Movement: Quake Pro" : "Movement: Classic",
			options->move_dpad ? "Controls: D-Pad" : "Controls: Left Analog",
			opt_renderdist_txt[options->render_distance % 4],
			options->show_fps ? "Show FPS: On" : "Show FPS: Off",
			options->fog_on ? "Fog: On" : "Fog: Off",
			opt_fovmodes_txt[options->fov_mode % 3],
			options->sound_on ? "Sound: On" : "Sound: Off",
			options->music_on ? "Music: On" : "Music: Off",
			NULL,
			"Done"
		);
#endif
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
			case 4:
				options->fog_on = !options->fog_on;
				break;
			case 5:
				options->fov_mode = (options->fov_mode + 1) % 3;
				break;
#ifdef STANDALONE_EXE
			case 7:
				return 0;
#else
			case 6:
				options->sound_on = !options->sound_on;
				break;
			case 7:
				options->music_on = !options->music_on;
				break;
			case 9:
				return 0;
#endif
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
			4, last_option,
			opt_genmodes_txt[wgen_mode],
			NULL,
			"Generate",
			"Return"
		);
		switch (last_option) {
			case 0:
				wgen_mode = (wgen_mode + 1) % 2;
				break;
			case 2:
				return wgen_mode;
			default:
				return -1;
		}
	}
}
