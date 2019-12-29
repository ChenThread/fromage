#include "common.h"

FASTMEM uint32_t dma_pos = 0;
uint32_t dma_pos_start = 0;
uint32_t dma_start_ptr;

volatile uint32_t dma_next_start = 0;
uint32_t dma_buffer[DMA_BUFFER_SIZE];
uint32_t dma_order_table[DMA_BUFFER_COUNT][DMA_ORDER_MAX];
uint32_t dma_buffer_current = 0;

void gpu_dma_init(void) {
	if(dma_pos_start >= sizeof(dma_buffer)/sizeof(dma_buffer[0])/2) {
		dma_pos = 0;
	}

	dma_pos_start = dma_pos;
	dma_buffer_current += 1;
	dma_buffer_current &= (DMA_BUFFER_COUNT-1);
	dma_start_ptr = 0x00FFFFFF&(uint32_t)&dma_order_table[dma_buffer_current][DMA_ORDER_MAX-1];

	for(int i = 0; i < DMA_ORDER_MAX; i++) {
		dma_order_table[dma_buffer_current][i] = (i == 0
			? 0x00FFFFFF
			: ((uint32_t)&dma_order_table[dma_buffer_current][i-1])&0xFFFFFF);
	}
}

int gpu_dma_finish(void) {
	DMA_PUSH(1, 0);
	dma_buffer[dma_pos++] = 0x00000000;

	while((DMA_n_CHCR(2) & (1<<24)) != 0) {}

	DMA_n_CHCR(2) = 0x00000001;
	DMA_DICR = 0;
	DMA_DPCR = 0x07654321;
	DMA_n_MADR(2) = dma_start_ptr;
	DMA_n_BCR(2)  = 0;
	DMA_DPCR |= (0x8<<(4*2)); // Enable DMA
	DMA_n_CHCR(2) = 0x01000401;

	if (dma_pos_start > dma_pos) {
		return (sizeof(dma_buffer)/sizeof(int32_t)) + dma_pos - dma_pos_start;
	} else {
		return dma_pos - dma_pos_start;
	}
}

void gpu_dma_load(uint32_t *buffer, int x, int y, int width, int height, int use_lz4)
{
	if (use_lz4 > 0) {
		buffer = (uint32_t *) lz4_alloc_and_unpack((uint8_t *) buffer, use_lz4, width*height*2);
	}

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
	if (use_lz4) {
		free(buffer);
	}
}
