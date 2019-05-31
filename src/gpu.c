#include "common.h"

volatile uint32_t vblank_counter = 0;

void wait_for_next_vblank(void)
{
	uint32_t last_counter = vblank_counter;

	while(last_counter == vblank_counter) {
		// do nothing
	}
}

void gpu_dma_load(uint32_t *buffer, int x, int y, int width, int height)
{
#if 0
	// DMA-less version (FIFO)
	gp1_command(0x04000001); // DMA mode: FIFO (1)
	gp0_command(0xA0000000);
	gp0_data_xy(x,y);
	gp0_data_xy(width,height);
	for(int i = 0; i < width*height/2; i++) {
		gp0_data(buffer[i]);
	}
	gp0_command(0x01000000);
	gp1_command(0x04000002); // DMA mode: DMA to GPU (2)
#else
	DMA_n_CHCR(2) = 1;
	DMA_DICR = 0;
	DMA_DPCR = 0x07654321;
	DMA_n_MADR(2) = ((uint32_t) buffer)&0x00FFFFFF;
	DMA_n_BCR(2)  = ((width*height/2)<<13)|0x08;
	gp0_command(0xA0000000);
	gp0_data_xy(x,y);
	gp0_data_xy(width,height);
	DMA_DPCR |= (0x8<<(4*2)); // Enable DMA
	DMA_n_CHCR(2) = 0x01000201;
	while((DMA_n_CHCR(2) & (1<<24)) != 0) {
		//
	}
	//DMA_n_CHCR(2) = 1;
	//gp0_command(0x01000000);
#endif
}

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


