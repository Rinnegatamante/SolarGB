#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "cpu.h"
#include "emu.h"

typedef void (*func_t)();
cpu_t cpu;
static char serial_out[1024 * 1024] = {};
static size_t serial_len = 0;

// CPU initialization
void cpu_init() {
	cpu.regs.PC = 0x100; // Entrypoint is fixed to 0x100
	cpu.regs.A = 0x01;
	cpu.regs.B = 0x00;
	cpu.regs.C = 0x13;
	cpu.regs.D = 0x00;
	cpu.regs.E = 0xD8;
	cpu.regs.F = 0xB0;
	cpu.regs.H = 0x01;
	cpu.regs.L = 0x4D;
	cpu.regs.SP = 0xFFFE;
	cpu.regs.IE = 0;
	cpu.master_interrupts = 0;
	cpu.enable_interrupts = 0;
	cpu.interrupts = 0x00;
	cpu.halted = 0;
}

// Bitwise instructions functions
void IN_xCB00() {
	uint8_t set = 0;
	uint8_t rval = cpu.regs.B;
	uint8_t res = rval << 1;
	if ((rval & 0x80) == 0x80) {
		res |= 1;
		set = 1;
	}
	cpu.regs.B = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, set);
}

void IN_xCB01() {
	uint8_t set = 0;
	uint8_t rval = cpu.regs.C;
	uint8_t res = rval << 1;
	if ((rval & 0x80) == 0x80) {
		res |= 1;
		set = 1;
	}
	cpu.regs.C = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, set);
}

void IN_xCB02() {
	uint8_t set = 0;
	uint8_t rval = cpu.regs.D;
	uint8_t res = rval << 1;
	if ((rval & 0x80) == 0x80) {
		res |= 1;
		set = 1;
	}
	cpu.regs.D = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, set);
}

void IN_xCB03() {
	uint8_t set = 0;
	uint8_t rval = cpu.regs.E;
	uint8_t res = rval << 1;
	if ((rval & 0x80) == 0x80) {
		res |= 1;
		set = 1;
	}
	cpu.regs.E = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, set);
}

void IN_xCB04() {
	uint8_t set = 0;
	uint8_t rval = cpu.regs.H;
	uint8_t res = rval << 1;
	if ((rval & 0x80) == 0x80) {
		res |= 1;
		set = 1;
	}
	cpu.regs.H = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, set);
}

void IN_xCB05() {
	uint8_t set = 0;
	uint8_t rval = cpu.regs.L;
	uint8_t res = rval << 1;
	if ((rval & 0x80) == 0x80) {
		res |= 1;
		set = 1;
	}
	cpu.regs.L = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, set);
}

void IN_xCB06() {
	emu_incr_cycles(2);
	uint8_t set = 0;
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t res = rval << 1;
	if ((rval & 0x80) == 0x80) {
		res |= 1;
		set = 1;
	}
	bus_write(*(uint16_t *)&cpu.regs.L, res);
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, set);
}

void IN_xCB07() {
	uint8_t set = 0;
	uint8_t rval = cpu.regs.A;
	uint8_t res = rval << 1;
	if ((rval & 0x80) == 0x80) {
		res |= 1;
		set = 1;
	}
	cpu.regs.A = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, set);
}

void IN_xCB08() {
	uint8_t res = cpu.regs.B;
	uint8_t rval = (res >> 1) | (res << 7);
	cpu.regs.B = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB09() {
	uint8_t res = cpu.regs.C;
	uint8_t rval = (res >> 1) | (res << 7);
	cpu.regs.C = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB0A() {
	uint8_t res = cpu.regs.D;
	uint8_t rval = (res >> 1) | (res << 7);
	cpu.regs.D = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB0B() {
	uint8_t res = cpu.regs.E;
	uint8_t rval = (res >> 1) | (res << 7);
	cpu.regs.E = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB0C() {
	uint8_t res = cpu.regs.H;
	uint8_t rval = (res >> 1) | (res << 7);
	cpu.regs.H = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB0D() {
	uint8_t res = cpu.regs.L;
	uint8_t rval = (res >> 1) | (res << 7);
	cpu.regs.L = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB0E() {
	emu_incr_cycles(2);
	uint8_t res = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t rval = (res >> 1) | (res << 7);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB0F() {
	uint8_t res = cpu.regs.A;
	uint8_t rval = (res >> 1) | (res << 7);
	cpu.regs.A = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB10() {
	uint8_t res = cpu.regs.B;
	uint8_t rval = (res << 1) | CPU_FLAG_C_SET;
	cpu.regs.B = rval;
	CPU_SET_FLAG(FLAG_Z, !rval);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB11() {
	uint8_t res = cpu.regs.C;
	uint8_t rval = (res << 1) | CPU_FLAG_C_SET;
	cpu.regs.C = rval;
	CPU_SET_FLAG(FLAG_Z, !rval);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB12() {
	uint8_t res = cpu.regs.D;
	uint8_t rval = (res << 1) | CPU_FLAG_C_SET;
	cpu.regs.D = rval;
	CPU_SET_FLAG(FLAG_Z, !rval);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB13() {
	uint8_t res = cpu.regs.E;
	uint8_t rval = (res << 1) | CPU_FLAG_C_SET;
	cpu.regs.E = rval;
	CPU_SET_FLAG(FLAG_Z, !rval);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB14() {
	uint8_t res = cpu.regs.H;
	uint8_t rval = (res << 1) | CPU_FLAG_C_SET;
	cpu.regs.H = rval;
	CPU_SET_FLAG(FLAG_Z, !rval);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB15() {
	uint8_t res = cpu.regs.L;
	uint8_t rval = (res << 1) | CPU_FLAG_C_SET;
	cpu.regs.L = rval;
	CPU_SET_FLAG(FLAG_Z, !rval);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB16() {
	emu_incr_cycles(2);
	uint8_t res = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t rval = (res << 1) | CPU_FLAG_C_SET;
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
	CPU_SET_FLAG(FLAG_Z, !rval);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB17() {
	uint8_t res = cpu.regs.A;
	uint8_t rval = (res << 1) | CPU_FLAG_C_SET;
	cpu.regs.A = rval;
	CPU_SET_FLAG(FLAG_Z, !rval);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB18() {
	uint8_t res = cpu.regs.B;
	uint8_t rval = (res >> 1) | (CPU_FLAG_C_SET << 7);
	cpu.regs.B = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB19() {
	uint8_t res = cpu.regs.C;
	uint8_t rval = (res >> 1) | (CPU_FLAG_C_SET << 7);
	cpu.regs.C = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB1A() {
	uint8_t res = cpu.regs.D;
	uint8_t rval = (res >> 1) | (CPU_FLAG_C_SET << 7);
	cpu.regs.D = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB1B() {
	uint8_t res = cpu.regs.E;
	uint8_t rval = (res >> 1) | (CPU_FLAG_C_SET << 7);
	cpu.regs.E = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB1C() {
	uint8_t res = cpu.regs.H;
	uint8_t rval = (res >> 1) | (CPU_FLAG_C_SET << 7);
	cpu.regs.H = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB1D() {
	uint8_t res = cpu.regs.L;
	uint8_t rval = (res >> 1) | (CPU_FLAG_C_SET << 7);
	cpu.regs.L = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB1E() {
	emu_incr_cycles(2);
	uint8_t res = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t rval = (res >> 1) | (CPU_FLAG_C_SET << 7);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB1F() {
	uint8_t res = cpu.regs.A;
	uint8_t rval = (res >> 1) | (CPU_FLAG_C_SET << 7);
	cpu.regs.A = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, res & 1);
}

void IN_xCB20() {
	uint8_t res = cpu.regs.B;
	uint8_t rval = res << 1;
	cpu.regs.B = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB21() {
	uint8_t res = cpu.regs.C;
	uint8_t rval = res << 1;
	cpu.regs.C = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB22() {
	uint8_t res = cpu.regs.D;
	uint8_t rval = res << 1;
	cpu.regs.D = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB23() {
	uint8_t res = cpu.regs.E;
	uint8_t rval = res << 1;
	cpu.regs.E = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB24() {
	uint8_t res = cpu.regs.H;
	uint8_t rval = res << 1;
	cpu.regs.H = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB25() {
	uint8_t res = cpu.regs.L;
	uint8_t rval = res << 1;
	cpu.regs.L = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB26() {
	emu_incr_cycles(2);
	uint8_t res = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t rval = res << 1;
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB27() {
	uint8_t res = cpu.regs.A;
	uint8_t rval = res << 1;
	cpu.regs.A = rval;
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
}

void IN_xCB28() {
	uint8_t rval = cpu.regs.B;
	uint8_t res = (int8_t)rval >> 1;
	cpu.regs.B = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB29() {
	uint8_t rval = cpu.regs.C;
	uint8_t res = (int8_t)rval >> 1;
	cpu.regs.C = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB2A() {
	uint8_t rval = cpu.regs.D;
	uint8_t res = (int8_t)rval >> 1;
	cpu.regs.D = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB2B() {
	uint8_t rval = cpu.regs.E;
	uint8_t res = (int8_t)rval >> 1;
	cpu.regs.E = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB2C() {
	uint8_t rval = cpu.regs.H;
	uint8_t res = (int8_t)rval >> 1;
	cpu.regs.H = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB2D() {
	uint8_t rval = cpu.regs.L;
	uint8_t res = (int8_t)rval >> 1;
	cpu.regs.L = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB2E() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t res = (int8_t)rval >> 1;
	bus_write(*(uint16_t *)&cpu.regs.L, res);
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB2F() {
	uint8_t rval = cpu.regs.A;
	uint8_t res = (int8_t)rval >> 1;
	cpu.regs.A = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB30() {
	cpu.regs.B = ((cpu.regs.B & 0xF0) >> 4) | ((cpu.regs.B & 0x0F) << 4);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.B == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xCB31() {
	cpu.regs.C = ((cpu.regs.C & 0xF0) >> 4) | ((cpu.regs.C & 0x0F) << 4);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.C == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xCB32() {
	cpu.regs.D = ((cpu.regs.D & 0xF0) >> 4) | ((cpu.regs.D & 0x0F) << 4);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.D == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xCB33() {
	cpu.regs.E = ((cpu.regs.E & 0xF0) >> 4) | ((cpu.regs.E & 0x0F) << 4);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.E == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xCB34() {
	cpu.regs.H = ((cpu.regs.H & 0xF0) >> 4) | ((cpu.regs.H & 0x0F) << 4);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.H == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xCB35() {
	cpu.regs.L = ((cpu.regs.L & 0xF0) >> 4) | ((cpu.regs.L & 0x0F) << 4);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.L == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xCB36() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	rval = ((rval & 0xF0) >> 4) | ((rval & 0x0F) << 4);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
	CPU_SET_FLAG(FLAG_Z, rval == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xCB37() {
	cpu.regs.A = ((cpu.regs.A & 0xF0) >> 4) | ((cpu.regs.A & 0x0F) << 4);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xCB38() {
	uint8_t rval = cpu.regs.B;
	uint8_t res = rval >> 1;
	cpu.regs.B = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB39() {
	uint8_t rval = cpu.regs.C;
	uint8_t res = rval >> 1;
	cpu.regs.C = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB3A() {
	uint8_t rval = cpu.regs.D;
	uint8_t res = rval >> 1;
	cpu.regs.D = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB3B() {
	uint8_t rval = cpu.regs.E;
	uint8_t res = rval >> 1;
	cpu.regs.E = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB3C() {
	uint8_t rval = cpu.regs.H;
	uint8_t res = rval >> 1;
	cpu.regs.H = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB3D() {
	uint8_t rval = cpu.regs.L;
	uint8_t res = rval >> 1;
	cpu.regs.L = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB3E() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t res = rval >> 1;
	bus_write(*(uint16_t *)&cpu.regs.L, res);
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB3F() {
	uint8_t rval = cpu.regs.A;
	uint8_t res = rval >> 1;
	cpu.regs.A = res;
	CPU_SET_FLAG(FLAG_Z, res == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, rval & 1);
}

void IN_xCB40() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.B & 0x01) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB41() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.C & 0x01) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB42() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.D & 0x01) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB43() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.E & 0x01) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB44() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.H & 0x01) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB45() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.L & 0x01) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB46() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, (rval & 0x01) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB47() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.A & 0x01) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB48() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.B & 0x02) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB49() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.C & 0x02) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB4A() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.D & 0x02) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB4B() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.E & 0x02) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB4C() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.H & 0x02) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB4D() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.L & 0x02) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB4E() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, (rval & 0x02) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB4F() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.A & 0x02) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB50() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.B & 0x04) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB51() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.C & 0x04) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB52() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.D & 0x04) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB53() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.E & 0x04) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB54() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.H & 0x04) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB55() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.L & 0x04) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB56() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, (rval & 0x04) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB57() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.A & 0x04) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB58() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.B & 0x08) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB59() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.C & 0x08) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB5A() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.D & 0x08) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB5B() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.E & 0x08) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB5C() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.H & 0x08) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB5D() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.L & 0x08) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB5E() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, (rval & 0x08) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB5F() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.A & 0x08) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB60() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.B & 0x10) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB61() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.C & 0x10) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB62() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.D & 0x10) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB63() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.E & 0x10) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB64() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.H & 0x10) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB65() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.L & 0x10) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB66() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, (rval & 0x10) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB67() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.A & 0x10) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB68() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.B & 0x20) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB69() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.C & 0x20) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB6A() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.D & 0x20) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB6B() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.E & 0x20) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB6C() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.H & 0x20) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB6D() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.L & 0x20) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB6E() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, (rval & 0x20) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB6F() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.A & 0x20) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB70() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.B & 0x40) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB71() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.C & 0x40) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB72() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.D & 0x40) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB73() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.E & 0x40) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB74() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.H & 0x40) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB75() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.L & 0x40) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB76() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, (rval & 0x40) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB77() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.A & 0x40) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB78() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.B & 0x80) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB79() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.C & 0x80) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB7A() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.D & 0x80) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB7B() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.E & 0x80) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB7C() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.H & 0x80) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB7D() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.L & 0x80) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB7E() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, (rval & 0x80) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB7F() {
	CPU_SET_FLAG(FLAG_Z, (cpu.regs.A & 0x80) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_xCB80() {
	cpu.regs.B &= ~(0x01);
}

void IN_xCB81() {
	cpu.regs.C &= ~(0x01);
}

void IN_xCB82() {
	cpu.regs.D &= ~(0x01);
}

void IN_xCB83() {
	cpu.regs.E &= ~(0x01);
}

void IN_xCB84() {
	cpu.regs.H &= ~(0x01);
}

void IN_xCB85() {
	cpu.regs.L &= ~(0x01);
}

void IN_xCB86() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L) & ~(0x01);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
}

