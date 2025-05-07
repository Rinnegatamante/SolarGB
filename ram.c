#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "ram.h"

uint8_t wram[0x2000] = {};
uint8_t hram[0x80] = {};

void ram_init() {
	sceClibMemset(wram, 0, 0x2000);
	sceClibMemset(hram, 0, 0x80);
}

void wram_write(uint16_t addr, uint8_t val) {
	wram[addr - ADDR_RAM_BANK_0] = val;
}

uint8_t wram_read(uint16_t addr) {
	return wram[addr - ADDR_RAM_BANK_0];
}

void hram_write(uint16_t addr, uint8_t val) {
	hram[addr - ADDR_HRAM] = val;
}

uint8_t hram_read(uint16_t addr) {
	return hram[addr - ADDR_HRAM];
}
