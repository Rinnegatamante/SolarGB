#include <vitasdk.h>
#include <vitaGL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "cpu.h"
#include "dma.h"
#include "emu.h"
#include "ppu.h"

typedef void (*func_t)();

ppu_t ppu = {};
lcd_t lcd = {};

#define SCREEN_BUFFERS (3)
static GLuint screen_gl_tex[SCREEN_BUFFERS], ppu_dbg_gl_tex[SCREEN_BUFFERS];

static uint32_t ppu_colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000}; 

static inline __attribute__((always_inline)) void ppu_draw_image(GLuint *tex, void *buf, float x, float y, float w, float h, float scale) {
	glBindTexture(GL_TEXTURE_2D, tex[ppu.cur_frame % SCREEN_BUFFERS]);
	sceClibMemcpy(vglGetTexDataPointer(GL_TEXTURE_2D), buf, w * h * 4);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, HOST_SCREEN_W, HOST_SCREEN_H, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	
	float vtxs[] = {x, y + (h * scale), x, y, x + (w * scale), y + (h * scale), x + (w * scale), y};
	float tcs[] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f};
	glVertexPointer(2, GL_FLOAT, 0, vtxs);
	glTexCoordPointer(2, GL_FLOAT, 0, tcs);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

#define ppu_draw_frame() ppu_draw_image(screen_gl_tex, ppu.screen_tex, 100.0f, 56.0f, GB_SCREEN_W, GB_SCREEN_H, 3.0f)

static void ppu_update_dbg_tile(int n, int x, int y) {
	for (int tile_y = 0; tile_y < 16; tile_y += 2) {
		int offs = (n * 16) + tile_y;
		for (int bit = 7; bit >= 0; bit--) {
			uint8_t high = (ppu.vram[offs] & (1 << bit)) != 0 ? 2 : 0;
			uint8_t low = (ppu.vram[offs + 1] & (1 << bit)) != 0 ? 1 : 0;
			ppu.dbg_tex[x + (7 - bit) + (y + (tile_y / 2)) * 128] = ppu_colors[high | low];
		}
	}
}

void ppu_show_dbg_tex() {
	int tile_id = 0;
	int tile_x = 0;
	int tile_y = 0;
	for (int y = 0; y < 24; y++) {
		for (int x = 0; x < 16; x++) {
			ppu_update_dbg_tile(tile_id, tile_x, tile_y);
			tile_x += 8;
			tile_id++;
		}
		tile_y += 8;
		tile_x = 0;
	}
	
	ppu_draw_image(ppu_dbg_gl_tex, ppu.dbg_tex, 650.0f, 80.0f, 128.0f, 192.0f, 2.0f);
}

static inline __attribute__((always_inline)) void ppu_clear_pipeline() {
	while (ppu.fifo.head) {
		pixel_t *p = ppu.fifo.head;
		ppu.fifo.head = ppu.fifo.head->next;
		free(p);
	}
	ppu.fifo.size = 0;
	ppu.fifo.tail = NULL;
}

