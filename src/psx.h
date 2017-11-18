#ifndef _PSX_H_
#define _PSX_H_
#define PSX_IOBASE 0x00000000

#define JOY_TX_DATA (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801040))
#define JOY_RX_DATA (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801040))
#define JOY_STAT (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801044))
#define JOY_MODE (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801048))
#define JOY_CTRL (*(volatile uint16_t *)(PSX_IOBASE + 0x1F80104A))
#define JOY_BAUD (*(volatile uint16_t *)(PSX_IOBASE + 0x1F80104E))

#define I_STAT (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801070))
#define I_MASK (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801074))

#define DMA_n_MADR(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801080 + (n)*0x10))
#define DMA_n_BCR(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801084 + (n)*0x10))
#define DMA_n_CHCR(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801088 + (n)*0x10))
#define DMA_DPCR (*(volatile uint32_t *)(PSX_IOBASE + 0x1F8010F0))
#define DMA_DICR (*(volatile uint32_t *)(PSX_IOBASE + 0x1F8010F4))

#define GP0 (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801810))
#define GP1 (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801814))

#define SPU_n_MVOL_L(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C00 + (n)*16))
#define SPU_n_MVOL_R(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C02 + (n)*16))
#define SPU_n_PITCH(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C04 + (n)*16))
#define SPU_n_START(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C06 + (n)*16))
#define SPU_n_ADSR(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801C08 + (n)*16))
#define SPU_n_REPEAT(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C0E + (n)*16))

#define TMR_n_COUNT(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801100 + (n)*16))
#define TMR_n_MODE(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801104 + (n)*16))
#define TMR_n_TARGET(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801108 + (n)*16))

#define SPU_MVOL_L (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801D80))
#define SPU_MVOL_R (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801D82))
#define SPU_KON (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801D88))
#define SPU_KOFF (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801D8C))
#define SPU_PMON (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801D90))
#define SPU_ENDX (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801D9C))
#define SPU_MEM_ADDR (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DA6))
#define SPU_MEM_DATA (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DA8))
#define SPU_CNT (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DAA))
#define SPU_MEM_CNT (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DAC))
#define SPU_STAT (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DAE))

#define PAD_SELECT 0x0001
#define PAD_L3 0x0002
#define PAD_R3 0x0004
#define PAD_START 0x0008
#define PAD_UP 0x0010
#define PAD_RIGHT 0x0020
#define PAD_DOWN 0x0040
#define PAD_LEFT 0x0080
#define PAD_L2 0x0100
#define PAD_R2 0x0200
#define PAD_L1 0x0400
#define PAD_R1 0x0800
#define PAD_T 0x1000
#define PAD_O 0x2000
#define PAD_X 0x4000
#define PAD_S 0x8000

//define InitHeap(a0, a1) ((void (*)(int, int, int))0x000000A0)(0x39, a0, a1);
//define malloc(a0) ((void *(*)(int, int))0xA0)(0x33, a0);
//define free(a0) ((void (*)(int, int))0xA0)(0x34, a0);

#endif

