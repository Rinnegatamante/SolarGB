#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "io.h"
#include "timer.h"

static uint8_t serial_data[2] = {};

void io_init() {
	serial_data[0] = serial_data[1] = 0;
}

uint16_t io_read(uint16_t addr) {
	if (addr == 0xFF01) {
		return serial_data[0];
	} else if (addr == 0xFF02) {
		return serial_data[1];
	} else if (addr >= 0xFF04 && addr <= 0xFF07) {
		return timer_read(addr);
	} else if (addr == 0xFF0F) {
		return cpu.interrupts;
	} else if (addr >= 0xFF40 && addr <= 0xFF4B) {
		return lcd_read(addr);
	}
}

void io_write(uint16_t addr, uint8_t val) {
	if (addr == 0xFF01) {
		serial_data[0] = val;
	} else if (addr == 0xFF02) {
		serial_data[1] = val;
	} else if (addr >= 0xFF04 && addr <= 0xFF07) {
		timer_write(addr, val);
	} else if (addr == 0xFF0F) {
		cpu.interrupts = val;
	} else if (addr >= 0xFF40 && addr <= 0xFF4B) {
		lcd_write(addr, val);
	}	
}
