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
	ppu.vram[addr - ADDR_CHR_RAM] = val;
}
void wram_write(uint16_t addr, uint8_t val) {
	wram[addr - ADDR_RAM_BANK_0] = val;
}
void oam_write(uint16_t addr, uint8_t val) {
	if (!dma.active) {
		ppu.oam_ram[addr - ADDR_OAM] = val;
	}
}
void ie_write(uint16_t addr, uint8_t val) {
	cpu.regs.IE = val;
}
void hram_write(uint16_t addr, uint8_t val) {
	hram[addr - ADDR_HRAM] = val;
}
void serial_write(uint16_t addr, uint8_t val) {
	serial_data[addr - (ADDR_IO_REGS + 1)] = 0;
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
void gamepad_write(uint16_t addr, uint8_t val) {
	keys = val;
}
void lcd_reg_write(uint16_t addr, uint8_t val) {
	uint8_t *p = (uint8_t *)&lcd;
	p[addr - ADDR_LCD_REGS] = val;
}
void lcd_dma_write(uint16_t addr, uint8_t val) {
	uint8_t *p = (uint8_t *)&lcd;
	p[6] = val;
	dma_start(val);
}
void lcd_bg_write(uint16_t addr, uint8_t val) {
	uint8_t *p = (uint8_t *)&lcd;
	p[7] = val;
	lcd.bg_cols[0] = ppu.cols[val & 0x03];
	lcd.bg_cols[1] = ppu.cols[(val >> 2) & 0x03];
	lcd.bg_cols[2] = ppu.cols[(val >> 4) & 0x03];
	lcd.bg_cols[3] = ppu.cols[(val >> 6) & 0x03];
}
void lcd_sp1_write(uint16_t addr, uint8_t val) {
	uint8_t *p = (uint8_t *)&lcd;
	p[8] = val;
	uint8_t v = val & 0b11111100;
	lcd.sp1_cols[0] = ppu.cols[0];
	lcd.sp1_cols[1] = ppu.cols[(v >> 2) & 0x03];
	lcd.sp1_cols[2] = ppu.cols[(v >> 4) & 0x03];
	lcd.sp1_cols[3] = ppu.cols[(v >> 6) & 0x03];
}
void lcd_sp2_write(uint16_t addr, uint8_t val) {
	uint8_t *p = (uint8_t *)&lcd;
	p[9] = val;
	uint8_t v = val & 0b11111100;
	lcd.sp2_cols[0] = ppu.cols[0];
	lcd.sp2_cols[1] = ppu.cols[(v >> 2) & 0x03];
	lcd.sp2_cols[2] = ppu.cols[(v >> 4) & 0x03];
	lcd.sp2_cols[3] = ppu.cols[(v >> 6) & 0x03];
}
void cart_ram_enable_write(uint16_t addr, uint8_t val) {
	rom.ram_enabled = ((val & 0x0F) == 0x0A);
}
void cart_rom_bank_swap_write(uint16_t addr, uint8_t val) {
	rom.rom_bank_num = val & 0x1F;
	rom.rom_bank = rom.data + (ADDR_ROM_BANK_1 * rom.rom_bank_num);
}
void cart_ram_bank_swap_write(uint16_t addr, uint8_t val) {
	rom.ram_bank_num = val & 0x03;
	rom.ram_bank = rom.ram_banks[rom.ram_bank_num];
	if (rom.ram_banking && rom.save_battery) {
		cart_save_battery();
	}
}
void cart_ram_bank_mode_write(uint16_t addr, uint8_t val) {
	rom.banking_mode = val & 1;
	rom.ram_banking = rom.banking_mode;
	if (rom.ram_banking && rom.save_battery) {
		cart_save_battery();
	}
}
void cart_ram_write(uint16_t addr, uint8_t val) {
	if (rom.ram_enabled && rom.ram_banks[rom.ram_bank_num] != NULL) {
		rom.ram_bank[addr - 0xA000] = val;
		if (rom.battery) {
			rom.save_battery = 1;
		}
	}
}
bus_wfuncs_t bus_write_funcs[0x10000] = {};

// Bus read functions
uint8_t null_read(uint16_t addr) { return 0; }
uint8_t ppu_vram_read(uint16_t addr) {
	return ppu.vram[addr - ADDR_CHR_RAM];
}
uint8_t wram_read(uint16_t addr) {
	return wram[addr - ADDR_RAM_BANK_0];
}
uint8_t oam_read(uint16_t addr) {
	if (dma.active) {
		return 0xFF;
	}
	return ppu.oam_ram[addr - ADDR_OAM];
}
uint8_t ie_read(uint16_t addr) {
	return cpu.regs.IE;
}
uint8_t hram_read(uint16_t addr) {
	return hram[addr - ADDR_HRAM];
}
uint8_t serial_read(uint16_t addr) {
	return serial_data[addr - (ADDR_IO_REGS + 1)];
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
	return p[addr - ADDR_LCD_REGS];
}
uint8_t cart_read(uint16_t addr) {
	return rom.data[addr];
}
uint8_t cart_ram_read(uint16_t addr) {
	if (!rom.ram_enabled || rom.ram_banks[rom.ram_bank_num] == NULL) {
		return 0xFF;
	}
	
	return rom.ram_banks[rom.ram_bank_num][addr - 0xA000];
}
uint8_t cart_rom_bank_read(uint16_t addr) {
	return rom.rom_bank[addr - 0x4000];
}
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

void bus_init() {
	serial_data[0] = serial_data[1] = 0;
	
	// ROM data
	if (rom.mbc1) {
		MAP_W_RANGE(0, 0x2000, cart_ram_enable_write);
		MAP_W_RANGE(0x2000, 0x4000, cart_rom_bank_swap_write);
		MAP_W_RANGE(0x4000, 0x6000, cart_ram_bank_swap_write);
		MAP_W_RANGE(0x6000, ADDR_CHR_RAM, cart_ram_bank_mode_write);
		MAP_R_RANGE(0, ADDR_ROM_BANK_1, cart_read);
		MAP_R_RANGE(ADDR_ROM_BANK_1, ADDR_CHR_RAM, cart_rom_bank_read);
	} else if (rom.mbc3) {
		
	} else {
		MAP_RW_RANGE(0, ADDR_CHR_RAM, cart_read, null_write);
	}
	// Char/Map data
	MAP_RW_RANGE(ADDR_CHR_RAM, ADDR_CART_RAM, ppu_vram_read, ppu_vram_write);
	// Cartridge RAM
	MAP_RW_RANGE(ADDR_CART_RAM, ADDR_RAM_BANK_0, cart_ram_read, cart_ram_write);
	// Working RAM
	MAP_RW_RANGE(ADDR_RAM_BANK_0, ADDR_ECHO_RAM, wram_read, wram_write);
	// Reserved echo RAM
	MAP_RW_RANGE(ADDR_ECHO_RAM, ADDR_OAM, null_read, null_write);
	// OAM
	MAP_RW_RANGE(ADDR_OAM, ADDR_RESERVED, oam_read, oam_write);
	// Reserved section
	MAP_RW_RANGE(ADDR_RESERVED, ADDR_IO_REGS, null_read, null_write);
	// IO Registers
	bus_write_funcs[ADDR_IO_REGS] = gamepad_write;
	bus_read_funcs[ADDR_IO_REGS] = gamepad_read;
	bus_write_funcs[ADDR_IO_REGS + 1] = serial_write;
	bus_read_funcs[ADDR_IO_REGS + 1] = serial_read;
	bus_write_funcs[ADDR_IO_REGS + 2] = serial_write;
	bus_read_funcs[ADDR_IO_REGS + 2] = serial_read;
	bus_write_funcs[ADDR_IO_REGS + 3] = null_write;
	bus_read_funcs[ADDR_IO_REGS + 3] = null_read;
	bus_write_funcs[ADDR_IO_REGS + 4] = timer_div_write;
	bus_read_funcs[ADDR_IO_REGS + 4] = timer_div_read;
	bus_write_funcs[ADDR_IO_REGS + 5] = timer_tima_write;
	bus_read_funcs[ADDR_IO_REGS + 5] = timer_tima_read;
	bus_write_funcs[ADDR_IO_REGS + 6] = timer_tma_write;
	bus_read_funcs[ADDR_IO_REGS + 6] = timer_tma_read;
	bus_write_funcs[ADDR_IO_REGS + 7] = timer_tac_write;
	bus_read_funcs[ADDR_IO_REGS + 7] = timer_tac_read;
	MAP_RW_RANGE(ADDR_IO_REGS + 8, ADDR_IO_REGS + 0x0F, null_read, null_write);
	bus_write_funcs[ADDR_IO_REGS + 0x0F] = cpu_intr_write;
	bus_read_funcs[ADDR_IO_REGS + 0x0F] = cpu_intr_read;
	MAP_RW_RANGE(ADDR_IO_REGS + 0x10, ADDR_LCD_REGS, null_read, null_write);
	MAP_RW_RANGE(ADDR_LCD_REGS, ADDR_LCD_REGS + 0x0C, lcd_read, lcd_reg_write);
	bus_write_funcs[ADDR_LCD_REGS + 6] = lcd_dma_write;
	bus_write_funcs[ADDR_LCD_REGS + 7] = lcd_bg_write;
	bus_write_funcs[ADDR_LCD_REGS + 8] = lcd_sp1_write;
	bus_write_funcs[ADDR_LCD_REGS + 9] = lcd_sp2_write;
	MAP_RW_RANGE(ADDR_LCD_REGS + 0x0C, ADDR_HRAM, null_read, null_write);
	// HRAM
	MAP_RW_RANGE(ADDR_HRAM, ADDR_IE_REG, hram_read, hram_write);
	// Interrupt Enable register
	bus_write_funcs[ADDR_IE_REG] = ie_write;
	bus_read_funcs[ADDR_IE_REG] = ie_read;

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
