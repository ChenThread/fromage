#include "common.h"
#include "seedy.h"

void cdrom_isr(void) {
	seedy_isr_cdrom();
}

void cdrom_init(void) {
	seedy_init_cdrom();
}

