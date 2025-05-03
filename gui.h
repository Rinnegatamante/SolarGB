#ifndef _GUI_H_
#define _GUI_H_

#ifdef __cplusplus
extern "C" {
#endif

void gui_init();
rom_t *gui_rom_selector();
void gui_emu_options();

#ifdef __cplusplus
}
#endif

#endif