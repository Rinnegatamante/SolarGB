#ifndef _DMA_H_
#define _DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t active;
	uint8_t byte;
	uint8_t delay;
	uint8_t val;
} dma_t;

extern dma_t dma;

void dma_start(uint8_t val);
void dma_tick();

#ifdef __cplusplus
}
#endif

#endif
