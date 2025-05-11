#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "apu.h"
#include "emu.h"

#define AUDIO_SAMPLES_NUM (512)
#define AUDIO_BUFFERS_NUM (2)
apu_t apu;

static uint8_t duty_tbl[4][8] = {
	{0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 1, 1, 1},
	{0, 1, 1, 1, 1, 1, 1, 0}
};
static uint8_t wave_tbl[16] = {};

static volatile int16_t audio_buffers[2][AUDIO_SAMPLES_NUM];
static uint8_t audio_pull = 0;
static uint8_t audio_push = 0;
static uint16_t samples_num = 0;
static SceUID audio_mutex = 0;

int audio_thread(unsigned int argc, void *argv) {
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, AUDIO_SAMPLES_NUM, 44100, SCE_AUDIO_OUT_MODE_MONO);

	for (;;) {
		if (emu.state == EMU_PAUSED) {
			sceKernelDelayThread(100);
		} else {
			sceKernelWaitSema(audio_mutex, 1, NULL);
			sceAudioOutOutput(ch, (void *)audio_buffers[audio_pull]);
			audio_pull = (audio_pull + 1) % 2;
		}
	}

	return sceKernelExitDeleteThread(0);
}

void apu_init() {
	if (!audio_mutex) {
		audio_mutex = sceKernelCreateSema("Audio Mutex", 0, 0, 1, NULL);
		SceUID audio_thid = sceKernelCreateThread("Audio Thread", audio_thread, 0x40, 0x100000, 0, 0, NULL);
		sceKernelStartThread(audio_thid, 0, NULL);
	}
	
	apu.active = 0;
	apu.sequencer_counter = 8192;
	apu.sequencer_frame = 0;
	apu.freq_counter = 95;
	samples_num = 0;
	
	sceClibMemset(&apu.chn1, 0, sizeof(apu_chn_t));
	sceClibMemset(&apu.chn2, 0, sizeof(apu_chn_t));
	sceClibMemset(&apu.chn3, 0, sizeof(apu_chn_t));
	sceClibMemset(&apu.chn4, 0, sizeof(apu_chn_t));
}

int freq_sweep_calc(apu_freq_sweep_t *this) {
	int res = this->shadow_freq >> this->shift;
	if (this->negate) {
		res = this->shadow_freq - res;
		this->negated = 1;
	} else {
		res = this->shadow_freq + res;
	}
	if (res > 2047) {
		this->overflown = 1;
	}
	return res;
}

void freq_sweep_trigger(apu_freq_sweep_t *this) {
	this->overflown = 0;
	this->negated = 0;
	this->shadow_freq = this->freq;
	this->timer = this->period ? this->period : 8;
	this->active = (this->period != 0) || (this->shift != 0);
	if (this->shift > 0) {
		freq_sweep_calc(this);
	}
}

void freq_sweep_step(apu_freq_sweep_t *this) {
	if (this->active) {
		this->timer--;
		if (this->timer <= 0) {
			if (this->period) {
				this->timer = this->period;
				int new_freq = freq_sweep_calc(this);
				if (this->shift != 0 && this->overflown != 0) {
					this->shadow_freq = new_freq;
					this->freq = new_freq;
					freq_sweep_calc(this);
				}
			} else {
				this->timer = 8;
			}
		}
	}
}

void vol_envelope_trigger(apu_vol_env_t *this) {
	this->vol = this->start_vol;
	this->active = 1;
	this->timer = this->period ? this->period : 8;
}

void vol_envelope_step(apu_vol_env_t *this) {
	if (this->active) {
		this->timer--;
		if (this->timer <= 0) {
			this->timer = this->period ? this->period : 8;
			if (this->add_mode && this->vol < 15) {
				this->vol++;
			} else if ((!this->add_mode) && this->vol > 0) {
				this->vol--;
			}
			if (this->vol == 0 || this->vol == 15) {
				this->active = 0;
			}
		}
	}
}

