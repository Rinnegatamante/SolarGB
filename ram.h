#ifndef _RAM_H_
#define _RAM_H_

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t wram[0x2000];
extern uint8_t hram[0x80];

void ram_init();

uint8_t wram_read(uint16_t addr);
uint8_t hram_read(uint16_t addr);

void wram_write(uint16_t addr, uint8_t val);
void hram_write(uint16_t addr, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif
