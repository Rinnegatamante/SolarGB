#ifndef _PPU_H_
#define _PPU_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LINES_PER_FRAME (154)
#define TICKS_PER_LINE  (456)

// LCDS modes
enum {
	MODE_HBLANK = 0,
	MODE_VBLANK = 1,
	MODE_OAM    = 2,
	MODE_XFER   = 3,
};

// LCDC bitmask
enum {
	BGW_ENABLE    = 0x01,
	OBJ_ENABLE    = 0x02,
	OBJ_HEIGHT    = 0x04,
	BG_MAP_AREA   = 0x08,
	BGW_DATA_AREA = 0x10,
	WIN_ENABLE    = 0x20,
	WIN_MAP_AREA  = 0x40,
	LCD_ENABLE    = 0x80
};

// LCDS bitmask
enum {
	SS_PPU_MODE  = 0x03,
	SS_LY_EQ_LYC = 0x04,
	SS_HBLANK    = 0x08,
	SS_VBLANK    = 0x10,
	SS_OAM       = 0x20,
	SS_LYC       = 0x40,	
};

//Fetch state modes
enum {
	FS_TILE  = 0,
	FS_DATA0 = 1,
	FS_DATA1 = 2,
	FS_IDLE  = 3,
	FS_PUSH  = 4,
};

typedef struct {
	uint8_t lcdc;
	uint8_t lcds;
	uint8_t scroll_y;
	uint8_t scroll_x;
	uint8_t ly;
	uint8_t ly_cmp;
	uint8_t dma;
	uint8_t bg_pal;
	uint8_t obj_pal[2];
	uint8_t win_y;
	uint8_t win_x;
	uint32_t bg_cols[4];
	uint32_t sp1_cols[4];
	uint32_t sp2_cols[4];
} lcd_t;

typedef struct pixel_t {
	uint32_t col;
	struct pixel_t *prev;
	struct pixel_t *next;
} pixel_t;

typedef struct {
	uint8_t fetch_state;
	uint8_t line_x;
	uint8_t pushed_x;
	uint8_t fetch_;
	uint8_t bgw_fetch_data[3];
	uint8_t sprite_fetch_data[6];
	uint8_t map_x;
	uint8_t map_y;
	uint8_t tile_y;
	uint8_t fifo_x;
	uint8_t fetch_x;
	pixel_t *head;
	pixel_t *tail;
	size_t size;
} fifo_t;

typedef struct sprite_t {
	uint8_t x;
	uint8_t y;
	uint8_t tile;
	uint8_t cgb_pal_num : 3;
	uint8_t vram_bank : 1;
	uint8_t pal_num : 1;
	uint8_t x_flip : 1;
	uint8_t y_flip : 1;
	uint8_t bg_prio : 1;
} sprite_t;

typedef struct spritelist_t {
	sprite_t s;
	struct spritelist_t *next;
} spritelist_t;

typedef struct {
	uint64_t cur_frame;
	uint8_t draw_frame;
	uint8_t oam_ram[0x40];
	uint8_t vram[0x2000];
	uint32_t lines;
	uint8_t num_sprites;
	uint8_t num_fetched;
	uint32_t *screen_tex;
	uint32_t *dbg_tex;
	uint32_t cols[4];
	spritelist_t *sprites;
	spritelist_t sprite_slots[10];
	sprite_t fetched_sprites[3];
	fifo_t fifo;
} ppu_t;

extern ppu_t ppu;
extern lcd_t lcd;

void ppu_init();
void ppu_tick();
void ppu_show_dbg_tex();

#define LCD_SET_MODE(x) \
	lcd.lcds &= ~SS_PPU_MODE; \
	lcd.lcds |= x;

#define LCD_SS_SET(x) ((lcd.lcds & x) == x)

#define LCDC_SET(x) ((lcd.lcdc & x) == x)

void ppu_draw_last_frame();

#ifdef __cplusplus
}
#endif

#endif
