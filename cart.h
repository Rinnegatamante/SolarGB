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

typedef struct {
	uint8_t *data;
	size_t size;
	uint8_t type;
	uint32_t ram_size;
	char name[17];
	char licensee[64];
	char fname[256];
	uint8_t mbc1;
	uint8_t ram_enabled;
	uint8_t ram_banking;
	uint8_t *rom_bank;
	uint8_t *ram_bank;
	uint8_t *ram_banks[16];
	uint8_t banking_mode;
	uint8_t rom_bank_num;
	uint8_t ram_bank_num;
	uint8_t battery;
	uint8_t save_battery;
} cart_t;

extern cart_t rom;

void cart_load(const char *path);
void cart_save_battery();

#ifdef __cplusplus
}
#endif

#endif
