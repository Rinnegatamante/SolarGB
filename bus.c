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
#include "emu.h"
#include "ppu.h"
#include "ram.h"
#include "timer.h"

static uint8_t serial_data[2] = {};
static uint8_t keys = 0;

// Bus write functions
void null_write(uint16_t addr, uint8_t val) {}
void ppu_vram_write(uint16_t addr, uint8_t val) {
	ppu.vram[addr - 0x8000] = val;
}
void wram_write(uint16_t addr, uint8_t val) {
	wram[addr - 0xC000] = val;
}
void oam_write(uint16_t addr, uint8_t val) {
	if (!dma.active) {
		ppu.oam_ram[addr - 0xFE00] = val;
	}
}
void ie_write(uint16_t addr, uint8_t val) {
	cpu.regs.IE = val;
}
void hram_write(uint16_t addr, uint8_t val) {
	hram[addr - 0xFF80] = val;
}
void serial_write(uint16_t addr, uint8_t val) {
	serial_data[addr - 0xFF01] = 0;
}
void timer_div_write(uint16_t addr, uint8_t val) {
	timer.div = 0;
}
void timer_tima_write(uint16_t addr, uint8_t val) {
	timer.tima = val;
}
void timer_tma_write(uint16_t addr, uint8_t val) {
	timer.tma = val;
}
void timer_tac_write(uint16_t addr, uint8_t val) {
	timer.tac = val;
}
void cpu_intr_write(uint16_t addr, uint8_t val) {
	cpu.interrupts = val;
}
void lcd_write(uint16_t addr, uint8_t val) {
void gamepad_write(uint16_t addr, uint8_t val) {
	keys = val;
}
	uint8_t *p = (uint8_t *)&lcd;
	p[addr - 0xFF40] = val;
	uint8_t v;
	switch (addr) {
	case 0xFF46:
		dma_start(val);
		break;
	case 0xFF47:
		lcd.bg_cols[0] = ppu.cols[val & 0x03];
		lcd.bg_cols[1] = ppu.cols[(val >> 2) & 0x03];
		lcd.bg_cols[2] = ppu.cols[(val >> 4) & 0x03];
		lcd.bg_cols[3] = ppu.cols[(val >> 6) & 0x03];
		break;
	case 0xFF48:
		v = val & 0b11111100;
		lcd.sp1_cols[0] = ppu.cols[0];
		lcd.sp1_cols[1] = ppu.cols[(v >> 2) & 0x03];
		lcd.sp1_cols[2] = ppu.cols[(v >> 4) & 0x03];
		lcd.sp1_cols[3] = ppu.cols[(v >> 6) & 0x03];
		break;
	case 0xFF49:
		v = val & 0b11111100;
		lcd.sp2_cols[0] = ppu.cols[0];
		lcd.sp2_cols[1] = ppu.cols[(v >> 2) & 0x03];
		lcd.sp2_cols[2] = ppu.cols[(v >> 4) & 0x03];
		lcd.sp2_cols[3] = ppu.cols[(v >> 6) & 0x03];
		break;
	default:
		break;
	}
}
bus_wfuncs_t bus_write_funcs[0x10000] = {};

