#include "common.h"
#include <sawpads.h>

// approx. 1/5 a second
#ifdef TV_PAL
#define MAX_PRESS_TIME 10
#else
#define MAX_PRESS_TIME 12
#endif

int joy_pressed;
static uint8_t press_time[16];
static int jbp = 0;

void joy_update(int autorepeat_divisor)
{
	sawpads_do_read();
	joy_pressed = 0;

	for (int i = 0; i < 16; i++) {
		int mask = 1 << i;
		int pressed = (sawpads_buttons & mask) == 0;
		if (pressed) {
			press_time[i] = (press_time[i] + 1) % (MAX_PRESS_TIME / autorepeat_divisor);
		} else {
			press_time[i] = 0;
		}

		if (press_time[i] == 1) joy_pressed |= mask;
	}
}

