#include "common.h"

volatile uint8_t joy_id = 0;
volatile uint8_t joy_hid = 0;
volatile uint16_t joy_buttons = 0;
volatile uint8_t joy_axes[4];
volatile uint32_t joy_read_counter = 0;
volatile uint8_t joy_has_ack = 0;
volatile uint16_t joy_buffer[16];
uint8_t joy_rumble[2];

void joy_stop_read(void)
{
	JOY_CTRL = 0x0010;
	joy_read_counter++;

	// Also kill time
	for(uint32_t i = 0; i < 1000; i++) {
		asm volatile ("");
	}
	joy_has_ack = 0;
}

uint8_t joy_recv_solo(void)
{
	while((JOY_STAT & (1<<1)) == 0) {}
	return JOY_RX_DATA;
}

uint8_t joy_send(uint8_t data, bool wait_ack)
{
	joy_has_ack = 0;
	if(!wait_ack) {
		while((JOY_STAT & (1<<1)) != 0) {
			asm volatile (""
				:
				: "r"(JOY_RX_DATA)
				:);
		}
	}
	JOY_TX_DATA = data;

	if(wait_ack) {
#if 0
		while(joy_has_ack == 0) {}
#else
		for(uint32_t i = 0; i < 0x44*100; i++) {
			asm volatile ("");
			if(joy_has_ack != 0) {
				break;
			}
		}
#endif
		joy_has_ack--;
	} else {
		while((JOY_STAT & (1<<1)) == 0) {}
		// But wait anyway
		for(uint32_t i = 0; i < 0x44*10; i++) { asm volatile (""); }
	}
	return JOY_RX_DATA;
}

void joy_start_read(void)
{
	JOY_CTRL = 0x1013;

	// Kill time (not known how long)
	for(uint32_t i = 0; i < 10000; i++) {
		asm volatile ("");
	}
}

uint8_t joy_read_words(uint8_t cmd, uint8_t* response, int response_len)
{
	uint8_t joy_hwords = 0;

	joy_start_read();
	joy_send(0x01, true);
	joy_id = joy_send(cmd, true);
	joy_hwords = joy_id;
	joy_hwords -= 1;
	joy_hwords &= 0x0F;
	joy_hwords += 1;
	joy_hid = joy_send(0x00, true);
	int j = 0;
	for (int i = 0; i < joy_hwords; i++) {
		joy_buffer[i] = joy_send(j < response_len ? response[j++] : 0xFF, true);
		joy_buffer[i] |= joy_send(j < response_len ? response[j++] : 0xFF, i != (joy_hwords - 1)) << 8;
	}
	joy_stop_read();

	return joy_hwords;
}

void joy_do_read(void)
{
	uint8_t joy_words = joy_read_words(0x42, joy_rumble, 2);
	uint8_t joy_analogs = 0;

	if(joy_words >= 1) {
		joy_buttons = joy_buffer[0];
		if (joy_words >= 2) {
			joy_analogs = 2;
			joy_axes[0] = joy_buffer[1] & 0xFF;
			joy_axes[1] = joy_buffer[1] >> 8;
			if (joy_words >= 3) {
				joy_analogs = 4;
				joy_axes[2] = joy_buffer[2] & 0xFF;
				joy_axes[3] = joy_buffer[2] >> 8;
			}
		}
	}

	for (int i = 0; i < joy_analogs; i++) {
		if (joy_axes[i] > 0x60 && joy_axes[i] < 0xA0)
			joy_axes[i] = 0x00;
		else
			joy_axes[i] ^= 0x80;
	}
}

void joy_unlock_dualshock(void)
{
	static uint8_t response[6];

	// Kick joypad into config mode
	response[0] = 0x01;
	response[1] = 0x00;
	joy_read_words(0x43, response, 2);

	// Turn on analog mode
	response[0] = 0x01;
	response[1] = 0x03;
	joy_read_words(0x44, response, 2);

	// Enable rumble
	response[0] = 0x00;
	response[1] = 0x01;
	response[2] = 0xFF;
	response[3] = 0xFF;
	response[4] = 0xFF;
	response[5] = 0xFF;
	joy_read_words(0x4D, response, 6);

	// Enable pressure
	response[0] = 0xFF;
	response[1] = 0xFF;
	response[2] = 0x03;
	response[3] = 0x00;
	response[4] = 0x00;
	response[5] = 0x00;
	joy_read_words(0x4F, response, 6);

	// Revert to normal mode
	response[0] = 0x00;
	response[1] = 0x5A;
	response[2] = 0x5A;
	response[3] = 0x5A;
	response[4] = 0x5A;
	response[5] = 0x5A;
	joy_read_words(0x43, response, 6);
}

