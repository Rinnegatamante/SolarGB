#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "cpu.h"
#include "timer.h"

tmr_t timer;

void timer_init() {
	timer.div = 0xAC00;
	timer.tima = 0;
	timer.tma = 0;
	timer.tac = 0;
}

void timer_tick() {
	uint16_t prev_div = timer.div;
	timer.div++;
	uint8_t timer_update;
	
	switch (timer.tac & 0x03) {
	case 0:
		timer_update = (((prev_div & 0x200) == 0x200) && ((timer.div & 0x200) == 0));
		break;
	case 1:
		timer_update = (((prev_div & 0x08) == 0x08) && ((timer.div & 0x08) == 0));
		break;
	case 2:
		timer_update = (((prev_div & 0x20) == 0x20) && ((timer.div & 0x20) == 0));
		break;
	default:
		timer_update = (((prev_div & 0x80) == 0x80) && ((timer.div & 0x80) == 0));
		break;
	}
	
	if (timer_update && ((timer.tac & 0x04) == 0x04)) {
		timer.tima++;
		if (timer.tima == 0xFF) {
			timer.tima = timer.tma;
			CPU_SET_INTERRUPT(IT_TIMER);
		}
	}
}