// PPU FIFO fetch functions
void ppu_tile_fetch() {
	if (LCDC_SET(BGW_ENABLE)) {
		uint16_t bg_map_area = LCDC_SET(BG_MAP_AREA) ? 0x9C00: 0x9800;
		ppu.fifo.bgw_fetch_data[0] = bus_read(bg_map_area + (ppu.fifo.map_x / 8) + ((ppu.fifo.map_y / 8) * 32));
		if (!(LCDC_SET(BGW_DATA_AREA))) {
			ppu.fifo.bgw_fetch_data[0] += 128;
		}
	}
	if (LCDC_SET(OBJ_ENABLE) && ppu.sprites) {
		spritelist_t *s = ppu.sprites;
		while (s) {
			int sprite_x = (s->s.x - 8) + (lcd.scroll_x % 8);
			if ((sprite_x >= ppu.fifo.fetch_x && sprite_x < (ppu.fifo.fetch_x + 8)) || ((sprite_x + 8) >= ppu.fifo.fetch_x && (sprite_x + 8) < (ppu.fifo.fetch_x + 8))) {
				ppu.fetched_sprites[ppu.num_fetched++] = s->s;
			}
			s = s->next;
			if (ppu.num_fetched >= 3) {
				break;
			}
		}
	}
	ppu.fifo.fetch_state = FS_DATA0;
	ppu.fifo.fetch_x += 8;
}
void ppu_data0_fetch() {
	uint16_t data_map_area = LCDC_SET(BGW_DATA_AREA) ? 0x8000: 0x8800;
	ppu.fifo.bgw_fetch_data[1] = bus_read(data_map_area + (ppu.fifo.bgw_fetch_data[0] * 16) + ppu.fifo.tile_y);
	uint8_t sprite_h = LCDC_SET(OBJ_HEIGHT) ? 16 : 8;
    for (int i = 0; i < ppu.num_fetched; i++) {
		uint8_t y = ((lcd.ly + 16) - ppu.fetched_sprites[i].y) * 2;
		if (ppu.fetched_sprites[i].y_flip) {
			y = ((sprite_h * 2) - 2) - y;
		}
        uint8_t tile_idx = ppu.fetched_sprites[i].tile;
        if (sprite_h == 16) {
            tile_idx &= 0xFE;
        }
		ppu.fifo.sprite_fetch_data[i * 2] = bus_read(0x8000 + (tile_idx * 16) + y);
    }
	ppu.fifo.fetch_state = FS_DATA1;
}
void ppu_data1_fetch() {
	uint16_t data_map_area = LCDC_SET(BGW_DATA_AREA) ? 0x8000: 0x8800;
	ppu.fifo.bgw_fetch_data[2] = bus_read(data_map_area + (ppu.fifo.bgw_fetch_data[0] * 16) + ppu.fifo.tile_y + 1);
	uint8_t sprite_h = LCDC_SET(OBJ_HEIGHT) ? 16 : 8;
    for (int i = 0; i < ppu.num_fetched; i++) {
		uint8_t y = ((lcd.ly + 16) - ppu.fetched_sprites[i].y) * 2;
		if (ppu.fetched_sprites[i].y_flip) {
			y = ((sprite_h * 2) - 2) - y;
		}
        uint8_t tile_idx = ppu.fetched_sprites[i].tile;
        if (sprite_h == 16) {
            tile_idx &= 0xFE;
        }
		ppu.fifo.sprite_fetch_data[(i * 2) + 1] = bus_read(0x8000 + (tile_idx * 16) + y + 1);
    }
	ppu.fifo.fetch_state = FS_IDLE;
}
void ppu_idle_fetch() {
	ppu.fifo.fetch_state = FS_PUSH;
}
void ppu_push_fetch() {
	if (ppu.fifo.size <= 8) {
		int x = ppu.fifo.fetch_x - (8 - (lcd.scroll_x % 8));
		if (x >= 0) {
			for (int i = 0; i < 8; i++) {
				pixel_t *p = (pixel_t *)malloc(sizeof(pixel_t));
				int bitmask = 1 << (7 - i);
				uint8_t high = (ppu.fifo.bgw_fetch_data[1] & bitmask) != 0 ? 1 : 0;
				uint8_t low = (ppu.fifo.bgw_fetch_data[2] & bitmask) != 0 ? 2 : 0;
				uint8_t bg_idx = high | low;
				
				if (!(LCDC_SET(BGW_ENABLE))) {
					p->col = lcd.bg_cols[0];
				} else {
					p->col = lcd.bg_cols[bg_idx];
				}
				
				if (LCDC_SET(OBJ_ENABLE)) {
					for (int j = 0; j < ppu.num_fetched; j++) {
						int sprite_x = (ppu.fetched_sprites[j].x - 8) + (lcd.scroll_x % 8);
						if (sprite_x + 8 >= ppu.fifo.fifo_x) {
							int offs = ppu.fifo.fifo_x - sprite_x;
							if (offs >= 0 && offs <= 7) {
								bitmask = ppu.fetched_sprites[j].x_flip ? (1 << (offs)) : (1 << (7 - offs));
								high = (ppu.fifo.sprite_fetch_data[j * 2] & bitmask) != 0 ? 1 : 0;
								low = (ppu.fifo.sprite_fetch_data[(j * 2) + 1] & bitmask) != 0 ? 2 : 0;
								uint8_t col = high | low;
								if (col && ((!ppu.fetched_sprites[j].bg_prio) || bg_idx)) {
									p->col = ppu.fetched_sprites[j].pal_num ? lcd.sp2_cols[col] : lcd.sp1_cols[col];
									break;
								}
							}
						}
					}
				}
				
				p->next = NULL;
				p->prev = ppu.fifo.tail;
				ppu.fifo.size++;
				if (ppu.fifo.tail) {
					ppu.fifo.tail->next = p;
				} else {
					ppu.fifo.head = p;
				}
				ppu.fifo.tail = p;
				ppu.fifo.fifo_x++;
			}
		}
		ppu.fifo.fetch_state = FS_TILE;
	}
}
func_t ppu_fetch_funcs[] = {
	[FS_TILE] = ppu_tile_fetch,
	[FS_DATA0] = ppu_data0_fetch,
	[FS_DATA1] = ppu_data1_fetch,
	[FS_IDLE] = ppu_idle_fetch,
	[FS_PUSH] = ppu_push_fetch,
};
# define ppu_fetch_pipeline() ppu_fetch_funcs[ppu.fifo.fetch_state]()