void IN_xCB87() {
	cpu.regs.A &= ~(0x01);
}

void IN_xCB88() {
	cpu.regs.B &= ~(0x02);
}

void IN_xCB89() {
	cpu.regs.C &= ~(0x02);
}

void IN_xCB8A() {
	cpu.regs.D &= ~(0x02);
}

void IN_xCB8B() {
	cpu.regs.E &= ~(0x02);
}

void IN_xCB8C() {
	cpu.regs.H &= ~(0x02);
}

void IN_xCB8D() {
	cpu.regs.L &= ~(0x02);
}

void IN_xCB8E() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L) & ~(0x02);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
}

void IN_xCB8F() {
	cpu.regs.A &= ~(0x02);
}

void IN_xCB90() {
	cpu.regs.B &= ~(0x04);
}

void IN_xCB91() {
	cpu.regs.C &= ~(0x04);
}

void IN_xCB92() {
	cpu.regs.D &= ~(0x04);
}

void IN_xCB93() {
	cpu.regs.E &= ~(0x04);
}

void IN_xCB94() {
	cpu.regs.H &= ~(0x04);
}

void IN_xCB95() {
	cpu.regs.L &= ~(0x04);
}

void IN_xCB96() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L) & ~(0x04);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
}

void IN_xCB97() {
	cpu.regs.A &= ~(0x04);
}

void IN_xCB98() {
	cpu.regs.B &= ~(0x08);
}

void IN_xCB99() {
	cpu.regs.C &= ~(0x08);
}

void IN_xCB9A() {
	cpu.regs.D &= ~(0x08);
}

void IN_xCB9B() {
	cpu.regs.E &= ~(0x08);
}

void IN_xCB9C() {
	cpu.regs.H &= ~(0x08);
}

void IN_xCB9D() {
	cpu.regs.L &= ~(0x08);
}

void IN_xCB9E() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L) & ~(0x08);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
}

void IN_xCB9F() {
	cpu.regs.A &= ~(0x08);
}

void IN_xCBA0() {
	cpu.regs.B &= ~(0x10);
}

void IN_xCBA1() {
	cpu.regs.C &= ~(0x10);
}

void IN_xCBA2() {
	cpu.regs.D &= ~(0x10);
}

void IN_xCBA3() {
	cpu.regs.E &= ~(0x10);
}

void IN_xCBA4() {
	cpu.regs.H &= ~(0x10);
}

void IN_xCBA5() {
	cpu.regs.L &= ~(0x10);
}

void IN_xCBA6() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L) & ~(0x10);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
}

void IN_xCBA7() {
	cpu.regs.A &= ~(0x10);
}

void IN_xCBA8() {
	cpu.regs.B &= ~(0x20);
}

void IN_xCBA9() {
	cpu.regs.C &= ~(0x20);
}

void IN_xCBAA() {
	cpu.regs.D &= ~(0x20);
}

void IN_xCBAB() {
	cpu.regs.E &= ~(0x20);
}

void IN_xCBAC() {
	cpu.regs.H &= ~(0x20);
}

void IN_xCBAD() {
	cpu.regs.L &= ~(0x20);
}

void IN_xCBAE() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L) & ~(0x20);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
}

void IN_xCBAF() {
	cpu.regs.A &= ~(0x20);
}

void IN_xCBB0() {
	cpu.regs.B &= ~(0x40);
}

void IN_xCBB1() {
	cpu.regs.C &= ~(0x40);
}

void IN_xCBB2() {
	cpu.regs.D &= ~(0x40);
}

void IN_xCBB3() {
	cpu.regs.E &= ~(0x40);
}

void IN_xCBB4() {
	cpu.regs.H &= ~(0x40);
}

void IN_xCBB5() {
	cpu.regs.L &= ~(0x40);
}

void IN_xCBB6() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L) & ~(0x40);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
}

void IN_xCBB7() {
	cpu.regs.A &= ~(0x40);
}

void IN_xCBB8() {
	cpu.regs.B &= ~(0x80);
}

void IN_xCBB9() {
	cpu.regs.C &= ~(0x80);
}

void IN_xCBBA() {
	cpu.regs.D &= ~(0x80);
}

void IN_xCBBB() {
	cpu.regs.E &= ~(0x80);
}

void IN_xCBBC() {
	cpu.regs.H &= ~(0x80);
}

void IN_xCBBD() {
	cpu.regs.L &= ~(0x80);
}

void IN_xCBBE() {
	emu_incr_cycles(2);
	uint8_t rval = bus_read(*(uint16_t *)&cpu.regs.L) & ~(0x80);
	bus_write(*(uint16_t *)&cpu.regs.L, rval);
}

void IN_xCBBF() {
	cpu.regs.A &= ~(0x80);
}

void IN_xCBC0() {
	cpu.regs.B |= 0x01;
}

void IN_xCBC1() {
	cpu.regs.C |= 0x01;
}

void IN_xCBC2() {
	cpu.regs.D |= 0x01;
}

void IN_xCBC3() {
	cpu.regs.E |= 0x01;
}

void IN_xCBC4() {
	cpu.regs.H |= 0x01;
}

void IN_xCBC5() {
	cpu.regs.L |= 0x01;
}

void IN_xCBC6() {
	emu_incr_cycles(2);
	bus_write(*(uint16_t *)&cpu.regs.L, bus_read(*(uint16_t *)&cpu.regs.L) | 0x01);
}

void IN_xCBC7() {
	cpu.regs.A |= 0x01;
}

void IN_xCBC8() {
	cpu.regs.B |= 0x02;
}

void IN_xCBC9() {
	cpu.regs.C |= 0x02;
}

void IN_xCBCA() {
	cpu.regs.D |= 0x02;
}

void IN_xCBCB() {
	cpu.regs.E |= 0x02;
}

void IN_xCBCC() {
	cpu.regs.H |= 0x02;
}

void IN_xCBCD() {
	cpu.regs.L |= 0x02;
}

void IN_xCBCE() {
	emu_incr_cycles(2);
	bus_write(*(uint16_t *)&cpu.regs.L, bus_read(*(uint16_t *)&cpu.regs.L) | 0x02);
}

void IN_xCBCF() {
	cpu.regs.A |= 0x02;
}

void IN_xCBD0() {
	cpu.regs.B |= 0x04;
}

void IN_xCBD1() {
	cpu.regs.C |= 0x04;
}

void IN_xCBD2() {
	cpu.regs.D |= 0x04;
}

void IN_xCBD3() {
	cpu.regs.E |= 0x04;
}

void IN_xCBD4() {
	cpu.regs.H |= 0x04;
}

void IN_xCBD5() {
	cpu.regs.L |= 0x04;
}

void IN_xCBD6() {
	emu_incr_cycles(2);
	bus_write(*(uint16_t *)&cpu.regs.L, bus_read(*(uint16_t *)&cpu.regs.L) | 0x04);
}

void IN_xCBD7() {
	cpu.regs.A |= 0x04;
}

void IN_xCBD8() {
	cpu.regs.B |= 0x08;
}

void IN_xCBD9() {
	cpu.regs.C |= 0x08;
}

void IN_xCBDA() {
	cpu.regs.D |= 0x08;
}

void IN_xCBDB() {
	cpu.regs.E |= 0x08;
}

void IN_xCBDC() {
	cpu.regs.H |= 0x08;
}

void IN_xCBDD() {
	cpu.regs.L |= 0x08;
}

void IN_xCBDE() {
	emu_incr_cycles(2);
	bus_write(*(uint16_t *)&cpu.regs.L, bus_read(*(uint16_t *)&cpu.regs.L) | 0x08);
}

void IN_xCBDF() {
	cpu.regs.A |= 0x08;
}

void IN_xCBE0() {
	cpu.regs.B |= 0x10;
}

void IN_xCBE1() {
	cpu.regs.C |= 0x10;
}

void IN_xCBE2() {
	cpu.regs.D |= 0x10;
}

void IN_xCBE3() {
	cpu.regs.E |= 0x10;
}

void IN_xCBE4() {
	cpu.regs.H |= 0x10;
}

void IN_xCBE5() {
	cpu.regs.L |= 0x10;
}

void IN_xCBE6() {
	emu_incr_cycles(2);
	bus_write(*(uint16_t *)&cpu.regs.L, bus_read(*(uint16_t *)&cpu.regs.L) | 0x10);
}

void IN_xCBE7() {
	cpu.regs.A |= 0x10;
}

void IN_xCBE8() {
	cpu.regs.B |= 0x20;
}

void IN_xCBE9() {
	cpu.regs.C |= 0x20;
}

void IN_xCBEA() {
	cpu.regs.D |= 0x20;
}

void IN_xCBEB() {
	cpu.regs.E |= 0x20;
}

void IN_xCBEC() {
	cpu.regs.H |= 0x20;
}

void IN_xCBED() {
	cpu.regs.L |= 0x20;
}

void IN_xCBEE() {
	emu_incr_cycles(2);
	bus_write(*(uint16_t *)&cpu.regs.L, bus_read(*(uint16_t *)&cpu.regs.L) | 0x20);
}

void IN_xCBEF() {
	cpu.regs.A |= 0x20;
}

void IN_xCBF0() {
	cpu.regs.B |= 0x40;
}

void IN_xCBF1() {
	cpu.regs.C |= 0x40;
}

void IN_xCBF2() {
	cpu.regs.D |= 0x40;
}

void IN_xCBF3() {
	cpu.regs.E |= 0x40;
}

void IN_xCBF4() {
	cpu.regs.H |= 0x40;
}

void IN_xCBF5() {
	cpu.regs.L |= 0x40;
}

void IN_xCBF6() {
	emu_incr_cycles(2);
	bus_write(*(uint16_t *)&cpu.regs.L, bus_read(*(uint16_t *)&cpu.regs.L) | 0x40);
}

void IN_xCBF7() {
	cpu.regs.A |= 0x40;
}

void IN_xCBF8() {
	cpu.regs.B |= 0x80;
}

void IN_xCBF9() {
	cpu.regs.C |= 0x80;
}

void IN_xCBFA() {
	cpu.regs.D |= 0x80;
}

void IN_xCBFB() {
	cpu.regs.E |= 0x80;
}

void IN_xCBFC() {
	cpu.regs.H |= 0x80;
}

void IN_xCBFD() {
	cpu.regs.L |= 0x80;
}

void IN_xCBFE() {
	emu_incr_cycles(2);
	bus_write(*(uint16_t *)&cpu.regs.L, bus_read(*(uint16_t *)&cpu.regs.L) | 0x80);
}

void IN_xCBFF() {
	cpu.regs.A |= 0x80;
}

