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

// LCDS stat modes
enum {
	SS_HBLANK = 0x08,
	SS_VBLANK = 0x10,
	SS_OAM    = 0x20,
	SS_LYC    = 0x40,	
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
	uint8_t map_x;
	uint8_t map_y;
	uint8_t tile_y;
	uint8_t fifo_x;
	uint8_t fetch_x;
	pixel_t *head;
	pixel_t *tail;
	size_t size;
} fifo_t;

typedef struct {
	uint64_t cur_frame;
	uint8_t oam_ram[0x40];
	uint8_t vram[0x2000];
	uint32_t lines;
	uint32_t *screen_tex;
	uint32_t *dbg_tex;
	fifo_t fifo;
} ppu_t;

extern ppu_t ppu;
extern lcd_t lcd;

void lcd_write(uint16_t addr, uint8_t val);

static inline __attribute__((always_inline)) uint8_t lcd_read(uint16_t addr) {
	uint8_t *p = (uint8_t *)&lcd;
	return p[addr - 0xFF40];
}

static inline __attribute__((always_inline)) void ppu_vram_write(uint16_t addr, uint8_t val) {
	ppu.vram[addr - 0x8000] = val;
}

static inline __attribute__((always_inline)) uint8_t ppu_vram_read(uint16_t addr) {
	return ppu.vram[addr - 0x8000];
}

static inline __attribute__((always_inline)) void ppu_oam_write(uint16_t addr, uint8_t val) {
	if (addr >= 0xFE00) {
		addr -= 0xFE00;
	}
	
	ppu.oam_ram[addr] = val;
}

static inline __attribute__((always_inline)) uint8_t ppu_oam_read(uint16_t addr) {
	if (addr >= 0xFE00) {
		addr -= 0xFE00;
	}
	return ppu.oam_ram[addr];
}

void ppu_init();
void ppu_tick();

#define LCD_SET_MODE(x) \
	lcd.lcds &= ~0x03; \
	lcd.lcds |= x;

#define LCD_SS_SET(x) ((lcd.lcds & x) == x)

#ifdef __cplusplus
}
#endif

#endif
