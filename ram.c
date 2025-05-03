#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ram.h"

uint8_t wram[0x2000] = {};
uint8_t hram[0x80] = {};

void ram_init() {
	sceClibMemset(wram, 0, 0x2000);
	sceClibMemset(hram, 0, 0x80);
}