static func_t cb_instrs[] = {
	[0x00] = IN_xCB00,
	[0x01] = IN_xCB01,
	[0x02] = IN_xCB02,
	[0x03] = IN_xCB03,
	[0x04] = IN_xCB04,
	[0x05] = IN_xCB05,
	[0x06] = IN_xCB06,
	[0x07] = IN_xCB07,
	[0x08] = IN_xCB08,
	[0x09] = IN_xCB09,
	[0x0A] = IN_xCB0A,
	[0x0B] = IN_xCB0B,
	[0x0C] = IN_xCB0C,
	[0x0D] = IN_xCB0D,
	[0x0E] = IN_xCB0E,
	[0x0F] = IN_xCB0F,
	[0x10] = IN_xCB10,
	[0x11] = IN_xCB11,
	[0x12] = IN_xCB12,
	[0x13] = IN_xCB13,
	[0x14] = IN_xCB14,
	[0x15] = IN_xCB15,
	[0x16] = IN_xCB16,
	[0x17] = IN_xCB17,
	[0x18] = IN_xCB18,
	[0x19] = IN_xCB19,
	[0x1A] = IN_xCB1A,
	[0x1B] = IN_xCB1B,
	[0x1C] = IN_xCB1C,
	[0x1D] = IN_xCB1D,
	[0x1E] = IN_xCB1E,
	[0x1F] = IN_xCB1F,
	[0x20] = IN_xCB20,
	[0x21] = IN_xCB21,
	[0x22] = IN_xCB22,
	[0x23] = IN_xCB23,
	[0x24] = IN_xCB24,
	[0x25] = IN_xCB25,
	[0x26] = IN_xCB26,
	[0x27] = IN_xCB27,
	[0x28] = IN_xCB28,
	[0x29] = IN_xCB29,
	[0x2A] = IN_xCB2A,
	[0x2B] = IN_xCB2B,
	[0x2C] = IN_xCB2C,
	[0x2D] = IN_xCB2D,
	[0x2E] = IN_xCB2E,
	[0x2F] = IN_xCB2F,
	[0x30] = IN_xCB30,
	[0x31] = IN_xCB31,
	[0x32] = IN_xCB32,
	[0x33] = IN_xCB33,
	[0x34] = IN_xCB34,
	[0x35] = IN_xCB35,
	[0x36] = IN_xCB36,
	[0x37] = IN_xCB37,
	[0x38] = IN_xCB38,
	[0x39] = IN_xCB39,
	[0x3A] = IN_xCB3A,
	[0x3B] = IN_xCB3B,
	[0x3C] = IN_xCB3C,
	[0x3D] = IN_xCB3D,
	[0x3E] = IN_xCB3E,
	[0x3F] = IN_xCB3F,
	[0x40] = IN_xCB40,
	[0x41] = IN_xCB41,
	[0x42] = IN_xCB42,
	[0x43] = IN_xCB43,
	[0x44] = IN_xCB44,
	[0x45] = IN_xCB45,
	[0x46] = IN_xCB46,
	[0x47] = IN_xCB47,
	[0x48] = IN_xCB48,
	[0x49] = IN_xCB49,
	[0x4A] = IN_xCB4A,
	[0x4B] = IN_xCB4B,
	[0x4C] = IN_xCB4C,
	[0x4D] = IN_xCB4D,
	[0x4E] = IN_xCB4E,
	[0x4F] = IN_xCB4F,
	[0x50] = IN_xCB50,
	[0x51] = IN_xCB51,
	[0x52] = IN_xCB52,
	[0x53] = IN_xCB53,
	[0x54] = IN_xCB54,
	[0x55] = IN_xCB55,
	[0x56] = IN_xCB56,
	[0x57] = IN_xCB57,
	[0x58] = IN_xCB58,
	[0x59] = IN_xCB59,
	[0x5A] = IN_xCB5A,
	[0x5B] = IN_xCB5B,
	[0x5C] = IN_xCB5C,
	[0x5D] = IN_xCB5D,
	[0x5E] = IN_xCB5E,
	[0x5F] = IN_xCB5F,
	[0x60] = IN_xCB60,
	[0x61] = IN_xCB61,
	[0x62] = IN_xCB62,
	[0x63] = IN_xCB63,
	[0x64] = IN_xCB64,
	[0x65] = IN_xCB65,
	[0x66] = IN_xCB66,
	[0x67] = IN_xCB67,
	[0x68] = IN_xCB68,
	[0x69] = IN_xCB69,
	[0x6A] = IN_xCB6A,
	[0x6B] = IN_xCB6B,
	[0x6C] = IN_xCB6C,
	[0x6D] = IN_xCB6D,
	[0x6E] = IN_xCB6E,
	[0x6F] = IN_xCB6F,
	[0x70] = IN_xCB70,
	[0x71] = IN_xCB71,
	[0x72] = IN_xCB72,
	[0x73] = IN_xCB73,
	[0x74] = IN_xCB74,
	[0x75] = IN_xCB75,
	[0x76] = IN_xCB76,
	[0x77] = IN_xCB77,
	[0x78] = IN_xCB78,
	[0x79] = IN_xCB79,
	[0x7A] = IN_xCB7A,
	[0x7B] = IN_xCB7B,
	[0x7C] = IN_xCB7C,
	[0x7D] = IN_xCB7D,
	[0x7E] = IN_xCB7E,
	[0x7F] = IN_xCB7F,
	[0x80] = IN_xCB80,
	[0x81] = IN_xCB81,
	[0x82] = IN_xCB82,
	[0x83] = IN_xCB83,
	[0x84] = IN_xCB84,
	[0x85] = IN_xCB85,
	[0x86] = IN_xCB86,
	[0x87] = IN_xCB87,
	[0x88] = IN_xCB88,
	[0x89] = IN_xCB89,
	[0x8A] = IN_xCB8A,
	[0x8B] = IN_xCB8B,
	[0x8C] = IN_xCB8C,
	[0x8D] = IN_xCB8D,
	[0x8E] = IN_xCB8E,
	[0x8F] = IN_xCB8F,
	[0x90] = IN_xCB90,
	[0x91] = IN_xCB91,
	[0x92] = IN_xCB92,
	[0x93] = IN_xCB93,
	[0x94] = IN_xCB94,
	[0x95] = IN_xCB95,
	[0x96] = IN_xCB96,
	[0x97] = IN_xCB97,
	[0x98] = IN_xCB98,
	[0x99] = IN_xCB99,
	[0x9A] = IN_xCB9A,
	[0x9B] = IN_xCB9B,
	[0x9C] = IN_xCB9C,
	[0x9D] = IN_xCB9D,
	[0x9E] = IN_xCB9E,
	[0x9F] = IN_xCB9F,
	[0xA0] = IN_xCBA0,
	[0xA1] = IN_xCBA1,
	[0xA2] = IN_xCBA2,
	[0xA3] = IN_xCBA3,
	[0xA4] = IN_xCBA4,
	[0xA5] = IN_xCBA5,
	[0xA6] = IN_xCBA6,
	[0xA7] = IN_xCBA7,
	[0xA8] = IN_xCBA8,
	[0xA9] = IN_xCBA9,
	[0xAA] = IN_xCBAA,
	[0xAB] = IN_xCBAB,
	[0xAC] = IN_xCBAC,
	[0xAD] = IN_xCBAD,
	[0xAE] = IN_xCBAE,
	[0xAF] = IN_xCBAF,
	[0xB0] = IN_xCBB0,
	[0xB1] = IN_xCBB1,
	[0xB2] = IN_xCBB2,
	[0xB3] = IN_xCBB3,
	[0xB4] = IN_xCBB4,
	[0xB5] = IN_xCBB5,
	[0xB6] = IN_xCBB6,
	[0xB7] = IN_xCBB7,
	[0xB8] = IN_xCBB8,
	[0xB9] = IN_xCBB9,
	[0xBA] = IN_xCBBA,
	[0xBB] = IN_xCBBB,
	[0xBC] = IN_xCBBC,
	[0xBD] = IN_xCBBD,
	[0xBE] = IN_xCBBE,
	[0xBF] = IN_xCBBF,
	[0xC0] = IN_xCBC0,
	[0xC1] = IN_xCBC1,
	[0xC2] = IN_xCBC2,
	[0xC3] = IN_xCBC3,
	[0xC4] = IN_xCBC4,
	[0xC5] = IN_xCBC5,
	[0xC6] = IN_xCBC6,
	[0xC7] = IN_xCBC7,
	[0xC8] = IN_xCBC8,
	[0xC9] = IN_xCBC9,
	[0xCA] = IN_xCBCA,
	[0xCB] = IN_xCBCB,
	[0xCC] = IN_xCBCC,
	[0xCD] = IN_xCBCD,
	[0xCE] = IN_xCBCE,
	[0xCF] = IN_xCBCF,
	[0xD0] = IN_xCBD0,
	[0xD1] = IN_xCBD1,
	[0xD2] = IN_xCBD2,
	[0xD3] = IN_xCBD3,
	[0xD4] = IN_xCBD4,
	[0xD5] = IN_xCBD5,
	[0xD6] = IN_xCBD6,
	[0xD7] = IN_xCBD7,
	[0xD8] = IN_xCBD8,
	[0xD9] = IN_xCBD9,
	[0xDA] = IN_xCBDA,
	[0xDB] = IN_xCBDB,
	[0xDC] = IN_xCBDC,
	[0xDD] = IN_xCBDD,
	[0xDE] = IN_xCBDE,
	[0xDF] = IN_xCBDF,
	[0xE0] = IN_xCBE0,
	[0xE1] = IN_xCBE1,
	[0xE2] = IN_xCBE2,
	[0xE3] = IN_xCBE3,
	[0xE4] = IN_xCBE4,
	[0xE5] = IN_xCBE5,
	[0xE6] = IN_xCBE6,
	[0xE7] = IN_xCBE7,
	[0xE8] = IN_xCBE8,
	[0xE9] = IN_xCBE9,
	[0xEA] = IN_xCBEA,
	[0xEB] = IN_xCBEB,
	[0xEC] = IN_xCBEC,
	[0xED] = IN_xCBED,
	[0xEE] = IN_xCBEE,
	[0xEF] = IN_xCBEF,
	[0xF0] = IN_xCBF0,
	[0xF1] = IN_xCBF1,
	[0xF2] = IN_xCBF2,
	[0xF3] = IN_xCBF3,
	[0xF4] = IN_xCBF4,
	[0xF5] = IN_xCBF5,
	[0xF6] = IN_xCBF6,
	[0xF7] = IN_xCBF7,
	[0xF8] = IN_xCBF8,
	[0xF9] = IN_xCBF9,
	[0xFA] = IN_xCBFA,
	[0xFB] = IN_xCBFB,
	[0xFC] = IN_xCBFC,
	[0xFD] = IN_xCBFD,
	[0xFE] = IN_xCBFE,
	[0xFF] = IN_xCBFF,
};

void IN_x00() {
}

void IN_x01() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	uint16_t fetched_data = low | (high << 8);
	cpu.regs.PC += 2;
	*(uint16_t *)&cpu.regs.C = fetched_data;
}

void IN_x02() {
	bus_write(*(uint16_t *)&cpu.regs.C, cpu.regs.A);
	emu_incr_cycles(1);
}

void IN_x03() {
	emu_incr_cycles(1);
	*(uint16_t*)&cpu.regs.C = *(uint16_t*)&cpu.regs.C + 1;
}

void IN_x04() {
	cpu.regs.B++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.B == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.B & 0x0F) == 0);
}

void IN_x05() {
	cpu.regs.B--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.B == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.B & 0x0F) == 0x0F);
}

void IN_x06() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.B = fetched_data;
}

void IN_x07() {
	uint8_t val = (cpu.regs.A >> 7) & 1;
	cpu.regs.A = val | (cpu.regs.A << 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}

void IN_x08() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	uint16_t mem_dest = low | (high << 8);
	cpu.regs.PC += 2;
	emu_incr_cycles(1);
	bus_write16(mem_dest, cpu.regs.SP);
	emu_incr_cycles(1);
}

void IN_x09() {
	uint16_t fetched_data = *(uint16_t *)&cpu.regs.C;
	emu_incr_cycles(1);
	uint32_t reg = *(uint16_t *)&cpu.regs.L;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (fetched_data & 0x0FFF)) >= 0x1000 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
	*(uint16_t *)&cpu.regs.L = val & 0xFFFF;
}

void IN_x0A() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.C);
	emu_incr_cycles(1);
	cpu.regs.A = fetched_data;
}

void IN_x0B() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.C = *(uint16_t *)&cpu.regs.C - 1;
}

void IN_x0C() {
	cpu.regs.C++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.C == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.C & 0x0F) == 0);
}

void IN_x0D() {
	cpu.regs.C--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.C == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.C & 0x0F) == 0x0F);
}

void IN_x0E() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.C = fetched_data;
}