void len_counter_step(apu_len_cnt_t *this) {
	if (this->active && this->len > 0) {
		this->len--;
	}
}

void chn_len_clk(apu_chn_t *this) {
	len_counter_step(&this->len_counter);
	if (this->len_counter.active && this->len_counter.len != 0) {
		this->active = 0;
	}
}

void chn1_trigger() {
	apu.chn1.timer = (2048 - apu.chn1.freq_sweep.freq) << 2;
	vol_envelope_trigger(&apu.chn1.vol_envelope);
	freq_sweep_trigger(&apu.chn1.freq_sweep);
	if (apu.chn1.freq_sweep.active) {
		apu.chn1.active = apu.chn1.dac_active;
	} else {
		apu.chn1.active = 0;
	}
}

void chn1_sweep_clk() {
	freq_sweep_step(&apu.chn1.freq_sweep);
	apu.chn1.active = apu.chn1.freq_sweep.active;
}

void chn1_tick() {
	apu.chn1.timer--;
	if (apu.chn1.timer <= 0) {
		apu.chn1.timer = (2048 - apu.chn1.freq_sweep.freq) << 2;
		apu.chn1.seq = (apu.chn1.seq + 1) % 8;
		if (apu.chn1.active && duty_tbl[apu.chn1.duty][apu.chn1.seq]) {
			if (apu.chn1.vol_envelope.period > 0) {
				apu.chn1.out = apu.chn1.vol_envelope.vol;
			} else {
				apu.chn1.out = apu.chn1.vol_envelope.start_vol;
			}
		} else {
			apu.chn1.out = 0;
		}
	}
}

void chn2_trigger() {
	apu.chn2.timer = (2048 - apu.chn2.freq_sweep.freq) << 2;
	vol_envelope_trigger(&apu.chn2.vol_envelope);
	apu.chn2.active = apu.chn2.dac_active;
}

void chn2_tick() {
	apu.chn2.timer--;
	if (apu.chn2.timer <= 0) {
		apu.chn2.timer = (2048 - apu.chn2.freq_sweep.freq) << 2;
		apu.chn2.seq = (apu.chn2.seq + 1) % 8;
		if (apu.chn2.active && duty_tbl[apu.chn2.duty][apu.chn2.seq]) {
			if (apu.chn2.vol_envelope.period > 0) {
				apu.chn2.out = apu.chn2.vol_envelope.vol;
			} else {
				apu.chn2.out = apu.chn2.vol_envelope.start_vol;
			}
		} else {
			apu.chn2.out = 0;
		}
	}
}

void chn3_trigger() {
	if (apu.chn3.active && apu.chn3.timer == 2) {
		int pos = apu.chn3.pos >> 1;
		if (pos < 4) {
			wave_tbl[0] = wave_tbl[pos];
		} else {
			pos &= 0xFC;
			sceClibMemmove(&wave_tbl[0], &wave_tbl[pos], 4);
		}
	}
	apu.chn3.timer = 6;
	apu.chn3.pos = 0;
	apu.chn3.last_addr = 0;
	apu.chn3.active = apu.chn3.dac_active;
}

void chn3_tick() {
	apu.chn3.ticks++;
	apu.chn3.timer--;
	if (apu.chn3.timer <= 0) {
		apu.chn3.timer = (2048 - apu.chn3.freq_sweep.freq) << 1;
		if (apu.chn3.active) {
			apu.chn3.ticks = 0;
			apu.chn3.last_addr = apu.chn3.pos >> 1;
			if (apu.chn3.vol_code > 0) {
				apu.chn3.out = wave_tbl[apu.chn3.last_addr];
				if (apu.chn3.pos & 1) {
					apu.chn3.out &= 0x0F;
				} else {
					apu.chn3.out >>= 4;
				}
				apu.chn3.out >>= (apu.chn3.vol_code - 1);
			} else {
				apu.chn3.out = 0;
			}
			apu.chn3.pos = (apu.chn3.pos + 1) % 32;
		} else {
			apu.chn3.out = 0;
		}
	}
}

void chn4_tick() {
	// TODO
}

void apu_tick() {
	apu.sequencer_counter--;
	if (apu.sequencer_counter <= 0) {
		apu.sequencer_counter = 8192;
		if ((apu.sequencer_frame % 2) == 0) {
			if (apu.sequencer_frame != 4) {
				chn1_sweep_clk();
			}
			chn_len_clk(&apu.chn1);
			chn_len_clk(&apu.chn2);
			chn_len_clk(&apu.chn3);
			chn_len_clk(&apu.chn4);
		} else if (apu.sequencer_frame == 7) {
			vol_envelope_step(&apu.chn1.vol_envelope);
			vol_envelope_step(&apu.chn2.vol_envelope);
			vol_envelope_step(&apu.chn4.vol_envelope);
		}
		apu.sequencer_frame = (apu.sequencer_frame + 1) % 8;
		apu.chn1.len_counter.sequencer_frame = apu.sequencer_frame;
		apu.chn2.len_counter.sequencer_frame = apu.sequencer_frame;
		apu.chn3.len_counter.sequencer_frame = apu.sequencer_frame;
		apu.chn4.len_counter.sequencer_frame = apu.sequencer_frame;
	}
	
	chn1_tick();
	chn2_tick();
	chn3_tick();
	chn4_tick();
	
	apu.freq_counter--;
	if (apu.freq_counter <= 0) {
		apu.freq_counter = 95;
		
		int sample = 0;
		sample += apu.chn1.out;
		sample += apu.chn2.out;
		sample += apu.chn3.out;
		//sample += apu.chn4.out;
		audio_buffers[audio_push][samples_num++] = sample << 10;
		
		if (samples_num >= AUDIO_SAMPLES_NUM) {
			sceKernelSignalSema(audio_mutex, 1);
			audio_push = (audio_push + 1) % AUDIO_BUFFERS_NUM;
			samples_num = 0;
		}
	}
}

uint8_t apu_read_nr10(uint16_t addr) {
	uint8_t res = (apu.chn1.freq_sweep.period << 4) | apu.chn1.freq_sweep.shift;
	if (apu.chn1.freq_sweep.negate)
		res |= 0x08;
	return res;
}

void apu_write_nr10(uint16_t addr, uint8_t val) {
	apu.chn1.freq_sweep.period = (val >> 4) & 0x07;
	apu.chn1.freq_sweep.negate = (val & 0x08) == 0x08;
	apu.chn1.freq_sweep.shift = val & 0x07;
	
	if (apu.chn1.freq_sweep.negated && !apu.chn1.freq_sweep.negate) {
		apu.chn1.freq_sweep.overflown = 1;
	}
	
	if (!apu.chn1.freq_sweep.active) {
		apu.chn1.active = 0;
	}
}

uint8_t apu_read_nr11(uint16_t addr) {
	return (apu.chn1.duty << 6) | 0x3F;
}

void apu_write_nr11(uint16_t addr, uint8_t val) {
	apu.chn1.duty = val >> 6;
	apu.chn1.len_counter.len = apu.chn1.len_counter.full_len - (val & 0x3F);
}

uint8_t apu_read_nr12(uint16_t addr) {
	uint8_t res = (apu.chn1.vol_envelope.start_vol << 4) | apu.chn1.vol_envelope.period;
	if (apu.chn1.vol_envelope.add_mode) {
		res |= 0x08;
	}
	return res;
}

void apu_write_nr12(uint16_t addr, uint8_t val) {
	apu.chn1.dac_active = (val & 0xF8) != 0;
	apu.chn1.active &= apu.chn1.dac_active;
	apu.chn1.vol_envelope.start_vol = val >> 4;
	apu.chn1.vol_envelope.add_mode = (val & 0x08) == 0x08;
	apu.chn1.vol_envelope.period = val & 0x07;
}

uint8_t apu_read_nr13(uint16_t addr) {
	return 0xFF;
}

void apu_write_nr13(uint16_t addr, uint8_t val) {
	apu.chn1.freq_sweep.freq = (apu.chn1.freq_sweep.freq & 0x700) | val;
}

uint8_t apu_read_nr14(uint16_t addr) {
	if (apu.chn1.len_counter.active) {
		return 0xFF;
	} else {
		return 0xBF;
	}
}

void apu_write_nr14(uint16_t addr, uint8_t val) {
	apu.chn1.freq_sweep.freq = (apu.chn1.freq_sweep.freq & 0xFF) | ((val & 0x07) << 8);
	uint8_t active = (val & 0x40) == 0x40;
	uint8_t trigger = (val & 0x80) == 0x80;
	if (apu.chn1.len_counter.active) {
		if (trigger && apu.chn1.len_counter.len == 0) {
			apu.chn1.len_counter.len = apu.chn1.len_counter.full_len;
			if (active && (apu.chn1.len_counter.sequencer_frame & 1)) {
				apu.chn1.len_counter.len--;
			}
		}
	} else if (active) {
		if (apu.chn1.len_counter.sequencer_frame & 1) {
			if (apu.chn1.len_counter.len != 0) {
				apu.chn1.len_counter.len--;
			}
			if (trigger && apu.chn1.len_counter.len == 0) {
				apu.chn1.len_counter.len = apu.chn1.len_counter.full_len - 1;
			}
		}
	} else {
		if (trigger && apu.chn1.len_counter.len == 0) {
			apu.chn1.len_counter.len = apu.chn1.len_counter.full_len;
		}
	}
	apu.chn1.len_counter.active = active;
	if (apu.chn1.len_counter.active && apu.chn1.len_counter.len == 0) {
		apu.chn1.active = 0;
	} else if (trigger) {
		chn1_trigger();
	}
}

uint8_t apu_read_nr21(uint16_t addr) {
	return (apu.chn2.duty << 6) | 0x3F;
}

void apu_write_nr21(uint16_t addr, uint8_t val) {
	apu.chn2.duty = val >> 6;
	apu.chn2.len_counter.len = apu.chn2.len_counter.full_len - (val & 0x3F);
}

uint8_t apu_read_nr22(uint16_t addr) {
	uint8_t res = (apu.chn2.vol_envelope.start_vol << 4) | apu.chn2.vol_envelope.period;
	if (apu.chn2.vol_envelope.add_mode) {
		res |= 0x08;
	}
	return res;
}

void apu_write_nr22(uint16_t addr, uint8_t val) {
	apu.chn2.dac_active = (val & 0xF8) != 0;
	apu.chn2.active &= apu.chn1.dac_active;
	apu.chn2.vol_envelope.start_vol = val >> 4;
	apu.chn2.vol_envelope.add_mode = (val & 0x08) == 0x08;
	apu.chn2.vol_envelope.period = val & 0x07;
}

uint8_t apu_read_nr23(uint16_t addr) {
	return 0xFF;
}

void apu_write_nr23(uint16_t addr, uint8_t val) {
	apu.chn2.freq_sweep.freq = (apu.chn2.freq_sweep.freq & 0x700) | val;
}

uint8_t apu_read_nr24(uint16_t addr) {
	if (apu.chn2.len_counter.active) {
		return 0xFF;
	} else {
		return 0xBF;
	}
}

