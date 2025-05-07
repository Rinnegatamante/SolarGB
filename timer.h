#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint16_t div;
	uint8_t tima;
	uint8_t tma;
	uint8_t tac;
} tmr_t;

extern tmr_t timer;

void timer_init();
void timer_tick();

uint8_t timer_div_read(uint16_t addr);
uint8_t timer_tima_read(uint16_t addr);
uint8_t timer_tma_read(uint16_t addr);
uint8_t timer_tac_read(uint16_t addr);

void timer_div_write(uint16_t addr, uint8_t val);
void timer_tima_write(uint16_t addr, uint8_t val);
void timer_tma_write(uint16_t addr, uint8_t val);
void timer_tac_write(uint16_t addr, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif
