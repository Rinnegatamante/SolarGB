#ifndef _CART_H_
#define _CART_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CART_ENTRYPOINT      = 0x100, // Size 4
	CART_LOGO            = 0x104, // Size 48
	CART_TITLE           = 0x134, // Size 16
	CART_MAN_CODE        = 0x13F, // Size 4
	CART_CGB_FLAG        = 0x143, // Size 1
	CART_NEW_LIC_CODE    = 0x144, // Size 2
	CART_SGB_FLAG        = 0x146, // Size 1
	CART_TYPE            = 0x147, // Size 1
	CART_ROM_SIZE        = 0x148, // Size 1
	CART_RAM_SIZE        = 0x149, // Size 1
	CART_DEST_CODE       = 0x14A, // Size 1
	CART_LIC_CODE        = 0x14B, // Size 1
	CART_VERSION         = 0x14C, // Size 1
	CART_HDR_CHECKSUM    = 0x14D, // Size 1
	CART_GLOBAL_CHECKSUM = 0x14E, // Size 2
};

enum {
	MAPPER_NONE,
	MAPPER_MBC1,
	MAPPER_MBC3,
};

typedef struct {
	uint8_t s;
	uint8_t m;
	uint8_t h;
	uint8_t day_count_low;
	uint8_t day_count_high;
} rtc_t;

typedef struct {
	uint8_t *data;
	size_t size;
	uint8_t type;
	uint32_t ram_size;
	char name[17];
	char licensee[64];
	char fname[256];
	uint8_t mapper;
	uint8_t ram_enabled;
	uint8_t ram_banking;
	uint8_t *rom_bank;
	uint8_t *ram_bank;
	uint8_t *ram_banks[16];
	uint8_t rom_bank_num;
	uint8_t ram_bank_num;
	uint8_t rtc_reg_num;
	uint8_t battery;
	uint8_t save_battery;
	rtc_t rtc_regs;
} cart_t;

extern cart_t rom;

void cart_load(const char *path);
void cart_save_battery();

uint8_t cart_read(uint16_t addr);
uint8_t cart_ram_read(uint16_t addr);
uint8_t cart_rom_bank_read(uint16_t addr);
uint8_t cart_mbc3_ram_read(uint16_t addr);

void cart_clock_data_write(uint16_t addr, uint8_t val);
void cart_ram_write(uint16_t addr, uint8_t val);
void cart_ram_bank_swap_write(uint16_t addr, uint8_t val);
void cart_mbc3_rom_bank_swap_write(uint16_t addr, uint8_t val);
void cart_rom_bank_swap_write(uint16_t addr, uint8_t val);
void cart_ram_enable_write(uint16_t addr, uint8_t val);
void cart_mbc3_ram_bank_swap_write(uint16_t addr, uint8_t val);
void cart_ram_bank_mode_write(uint16_t addr, uint8_t val);
void cart_mbc3_ram_write(uint16_t addr, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif
