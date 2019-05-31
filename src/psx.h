#ifndef _PSX_H_
#define _PSX_H_

#define DMA_n_MADR(n) (*(volatile uint32_t *)(0x1F801080 + (n)*0x10))
#define DMA_n_BCR(n) (*(volatile uint32_t *)(0x1F801084 + (n)*0x10))
#define DMA_n_CHCR(n) (*(volatile uint32_t *)(0x1F801088 + (n)*0x10))
#define DMA_DPCR (*(volatile uint32_t *)(0x1F8010F0))
#define DMA_DICR (*(volatile uint32_t *)(0x1F8010F4))

#endif

