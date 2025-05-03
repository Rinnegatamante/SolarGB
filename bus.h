#ifndef _BUS_H_
#define _BUS_H_

#ifdef __cplusplus
extern "C" {
#endif

void bus_write(uint16_t addr, uint8_t val);
uint8_t bus_read(uint16_t addr);

static inline __attribute__((always_inline)) void bus_write16(uint16_t addr, uint16_t val) {
	bus_write(addr, val & 0xFF);
	bus_write(addr + 1, (val >> 8) & 0xFF);
}

static inline __attribute__((always_inline)) uint16_t bus_read16(uint16_t addr) {
	uint16_t low = bus_read(addr);
	uint16_t high = bus_read(addr + 1);
	return (low | (high << 8));
}

#ifdef __cplusplus
}
#endif

#endif