static void lcd_init() {
	lcd.lcdc = 0x91;
	lcd.lcds = 0x00;
	lcd.scroll_x = lcd.scroll_y = 0;
	lcd.ly = lcd.ly_cmp = lcd.dma = 0;
	lcd.bg_pal = 0xFC;
	lcd.obj_pal[0] = lcd.obj_pal[1] = 0xFF;
	lcd.win_x = lcd.win_y = 0;
	for (int i = 0; i < 4; i++) {
		lcd.bg_cols[i] = lcd.sp1_cols[i] = lcd.sp2_cols[i] = ppu_colors[i];
	}
}

void ppu_init() {
	vglUseVram(GL_FALSE);
	if (ppu.screen_tex == NULL) {
		glGenTextures(SCREEN_BUFFERS, screen_gl_tex);
		for (int i = 0; i < SCREEN_BUFFERS; i++) {
			glBindTexture(GL_TEXTURE_2D, screen_gl_tex[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GB_SCREEN_W, GB_SCREEN_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		ppu.screen_tex = (uint32_t *)malloc(GB_SCREEN_W * GB_SCREEN_H * 4);
	}
	if (emu.debug_ppu && ppu.dbg_tex == NULL) {
		glGenTextures(SCREEN_BUFFERS, ppu_dbg_gl_tex);
		for (int i = 0; i < SCREEN_BUFFERS; i++) {
			glBindTexture(GL_TEXTURE_2D, ppu_dbg_gl_tex[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		ppu.dbg_tex = (uint32_t *)malloc(128 * 192 * 4);
	}
	vglUseVram(GL_TRUE);
	
	ppu.cur_frame = 0;
	ppu.lines = 0;
	ppu.num_sprites = 0;
	ppu.num_fetched = 0;
	ppu.fifo.line_x = 0;
	ppu.fifo.pushed_x = 0;
	ppu.fifo.fetch_x = 0;
	ppu.fifo.fetch_state = FS_TILE;
	ppu_clear_pipeline();
	
	lcd_init();
	LCD_SET_MODE(MODE_OAM);
	
	sceClibMemset(ppu.oam_ram, 0, 0x40);
}

static inline __attribute__((always_inline)) void ppu_inc_ly() {
	lcd.ly++;
	if (lcd.ly == lcd.ly_cmp) {
		lcd.lcds |= 0x04;
		if (LCD_SS_SET(SS_LYC)) {
			CPU_SET_INTERRUPT(IT_LCD_STAT);
		}
	} else {
		lcd.lcds &= ~0x04;
	}
}

// PPU modes functions
void ppu_hblank() {
	if (ppu.lines >= TICKS_PER_LINE) {
		ppu_inc_ly();
		if (lcd.ly >= GB_SCREEN_H) {
			LCD_SET_MODE(MODE_VBLANK);
			CPU_SET_INTERRUPT(IT_VBLANK);
			if (LCD_SS_SET(SS_VBLANK)) {
				CPU_SET_INTERRUPT(IT_LCD_STAT);
			}
			ppu.cur_frame++;
			ppu_draw_frame();
		} else {
			LCD_SET_MODE(MODE_OAM);
		}
		ppu.lines = 0;
	}
}
void ppu_vblank() {
	if (ppu.lines >= TICKS_PER_LINE) {
		ppu_inc_ly();
		if (lcd.ly >= LINES_PER_FRAME) {
			LCD_SET_MODE(MODE_OAM);
			lcd.ly = 0;
		}
		ppu.lines = 0;
	}
}
void ppu_oam() {
	if (ppu.lines >= 80) {
		LCD_SET_MODE(MODE_XFER);
		ppu.fifo.fetch_state = FS_TILE;
		ppu.fifo.line_x = 0;
		ppu.fifo.fetch_x = 0;
		ppu.fifo.pushed_x = 0;
		ppu.fifo.fifo_x = 0;
	}
	if (ppu.lines == 1) {
		ppu.num_sprites = 0;
		ppu.sprites = NULL;
		uint8_t sprite_h = LCDC_SET(OBJ_HEIGHT) ? 16 : 8;
		sceClibMemset(ppu.sprite_slots, 0, sizeof(spritelist_t) * 10);
		for (int i = 0; i < 0x40; i += sizeof(sprite_t)) {
			sprite_t *s = (sprite_t *)&ppu.oam_ram[i];
			if (s->x) {
				if (ppu.num_sprites >= 10) {
					break;
				}
				if (s->y <= (lcd.ly + 16) && s->y + sprite_h > (lcd.ly + 16)) {
					spritelist_t *en = &ppu.sprite_slots[ppu.num_sprites++];
					en->s = *s;
					en->next = NULL;
					if (ppu.sprites == NULL || ppu.sprites->s.x > s->x) {
						en->next = ppu.sprites;
						ppu.sprites = en;
					} else {
						spritelist_t *le = ppu.sprites;
						spritelist_t *prev = le;
						while (le) {
							if (le->s.x > s->x) {
								prev->next = en;
								en->next = le;
								break;
							} else if (le->next == NULL) {
								le->next = en;
								break;
							}
							prev = le;
							le = le->next;
						}
					}
				}
			}
		}
	}
}
void ppu_xfer() {
	ppu.fifo.map_y = lcd.ly + lcd.scroll_y;
	ppu.fifo.map_x = ppu.fifo.fetch_x + lcd.scroll_x;
	ppu.fifo.tile_y = (ppu.fifo.map_y % 8) * 2;
	if ((ppu.lines & 1) == 0) {
		ppu_fetch_pipeline();
	}
	if (ppu.fifo.size > 8) {
		uint32_t clr = ppu.fifo.head->col;
		pixel_t *p = ppu.fifo.head->next;
		free(ppu.fifo.head);
		ppu.fifo.head = p;
		if (p) {
			p->prev = NULL;
		} else {
			ppu.fifo.tail = NULL;
		}
		ppu.fifo.size--;
		if (ppu.fifo.line_x >= (lcd.scroll_x % 8)) {
			ppu.screen_tex[ppu.fifo.pushed_x + (lcd.ly * GB_SCREEN_W)] = clr;
			ppu.fifo.pushed_x++;
		}
		ppu.fifo.line_x++;
	}
	if (ppu.fifo.pushed_x >= GB_SCREEN_W) {
		ppu_clear_pipeline();
		LCD_SET_MODE(MODE_HBLANK);
		if (LCD_SS_SET(SS_HBLANK)) {
			CPU_SET_INTERRUPT(IT_LCD_STAT);
		}
	}
}
func_t ppu_mode_funcs[] = {
	[MODE_HBLANK] = ppu_hblank,
	[MODE_VBLANK] = ppu_vblank,
	[MODE_OAM] = ppu_oam,
	[MODE_XFER] = ppu_xfer,
};

void ppu_tick() {
	ppu.lines++;
	ppu_mode_funcs[lcd.lcds & 0x03]();
}

void lcd_write(uint16_t addr, uint8_t val) {
	uint8_t *p = (uint8_t *)&lcd;
	p[addr - 0xFF40] = val;
	
	uint8_t v;
	switch (addr) {
	case 0xFF46:
		dma_start(val);
		break;
	case 0xFF47:
		lcd.bg_cols[0] = ppu_colors[val & 0x03];
		lcd.bg_cols[1] = ppu_colors[(val >> 2) & 0x03];
		lcd.bg_cols[2] = ppu_colors[(val >> 4) & 0x03];
		lcd.bg_cols[3] = ppu_colors[(val >> 6) & 0x03];
		break;
	case 0xFF48:
		v = val & 0b11111100;
		lcd.sp1_cols[0] = ppu_colors[0];
		lcd.sp1_cols[1] = ppu_colors[(v >> 2) & 0x03];
		lcd.sp1_cols[2] = ppu_colors[(v >> 4) & 0x03];
		lcd.sp1_cols[3] = ppu_colors[(v >> 6) & 0x03];
		break;
	case 0xFF49:
		v = val & 0b11111100;
		lcd.sp2_cols[0] = ppu_colors[0];
		lcd.sp2_cols[1] = ppu_colors[(v >> 2) & 0x03];
		lcd.sp2_cols[2] = ppu_colors[(v >> 4) & 0x03];
		lcd.sp2_cols[3] = ppu_colors[(v >> 6) & 0x03];
		break;
	default:
		break;
	}
}
