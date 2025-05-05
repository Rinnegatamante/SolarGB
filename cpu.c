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

// Registers lookup table for bitwise operations
uint8_t rt_lookup[] = {
	RT_B,
	RT_C,
	RT_D,
	RT_E,
	RT_H,
	RT_L,
	RT_HL,
	RT_A,	
};

void _IN_LD_BC_D16() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	*(uint16_t *)&cpu.regs.B = low | (high << 8);
}
void _IN_LD_BC_A() {
	bus_write(*(uint16_t *)&cpu.regs.B, cpu.regs.A);
	emu_incr_cycles(1);
}
void _IN_INC_BC() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.B = *(uint16_t *)&cpu.regs.B + 1;
}
void _IN_INC_B() {
	cpu.regs.B++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.B == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.B & 0x0F) == 0);
}
void _IN_DEC_B() {
	cpu.regs.B--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.B == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.B & 0x0F) == 0x0F);
}
void _IN_LD_B_D8() {
	cpu.regs.B = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
}
void _IN_RLCA() {
	uint8_t val = (cpu.regs.A >> 7) & 1;
	cpu.regs.A = val | (cpu.regs.A << 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}
void _IN_LD_A16_SP() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	emu_incr_cycles(1);
	bus_write16(low | (high << 8), cpu.regs.SP);
	emu_incr_cycles(1);
}
void _IN_ADD_HL_BC() {
	uint16_t fetched_data = *(uint16_t *)&cpu.regs.B;
	emu_incr_cycles(1);
	uint32_t reg = *(uint16_t *)&cpu.regs.H;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (fetched_data & 0x0FFF)) >= 0x1000 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
	*(uint16_t *)&cpu.regs.H = val & 0xFFFF;
}
void _IN_LD_A_BC() {
	uint16_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.B);
	emu_incr_cycles(1);
	cpu.regs.A = fetched_data;
}
void _IN_DEC_BC() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.B = *(uint16_t *)&cpu.regs.B - 1;
}
void _IN_INC_C() {
	cpu.regs.C++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.C == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.C & 0x0F) == 0);
}
void _IN_DEC_C() {
	cpu.regs.C--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.C == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.C & 0x0F) == 0x0F);
}
void _IN_LD_C_D8() {
	cpu.regs.C = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
}
void _IN_RRCA() {
	uint8_t val = cpu.regs.A & 1;
	cpu.regs.A = (val << 7) | (cpu.regs.A >> 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}
void _IN_STOP() {
	sceClibPrintf("IN_STOP: NOIMPL\n");
}
void _IN_LD_DE_D16() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	*(uint16_t *)&cpu.regs.D = low | (high << 8);
}
void _IN_LD_DE_A() {
	bus_write(*(uint16_t *)&cpu.regs.D, cpu.regs.A);
	emu_incr_cycles(1);
}
void _IN_INC_DE() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.D = *(uint16_t *)&cpu.regs.D + 1;
}
void _IN_INC_D() {
	cpu.regs.D++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.D == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.D & 0x0F) == 0);
}
void _IN_DEC_D() {
	cpu.regs.D--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.D == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.D & 0x0F) == 0x0F);
}
void _IN_LD_D_D8() {
	cpu.regs.D = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
}
void _IN_RLA() {
	uint8_t val = (cpu.regs.A >> 7) & 1;
	cpu.regs.A = CPU_FLAG_C_SET | (cpu.regs.A << 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}
void _IN_JR_S8() {
	uint16_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	int8_t val = (int8_t)(fetched_data & 0xFF);
	cpu.regs.PC += val;
	emu_incr_cycles(1);
}
void _IN_ADD_HL_DE() {
	uint16_t fetched_data = *(uint16_t *)&cpu.regs.D;
	emu_incr_cycles(1);
	uint32_t reg = *(uint16_t *)&cpu.regs.H;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (fetched_data & 0x0FFF)) >= 0x1000 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
	*(uint16_t *)&cpu.regs.H = val & 0xFFFF;
}
void _IN_LD_A_DE() {
	cpu.regs.A = bus_read(*(uint16_t *)&cpu.regs.D);
	emu_incr_cycles(1);
}
void _IN_DEC_DE() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.D = *(uint16_t *)&cpu.regs.D - 1;
}
void _IN_INC_E() {
	cpu.regs.E++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.E == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.E & 0x0F) == 0);
}
void _IN_DEC_E() {
	cpu.regs.E--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.E == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.E & 0x0F) == 0x0F);
}
void _IN_LD_E_D8() {
	cpu.regs.E = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
}
void _IN_RRA() {
	uint8_t val = cpu.regs.A & 1;
	cpu.regs.A = (CPU_FLAG_C_SET << 7) | (cpu.regs.A >> 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}

void _IN_JR_NZ_S8() {
	emu_incr_cycles(1);
	if ((cpu.regs.F & FLAG_Z) == 0) {
		uint8_t fetched_data = bus_read(cpu.regs.PC++);
		int8_t val = (int8_t)(fetched_data & 0xFF);
		cpu.regs.PC += val;
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC++;
	}
}
void _IN_LD_HL_D16() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	*(uint16_t *)&cpu.regs.H = low | (high << 8);
}
void _IN_LD_HLP_A() {
	*(uint16_t *)&cpu.regs.H = *(uint16_t *)&cpu.regs.H + 1;
	bus_write(*(uint16_t *)&cpu.regs.H, cpu.regs.A);
	emu_incr_cycles(1);
}
void _IN_INC_HL() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.H = *(uint16_t *)&cpu.regs.H + 1;
}
void _IN_INC_H() {
	cpu.regs.H++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.H == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.H & 0x0F) == 0);
}
void _IN_DEC_H() {
	cpu.regs.H--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.H == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.H & 0x0F) == 0x0F);
}
void _IN_LD_H_D8() {
	cpu.regs.H = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
}
void _IN_DAA() {
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
void _IN_JR_Z_S8() {
	emu_incr_cycles(1);
	if ((cpu.regs.F & FLAG_Z) == FLAG_Z) {
		uint16_t fetched_data = bus_read(cpu.regs.PC++);
		int8_t val = (int8_t)(fetched_data & 0xFF);
		cpu.regs.PC += val;
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC++;
	}
}
void _IN_ADD_HL_HL() {
	emu_incr_cycles(1);
	uint32_t reg = *(uint16_t *)&cpu.regs.H;
	uint32_t val = reg * 2;
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (reg & 0x0FFF)) >= 0x1000 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
	*(uint16_t *)&cpu.regs.H = val & 0xFFFF;
}
void _IN_LD_A_HLP() {
	uint16_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.H = *(uint16_t *)&cpu.regs.H + 1;
	cpu.regs.A = fetched_data;
}
void _IN_DEC_HL() {
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.H = *(uint16_t *)&cpu.regs.H - 1;	
}
void _IN_INC_L() {
	cpu.regs.L++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.L == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.L & 0x0F) == 0);
}
void _IN_DEC_L() {
	cpu.regs.L--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.L == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.L & 0x0F) == 0x0F);
}
void _IN_LD_L_D8() {
	cpu.regs.L = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
}
void _IN_CPL() {
	cpu.regs.A = ~cpu.regs.A;
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, 1);
}
void _IN_DI() {
	cpu.master_interrupts = 0;
}
void _IN_EI() {
	cpu.enable_interrupts = 1;
}
void _IN_SCF() {
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 1);
}
void _IN_CCF() {
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, CPU_FLAG_C_SET ^ 1);
}
void _IN_HALT() {
	cpu.halted = 1;
}
void _IN_JR_NC_S8() {
	emu_incr_cycles(1);
	if ((cpu.regs.F & FLAG_C) == 0) {
		uint16_t fetched_data = bus_read(cpu.regs.PC++);
		int8_t val = (int8_t)(fetched_data & 0xFF);
		cpu.regs.PC += val;
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC++;
	}
}
void _IN_LD_SP_D16() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	cpu.regs.SP = low | (high << 8);
}
void _IN_LD_HLM_A() {
	uint16_t mem_dest = *(uint16_t *)&cpu.regs.H;
	*(uint16_t *)&cpu.regs.H = *(uint16_t *)&cpu.regs.H - 1;
	bus_write(mem_dest, cpu.regs.A);
	emu_incr_cycles(1);
}
void _IN_INC_SP() {
	emu_incr_cycles(1);
	cpu.regs.SP++;
}
void _IN_INC_MHL() {
	emu_incr_cycles(2);
	uint16_t hl = *(uint16_t *)&cpu.regs.H;
	uint16_t val = (bus_read(hl) + 1) & 0xFF;
	bus_write(hl, val);
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (val & 0x0F) == 0);
}
void _IN_DEC_MHL() {
	emu_incr_cycles(2);
	uint16_t hl = *(uint16_t *)&cpu.regs.H;
	uint16_t val = bus_read(hl) - 1;
	bus_write(hl, val);
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (val & 0x0F) == 0x0F);
}
void _IN_LD_HL_D8() {
	emu_incr_cycles(1);
	bus_write(*(uint16_t *)&cpu.regs.H, bus_read(cpu.regs.PC++));
	emu_incr_cycles(1);
}
void _IN_NOP() {}
void _IN_JR_C_E8() {
	if ((cpu.regs.F & FLAG_C) == FLAG_C) {
		uint16_t fetched_data = bus_read(cpu.regs.PC++);
		int8_t val = (int8_t)(fetched_data & 0xFF);
		cpu.regs.PC += val;
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC++;
	}
}
void _IN_ADD_HL_SP() {
	uint16_t fetched_data = cpu.regs.SP;
	emu_incr_cycles(1);
	uint32_t reg = *(uint16_t *)&cpu.regs.H;
	uint32_t val = reg + fetched_data;
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (fetched_data & 0x0FFF)) >= 0x1000 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
	*(uint16_t *)&cpu.regs.H = val & 0xFFFF;
}
void _IN_LD_A_HLM() {
	cpu.regs.A = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.H = *(uint16_t *)&cpu.regs.H - 1;
}
void _IN_DEC_SP() {
	emu_incr_cycles(1);
	cpu.regs.SP--;
}
void _IN_INC_A() {
	cpu.regs.A++;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) == 0);
}
void _IN_DEC_A() {
	cpu.regs.A--;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) == 0x0F);
}
void _IN_LD_A_N8() {
	cpu.regs.A = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
}
void _IN_LD_B_C() {
	cpu.regs.B = cpu.regs.C;
}
void _IN_LD_B_D() {
	cpu.regs.B = cpu.regs.D;
}
void _IN_LD_B_E() {
	cpu.regs.B = cpu.regs.E;
}
void _IN_LD_B_H() {
	cpu.regs.B = cpu.regs.H;
}
void _IN_LD_B_L() {
	cpu.regs.B = cpu.regs.L;
}
void _IN_LD_B_HL() {
	cpu.regs.B = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
}
void _IN_LD_B_A() {
	cpu.regs.B = cpu.regs.A;
}
void _IN_LD_C_B() {
	cpu.regs.C = cpu.regs.B;
}
void _IN_LD_C_D() {
	cpu.regs.C = cpu.regs.D;
}
void _IN_LD_C_E() {
	cpu.regs.C = cpu.regs.E;
}
void _IN_LD_C_H() {
	cpu.regs.C = cpu.regs.H;
}
void _IN_LD_C_L() {
	cpu.regs.C = cpu.regs.L;
}
void _IN_LD_C_HL() {
	cpu.regs.C = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
}
void _IN_LD_C_A() {
	cpu.regs.D = cpu.regs.A;
}
void _IN_LD_D_B() {
	cpu.regs.D = cpu.regs.B;
}
void _IN_LD_D_C() {
	cpu.regs.D = cpu.regs.C;
}
void _IN_LD_D_E() {
	cpu.regs.D = cpu.regs.E;
}
void _IN_LD_D_H() {
	cpu.regs.D = cpu.regs.H;
}
void _IN_LD_D_L() {
	cpu.regs.D = cpu.regs.L;
}
void _IN_LD_D_HL() {
	cpu.regs.D = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
}
void _IN_LD_D_A() {
	cpu.regs.D = cpu.regs.A;
}
void _IN_LD_E_B() {
	cpu.regs.D = cpu.regs.B;
}
void _IN_LD_E_C() {
	cpu.regs.D = cpu.regs.C;
}
void _IN_LD_E_D() {
	cpu.regs.E = cpu.regs.D;
}
void _IN_LD_E_H() {
	cpu.regs.E = cpu.regs.H;
}
void _IN_LD_E_L() {
	cpu.regs.E = cpu.regs.L;
}
void _IN_LD_E_HL() {
	cpu.regs.E = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
}
void _IN_LD_E_A() {
	cpu.regs.E = cpu.regs.A;
}
void _IN_LD_H_B() {
	cpu.regs.H = cpu.regs.B;
}
void _IN_LD_H_C() {
	cpu.regs.H = cpu.regs.C;
}
void _IN_LD_H_D() {
	cpu.regs.H = cpu.regs.D;
}
void _IN_LD_H_E() {
	cpu.regs.H = cpu.regs.E;
}
void _IN_LD_H_L() {
	cpu.regs.H = cpu.regs.L;
}
void _IN_LD_H_HL() {
	cpu.regs.H = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
}
void _IN_LD_H_A() {
	cpu.regs.H = cpu.regs.A;
}
void _IN_LD_L_B() {
	cpu.regs.L = cpu.regs.B;
}
void _IN_LD_L_C() {
	cpu.regs.L = cpu.regs.C;
}
void _IN_LD_L_D() {
	cpu.regs.L = cpu.regs.D;
}
void _IN_LD_L_E() {
	cpu.regs.L = cpu.regs.E;
}
void _IN_LD_L_H() {
	cpu.regs.L = cpu.regs.H;
}
void _IN_LD_L_HL() {
	cpu.regs.L = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
}
void _IN_LD_L_A() {
	cpu.regs.L = cpu.regs.A;
}
void _IN_LD_HL_B() {
	bus_write(*(uint16_t *)&cpu.regs.H, cpu.regs.B);
	emu_incr_cycles(1);
}
void _IN_LD_HL_C() {
	bus_write(*(uint16_t *)&cpu.regs.H, cpu.regs.C);
	emu_incr_cycles(1);
}
void _IN_LD_HL_D() {
	bus_write(*(uint16_t *)&cpu.regs.H, cpu.regs.D);
	emu_incr_cycles(1);
}
void _IN_LD_HL_E() {
	bus_write(*(uint16_t *)&cpu.regs.H, cpu.regs.E);
	emu_incr_cycles(1);
}
void _IN_LD_HL_H() {
	bus_write(*(uint16_t *)&cpu.regs.H, cpu.regs.H);
	emu_incr_cycles(1);
}
void _IN_LD_HL_L() {
	bus_write(*(uint16_t *)&cpu.regs.H, cpu.regs.L);
	emu_incr_cycles(1);
}
void _IN_LD_HL_A() {
	bus_write(*(uint16_t *)&cpu.regs.H, cpu.regs.A);
	emu_incr_cycles(1);
}
void _IN_LD_A_B() {
	cpu.regs.A = cpu.regs.B;
}
void _IN_LD_A_C() {
	cpu.regs.A = cpu.regs.C;
}
void _IN_LD_A_D() {
	cpu.regs.A = cpu.regs.D;
}
void _IN_LD_A_E() {
	cpu.regs.A = cpu.regs.E;
}
void _IN_LD_A_H() {
	cpu.regs.A = cpu.regs.H;
}
void _IN_LD_A_HL() {
	cpu.regs.A = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
}
void _IN_LD_A_L() {
	cpu.regs.A = cpu.regs.L;
}
void _IN_ADD_A_B() {
	uint32_t val = cpu.regs.A + cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (cpu.regs.B & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}
void _IN_ADD_A_C() {
	uint32_t val = cpu.regs.A + cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (cpu.regs.C & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}
void _IN_ADD_A_D() {
	uint32_t val = cpu.regs.A + cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (cpu.regs.D & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}
void _IN_ADD_A_E() {
	uint32_t val = cpu.regs.A + cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (cpu.regs.E & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}
void _IN_ADD_A_H() {
	uint32_t val = cpu.regs.A + cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (cpu.regs.H & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}
void _IN_ADD_A_L() {
	uint32_t val = cpu.regs.A + cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (cpu.regs.L & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}

void _IN_ADD_A_HL() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	uint32_t val = cpu.regs.A + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}
void _IN_ADD_A_A() {
	uint32_t val = cpu.regs.A * 2;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (cpu.regs.A & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, val >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}
void _IN_ADC_A_A() {
	cpu.regs.A = ((uint16_t)cpu.regs.A + (uint16_t)cpu.regs.A + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) + (cpu.regs.A & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, (uint16_t)cpu.regs.A + (uint16_t)cpu.regs.A + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_ADC_A_B() {
	cpu.regs.A = ((uint16_t)cpu.regs.A + (uint16_t)cpu.regs.B + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) + (cpu.regs.B & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, (uint16_t)cpu.regs.A + (uint16_t)cpu.regs.B + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_ADC_A_C() {
	cpu.regs.A = ((uint16_t)cpu.regs.A + (uint16_t)cpu.regs.C + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) + (cpu.regs.C & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, (uint16_t)cpu.regs.A + (uint16_t)cpu.regs.C + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_ADC_A_D() {
	cpu.regs.A = ((uint16_t)cpu.regs.A + (uint16_t)cpu.regs.D + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) + (cpu.regs.D & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, (uint16_t)cpu.regs.A + (uint16_t)cpu.regs.D + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_ADC_A_E() {
	cpu.regs.A = ((uint16_t)cpu.regs.A + (uint16_t)cpu.regs.E + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) + (cpu.regs.E & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, (uint16_t)cpu.regs.A + (uint16_t)cpu.regs.E + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_ADC_A_H() {
	cpu.regs.A = ((uint16_t)cpu.regs.A + (uint16_t)cpu.regs.H + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) + (cpu.regs.H & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, (uint16_t)cpu.regs.A + (uint16_t)cpu.regs.H + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_ADC_A_L() {
	cpu.regs.A = ((uint16_t)cpu.regs.A + (uint16_t)cpu.regs.L + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) + (cpu.regs.L & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, (uint16_t)cpu.regs.A + (uint16_t)cpu.regs.L + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_ADC_A_HL() {
	uint16_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	cpu.regs.A = ((uint16_t)cpu.regs.A + fetched_data + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.A & 0x0F) + (fetched_data & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, (uint16_t)cpu.regs.A + fetched_data + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_RETI() {
	cpu.master_interrupts = 1;
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	cpu.regs.PC = (high << 8) | low;
	emu_incr_cycles(1);
}
void _IN_RET() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	cpu.regs.PC = (high << 8) | low;
	emu_incr_cycles(1);
}
void _IN_RET_C() {
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
void _IN_RET_NC() {
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
void _IN_RET_Z() {
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
void _IN_RET_NZ() {
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
void _IN_RST_00() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x00;
	emu_incr_cycles(1);
}
void _IN_RST_08() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x08;
	emu_incr_cycles(1);
}
void _IN_RST_10() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x10;
	emu_incr_cycles(1);
}
void _IN_RST_18() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x18;
	emu_incr_cycles(1);
}
void _IN_RST_20() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x20;
	emu_incr_cycles(1);
}
void _IN_RST_28() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x28;
	emu_incr_cycles(1);
}
void _IN_RST_30() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x30;
	emu_incr_cycles(1);
}
void _IN_RST_38() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = 0x38;
	emu_incr_cycles(1);
}

void _IN_SUB_A_B() {
	uint16_t val = cpu.regs.A - cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.B & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.B) < 0);
	cpu.regs.A = val;
}
void _IN_SUB_A_C() {
	uint16_t val = cpu.regs.A - cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.C & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.C) < 0);
	cpu.regs.A = val;
}
void _IN_SUB_A_D() {
	uint16_t val = cpu.regs.A - cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.D & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.D) < 0);
	cpu.regs.A = val;
}
void _IN_SUB_A_E() {
	uint16_t val = cpu.regs.A - cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.E & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.E) < 0);
	cpu.regs.A = val;
}
void _IN_SUB_A_H() {
	uint16_t val = cpu.regs.A - cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.H & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.H) < 0);
	cpu.regs.A = val;
}
void _IN_SUB_A_L() {
	uint16_t val = cpu.regs.A - cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.L & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)cpu.regs.L) < 0);
	cpu.regs.A = val;
}
void _IN_SUB_A_A() {
	CPU_SET_FLAG(FLAG_Z, 1);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
	cpu.regs.A = 0;
}
void _IN_SUB_A_HL() {
	uint16_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	uint16_t val = cpu.regs.A - fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)fetched_data) < 0);
	cpu.regs.A = val;
}
void _IN_SBC_A_B() {
	uint8_t val = cpu.regs.B + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A -= val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)cpu.regs.B & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)cpu.regs.B - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_SBC_A_C() {
	uint8_t val = cpu.regs.B + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A -= val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)cpu.regs.C & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)cpu.regs.C - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_SBC_A_D() {
	uint8_t val = cpu.regs.B + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A -= val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)cpu.regs.D & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)cpu.regs.D - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_SBC_A_E() {
	uint8_t val = cpu.regs.B + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A -= val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)cpu.regs.E & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)cpu.regs.E - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_SBC_A_H() {
	uint8_t val = cpu.regs.B + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A -= val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)cpu.regs.H & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)cpu.regs.H - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_SBC_A_L() {
	uint8_t val = cpu.regs.B + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A -= val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)cpu.regs.L & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)cpu.regs.L - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_SBC_A_A() {
	uint8_t val = cpu.regs.A + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A -= val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)cpu.regs.A & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)cpu.regs.A - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_SBC_A_HL() {
	uint16_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	uint8_t val = cpu.regs.A + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A -= val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_CP_A_N8() {
	uint16_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	int val = (int)cpu.regs.A - (int)fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_CP_A_B() {
	int val = (int)cpu.regs.A - (int)cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.B & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_CP_A_C() {
	int val = (int)cpu.regs.A - (int)cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.C & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_CP_A_D() {
	int val = (int)cpu.regs.A - (int)cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.D & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_CP_A_E() {
	int val = (int)cpu.regs.A - (int)cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.E & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_CP_A_H() {
	int val = (int)cpu.regs.A - (int)cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.H & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_CP_A_L() {
	int val = (int)cpu.regs.A - (int)cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.regs.L & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_CP_A_A() {
	CPU_SET_FLAG(FLAG_Z, 1);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_CP_A_HL() {
	uint16_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	int val = (int)cpu.regs.A - (int)fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_JP_NZ_A16() {
	if ((cpu.regs.F & FLAG_Z) == 0) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC += 2;
	}
}
void _IN_JP_A16() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC = low | (high << 8);
	emu_incr_cycles(1);
}
void _IN_JP_Z_A16() {
	if ((cpu.regs.F & FLAG_Z) == FLAG_Z) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC += 2;
	}
}
void _IN_JP_C_A16() {
	if ((cpu.regs.F & FLAG_C) == FLAG_C) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC += 2;
	}
}
void _IN_JP_NC_A16() {
	if ((cpu.regs.F & FLAG_C) == 0) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC += 2;
	}
}
void _IN_JP_HL() {
	cpu.regs.PC = *(uint16_t *)&cpu.regs.H;
	emu_incr_cycles(1);
}
void _IN_AND_A_B() {
	cpu.regs.A &= cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_AND_A_C() {
	cpu.regs.A &= cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_AND_A_D() {
	cpu.regs.A &= cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_AND_A_E() {
	cpu.regs.A &= cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_AND_A_H() {
	cpu.regs.A &= cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_AND_A_L() {
	cpu.regs.A &= cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_AND_A_A() {
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_AND_A_HL() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	cpu.regs.A &= fetched_data;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_XOR_A_B() {
	cpu.regs.A ^= cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_XOR_A_C() {
	cpu.regs.A ^= cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_XOR_A_D() {
	cpu.regs.A ^= cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_XOR_A_E() {
	cpu.regs.A ^= cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_XOR_A_H() {
	cpu.regs.A ^= cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_XOR_A_L() {
	cpu.regs.A ^= cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_XOR_A_A() {
	cpu.regs.A = 0;
	CPU_SET_FLAG(FLAG_Z, 1);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_XOR_A_HL() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	cpu.regs.A ^= fetched_data;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A_B() {
	cpu.regs.A |= cpu.regs.B;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A_C() {
	cpu.regs.A |= cpu.regs.C;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A_D() {
	cpu.regs.A |= cpu.regs.D;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A_E() {
	cpu.regs.A |= cpu.regs.E;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A_H() {
	cpu.regs.A |= cpu.regs.H;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A_L() {
	cpu.regs.A |= cpu.regs.L;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A_HL() {
	uint8_t fetched_data = bus_read(*(uint16_t *)&cpu.regs.H);
	emu_incr_cycles(1);
	cpu.regs.A |= fetched_data;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A_A() {
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_POP_BC() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.B = (high << 8) | low;
}
void _IN_POP_DE() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.D = (high << 8) | low;
}
void _IN_POP_HL() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.H = (high << 8) | low;
}
void _IN_POP_AF() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	*(uint16_t *)&cpu.regs.A = ((high << 8) | low) & 0xFFF0;
}
void _IN_XOR_A() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	cpu.regs.A ^= fetched_data & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_CALL() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = low | (high << 8);
	emu_incr_cycles(1);
}
void _IN_CALL_C() {	
	if ((cpu.regs.F & FLAG_C) == FLAG_C) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC += 2;
		emu_incr_cycles(2);
		stack_push16(cpu.regs.PC);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC += 2;
	}
}
void _IN_CALL_Z() {	
	if ((cpu.regs.F & FLAG_Z) == FLAG_Z) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC += 2;
		emu_incr_cycles(2);
		stack_push16(cpu.regs.PC);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC += 2;
	}
}
void _IN_CALL_NC() {	
	if ((cpu.regs.F & FLAG_C) == 0) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC += 2;
		emu_incr_cycles(2);
		stack_push16(cpu.regs.PC);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC += 2;
	}
}
void _IN_CALL_NZ() {	
	if ((cpu.regs.F & FLAG_Z) == 0) {
		uint16_t low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		uint16_t high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC += 2;
		emu_incr_cycles(2);
		stack_push16(cpu.regs.PC);
		cpu.regs.PC = low | (high << 8);
		emu_incr_cycles(1);
	} else {
		cpu.regs.PC += 2;
	}
}
void _IN_PUSH_BC() {
	uint16_t val = *(uint16_t *)&cpu.regs.B;
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}
void _IN_PUSH_DE() {
	uint16_t val = *(uint16_t *)&cpu.regs.D;
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}
void _IN_PUSH_HL() {
	uint16_t val = *(uint16_t *)&cpu.regs.H;
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}
void _IN_PUSH_AF() {
	uint16_t val = *(uint16_t *)&cpu.regs.A;
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}
void _IN_ADD_A() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	uint32_t val = cpu.regs.A + fetched_data;
	CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.A & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((cpu.regs.A & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.A = val & 0xFF;
}
void _IN_ADD_SP() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	emu_incr_cycles(1);
	uint32_t val = cpu.regs.SP + (int8_t)fetched_data;
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, ((cpu.regs.SP & 0x0F) + (fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
	CPU_SET_FLAG(FLAG_C, ((cpu.regs.SP & 0xFF) + (fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	cpu.regs.SP = val & 0xFFFF;
}
void _IN_ADC_A() {
	uint16_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + fetched_data + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (fetched_data & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + fetched_data + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_SUB_A() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	uint16_t val = cpu.regs.A - fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu.regs.A) - ((int)fetched_data) < 0);
	cpu.regs.A = val;
}
void _IN_SBC_A() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	uint8_t val = fetched_data + CPU_FLAG_C_SET;
	int reg = cpu.regs.A;
	cpu.regs.A = reg - val;
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)fetched_data - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_AND_A() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	cpu.regs.A &= fetched_data & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_OR_A() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	cpu.regs.A |= fetched_data & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_CB() {
	sceClibPrintf("IN_CB: NOIMPL\n");
}
void _IN_LDH_A() {
	uint16_t mem_dest = bus_read(cpu.regs.PC++) | 0xFF00;
	emu_incr_cycles(1);
	bus_write(mem_dest, cpu.regs.A);
	emu_incr_cycles(1);
}
void _IN_LD_C_AM() {
	bus_write(cpu.regs.C | 0xFF00, cpu.regs.A);
	emu_incr_cycles(1);
}
void _IN_LD_A() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	bus_write(low | (high << 8), cpu.regs.A);
	emu_incr_cycles(1);
}
void _IN_LDH_AM() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	cpu.regs.A = bus_read(fetched_data | 0xFF00);
	emu_incr_cycles(1);
}
void _IN_LD_A_CM() {
	cpu.regs.A = bus_read(((uint16_t)cpu.regs.C) | 0xFF00);
	emu_incr_cycles(1);
}
void _IN_LD_HL_SPR() {
	uint8_t fetched_data = bus_read(cpu.regs.PC++);
	emu_incr_cycles(1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (cpu.regs.SP & 0x0F) + (fetched_data & 0x0F) >= 0x10);
	CPU_SET_FLAG(FLAG_C, (cpu.regs.SP & 0xFF) + (fetched_data & 0xFF) >= 0x100);
	*(uint16_t *)&cpu.regs.H = cpu.regs.SP + (int8_t)fetched_data;
}
void _IN_LD_SP_HL() {
	cpu.regs.SP = *(uint16_t *)&cpu.regs.H;
}
void _IN_LD_A16() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	cpu.regs.A = bus_read(low | (high << 8));
	emu_incr_cycles(1);
}
static func_t instrs[] = {
	[0x00] = _IN_NOP,
	[0x01] = _IN_LD_BC_D16,
	[0x02] = _IN_LD_BC_A,
	[0x03] = _IN_INC_BC,
	[0x04] = _IN_INC_B,
	[0x05] = _IN_DEC_B,
	[0x06] = _IN_LD_B_D8,
	[0x07] = _IN_RLCA,
	[0x08] = _IN_LD_A16_SP,
	[0x09] = _IN_ADD_HL_BC,
	[0x0A] = _IN_LD_A_BC,
	[0x0B] = _IN_DEC_BC,
	[0x0C] = _IN_INC_C,
	[0x0D] = _IN_DEC_C,
	[0x0E] = _IN_LD_C_D8,
	[0x0F] = _IN_RRCA,
	[0x10] = _IN_STOP,
	[0x11] = _IN_LD_DE_D16,
	[0x12] = _IN_LD_DE_A,
	[0x13] = _IN_INC_DE,
	[0x14] = _IN_INC_D,
	[0x15] = _IN_DEC_D,
	[0x16] = _IN_LD_D_D8,
	[0x17] = _IN_RLA,
	[0x18] = _IN_JR_S8,
	[0x19] = _IN_ADD_HL_DE,
	[0x1A] = _IN_LD_A_DE,
	[0x1B] = _IN_DEC_DE,
	[0x1C] = _IN_INC_E,
	[0x1D] = _IN_DEC_E,
	[0x1E] = _IN_LD_E_D8,
	[0x1F] = _IN_RRA,
	[0x20] = _IN_JR_NZ_S8,
	[0x21] = _IN_LD_HL_D16,
	[0x22] = _IN_LD_HLP_A,
	[0x23] = _IN_INC_HL,
	[0x24] = _IN_INC_H,
	[0x25] = _IN_DEC_H,
	[0x26] = _IN_LD_H_D8,
	[0x27] = _IN_DAA,
	[0x28] = _IN_JR_Z_S8,
	[0x29] = _IN_ADD_HL_HL,
	[0x2A] = _IN_LD_A_HLP,
	[0x2B] = _IN_DEC_HL,
	[0x2C] = _IN_INC_L,
	[0x2D] = _IN_DEC_L,
	[0x2E] = _IN_LD_L_D8,
	[0x2F] = _IN_CPL,
	[0x30] = _IN_JR_NC_S8,
	[0x31] = _IN_LD_SP_D16,
	[0x32] = _IN_LD_HLM_A,
	[0x33] = _IN_INC_SP,
	[0x34] = _IN_INC_MHL,
	[0x35] = _IN_DEC_MHL,
	[0x36] = _IN_LD_HL_D8,
	[0x37] = _IN_SCF,
	[0x38] = _IN_JR_C_E8,
	[0x39] = _IN_ADD_HL_SP,
	[0x3A] = _IN_LD_A_HLM,
	[0x3B] = _IN_DEC_SP,
	[0x3C] = _IN_INC_A,
	[0x3D] = _IN_DEC_A,
	[0x3E] = _IN_LD_A_N8,
	[0x3F] = _IN_CCF,
	[0x40] = _IN_NOP,
	[0x41] = _IN_LD_B_C,
	[0x42] = _IN_LD_B_D,
	[0x43] = _IN_LD_B_E,
	[0x44] = _IN_LD_B_H,
	[0x45] = _IN_LD_B_L,
	[0x46] = _IN_LD_B_HL,
	[0x47] = _IN_LD_B_A,
	[0x48] = _IN_LD_C_B,
	[0x49] = _IN_NOP,
	[0x4A] = _IN_LD_C_D,
	[0x4B] = _IN_LD_C_E,
	[0x4C] = _IN_LD_C_H,
	[0x4D] = _IN_LD_C_L,
	[0x4E] = _IN_LD_C_HL,
	[0x4F] = _IN_LD_C_A,
	[0x50] = _IN_LD_D_B,
	[0x51] = _IN_LD_D_C,
	[0x52] = _IN_NOP,
	[0x53] = _IN_LD_D_E,
	[0x54] = _IN_LD_D_H,
	[0x55] = _IN_LD_D_L,
	[0x56] = _IN_LD_D_HL,
	[0x57] = _IN_LD_D_A,
	[0x58] = _IN_LD_E_B,
	[0x59] = _IN_LD_E_C,
	[0x5A] = _IN_LD_E_D,
	[0x5B] = _IN_NOP,
	[0x5C] = _IN_LD_E_H,
	[0x5D] = _IN_LD_E_L,
	[0x5E] = _IN_LD_E_HL,
	[0x5F] = _IN_LD_E_A,
	[0x60] = _IN_LD_H_B,
	[0x61] = _IN_LD_H_C,
	[0x62] = _IN_LD_H_D,
	[0x63] = _IN_LD_H_E,
	[0x64] = _IN_NOP,
	[0x65] = _IN_LD_H_L,
	[0x66] = _IN_LD_H_HL,
	[0x67] = _IN_LD_H_A,
	[0x68] = _IN_LD_L_B,
	[0x69] = _IN_LD_L_C,
	[0x6A] = _IN_LD_L_D,
	[0x6B] = _IN_LD_L_E,
	[0x6C] = _IN_LD_L_H,
	[0x6D] = _IN_NOP,
	[0x6E] = _IN_LD_L_HL,
	[0x6F] = _IN_LD_L_A,
	[0x70] = _IN_LD_HL_B,
	[0x71] = _IN_LD_HL_C,
	[0x72] = _IN_LD_HL_D,
	[0x73] = _IN_LD_HL_E,
	[0x74] = _IN_LD_HL_H,
	[0x75] = _IN_LD_HL_L,
	[0x76] = _IN_HALT,
	[0x77] = _IN_LD_HL_A,
	[0x78] = _IN_LD_A_B,
	[0x79] = _IN_LD_A_C,
	[0x7A] = _IN_LD_A_D,
	[0x7B] = _IN_LD_A_E,
	[0x7C] = _IN_LD_A_H,
	[0x7D] = _IN_LD_A_L,
	[0x7E] = _IN_LD_A_HL,
	[0x7F] = _IN_NOP,
	[0x80] = _IN_ADD_A_B,
	[0x81] = _IN_ADD_A_C,
	[0x82] = _IN_ADD_A_D,
	[0x83] = _IN_ADD_A_E,
	[0x84] = _IN_ADD_A_H,
	[0x85] = _IN_ADD_A_L,
	[0x86] = _IN_ADD_A_HL,
	[0x87] = _IN_ADD_A_A,
	[0x88] = _IN_ADC_A_B,
	[0x89] = _IN_ADC_A_C,
	[0x8A] = _IN_ADC_A_D,
	[0x8B] = _IN_ADC_A_E,
	[0x8C] = _IN_ADC_A_H,
	[0x8D] = _IN_ADC_A_L,
	[0x8E] = _IN_ADC_A_HL,
	[0x8F] = _IN_ADC_A_A,
	[0x90] = _IN_SUB_A_B,
	[0x91] = _IN_SUB_A_C,
	[0x92] = _IN_SUB_A_D,
	[0x93] = _IN_SUB_A_E,
	[0x94] = _IN_SUB_A_H,
	[0x95] = _IN_SUB_A_L,
	[0x96] = _IN_SUB_A_HL,
	[0x97] = _IN_SUB_A_A,
	[0x98] = _IN_SBC_A_B,
	[0x99] = _IN_SBC_A_C,
	[0x9A] = _IN_SBC_A_D,
	[0x9B] = _IN_SBC_A_E,
	[0x9C] = _IN_SBC_A_H,
	[0x9D] = _IN_SBC_A_L,
	[0x9E] = _IN_SBC_A_HL,
	[0x9F] = _IN_SBC_A_A,
	[0xA0] = _IN_AND_A_B,
	[0xA1] = _IN_AND_A_C,
	[0xA2] = _IN_AND_A_D,
	[0xA3] = _IN_AND_A_E,
	[0xA4] = _IN_AND_A_H,
	[0xA5] = _IN_AND_A_L,
	[0xA6] = _IN_AND_A_HL,
	[0xA7] = _IN_AND_A_A,
	[0xA8] = _IN_XOR_A_B,
	[0xA9] = _IN_XOR_A_C,
	[0xAA] = _IN_XOR_A_D,
	[0xAB] = _IN_XOR_A_E,
	[0xAC] = _IN_XOR_A_H,
	[0xAD] = _IN_XOR_A_L,
	[0xAE] = _IN_XOR_A_HL,
	[0xAF] = _IN_XOR_A_A,
	[0xB0] = _IN_OR_A_B,
	[0xB1] = _IN_OR_A_C,
	[0xB2] = _IN_OR_A_D,
	[0xB3] = _IN_OR_A_E,
	[0xB4] = _IN_OR_A_H,
	[0xB5] = _IN_OR_A_L,
	[0xB6] = _IN_OR_A_HL,
	[0xB7] = _IN_OR_A_A,
	[0xB8] = _IN_CP_A_B,
	[0xB9] = _IN_CP_A_C,
	[0xBA] = _IN_CP_A_D,
	[0xBB] = _IN_CP_A_E,
	[0xBC] = _IN_CP_A_H,
	[0xBD] = _IN_CP_A_L,
	[0xBE] = _IN_CP_A_HL,
	[0xBF] = _IN_CP_A_A,
	[0xC0] = _IN_RET_NZ,
	[0xC1] = _IN_POP_BC,
	[0xC2] = _IN_JP_NZ_A16,
	[0xC3] = _IN_JP_A16,
	[0xC4] = _IN_CALL_NZ,
	[0xC5] = _IN_PUSH_BC,
	[0xC6] = _IN_ADD_A,
	[0xC7] = _IN_RST_00,
	[0xC8] = _IN_RET_Z,
	[0xC9] = _IN_RET,
	[0xCA] = _IN_JP_Z_A16,
	[0xCB] = _IN_CB,
	[0xCC] = _IN_CALL_Z,
	[0xCD] = _IN_CALL,
	[0xCE] = _IN_ADC_A,
	[0xCF] = _IN_RST_08,
	[0xD0] = _IN_RET_NC,
	[0xD1] = _IN_POP_DE,
	[0xD2] = _IN_JP_NC_A16,
	[0xD4] = _IN_CALL_NC,
	[0xD5] = _IN_PUSH_DE,
	[0xD6] = _IN_SUB_A,
	[0xD7] = _IN_RST_10,
	[0xD8] = _IN_RET_C,
	[0xD9] = _IN_RETI,
	[0xDA] = _IN_JP_C_A16,
	[0xDC] = _IN_CALL_C,
	[0xDE] = _IN_SBC_A,
	[0xDF] = _IN_RST_18,
	[0xE0] = _IN_LDH_A,
	[0xE1] = _IN_POP_HL,
	[0xE2] = _IN_LD_C_AM,
	[0xE5] = _IN_PUSH_HL,
	[0xE6] = _IN_AND_A,
	[0xE7] = _IN_RST_20,
	[0xE8] = _IN_ADD_SP,
	[0xE9] = _IN_JP_HL,
	[0xEA] = _IN_LD_A,
	[0xEE] = _IN_XOR_A,
	[0xEF] = _IN_RST_28,
	[0xF0] = _IN_LDH_AM,
	[0xF1] = _IN_POP_AF,
	[0xF2] = _IN_LD_A_CM,
	[0xF3] = _IN_DI,
	[0xF5] = _IN_PUSH_AF,
	[0xF6] = _IN_OR_A,
	[0xF7] = _IN_RST_30,
	[0xF8] = _IN_LD_HL_SPR,
	[0xF9] = _IN_LD_SP_HL,
	[0xFA] = _IN_LD_A16,
	[0xFB] = _IN_EI,
	[0xFE] = _IN_CP_A_N8,
	[0xFF] = _IN_RST_38,
};
#define cpu_exec_instr() instrs[opcode]()

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
	cpu.interrupts = 0x00;
	cpu.enable_interrupts = 0;
	cpu.halted = 0;
}

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
		uint8_t opcode = bus_read(cpu.regs.PC++);
		emu_incr_cycles(1);
		
		// Interpreter debugger
		if (emu.opts.debug_log) {
			char c = CPU_FLAG_C_SET ? 'C' : '-';
			char z = CPU_FLAG_Z_SET ? 'Z' : '-';
			char n = CPU_FLAG_N_SET ? 'N' : '-';
			char h = CPU_FLAG_H_SET ? 'H' : '-';
			sceClibPrintf("%04X: (%02X) A: %02X F: %c%c%c%c BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
				instr_PC, opcode, cpu.regs.A, z, n, h, c, cpu.regs.B, cpu.regs.C, cpu.regs.D, cpu.regs.E, cpu.regs.H, cpu.regs.L);
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
		
		// Executing the given instruction
		cpu_exec_instr();
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
