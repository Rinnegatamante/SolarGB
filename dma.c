#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "dma.h"
#include "ppu.h"

dma_t dma = {};

void dma_start(uint8_t val) {
	dma.active = 1;
	dma.byte = 0;
	dma.delay = 2;
	dma.val = val;
}

void dma_tick() {
	if (dma.active) {
		if (dma.delay) {
			dma.delay--;
		} else {
			ppu.oam_ram[dma.byte] = bus_read(((uint16_t)dma.val * 0x100) + dma.byte);
		}
		dma.byte++;
		dma.active = dma.byte < 0xA0;
	}
}