void IN_x0F() {
	uint8_t val = cpu.regs.A & 1;
	cpu.regs.A = (val << 7) | (cpu.regs.A >> 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}

void IN_x10() {
	sceClibPrintf("IN_STOP: NOIMPL\n");
}

void IN_x11() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	uint16_t fetched_data = low | (high << 8);
	cpu.regs.PC += 2;
	*(uint16_t *)&cpu.regs.E = fetched_data;
}

void IN_x12() {
	bus_write(*(uint16_t *)&cpu.regs.E, cpu.regs.A);
	emu_incr_cycles(1);
}

void IN_x13() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.E = *(uint16_t *)&cpu.regs.E + 1;
}

void IN_x14() {
	cpu.regs.D++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.D == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.D & 0x0F) == 0);
}

void IN_x15() {
	cpu.regs.D--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.D == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.D & 0x0F) == 0x0F);
}

void IN_x16() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.D = fetched_data;
}

void IN_x17() {
	uint8_t val = (cpu.regs.A >> 7) & 1;
	cpu.regs.A = CPU_FLAG_C_SET | (cpu.regs.A << 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}

void IN_x18() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	int8_t val = (int8_t)fetched_data;
	cpu.regs.PC += val;
	emu_incr_cycles(1);
}

void IN_x19() {
	uint16_t fetched_data = *(uint16_t *)&cpu.regs.E;
	emu_incr_cycles(1);
	uint32_t reg = *(uint16_t *)&cpu.regs.L;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (fetched_data & 0x0FFF)) >= 0x1000 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
	*(uint16_t *)&cpu.regs.L = val & 0xFFFF;
}

void IN_x1A() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.E);
	emu_incr_cycles(1);
	cpu.regs.A = fetched_data;
}

void IN_x1B() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.E = *(uint16_t *)&cpu.regs.E - 1;
}	

void IN_x1C() {
	cpu.regs.E++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.E == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.E & 0x0F) == 0);
}

void IN_x1D() {
	cpu.regs.E--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.E == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.E & 0x0F) == 0x0F);
}

void IN_x1E() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.E = fetched_data;
}

void IN_x1F() {
	uint8_t val = cpu.regs.A & 1;
	cpu.regs.A = (CPU_FLAG_C_SET << 7) | (cpu.regs.A >> 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}

void IN_x20() {
	if ((cpu.regs.F & FLAG_Z) == 0) {
		uint8_t fetched_data = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		int8_t val = (int8_t)fetched_data;
		cpu.regs.PC += 1 + val;
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(1);
		cpu.regs.PC++;
	}
}

void IN_x21() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	uint16_t fetched_data = low | (high << 8);
	cpu.regs.PC += 2;
	*(uint16_t *)&cpu.regs.L = fetched_data;
}

void IN_x22() {
	uint8_t fetched_data = cpu.regs.A;
	uint16_t mem_dest = *(uint16_t *)&cpu.regs.L;
	*(uint16_t *)&cpu.regs.L = *(uint16_t *)&cpu.regs.L + 1;
	bus_write(mem_dest, fetched_data);
	emu_incr_cycles(1);
}

void IN_x23() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.L = *(uint16_t *)&cpu.regs.L + 1;
}

void IN_x24() {
	cpu.regs.H++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.H == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.H & 0x0F) == 0);
}

void IN_x25() {
	cpu.regs.H--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.H == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.H & 0x0F) == 0x0F);
}

void IN_x26() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.H = fetched_data;
}

void IN_x27() {
	uint8_t val = 0;
	int val2 = 0;
	if (CPU_FLAG_H_SET || (((cpu.regs.A & 0x0F) > 9) && !CPU_FLAG_N_SET)) {
		val = 6;
	}
	if (CPU_FLAG_C_SET || ((cpu.regs.A > 0x99) && !CPU_FLAG_N_SET)) {
		val |= 0x60;
		val2 = 1;
	}
	cpu.regs.A += CPU_FLAG_N_SET ? -val : val;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val2);
}

void IN_x28() {
	if ((cpu.regs.F & FLAG_Z) == FLAG_Z) {
		uint8_t fetched_data = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		int8_t val = (int8_t)fetched_data;
		cpu.regs.PC += 1 + val;
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(1);
		cpu.regs.PC++;
	}
}

void IN_x29() {
	uint16_t fetched_data = *(uint16_t *)&cpu.regs.L;
	emu_incr_cycles(1);
	uint32_t reg = *(uint16_t *)&cpu.regs.L;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (fetched_data & 0x0FFF)) >= 0x1000 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
	*(uint16_t *)&cpu.regs.L = val & 0xFFFF;
}

void IN_x2A() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.L = *(uint16_t *)&cpu.regs.L + 1;
	cpu.regs.A = fetched_data;
}

void IN_x2B() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.L = *(uint16_t *)&cpu.regs.L - 1;
}

void IN_x2C() {
	cpu.regs.L++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.L == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.L & 0x0F) == 0);
}

void IN_x2D() {
	cpu.regs.L--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.L == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.L & 0x0F) == 0x0F);
}

void IN_x2E() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.L = fetched_data;
}

void IN_x2F() {
	cpu.regs.A = ~cpu.regs.A;
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, 1);
}

void IN_x30() {
	if ((cpu.regs.F & FLAG_C) == 0) {
		emu_incr_cycles(1);
		uint8_t fetched_data = bus_read(cpu.regs.PC);
		int8_t val = (int8_t)fetched_data;
		cpu.regs.PC += 1 + val;
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(1);
		cpu.regs.PC++;
	}
}

void IN_x31() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	uint16_t fetched_data = low | (high << 8);
	cpu.regs.PC += 2;
	cpu.regs.SP = fetched_data;
}

void IN_x32() {
	uint16_t mem_dest = *(uint16_t *)&cpu.regs.L;
	*(uint16_t *)&cpu.regs.L = *(uint16_t *)&cpu.regs.L - 1;
	bus_write(mem_dest, cpu.regs.A);
	emu_incr_cycles(1);
}

void IN_x33() {
	emu_incr_cycles(1);
	cpu.regs.SP++;
}

void IN_x34() {
	uint8_t val = (bus_read(*(uint16_t *)&cpu.regs.L) + 1) & 0xFF;
	bus_write(*(uint16_t *)&cpu.regs.L, val);
	emu_incr_cycles(2);
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (val & 0x0F) == 0);
}

void IN_x35() {
	uint8_t val = bus_read(*(uint16_t *)&cpu.regs.L) - 1;
	bus_write(*(uint16_t *)&cpu.regs.L, val);
	emu_incr_cycles(2);
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (val & 0x0F) == 0x0F);
}

void IN_x36() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	bus_write(*(uint16_t *)&cpu.regs.L, fetched_data);
	emu_incr_cycles(1);
}

void IN_x37() {
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 1);
}

void IN_x38() {
	if ((cpu.regs.F & FLAG_C) == FLAG_C) {
		emu_incr_cycles(1);
		uint8_t fetched_data = bus_read(cpu.regs.PC);
		int8_t val = (int8_t)fetched_data;
		cpu.regs.PC += 1 + val;
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(1);
		cpu.regs.PC++;
	}
}

void IN_x39() {
	emu_incr_cycles(1);
	uint32_t reg = *(uint16_t *)&cpu.regs.L;
	uint32_t val = reg + cpu.regs.SP;
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (cpu.regs.SP & 0x0FFF)) >= 0x1000 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
	*(uint16_t *)&cpu.regs.L = val & 0xFFFF;
}

void IN_x3A() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.L = *(uint16_t *)&cpu.regs.L - 1;
	cpu.regs.A = fetched_data;
}

void IN_x3B() {
	emu_incr_cycles(1);
	cpu.regs.SP--;
}

void IN_x3C() {
	cpu.regs.A++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) == 0);
}

void IN_x3D() {
	cpu.regs.A--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) == 0x0F);
}

void IN_x3E() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.A = fetched_data;
}

void IN_x3F() {
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, CPU_FLAG_C_SET ^ 1);
}

void IN_x40() {
}

void IN_x41() {
	cpu.regs.B = cpu.regs.C;
}

void IN_x42() {
	cpu.regs.B = cpu.regs.D;
}

void IN_x43() {
	cpu.regs.B = cpu.regs.E;
}

void IN_x44() {
	cpu.regs.B = cpu.regs.H;
}

void IN_x45() {
	cpu.regs.B = cpu.regs.L;
}

void IN_x46() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	cpu.regs.B = fetched_data;
}

void IN_x47() {
	cpu.regs.B = cpu.regs.A;
}

void IN_x48() {
	cpu.regs.C = cpu.regs.B;
}

void IN_x49() {
}

void IN_x4A() {
	cpu.regs.C = cpu.regs.D;
}

void IN_x4B() {
	cpu.regs.C = cpu.regs.E;
}

void IN_x4C() {
	cpu.regs.C = cpu.regs.H;
}

void IN_x4D() {
	cpu.regs.C = cpu.regs.L;
}

void IN_x4E() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	cpu.regs.C = fetched_data;
}

void IN_x4F() {
	cpu.regs.C = cpu.regs.A;
}

void IN_x50() {
	cpu.regs.D = cpu.regs.B;
}

void IN_x51() {
	cpu.regs.D = cpu.regs.C;
}

void IN_x52() {
}

void IN_x53() {
	cpu.regs.D = cpu.regs.E;
}

void IN_x54() {
	cpu.regs.D = cpu.regs.H;
}

void IN_x55() {
	cpu.regs.D = cpu.regs.L;
}

void IN_x56() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	cpu.regs.D = fetched_data;
}

void IN_x57() {
	cpu.regs.D = cpu.regs.A;
}

void IN_x58() {
	cpu.regs.E = cpu.regs.B;
}

void IN_x59() {
	cpu.regs.E = cpu.regs.C;
}

void IN_x5A() {
	cpu.regs.E = cpu.regs.D;
}

void IN_x5B() {
}

void IN_x5C() {
	cpu.regs.E = cpu.regs.H;
}

void IN_x5D() {
	cpu.regs.E = cpu.regs.L;
}

void IN_x5E() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	cpu.regs.E = fetched_data;
}

void IN_x5F() {
	cpu.regs.E = cpu.regs.A;
}

void IN_x60() {
	cpu.regs.H = cpu.regs.B;
}

void IN_x61() {
	cpu.regs.H = cpu.regs.C;
}

void IN_x62() {
	cpu.regs.H = cpu.regs.D;
}

void IN_x63() {
	cpu.regs.H = cpu.regs.E;
}

void IN_x64() {
}

void IN_x65() {
	cpu.regs.H = cpu.regs.L;
}

void IN_x66() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	cpu.regs.H = fetched_data;
}

void IN_x67() {
	cpu.regs.H = cpu.regs.A;
}

void IN_x68() {
	cpu.regs.L = cpu.regs.B;
}

void IN_x69() {
	cpu.regs.L = cpu.regs.C;
}

void IN_x6A() {
	cpu.regs.L = cpu.regs.D;
}

void IN_x6B() {
	cpu.regs.L = cpu.regs.E;
}

void IN_x6C() {
	cpu.regs.L = cpu.regs.H;
}

void IN_x6D() {
}

void IN_x6E() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	cpu.regs.L = fetched_data;
}

void IN_x6F() {
	cpu.regs.L = cpu.regs.A;
}

void IN_x70() {
	bus_write(*(uint16_t *)&cpu.regs.L, cpu.regs.B);
	emu_incr_cycles(1);
}

void IN_x71() {
	bus_write(*(uint16_t *)&cpu.regs.L, cpu.regs.C);
	emu_incr_cycles(1);
}

void IN_x72() {
	bus_write(*(uint16_t *)&cpu.regs.L, cpu.regs.D);
	emu_incr_cycles(1);
}

void IN_x73() {
	bus_write(*(uint16_t *)&cpu.regs.L, cpu.regs.E);
	emu_incr_cycles(1);
}

void IN_x74() {
	bus_write(*(uint16_t *)&cpu.regs.L, cpu.regs.H);
	emu_incr_cycles(1);
}

void IN_x75() {
	bus_write(*(uint16_t *)&cpu.regs.L, cpu.regs.L);
	emu_incr_cycles(1);
}

void IN_x76() {
	cpu.halted = 1;
}

void IN_x77() {
	bus_write(*(uint16_t *)&cpu.regs.L, cpu.regs.A);
	emu_incr_cycles(1);
}

void IN_x78() {
	cpu.regs.A = cpu.regs.B;
}

void IN_x79() {
	cpu.regs.A = cpu.regs.C;
}

void IN_x7A() {
	cpu.regs.A = cpu.regs.D;
}

void IN_x7B() {
	cpu.regs.A = cpu.regs.E;
}

void IN_x7C() {
	cpu.regs.A = cpu.regs.H;
}

void IN_x7D() {
	cpu.regs.A = cpu.regs.L;
}

void IN_x7E() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	cpu.regs.A = fetched_data;
}

void IN_x7F() {
}

