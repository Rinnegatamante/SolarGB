#ifndef _BUS_H_
#define _BUS_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
	ADDR_ROM_BANK_0 = 0x0000,
	ADDR_ROM_BANK_1 = 0x4000,
	ADDR_CHR_RAM    = 0x8000,
	ADDR_BG_MAP1    = 0x9800,
	ADDR_BG_MAP2    = 0x9C00,
	ADDR_CART_RAM   = 0xA000,
	ADDR_RAM_BANK_0 = 0xC000,
	ADDR_RAM_BANKS  = 0xD000,
	ADDR_ECHO_RAM   = 0xE000,
	ADDR_OAM        = 0xFE00,
	ADDR_RESERVED   = 0xFEA0,
	ADDR_IO_REGS    = 0xFF00,
	ADDR_LCD_REGS   = 0xFF40,
	ADDR_HRAM       = 0xFF80,
	ADDR_IE_REG     = 0xFFFF
};

typedef void (*bus_wfuncs_t)(uint16_t, uint8_t);
typedef uint8_t (*bus_rfuncs_t)(uint16_t);
extern bus_wfuncs_t bus_write_funcs[0x10000];
extern bus_rfuncs_t bus_read_funcs[0x10000];

static inline __attribute__((always_inline)) uint8_t bus_read(uint16_t addr) {
	return bus_read_funcs[addr](addr);
}

static inline __attribute__((always_inline)) void bus_write(uint16_t addr, uint8_t val) {
	bus_write_funcs[addr](addr, val);
}

static inline __attribute__((always_inline)) void bus_write16(uint16_t addr, uint16_t val) {
	bus_write_funcs[addr](addr, val & 0xFF);
	bus_write_funcs[addr](addr + 1, (val >> 8) & 0xFF);
}

static inline __attribute__((always_inline)) uint16_t bus_read16(uint16_t addr) {
	uint16_t low = bus_read_funcs[addr](addr);
	uint16_t high = bus_read_funcs[addr + 1](addr + 1);
	return (low | (high << 8));
}

void bus_init();

#ifdef __cplusplus
}
#endif

#endif
