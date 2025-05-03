// 0x0000 - 0x3FFF : ROM Bank 0
// 0x4000 - 0x7FFF : ROM Bank 1 - Switchable
// 0x8000 - 0x97FF : CHR RAM
// 0x9800 - 0x9BFF : BG Map 1
// 0x9C00 - 0x9FFF : BG Map 2
// 0xA000 - 0xBFFF : Cartridge RAM
// 0xC000 - 0xCFFF : RAM Bank 0
// 0xD000 - 0xDFFF : RAM Bank 1-7 - switchable - Color only
// 0xE000 - 0xFDFF : Reserved - Echo RAM
// 0xFE00 - 0xFE9F : Object Attribute Memory
// 0xFEA0 - 0xFEFF : Reserved - Unusable
// 0xFF00 - 0xFF7F : I/O Registers
// 0xFF80 - 0xFFFE : Zero Page

#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cart.h"
#include "bus.h"
#include "cpu.h"
#include "dma.h"
#include "io.h"
#include "ppu.h"
#include "ram.h"

void bus_write(uint16_t addr, uint8_t val) {
	if (addr < 0x8000) {
		// ROM data
		cart_write(addr, val);
	} else if (addr < 0xA000) {
		// Char/Map data
		ppu_vram_write(addr, val);
	} else if (addr < 0xC000) {
		// Cartridge RAM
		cart_write(addr, val);
	} else if (addr < 0xE000) {
		// Working RAM
		wram_write(addr, val);
	} else if (addr < 0xFE00) {
		// Reerved echo RAM
	} else if (addr < 0xFEA0) {
		// OAM
		if (dma.active) {
			ppu_oam_write(addr, val);
		}
	} else if (addr < 0xFF00) {
		// Reserved section
	} else if (addr < 0xFF80) {
		// IO Registers
		io_write(addr, val);
	} else if (addr == 0xFFFF) {
		// Interrupt Enable register
		cpu.regs.IE = val;
	} else {
		// HRAM
		hram_write(addr, val);
	}
}

uint8_t bus_read(uint16_t addr) {
	if (addr < 0x8000) {
		// ROM data
		return cart_read(addr);
	} else if (addr < 0xA000) {
		// Char/Map data
		return ppu_vram_read(addr);
	} else if (addr < 0xC000) {
		// Cartridge RAM
		return cart_read(addr);
	} else if (addr < 0xE000) {
		// Working RAM
		return wram_read(addr);
	} else if (addr < 0xFE00) {
		// Reerved echo RAM
		return 0;
	} else if (addr < 0xFEA0) {
		// OAM
		if (dma.active) {
			return 0xFF;
		}
		return ppu_oam_read(addr);
	} else if (addr < 0xFF00) {
		// Reserved section
		return 0;
	} else if (addr < 0xFF80) {
		// IO Registers
		return io_read(addr);
	} else if (addr == 0xFFFF) {
		// Interrupt Enable register
		return cpu.regs.IE;
	} else {
		// HRAM
		return hram_read(addr);
	}
}
