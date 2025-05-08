#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cart.h"
#include "apu.h"
#include "bus.h"
#include "cpu.h"
#include "dma.h"
#include "emu.h"
#include "ppu.h"
#include "ram.h"
#include "timer.h"

static uint8_t serial_data[2] = {};
static uint8_t keys = 0;

uint8_t null_read(uint16_t addr) { return 0; }
void null_write(uint16_t addr, uint8_t val) {}

uint8_t serial_read(uint16_t addr) {
	return serial_data[addr - (ADDR_IO_REGS + 1)];
}

void serial_write(uint16_t addr, uint8_t val) {
	serial_data[addr - (ADDR_IO_REGS + 1)] = 0;
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

void gamepad_write(uint16_t addr, uint8_t val) {
	keys = val;
}

bus_wfuncs_t bus_write_funcs[0x10000] = {};
bus_rfuncs_t bus_read_funcs[0x10000] = {};

#define MAP_R_RANGE(min, max, func) \
	for (int i = min; i < max; i++) { \
		bus_read_funcs[i] = func; \
	}
	
#define MAP_W_RANGE(min, max, func) \
	for (int i = min; i < max; i++) { \
		bus_write_funcs[i] = func; \
	}
	
#define MAP_RW_RANGE(min, max, rf, wf) \
	for (int i = min; i < max; i++) { \
		bus_read_funcs[i] = rf; \
		bus_write_funcs[i] = wf; \
	}

#define MAP_RW_FUNC(addr, rf, wf) \
	bus_read_funcs[addr] = rf; \
	bus_write_funcs[addr] = wf;

void bus_init() {
	serial_data[0] = serial_data[1] = 0;
	
	// ROM data
	if (rom.mapper == MAPPER_MBC1) {
		MAP_W_RANGE(0, 0x2000, cart_ram_enable_write)
		MAP_W_RANGE(0x2000, 0x4000, cart_rom_bank_swap_write)
		MAP_W_RANGE(0x4000, 0x6000, cart_ram_bank_swap_write)
		MAP_W_RANGE(0x6000, ADDR_CHR_RAM, cart_ram_bank_mode_write)
		MAP_R_RANGE(0, ADDR_ROM_BANK_1, cart_read)
		MAP_R_RANGE(ADDR_ROM_BANK_1, ADDR_CHR_RAM, cart_rom_bank_read)
	} else if (rom.mapper == MAPPER_MBC3) {
		MAP_W_RANGE(0, 0x2000, cart_ram_enable_write)
		MAP_W_RANGE(0x2000, 0x4000, cart_mbc3_rom_bank_swap_write)
		MAP_W_RANGE(0x4000, 0x6000, cart_mbc3_ram_bank_swap_write)
		MAP_W_RANGE(0x6000, ADDR_CHR_RAM, cart_clock_data_write)
		MAP_R_RANGE(0, ADDR_ROM_BANK_1, cart_read)
		MAP_R_RANGE(ADDR_ROM_BANK_1, ADDR_CHR_RAM, cart_rom_bank_read)
	} else {
		MAP_RW_RANGE(0, ADDR_CHR_RAM, cart_read, null_write)
	}
	// Char/Map data
	MAP_RW_RANGE(ADDR_CHR_RAM, ADDR_CART_RAM, ppu_vram_read, ppu_vram_write)
	// Cartridge RAM
	if (rom.mapper == MAPPER_MBC3) {
		MAP_RW_RANGE(ADDR_CART_RAM, ADDR_RAM_BANK_0, cart_mbc3_ram_read, cart_mbc3_ram_write)
	} else {
		MAP_RW_RANGE(ADDR_CART_RAM, ADDR_RAM_BANK_0, cart_ram_read, cart_ram_write)
	}
	// Working RAM
	MAP_RW_RANGE(ADDR_RAM_BANK_0, ADDR_ECHO_RAM, wram_read, wram_write)
	// Reserved echo RAM
	MAP_RW_RANGE(ADDR_ECHO_RAM, ADDR_OAM, null_read, null_write)
	// OAM
	MAP_RW_RANGE(ADDR_OAM, ADDR_RESERVED, ppu_oam_read, ppu_oam_write)
	// Reserved section
	MAP_RW_RANGE(ADDR_RESERVED, ADDR_IO_REGS, null_read, null_write)
	// IO Registers
	MAP_RW_FUNC(ADDR_IO_REGS, gamepad_read, gamepad_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x01, serial_read, serial_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x02, serial_read, serial_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x03, null_read, null_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x04, timer_div_read, timer_div_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x05, timer_tima_read, timer_tima_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x06, timer_tma_read, timer_tma_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x07, timer_tac_read, timer_tac_write)
	MAP_RW_RANGE(ADDR_IO_REGS + 0x08, ADDR_IO_REGS + 0x0F, null_read, null_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x0F, cpu_intr_read, cpu_intr_write)
	MAP_RW_RANGE(ADDR_IO_REGS + 0x1A, ADDR_LCD_REGS, null_read, null_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x10, apu_read_nr10, apu_write_nr10)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x11, apu_read_nr11, apu_write_nr11)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x12, apu_read_nr12, apu_write_nr12)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x13, apu_read_nr13, apu_write_nr13)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x14, apu_read_nr14, apu_write_nr14)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x15, null_read, null_write)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x16, apu_read_nr21, apu_write_nr21)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x17, apu_read_nr22, apu_write_nr22)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x18, apu_read_nr23, apu_write_nr23)
	MAP_RW_FUNC(ADDR_IO_REGS + 0x19, apu_read_nr24, apu_write_nr24)
	MAP_RW_RANGE(ADDR_LCD_REGS, ADDR_LCD_REGS + 0x0C, lcd_read, lcd_reg_write)
	bus_write_funcs[ADDR_LCD_REGS + 0x06] = lcd_dma_write;
	bus_write_funcs[ADDR_LCD_REGS + 0x07] = lcd_bg_write;
	bus_write_funcs[ADDR_LCD_REGS + 0x08] = lcd_sp1_write;
	bus_write_funcs[ADDR_LCD_REGS + 0x09] = lcd_sp2_write;
	MAP_RW_RANGE(ADDR_LCD_REGS + 0x0C, ADDR_HRAM, null_read, null_write)
	// HRAM
	MAP_RW_RANGE(ADDR_HRAM, ADDR_IE_REG, hram_read, hram_write)
	// Interrupt Enable register
	MAP_RW_FUNC(ADDR_IE_REG, cpu_ie_read, cpu_ie_write)

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
