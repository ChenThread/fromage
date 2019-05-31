#include "common.h"

#define CDROM_STATUS (*(volatile uint8_t *)(0x1F801800))
#define CDROM_PORT_1 (*(volatile uint8_t *)(0x1F801801))
#define CDROM_PORT_2 (*(volatile uint8_t *)(0x1F801802))
#define CDROM_PORT_3 (*(volatile uint8_t *)(0x1F801803))
#define COM_DELAY (*(volatile uint32_t *)(0x1F801020))

#define CDROM_CMD_GETSTAT 0x01
#define CDROM_CMD_INIT 0x0A
#define CDROM_CMD_DEMUTE 0x0C

#define WAIT_TRANSMIT() while (CDROM_STATUS & 0x80) {}

void cdrom_isr(void) {
	// TODO
}

void cdrom_init(void) {
	// TODO
}

