#ifndef _BUS_H_
#define _BUS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bus_wfuncs_t)(uint16_t, uint8_t);
typedef uint8_t (*bus_rfuncs_t)(uint16_t);
extern bus_wfuncs_t bus_write_funcs[0x10000];
extern bus_rfuncs_t bus_read_funcs[0x10000];

static inline __attribute__((always_inline)) uint8_t bus_read(uint16_t addr) {
	return bus_read_funcs[addr](addr);
}

static inline __attribute__((always_inline)) void bus_write(uint16_t addr, uint8_t val) {
	bus_write_funcs[addr](addr, val);
}

static inline __attribute__((always_inline)) void bus_write16(uint16_t addr, uint16_t val) {
	bus_write_funcs[addr](addr, val & 0xFF);
	bus_write_funcs[addr](addr + 1, (val >> 8) & 0xFF);
}

static inline __attribute__((always_inline)) uint16_t bus_read16(uint16_t addr) {
	uint16_t low = bus_read_funcs[addr](addr);
	uint16_t high = bus_read_funcs[addr + 1](addr + 1);
	return (low | (high << 8));
}

void bus_init();

#ifdef __cplusplus
}
#endif

#endif
