#ifndef _IO_H_
#define _IO_H_

#ifdef __cplusplus
extern "C" {
#endif

void io_init();
uint16_t io_read(uint16_t addr);
void io_write(uint16_t addr, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif
