#ifndef _EMU_H_
#define _EMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#define EMU_VERSION "0.1"
#define ROM_FOLDER "ux0:data/LunarGB/roms/"

#define HOST_SCREEN_W (960)
#define HOST_SCREEN_H (544)

#define GB_SCREEN_W (160)
#define GB_SCREEN_H (144)

enum {
	EMU_NOT_RUNNING,
	EMU_RUNNING,
	EMU_PAUSED
};

typedef struct {
	uint8_t debug_log; // Log debug info on system console
	uint8_t debug_ppu; // Show PPU data on screen
	uint8_t serial_port_enabled; // Log serial port output to system console
	uint8_t frametime_log; // Log time took to process a frame
	uint32_t frametime_tick;
	uint8_t state;
} emu_t;

typedef struct rom {
	char name[256];
	struct rom *next;
} rom_t;

extern emu_t emu;
extern rom_t *roms;

void emu_incr_cycles(uint32_t cycles);

#ifdef __cplusplus
}
#endif

#endif
