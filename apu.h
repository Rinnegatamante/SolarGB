#ifndef _APU_H_
#define _APU_H_

typedef struct {
	uint8_t sweep;
	uint8_t len_timer;
	uint8_t vol;
	uint8_t period;
	uint8_t control;
} apu_chn_regs_t;

typedef struct {
	apu_chn_regs_t chn1;
	apu_chn_regs_t chn2;
	apu_chn_regs_t chn3;
	apu_chn_regs_t chn4;
	uint8_t master_volume; //FF24 (NR50)
	uint8_t panning; //FF25 (NR51)
	uint8_t master_control; //FF26 (NR52)
	uint8_t unk[9];
	uint8_t pattern_ram[0x10]; //FF30-FF3F
} apu_regs_t;

typedef struct {
	uint8_t active;
	uint8_t overflown;
	uint8_t negated;
	int timer;
	int freq;
	int shadow_freq;
	uint8_t period;
	uint8_t negate;
	uint8_t shift;
} apu_freq_sweep_t;

typedef struct {
	uint8_t active;
	int full_len;
	int len;
	int sequencer_frame;
} apu_len_cnt_t;

typedef struct {
	uint8_t active;
	int timer;
	uint8_t start_vol;
	uint8_t add_mode;
	uint8_t period;
	uint8_t vol;
} apu_vol_env_t;

typedef struct {
	uint8_t active;
	uint8_t dac_active;
	int timer;
	int seq;
	uint8_t duty;
	uint8_t out;
	uint8_t vol_code;
	int last_addr;
	int pos;
	int ticks;
	apu_freq_sweep_t freq_sweep;
	apu_len_cnt_t len_counter;
	apu_vol_env_t vol_envelope;
} apu_chn_t;

typedef struct {
	apu_regs_t regs;
	int active;
	int sequencer_counter;
	int freq_counter;
	uint8_t sequencer_frame;
	apu_chn_t chn1;
	apu_chn_t chn2;
	apu_chn_t chn3;
	apu_chn_t chn4;
} apu_t;

extern apu_t apu;

void apu_init();
void apu_tick();

uint8_t apu_read_nr10(uint16_t addr);
uint8_t apu_read_nr11(uint16_t addr);
uint8_t apu_read_nr12(uint16_t addr);
uint8_t apu_read_nr13(uint16_t addr);
uint8_t apu_read_nr14(uint16_t addr);
uint8_t apu_read_nr21(uint16_t addr);
uint8_t apu_read_nr22(uint16_t addr);
uint8_t apu_read_nr23(uint16_t addr);
uint8_t apu_read_nr24(uint16_t addr);
uint8_t apu_read_nr30(uint16_t addr);
uint8_t apu_read_nr31(uint16_t addr);
uint8_t apu_read_nr32(uint16_t addr);
uint8_t apu_read_nr33(uint16_t addr);
uint8_t apu_read_nr34(uint16_t addr);

void apu_write_nr10(uint16_t addr, uint8_t val);
void apu_write_nr11(uint16_t addr, uint8_t val);
void apu_write_nr12(uint16_t addr, uint8_t val);
void apu_write_nr13(uint16_t addr, uint8_t val);
void apu_write_nr14(uint16_t addr, uint8_t val);
void apu_write_nr21(uint16_t addr, uint8_t val);
void apu_write_nr22(uint16_t addr, uint8_t val);
void apu_write_nr23(uint16_t addr, uint8_t val);
void apu_write_nr24(uint16_t addr, uint8_t val);
void apu_write_nr30(uint16_t addr, uint8_t val);
void apu_write_nr31(uint16_t addr, uint8_t val);
void apu_write_nr32(uint16_t addr, uint8_t val);
void apu_write_nr33(uint16_t addr, uint8_t val);
void apu_write_nr34(uint16_t addr, uint8_t val);

#endif