void IN_x80() {
	uint8_t fetched_data = cpu.regs.B;
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_x81() {
	uint8_t fetched_data = cpu.regs.C;
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_x82() {
	uint8_t fetched_data = cpu.regs.D;
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_x83() {
	uint8_t fetched_data = cpu.regs.E;
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_x84() {
	uint8_t fetched_data = cpu.regs.H;
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_x85() {
	uint8_t fetched_data = cpu.regs.L;
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_x86() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_x87() {
	uint8_t fetched_data = cpu.regs.A;
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_x88() {
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + cpu.regs.B + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (cpu.regs.B & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + cpu.regs.B + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_x89() {
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + cpu.regs.C + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (cpu.regs.C & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + cpu.regs.C + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_x8A() {
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + cpu.regs.D + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (cpu.regs.D & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + cpu.regs.D + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_x8B() {
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + cpu.regs.E + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (cpu.regs.E & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + cpu.regs.E + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_x8C() {
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + cpu.regs.H + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (cpu.regs.H & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + cpu.regs.H + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_x8D() {
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + cpu.regs.L + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (cpu.regs.L & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + cpu.regs.L + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_x8E() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + fetched_data + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (fetched_data & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + fetched_data + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_x8F() {
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + reg + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (reg & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + reg + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_x90() {
	uint8_t val = cpu.regs.A - cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.B & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.B) < 0);
	cpu.regs.A = val;
}

void IN_x91() {
	uint8_t val = cpu.regs.A - cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.C & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.C) < 0);
	cpu.regs.A = val;
}

void IN_x92() {
	uint8_t val = cpu.regs.A - cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.D & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.D) < 0);
	cpu.regs.A = val;
}

void IN_x93() {
	uint8_t val = cpu.regs.A - cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.E & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.E) < 0);
	cpu.regs.A = val;
}

void IN_x94() {
	uint8_t val = cpu.regs.A - cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.H & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.H) < 0);
	cpu.regs.A = val;
}

void IN_x95() {
	uint8_t val = cpu.regs.A - cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.L & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.L) < 0);
	cpu.regs.A = val;
}

void IN_x96() {
	emu_incr_cycles(1);
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t val = cpu.regs.A - fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)fetched_data) < 0);
	cpu.regs.A = val;
}

void IN_x97() {
	CPU_SET_FLAG(FLAG_Z, 1);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
	cpu.regs.A = 0;
}

void IN_x98() {
	uint8_t fetched_data = cpu.regs.B;
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_x99() {
	uint8_t fetched_data = cpu.regs.C;
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_x9A() {
	uint8_t fetched_data = cpu.regs.D;
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_x9B() {
	uint8_t fetched_data = cpu.regs.E;
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_x9C() {
	uint8_t fetched_data = cpu.regs.H;
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_x9D() {
	uint8_t fetched_data = cpu.regs.L;
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_x9E() {
	emu_incr_cycles(1);
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_x9F() {
	uint8_t fetched_data = cpu.regs.A;
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_xA0() {
	cpu.regs.A &= cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA1() {
	cpu.regs.A &= cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA2() {
	cpu.regs.A &= cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA3() {
	cpu.regs.A &= cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA4() {
	cpu.regs.A &= cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA5() {
	cpu.regs.A &= cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA6() {
	emu_incr_cycles(1);
	cpu.regs.A &= bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA7() {
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA8() {
	cpu.regs.A ^= cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xA9() {
	cpu.regs.A ^= cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xAA() {
	cpu.regs.A ^= cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xAB() {
	cpu.regs.A ^= cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xAC() {
	cpu.regs.A ^= cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xAD() {
	cpu.regs.A ^= cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xAE() {
	emu_incr_cycles(1);
	cpu.regs.A ^= bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xAF() {
	cpu.regs.A = 0;
	CPU_SET_FLAG(FLAG_Z, 1);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB0() {
	cpu.regs.A |= cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB1() {
	cpu.regs.A |= cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB2() {
	cpu.regs.A |= cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB3() {
	cpu.regs.A |= cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB4() {
	cpu.regs.A |= cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB5() {
	cpu.regs.A |= cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB6() {
	emu_incr_cycles(1);
	cpu.regs.A |= bus_read(*(uint16_t *)&cpu.regs.L);
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB7() {
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xB8() {
	int val = (int)cpu.regs.A - (int)cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.B & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}

void IN_xB9() {
	int val = (int)cpu.regs.A - (int)cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.C & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}

void IN_xBA() {
	int val = (int)cpu.regs.A - (int)cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.D & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}

void IN_xBB() {
	int val = (int)cpu.regs.A - (int)cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.E & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}

void IN_xBC() {
	int val = (int)cpu.regs.A - (int)cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.H & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}

void IN_xBD() {
	int val = (int)cpu.regs.A - (int)cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.L & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}

void IN_xBE() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.L);
	emu_incr_cycles(1);
	int val = (int)cpu.regs.A - (int)fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}

void IN_xBF() {
	CPU_SET_FLAG(FLAG_Z, 1);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xC0() {
	emu_incr_cycles(1);
	if ((cpu.regs.F & FLAG_Z) == 0) {
		uint16_t low = stack_pop();
		emu_incr_cycles(1);
		uint16_t high = stack_pop();
		emu_incr_cycles(1);
		cpu.regs.PC = (high << 8) | low;
		emu_incr_cycles(1);
	}
}

void IN_xC1() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	uint16_t val = (high << 8) | low;
	*(uint16_t *)&cpu.regs.C = val;
}

void IN_xC2() {
	if ((cpu.regs.F & FLAG_Z) == 0) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(2);
		cpu.regs.PC += 2;
	}
}

void IN_xC3() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC = low | (high << 8);
	emu_incr_cycles(1);
}

void IN_xC4() {
	if ((cpu.regs.F & FLAG_Z) == 0) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		uint16_t fetched_data = low | (high << 8);
		emu_incr_cycles(2);
		stack_push16(cpu.regs.PC + 2);
		cpu.regs.PC = fetched_data;
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(2);
		cpu.regs.PC += 2;
	}
}

void IN_xC5() {
	uint16_t val = *(uint16_t *)&cpu.regs.C;
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}

void IN_xC6() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	uint32_t reg = cpu.regs.A;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void IN_xC7() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x00;
	emu_incr_cycles(1);
}

void IN_xC8() {
	emu_incr_cycles(1);
	if ((cpu.regs.F & FLAG_Z) == FLAG_Z) {
		uint16_t low = stack_pop();
		emu_incr_cycles(1);
		uint16_t high = stack_pop();
		emu_incr_cycles(1);
		cpu.regs.PC = (high << 8) | low;
		emu_incr_cycles(1);
	}
}

void IN_xC9() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	cpu.regs.PC = (high << 8) | low;
	emu_incr_cycles(1);
}

void IN_xCA() {
	if ((cpu.regs.F & FLAG_Z) == FLAG_Z) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(2);
		cpu.regs.PC += 2;
	}
}

void IN_xCB() {
	uint8_t opcode = bus_read(cpu.regs.PC++);
	emu_incr_cycles(2);
	cb_instrs[opcode]();
}

void IN_xCC() {
	if ((cpu.regs.F & FLAG_Z) == FLAG_Z) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		uint16_t fetched_data = low | (high << 8);
		emu_incr_cycles(2);
		stack_push16(cpu.regs.PC + 2);
		cpu.regs.PC = fetched_data;
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(2);
		cpu.regs.PC += 2;
	}
}

void IN_xCD() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	uint16_t fetched_data = low | (high << 8);
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC + 2);
	cpu.regs.PC = fetched_data;
	emu_incr_cycles(1);
}

void IN_xCE() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + fetched_data + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (fetched_data & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + fetched_data + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}

void IN_xCF() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x08;
	emu_incr_cycles(1);
}

void IN_xD0() {
	emu_incr_cycles(1);
	if ((cpu.regs.F & FLAG_C) == 0) {
		uint16_t low = stack_pop();
		emu_incr_cycles(1);
		uint16_t high = stack_pop();
		emu_incr_cycles(1);
		cpu.regs.PC = (high << 8) | low;
		emu_incr_cycles(1);
	}
}

void IN_xD1() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	uint16_t val = (high << 8) | low;
	*(uint16_t *)&cpu.regs.E = val;
}

void IN_xD2() {
	if ((cpu.regs.F & FLAG_C) == 0) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(2);
		cpu.regs.PC += 2;
	}
}

void IN_xD4() {
	if ((cpu.regs.F & FLAG_C) == 0) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		uint16_t fetched_data = low | (high << 8);
		emu_incr_cycles(2);
		stack_push16(cpu.regs.PC + 2);
		cpu.regs.PC = fetched_data;
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(2);
		cpu.regs.PC += 2;
	}
}

void IN_xD5() {
	uint16_t val = *(uint16_t *)&cpu.regs.E;
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}

void IN_xD6() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	uint16_t val = cpu.regs.A - fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)fetched_data) < 0);
	cpu.regs.A = val;
}

void IN_xD7() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x10;
	emu_incr_cycles(1);
}

void IN_xD8() {
	emu_incr_cycles(1);
	if ((cpu.regs.F & FLAG_C) == FLAG_C) {
		uint16_t low = stack_pop();
		emu_incr_cycles(1);
		uint16_t high = stack_pop();
		emu_incr_cycles(1);
		cpu.regs.PC = (high << 8) | low;
		emu_incr_cycles(1);
	}
}

void IN_xD9() {
	cpu.master_interrupts = 1;
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	cpu.regs.PC = (high << 8) | low;
	emu_incr_cycles(1);
}

void IN_xDA() {
	if ((cpu.regs.F & FLAG_C) == FLAG_C) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(2);
		cpu.regs.PC += 2;
	}
}

void IN_xDC() {
	if ((cpu.regs.F & FLAG_C) == FLAG_C) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		uint16_t fetched_data = low | (high << 8);
		emu_incr_cycles(2);
		stack_push16(cpu.regs.PC + 2);
		cpu.regs.PC = fetched_data;
		emu_incr_cycles(1);
	} else {
		emu_incr_cycles(2);
		cpu.regs.PC += 2;
	}
}

void IN_xDE() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}

void IN_xDF() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x18;
	emu_incr_cycles(1);
}

void IN_xE0() {
	uint16_t mem_dest = bus_read(cpu.regs.PC) | 0xFF00;
	emu_incr_cycles(1);
	cpu.regs.PC++;
	bus_write(mem_dest, cpu.regs.A);
	emu_incr_cycles(1);
}

void IN_xE1() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	uint16_t val = (high << 8) | low;
	*(uint16_t *)&cpu.regs.L = val;
}

void IN_xE2() {
	uint16_t mem_dest = cpu.regs.C | 0xFF00;
	bus_write(mem_dest, cpu.regs.A);
	emu_incr_cycles(1);
}

void IN_xE5() {
	uint16_t val = *(uint16_t *)&cpu.regs.L;
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}

void IN_xE6() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.A &= fetched_data;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xE7() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x20;
	emu_incr_cycles(1);
}

void IN_xE8() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	emu_incr_cycles(1);
	uint32_t reg = cpu.regs.SP;
	uint32_t val = reg + (int8_t)fetched_data;
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.SP = val & 0xFFFF;
	emu_incr_cycles(1);
}

void IN_xE9() {
	cpu.regs.PC = *(uint16_t *)&cpu.regs.L;
}

void IN_xEA() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	uint16_t mem_dest = low | (high << 8);
	cpu.regs.PC += 2;
	bus_write(mem_dest, cpu.regs.A);
	emu_incr_cycles(1);
}

void IN_xEE() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.A ^= fetched_data & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xEF() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x28;
	emu_incr_cycles(1);
}

void IN_xF0() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.A = bus_read(fetched_data | 0xFF00);
	emu_incr_cycles(1);
}

void IN_xF1() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	uint16_t val = (high << 8) | low;
	*(uint16_t *)&cpu.regs.F = val & 0xFFF0;
}

void IN_xF2() {
	uint8_t fetched_data = bus_read(((uint16_t)cpu.regs.C) | 0xFF00);
	emu_incr_cycles(1);
	cpu.regs.A = fetched_data;
}
	
void IN_xF3() {
	cpu.master_interrupts = 0;
}

void IN_xF5() {
	uint16_t val = *(uint16_t *)&cpu.regs.F;
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}

void IN_xF6() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.regs.A |= fetched_data;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}

void IN_xF7() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x30;
	emu_incr_cycles(1);
}

void IN_xF8() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.SP & 0x0F) + (fetched_data & 0x0F) >= 0x10);
	CPU_SET_FLAG(FLAG_C, (cpu.regs.SP & 0xFF) + (fetched_data & 0xFF) >= 0x100);
	*(uint16_t *)&cpu.regs.L = cpu.regs.SP + (int8_t)fetched_data;
	emu_incr_cycles(1);
}

void IN_xF9() {
	emu_incr_cycles(1);
	cpu.regs.SP = *(uint16_t *)&cpu.regs.L;
}

void IN_xFA() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	uint8_t fetched_data = bus_read(low | (high << 8));
	emu_incr_cycles(1);
	cpu.regs.A = fetched_data;
}

void IN_xFB() {
	cpu.enable_interrupts = 1;
}

void IN_xFE() {
	uint8_t fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	int val = (int)cpu.regs.A - (int)fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}

void IN_xFF() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x38;
	emu_incr_cycles(1);
}

static func_t instrs[] = {
	[0x00] = IN_x00,
	[0x01] = IN_x01,
	[0x02] = IN_x02,
	[0x03] = IN_x03,
	[0x04] = IN_x04,
	[0x05] = IN_x05,
	[0x06] = IN_x06,
	[0x07] = IN_x07,
	[0x08] = IN_x08,
	[0x09] = IN_x09,
	[0x0A] = IN_x0A,
	[0x0B] = IN_x0B,
	[0x0C] = IN_x0C,
	[0x0D] = IN_x0D,
	[0x0E] = IN_x0E,
	[0x0F] = IN_x0F,
	[0x10] = IN_x10,
	[0x11] = IN_x11,
	[0x12] = IN_x12,
	[0x13] = IN_x13,
	[0x14] = IN_x14,
	[0x15] = IN_x15,
	[0x16] = IN_x16,
	[0x17] = IN_x17,
	[0x18] = IN_x18,
	[0x19] = IN_x19,
	[0x1A] = IN_x1A,
	[0x1B] = IN_x1B,
	[0x1C] = IN_x1C,
	[0x1D] = IN_x1D,
	[0x1E] = IN_x1E,
	[0x1F] = IN_x1F,
	[0x20] = IN_x20,
	[0x21] = IN_x21,
	[0x22] = IN_x22,
	[0x23] = IN_x23,
	[0x24] = IN_x24,
	[0x25] = IN_x25,
	[0x26] = IN_x26,
	[0x27] = IN_x27,
	[0x28] = IN_x28,
	[0x29] = IN_x29,
	[0x2A] = IN_x2A,
	[0x2B] = IN_x2B,
	[0x2C] = IN_x2C,
	[0x2D] = IN_x2D,
	[0x2E] = IN_x2E,
	[0x2F] = IN_x2F,
	[0x30] = IN_x30,
	[0x31] = IN_x31,
	[0x32] = IN_x32,
	[0x33] = IN_x33,
	[0x34] = IN_x34,
	[0x35] = IN_x35,
	[0x36] = IN_x36,
	[0x37] = IN_x37,
	[0x38] = IN_x38,
	[0x39] = IN_x39,
	[0x3A] = IN_x3A,
	[0x3B] = IN_x3B,
	[0x3C] = IN_x3C,
	[0x3D] = IN_x3D,
	[0x3E] = IN_x3E,
	[0x3F] = IN_x3F,
	[0x40] = IN_x40,
	[0x41] = IN_x41,
	[0x42] = IN_x42,
	[0x43] = IN_x43,
	[0x44] = IN_x44,
	[0x45] = IN_x45,
	[0x46] = IN_x46,
	[0x47] = IN_x47,
	[0x48] = IN_x48,
	[0x49] = IN_x49,
	[0x4A] = IN_x4A,
	[0x4B] = IN_x4B,
	[0x4C] = IN_x4C,
	[0x4D] = IN_x4D,
	[0x4E] = IN_x4E,
	[0x4F] = IN_x4F,
	[0x50] = IN_x50,
	[0x51] = IN_x51,
	[0x52] = IN_x52,
	[0x53] = IN_x53,
	[0x54] = IN_x54,
	[0x55] = IN_x55,
	[0x56] = IN_x56,
	[0x57] = IN_x57,
	[0x58] = IN_x58,
	[0x59] = IN_x59,
	[0x5A] = IN_x5A,
	[0x5B] = IN_x5B,
	[0x5C] = IN_x5C,
	[0x5D] = IN_x5D,
	[0x5E] = IN_x5E,
	[0x5F] = IN_x5F,
	[0x60] = IN_x60,
	[0x61] = IN_x61,
	[0x62] = IN_x62,
	[0x63] = IN_x63,
	[0x64] = IN_x64,
	[0x65] = IN_x65,
	[0x66] = IN_x66,
	[0x67] = IN_x67,
	[0x68] = IN_x68,
	[0x69] = IN_x69,
	[0x6A] = IN_x6A,
	[0x6B] = IN_x6B,
	[0x6C] = IN_x6C,
	[0x6D] = IN_x6D,
	[0x6E] = IN_x6E,
	[0x6F] = IN_x6F,
	[0x70] = IN_x70,
	[0x71] = IN_x71,
	[0x72] = IN_x72,
	[0x73] = IN_x73,
	[0x74] = IN_x74,
	[0x75] = IN_x75,
	[0x76] = IN_x76,
	[0x77] = IN_x77,
	[0x78] = IN_x78,
	[0x79] = IN_x79,
	[0x7A] = IN_x7A,
	[0x7B] = IN_x7B,
	[0x7C] = IN_x7C,
	[0x7D] = IN_x7D,
	[0x7E] = IN_x7E,
	[0x7F] = IN_x7F,
	[0x80] = IN_x80,
	[0x81] = IN_x81,
	[0x82] = IN_x82,
	[0x83] = IN_x83,
	[0x84] = IN_x84,
	[0x85] = IN_x85,
	[0x86] = IN_x86,
	[0x87] = IN_x87,
	[0x88] = IN_x88,
	[0x89] = IN_x89,
	[0x8A] = IN_x8A,
	[0x8B] = IN_x8B,
	[0x8C] = IN_x8C,
	[0x8D] = IN_x8D,
	[0x8E] = IN_x8E,
	[0x8F] = IN_x8F,
	[0x90] = IN_x90,
	[0x91] = IN_x91,
	[0x92] = IN_x92,
	[0x93] = IN_x93,
	[0x94] = IN_x94,
	[0x95] = IN_x95,
	[0x96] = IN_x96,
	[0x97] = IN_x97,
	[0x98] = IN_x98,
	[0x99] = IN_x99,
	[0x9A] = IN_x9A,
	[0x9B] = IN_x9B,
	[0x9C] = IN_x9C,
	[0x9D] = IN_x9D,
	[0x9E] = IN_x9E,
	[0x9F] = IN_x9F,
	[0xA0] = IN_xA0,
	[0xA1] = IN_xA1,
	[0xA2] = IN_xA2,
	[0xA3] = IN_xA3,
	[0xA4] = IN_xA4,
	[0xA5] = IN_xA5,
	[0xA6] = IN_xA6,
	[0xA7] = IN_xA7,
	[0xA8] = IN_xA8,
	[0xA9] = IN_xA9,
	[0xAA] = IN_xAA,
	[0xAB] = IN_xAB,
	[0xAC] = IN_xAC,
	[0xAD] = IN_xAD,
	[0xAE] = IN_xAE,
	[0xAF] = IN_xAF,
	[0xB0] = IN_xB0,
	[0xB1] = IN_xB1,
	[0xB2] = IN_xB2,
	[0xB3] = IN_xB3,
	[0xB4] = IN_xB4,
	[0xB5] = IN_xB5,
	[0xB6] = IN_xB6,
	[0xB7] = IN_xB7,
	[0xB8] = IN_xB8,
	[0xB9] = IN_xB9,
	[0xBA] = IN_xBA,
	[0xBB] = IN_xBB,
	[0xBC] = IN_xBC,
	[0xBD] = IN_xBD,
	[0xBE] = IN_xBE,
	[0xBF] = IN_xBF,
	[0xC0] = IN_xC0,
	[0xC1] = IN_xC1,
	[0xC2] = IN_xC2,
	[0xC3] = IN_xC3,
	[0xC4] = IN_xC4,
	[0xC5] = IN_xC5,
	[0xC6] = IN_xC6,
	[0xC7] = IN_xC7,
	[0xC8] = IN_xC8,
	[0xC9] = IN_xC9,
	[0xCA] = IN_xCA,
	[0xCB] = IN_xCB,
	[0xCC] = IN_xCC,
	[0xCD] = IN_xCD,
	[0xCE] = IN_xCE,
	[0xCF] = IN_xCF,
	[0xD0] = IN_xD0,
	[0xD1] = IN_xD1,
	[0xD2] = IN_xD2,
	[0xD4] = IN_xD4,
	[0xD5] = IN_xD5,
	[0xD6] = IN_xD6,
	[0xD7] = IN_xD7,
	[0xD8] = IN_xD8,
	[0xD9] = IN_xD9,
	[0xDA] = IN_xDA,
	[0xDC] = IN_xDC,
	[0xDE] = IN_xDE,
	[0xDF] = IN_xDF,
	[0xE0] = IN_xE0,
	[0xE1] = IN_xE1,
	[0xE2] = IN_xE2,
	[0xE5] = IN_xE5,
	[0xE6] = IN_xE6,
	[0xE7] = IN_xE7,
	[0xE8] = IN_xE8,
	[0xE9] = IN_xE9,
	[0xEA] = IN_xEA,
	[0xEE] = IN_xEE,
	[0xEF] = IN_xEF,
	[0xF0] = IN_xF0,
	[0xF1] = IN_xF1,
	[0xF2] = IN_xF2,
	[0xF3] = IN_xF3,
	[0xF5] = IN_xF5,
	[0xF6] = IN_xF6,
	[0xF7] = IN_xF7,
	[0xF8] = IN_xF8,
	[0xF9] = IN_xF9,
	[0xFA] = IN_xFA,
	[0xFB] = IN_xFB,
	[0xFE] = IN_xFE,
	[0xFF] = IN_xFF,
};

void cpu_step() {
	if (cpu.halted) {
		// CPU is halted due to an interrupt
		emu_incr_cycles(1);
		
		if (cpu.interrupts) {
			cpu.halted = 0;
		}
	} else {
		// Fetch next instruction to execute and move forward program counter
		uint16_t instr_PC = cpu.regs.PC;
		cpu.opcode = bus_read(cpu.regs.PC++);
		
		emu_incr_cycles(1);
		instrs[cpu.opcode]();
		
		// Interpreter debugger
		if (emu.opts.debug_log) {
			char c = CPU_FLAG_C_SET ? 'C' : '-';
			char z = CPU_FLAG_Z_SET ? 'Z' : '-';
			char n = CPU_FLAG_N_SET ? 'N' : '-';
			char h = CPU_FLAG_H_SET ? 'H' : '-';
			sceClibPrintf("%04X: (%02X) A: %02X F: %c%c%c%c BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
				instr_PC, cpu.opcode, cpu.regs.A, z, n, h, c, cpu.regs.B, cpu.regs.C, cpu.regs.D, cpu.regs.E, cpu.regs.H, cpu.regs.L);
		}
		
		// Serial data handling
		if (emu.opts.serial_port_enabled) {
			if (bus_read(0xFF02) == 0x81) {
				uint8_t ch = bus_read(0xFF01);
				serial_out[serial_len++] = ch;
				serial_out[serial_len] = 0;
				bus_write(0xFF02, 0);
				sceClibPrintf("I/O: %s\n", serial_out);
			}
		}
	}

	// Interrupts handling
	if (cpu.master_interrupts) {
		if (CPU_MASTER_INTR_SET(IT_VBLANK)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &= ~IT_VBLANK;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		} else if (CPU_MASTER_INTR_SET(IT_LCD_STAT)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &= ~IT_LCD_STAT;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		} else if (CPU_MASTER_INTR_SET(IT_TIMER)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &= ~IT_TIMER;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		} else if (CPU_MASTER_INTR_SET(IT_SERIAL)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &= ~IT_SERIAL;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		} else if (CPU_MASTER_INTR_SET(IT_JOYPAD)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &= ~IT_JOYPAD;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		}
		cpu.enable_interrupts = 0;
	} else if (cpu.enable_interrupts) {
		cpu.master_interrupts = 1;
	}
}

uint8_t cpu_intr_read(uint16_t addr) {
	return cpu.interrupts;
}

void cpu_intr_write(uint16_t addr, uint8_t val) {
	cpu.interrupts = val;
}

void cpu_ie_write(uint16_t addr, uint8_t val) {
	cpu.regs.IE = val;
}

uint8_t cpu_ie_read(uint16_t addr) {
	return cpu.regs.IE;
}