void apu_write_nr24(uint16_t addr, uint8_t val) {
	apu.chn2.freq_sweep.freq = (apu.chn2.freq_sweep.freq & 0xFF) | ((val & 0x07) << 8);
	uint8_t active = (val & 0x40) == 0x40;
	uint8_t trigger = (val & 0x80) == 0x80;
	if (apu.chn2.len_counter.active) {
		if (trigger && apu.chn2.len_counter.len == 0) {
			apu.chn2.len_counter.len = apu.chn2.len_counter.full_len;
			if (active && (apu.chn2.len_counter.sequencer_frame & 1)) {
				apu.chn2.len_counter.len--;
			}
		}
	} else if (active) {
		if (apu.chn2.len_counter.sequencer_frame & 1) {
			if (apu.chn2.len_counter.len != 0) {
				apu.chn2.len_counter.len--;
			}
			if (trigger && apu.chn2.len_counter.len == 0) {
				apu.chn2.len_counter.len = apu.chn2.len_counter.full_len - 1;
			}
		}
	} else {
		if (trigger && apu.chn2.len_counter.len == 0) {
			apu.chn2.len_counter.len = apu.chn2.len_counter.full_len;
		}
	}
	apu.chn2.len_counter.active = active;
	if (apu.chn2.len_counter.active && apu.chn2.len_counter.len == 0) {
		apu.chn2.active = 0;
	} else if (trigger) {
		chn2_trigger();
	}
}

uint8_t apu_read_nr30(uint16_t addr) {
	return apu.chn3.dac_active ? 0xFF : 0x7F;
}

void apu_write_nr30(uint16_t addr, uint8_t val) {
	if ((val & 0x80) == 0x80) {
		apu.chn3.dac_active = 1;
	}
	apu.chn3.active &= apu.chn3.dac_active;
}

uint8_t apu_read_nr31(uint16_t addr) {
	return 0xFF;
}

void apu_write_nr31(uint16_t addr, uint8_t val) {
	apu.chn3.len_counter.len = apu.chn3.len_counter.full_len - val;
}

uint8_t apu_read_nr32(uint16_t addr) {
	return 0x9F | (apu.chn3.vol_code << 5);
}

void apu_write_nr32(uint16_t addr, uint8_t val) {
	apu.chn3.vol_code = (val >> 5) & 0x03;
}

uint8_t apu_read_nr33(uint16_t addr) {
	return 0xFF;
}

void apu_write_nr33(uint16_t addr, uint8_t val) {
	apu.chn3.freq_sweep.freq = (apu.chn3.freq_sweep.freq & 0x700) | val;
}

uint8_t apu_read_nr34(uint16_t addr) {
	if (apu.chn1.len_counter.active) {
		return 0xFF;
	} else {
		return 0xBF;
	}
}

void apu_write_nr34(uint16_t addr, uint8_t val) {
	uint8_t active = (val & 0x40) == 0x40;
	uint8_t trigger = (val & 0x80) == 0x80;
	if (apu.chn3.len_counter.active) {
		if (trigger && apu.chn3.len_counter.len == 0) {
			apu.chn3.len_counter.len = apu.chn3.len_counter.full_len;
			if (active && (apu.chn3.len_counter.sequencer_frame & 1)) {
				apu.chn3.len_counter.len--;
			}
		}
	} else if (active) {
		if (apu.chn3.len_counter.sequencer_frame & 1) {
			if (apu.chn3.len_counter.len != 0) {
				apu.chn3.len_counter.len--;
			}
			if (trigger && apu.chn3.len_counter.len == 0) {
				apu.chn3.len_counter.len = apu.chn3.len_counter.full_len - 1;
			}
		}
	} else {
		if (trigger && apu.chn3.len_counter.len == 0) {
			apu.chn3.len_counter.len = apu.chn3.len_counter.full_len;
		}
	}
	apu.chn3.len_counter.active = active;
	apu.chn3.freq_sweep.freq = (apu.chn3.freq_sweep.freq & 0xFF) | ((val & 0x07) << 8);
	if (apu.chn3.len_counter.active && apu.chn3.len_counter.len == 0) {
		apu.chn3.active = 0;
	} else if (trigger) {
		chn3_trigger();
	}
}
