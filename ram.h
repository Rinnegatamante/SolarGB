#ifndef _RAM_H_
#define _RAM_H_

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t wram[0x2000];
extern uint8_t hram[0x80];

void ram_init();

#ifdef __cplusplus
}
#endif

#endif
