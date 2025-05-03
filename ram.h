#ifndef _RAM_H_
#define _RAM_H_

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t wram[0x2000];
extern uint8_t hram[0x80];

void ram_init();

static inline __attribute__((always_inline)) uint16_t hram_read(uint16_t addr) {
	return hram[addr - 0xFF80];
}

static inline __attribute__((always_inline)) void hram_write(uint16_t addr, uint8_t val) {
	hram[addr - 0xFF80] = val;
}

static inline __attribute__((always_inline)) uint16_t wram_read(uint16_t addr) {
	return wram[addr - 0xC000];
}
static inline __attribute__((always_inline)) void wram_write(uint16_t addr, uint8_t val) {
	wram[addr - 0xC000] = val;
}

#ifdef __cplusplus
}
#endif

#endif
