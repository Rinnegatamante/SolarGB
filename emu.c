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

int emu_main (unsigned int argc, void *argv) {
	bus_init();
	ram_init();
	ppu_init();
	cpu_init();
	timer_init();
	emu.state = EMU_RUNNING;
	emu.frametime_tick = sceKernelGetProcessTimeLow();

	while (emu.state != EMU_NOT_RUNNING) {
		if (emu.state == EMU_PAUSED) {
			sceKernelDelayThread(100);
		} else {
			cpu_step();
		}
	}
	
	return sceKernelExitDeleteThread(0);
}

int main(int argc, char *argv[]) {
#if 1
	sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
#endif

	// Initialize default emulator settings
	emu.opts.debug_log = 0;
	emu.opts.debug_ppu = 1;
	emu.opts.serial_port_enabled = 1;
	emu.opts.frametime_log = 1;
	
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
	
	for (;;) {
		rom_t *to_start = NULL;
		uint32_t oldpad = 0;
		uint8_t show_options = 0;
		while (!to_start) {
			SceCtrlData pad;
			sceCtrlPeekBufferPositive(0, &pad, 1);
			if (show_options) {
				gui_emu_options();
			} else {
				to_start = gui_rom_selector();
			}
			if ((pad.buttons & SCE_CTRL_LTRIGGER) && (!(oldpad & SCE_CTRL_LTRIGGER))) {
				show_options = !show_options;
			}
			vglSwapBuffers(GL_FALSE);
			oldpad = pad.buttons;
		}
	
		// Main emulator code start
		char rom_path[512];
		sprintf(rom_path, "%s%s", ROM_FOLDER, to_start->name);
		cart_load(rom_path);
		emu.state = EMU_NOT_RUNNING;
		SceUID cpu_thread = sceKernelCreateThread("SolarGB", emu_main, 0x40, 0x100000, 0, 0, NULL);
		sceKernelStartThread(cpu_thread, 0, NULL);
		while (emu.state == EMU_NOT_RUNNING) {
			sceKernelDelayThread(100);
		}
		
		while (emu.state != EMU_NOT_RUNNING) {
			glClear(GL_COLOR_BUFFER_BIT);
			SceCtrlData pad;
			if (emu.state == EMU_PAUSED) { // Emulation paused
				gui_pause_menu();
			} else { // Emulation active
				sceCtrlPeekBufferPositive(0, &pad, 1);
				if ((pad.buttons & SCE_CTRL_LTRIGGER) && (!(oldpad & SCE_CTRL_LTRIGGER))) {
					emu.state = EMU_PAUSED;
				}	
				oldpad = emu.buttons = pad.buttons;
				ppu_draw_last_frame();
				if (emu.opts.debug_ppu) {
					ppu_show_dbg_tex();
				}
			}
			vglSwapBuffers(GL_FALSE);
		}
	}
	
	return 0;
}