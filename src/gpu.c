#include "common.h"

volatile uint32_t vblank_counter = 0;

void gp0_command(uint32_t v)
{
	//while((PSXREG_GPU_GP1 & (1<<26)) == 0) {}
	for(int i = 0; i < 1000; i++) {
		if((PSXREG_GPU_GP1 & (1<<26)) != 0) {
			break;
		}
	}
	PSXREG_GPU_GP0 = v;
}

void gp0_data(uint32_t v)
{
	//while((PSXREG_GPU_GP1 & (1<<28)) == 0) {}
	for(int i = 0; i < 1000; i++) {
		if((PSXREG_GPU_GP1 & (1<<28)) != 0) {
			break;
		}
	}
	PSXREG_GPU_GP0 = v;
}

void gp0_data_xy(uint32_t x, uint32_t y)
{
	gp0_data((x&0xFFFF) | (y<<16));
}

void gp1_command(uint32_t v)
{
	//while ((PSXREG_GPU_GP1 & (1<<26)) == 0) {}
	for(int i = 0; i < 10000; i++) {
		if((PSXREG_GPU_GP1 & (1<<28)) != 0) {
			break;
		}
	}
	PSXREG_GPU_GP1 = v;
}


