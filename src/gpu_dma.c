#include "common.h"

uint32_t dma_pos = 0;
uint32_t dma_pos_start = 0;
uint32_t dma_start_ptr;

volatile uint32_t dma_next_start = 0;
uint32_t dma_buffer[DMA_BUFFER_SIZE];
uint32_t dma_order_table[4][DMA_ORDER_MAX];
uint32_t dma_buffer_current = 0;

void gpu_dma_init(void) {
	if(dma_pos_start >= sizeof(dma_buffer)*2/sizeof(dma_buffer[0])/3) {
		dma_pos = 0;
	}

	dma_pos_start = dma_pos;
	dma_buffer_current += 1;
	dma_buffer_current &= 3;
	dma_start_ptr = 0x00FFFFFF&(uint32_t)&dma_order_table[dma_buffer_current][DMA_ORDER_MAX-1];

	for(int i = 0; i < DMA_ORDER_MAX; i++) {
		dma_order_table[dma_buffer_current][i] = (i == 0
			? 0x00FFFFFF
			: ((uint32_t)&dma_order_table[dma_buffer_current][i-1])&0xFFFFFF);
	}
}

void gpu_dma_finish(void) {
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
}
