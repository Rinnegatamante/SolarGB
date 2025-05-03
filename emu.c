#include <vitasdk.h>
#include <vitaGL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "cart.h"
#include "cpu.h"
#include "dma.h"
#include "emu.h"
#include "gui.h"
#include "io.h"
#include "ppu.h"
#include "ram.h"
#include "timer.h"

int _newlib_heap_size_user = 256 * 1024 * 1024;

emu_t emu;
rom_t *roms = NULL;

void emu_incr_cycles(uint32_t cycles) {
	for (uint32_t i = 0; i < cycles; i++) {
		for (int j = 0; j < 4; j++) {
			timer_tick();
			ppu_tick();
		}
		dma_tick();
	}
}

int main(int argc, char *argv[]) {
#if 1
	sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
#endif

	// Initialize default emulator settings
	emu.debug_log = 1;
	emu.debug_ppu = 1;
	emu.serial_port_enabled = 0;
	
	// Scan roms folder and keep only .gb files
	SceUID d = sceIoDopen(ROM_FOLDER);
	SceIoDirent dent;
	while (sceIoDread(d, &dent) > 0) {
		if (!SCE_S_ISDIR(dent.d_stat.st_mode)) {
			if (!strcmp(&dent.d_name[strlen(dent.d_name) - 3], ".gb")) {
				rom_t *r = (rom_t *)malloc(sizeof(rom_t));
				strcpy(r->name, dent.d_name);
				r->next = roms;
				roms = r;
			}
		}
	}
	sceIoDclose(d);
	
	gui_init();
	rom_t *to_start = NULL;
	while (!to_start) {
		to_start = gui_rom_selector();
		vglSwapBuffers(GL_FALSE);
	}
	
	// Main emulator code start
	char rom_path[512];
	sprintf(rom_path, "%s%s", ROM_FOLDER, to_start->name);
	cart_load(rom_path);
	ram_init();
	ppu_init();
	cpu_init();
	timer_init();
	io_init();
	emu.state = EMU_RUNNING;
	
	uint32_t oldpad = 0;
	while (emu.state != EMU_NOT_RUNNING) {
		glClear(GL_COLOR_BUFFER_BIT);
		SceCtrlData pad;
		if (emu.state == EMU_PAUSED) { // Emulation paused
			// TODO
		} else { // Emulation active
			uint64_t work_frame = ppu.cur_frame;
			while ((work_frame == ppu.cur_frame) && (emu.state == EMU_RUNNING)) {
				// Perform one CPU step
				cpu_step();
				
				// Check if we want to pause the emulator
				sceCtrlPeekBufferPositive(0, &pad, 1);
				if ((pad.buttons & SCE_CTRL_LTRIGGER) && (!(oldpad & SCE_CTRL_LTRIGGER))) {
					emu.state = EMU_PAUSED;
				}
				oldpad = pad.buttons;
			}
			if (emu.debug_ppu) {
				ppu_show_dbg_tex();
			}
		}
		vglSwapBuffers(GL_FALSE);
		glFinish();
	}
	
	return 0;
}