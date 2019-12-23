#include "common.h"
#include <sawpads.h>

// approx. 1/4 a second
#ifdef TV_PAL
#define MAX_PRESS_TIME 12
#else
#define MAX_PRESS_TIME 15
#endif

int32_t joy_pressed;
static uint8_t press_time[32];
static int jbp = 0;

void joy_update(int ticks, int autorepeat_divisor)
{
	int max_time = (MAX_PRESS_TIME / autorepeat_divisor);
	sawpads_do_read();
	joy_pressed = 0;

	for (int i = 0; i < 32; i++) {
		int joy_mask = 1 << (i & 15);
		int mask = 1 << i;
		int pressed = (sawpads_controller[i >> 4].buttons & joy_mask) == 0;
		if (pressed) {
			press_time[i] = (press_time[i] + ticks);
		} else {
			press_time[i] = 0;
		}


		if (press_time[i] > max_time) {
			joy_pressed |= mask;
			while (press_time[i] > max_time) {
				press_time[i] -= max_time;
			}
		} else if (press_time[i] == ticks) joy_pressed |= mask;
	}
}