// Bus read functions
uint8_t null_read(uint16_t addr) { return 0; }
uint8_t ppu_vram_read(uint16_t addr) {
	return ppu.vram[addr - 0x8000];
}
uint8_t wram_read(uint16_t addr) {
	return wram[addr - 0xC000];
}
uint8_t oam_read(uint16_t addr) {
	if (dma.active) {
		return 0xFF;
	}
	return ppu.oam_ram[addr - 0xFE00];
}
uint8_t ie_read(uint16_t addr) {
	return cpu.regs.IE;
}
uint8_t hram_read(uint16_t addr) {
	return hram[addr - 0xFF80];
}
uint8_t serial_read(uint16_t addr) {
	return serial_data[addr - 0xFF01];
}
uint8_t timer_div_read(uint16_t addr) {
	return timer.div >> 8;
}
uint8_t timer_tima_read(uint16_t addr) {
	return timer.tima;
}
uint8_t timer_tma_read(uint16_t addr) {
	return timer.tma;
}
uint8_t timer_tac_read(uint16_t addr) {
	return timer.tac;
}
uint8_t cpu_intr_read(uint16_t addr) {
	return cpu.interrupts;
}
uint8_t gamepad_read(uint16_t addr) {
	uint8_t res = 0xCF;
	if ((keys & 0x10) == 0x10) {
		if ((emu.buttons & SCE_CTRL_CROSS) == SCE_CTRL_CROSS) {
			res &= BTN_A;
		}
		if ((emu.buttons & SCE_CTRL_SQUARE) == SCE_CTRL_SQUARE) {
			res &= BTN_B;
		}
		if ((emu.buttons & SCE_CTRL_START) == SCE_CTRL_START) {
			res &= BTN_START;
		}
		if ((emu.buttons & SCE_CTRL_SELECT) == SCE_CTRL_SELECT) {
			res &= BTN_SELECT;
		}
	}
	if ((keys & 0x20) == 0x20) {
		if ((emu.buttons & SCE_CTRL_UP) == SCE_CTRL_UP) {
			res &= DIR_UP;
		}
		if ((emu.buttons & SCE_CTRL_DOWN) == SCE_CTRL_DOWN) {
			res &= DIR_DOWN;
		}
		if ((emu.buttons & SCE_CTRL_LEFT) == SCE_CTRL_LEFT) {
			res &= DIR_LEFT;
		}
		if ((emu.buttons & SCE_CTRL_RIGHT) == SCE_CTRL_RIGHT) {
			res &= DIR_RIGHT;
		}
	}
	return res;
}
uint8_t lcd_read(uint16_t addr) {
	uint8_t *p = (uint8_t *)&lcd;
	return p[addr - 0xFF40];
}
uint8_t cart_read(uint16_t addr) {
	return rom.data[addr];
}
bus_rfuncs_t bus_read_funcs[0x10000] = {};
void bus_init() {
	serial_data[0] = serial_data[1] = 0;
	
	// ROM data
	for (int i = 0; i < 0x8000; i++) {
		bus_write_funcs[i] = null_write;
		bus_read_funcs[i] = cart_read;
	}
	// Char/Map data
	for (int i = 0x8000; i < 0xA000; i++) {
		bus_write_funcs[i] = ppu_vram_write;
		bus_read_funcs[i] = ppu_vram_read;
	}
	// Cartridge RAM
	for (int i = 0xA000; i < 0xC000; i++) {
		bus_write_funcs[i] = null_write;
		bus_read_funcs[i] = cart_read;
	}
	// Working RAM
	for (int i = 0xC000; i < 0xE000; i++) {
		bus_write_funcs[i] = wram_write;
		bus_read_funcs[i] = wram_read;
	}
	// Reserved echo RAM
	for (int i = 0xE000; i < 0xFE00; i++) {
		bus_write_funcs[i] = null_write;
		bus_read_funcs[i] = null_read;
	}
	// OAM
	for (int i = 0xFE00; i < 0xFEA0; i++) {
		bus_write_funcs[i] = oam_write;
		bus_read_funcs[i] = oam_read;
	}
	// Reserved section
	for (int i = 0xFEA0; i < 0xFF00; i++) {
		bus_write_funcs[i] = null_write;
		bus_read_funcs[i] = null_read;
	}
	// IO Registers
	bus_write_funcs[0xFF00] = gamepad_write;
	bus_read_funcs[0xFF00] = gamepad_read;
	bus_write_funcs[0xFF01] = serial_write;
	bus_read_funcs[0xFF01] = serial_read;
	bus_write_funcs[0xFF02] = serial_write;
	bus_read_funcs[0xFF02] = serial_read;
	bus_write_funcs[0xFF03] = null_write;
	bus_read_funcs[0xFF03] = null_read;
	bus_write_funcs[0xFF04] = timer_div_write;
	bus_read_funcs[0xFF04] = timer_div_read;
	bus_write_funcs[0xFF05] = timer_tima_write;
	bus_read_funcs[0xFF05] = timer_tima_read;
	bus_write_funcs[0xFF06] = timer_tma_write;
	bus_read_funcs[0xFF06] = timer_tma_read;
	bus_write_funcs[0xFF07] = timer_tac_write;
	bus_read_funcs[0xFF07] = timer_tac_read;
	for (int i = 0xFF08; i < 0xFF0F; i++) {
		bus_write_funcs[i] = null_write;
		bus_read_funcs[i] = null_read;
	}
	bus_write_funcs[0xFF0F] = cpu_intr_write;
	bus_read_funcs[0xFF0F] = cpu_intr_read;
	for (int i = 0xFF10; i < 0xFF40; i++) {
		bus_write_funcs[i] = null_write;
		bus_read_funcs[i] = null_read;
	}
	for (int i = 0xFF40; i < 0xFF4C; i++) {
		bus_write_funcs[i] = lcd_write;
		bus_read_funcs[i] = lcd_read;
	}
	for (int i = 0xFF4C; i < 0xFF80; i++) {
		bus_write_funcs[i] = null_write;
		bus_read_funcs[i] = null_read;
	}
	// HRAM
	for (int i = 0xFF80; i < 0xFFFF; i++) {
		bus_write_funcs[i] = hram_write;
		bus_read_funcs[i] = hram_read;
	}
	// Interrupt Enable register
	bus_write_funcs[0xFFFF] = ie_write;
	bus_read_funcs[0xFFFF] = ie_read;

#if 0
	// Sanity check
	sceClibPrintf("Bus mapping sanity check...\n");
	for (int i = 0; i < 0x10000; i++) {
		if (bus_write_funcs[i] == NULL) {
			sceClibPrintf("0x%04X write function not mapped!\n");
		}
		if (bus_read_funcs[i] == NULL) {
			sceClibPrintf("0x%04X read function not mapped!\n");
		}
	}
	sceClibPrintf("Bus mapping sanity check done\n");
#endif
}
