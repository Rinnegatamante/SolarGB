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

#ifdef __cplusplus
}
#endif

#endif
