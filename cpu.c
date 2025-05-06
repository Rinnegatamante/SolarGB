#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "cpu.h"
#include "emu.h"

typedef void (*func_t)();
typedef uint8_t (*ufunc_t)();
typedef void (*wfunc_t)(uint16_t);
typedef uint16_t (*rfunc_t)();
static void _NOP() {}
static uint8_t _RET1() { return 1; }

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

static instr_t instrs[] = {
	[0x00] = {.type = IN_NOP},
	[0x01] = {.type = IN_LD, .addr_mode = AM_R_D16, .reg1 = RT_BC},
	[0x02] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_BC, .reg2 = RT_A},
	[0x03] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_BC},
	[0x04] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_B},
	[0x05] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_B},
	[0x06] = {.type = IN_LD, .addr_mode = AM_R_D8, .reg1 = RT_B},
	[0x07] = {.type = IN_RLCA},
	[0x08] = {.type = IN_LD, .addr_mode = AM_A16_R, .reg2 = RT_SP},
	[0x09] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_HL, .reg2 = RT_BC},
	[0x0A] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_BC},
	[0x0B] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_BC},
	[0x0C] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_C},
	[0x0D] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_C},
	[0x0E] = {.type = IN_LD, .addr_mode = AM_R_D8, .reg1 = RT_C},
	[0x0F] = {.type = IN_RRCA},
	[0x10] = {.type = IN_STOP},
	[0x11] = {.type = IN_LD, .addr_mode = AM_R_D16, .reg1 = RT_DE},
	[0x12] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_DE, .reg2 = RT_A},
	[0x13] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_DE},
	[0x14] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_D},
	[0x15] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_D},
	[0x16] = {.type = IN_LD, .addr_mode = AM_R_D8, .reg1 = RT_D},
	[0x17] = {.type = IN_RLA},
	[0x18] = {.type = IN_JR, .addr_mode = AM_D8},
	[0x19] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_HL, .reg2 = RT_DE},
	[0x1A] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_DE},
	[0x1B] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_DE},
	[0x1C] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_E},
	[0x1D] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_E},
	[0x1E] = {.type = IN_LD, .addr_mode = AM_R_D8, .reg1 = RT_E},
	[0x1F] = {.type = IN_RRA},
	[0x20] = {.type = IN_JR, .addr_mode = AM_D8, .cnd = CT_NZ},
	[0x21] = {.type = IN_LD, .addr_mode = AM_R_D16, .reg1 = RT_HL},
	[0x22] = {.type = IN_LD, .addr_mode = AM_HLI_R, .reg1 = RT_HL, .reg2 = RT_A},
	[0x23] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_HL},
	[0x24] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_H},
	[0x25] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_H},
	[0x26] = {.type = IN_LD, .addr_mode = AM_R_D8, .reg1 = RT_H},
	[0x27] = {.type = IN_DAA},
	[0x28] = {.type = IN_JR, .addr_mode = AM_D8, .cnd = CT_Z},
	[0x29] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_HL, .reg2 = RT_HL},
	[0x2A] = {.type = IN_LD, .addr_mode = AM_R_HLI, .reg1 = RT_A, .reg2 = RT_HL},
	[0x2B] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_HL},
	[0x2C] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_L},
	[0x2D] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_L},
	[0x2E] = {.type = IN_LD, .addr_mode = AM_R_D8, .reg1 = RT_L},
	[0x2F] = {.type = IN_CPL},
	[0x30] = {.type = IN_JR, .addr_mode = AM_D8, .cnd = CT_NC},
	[0x31] = {.type = IN_LD, .addr_mode = AM_R_D16, .reg1 = RT_SP},
	[0x32] = {.type = IN_LD, .addr_mode = AM_HLD_R, .reg1 = RT_HL, .reg2 = RT_A},
	[0x33] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_SP},
	[0x34] = {.type = IN_INC, .addr_mode = AM_MR, .reg1 = RT_HL},
	[0x35] = {.type = IN_DEC, .addr_mode = AM_MR, .reg1 = RT_HL},
	[0x36] = {.type = IN_LD, .addr_mode = AM_MR_D8, .reg1 = RT_HL},
	[0x37] = {.type = IN_SCF},
	[0x38] = {.type = IN_JR, .addr_mode = AM_D8, .cnd = CT_C},
	[0x39] = {.type = IN_ADD, .addr_mode = AM_R_R, RT_HL, RT_SP},
	[0x3A] = {.type = IN_LD, .addr_mode = AM_R_HLD, .reg1 = RT_A, .reg2 = RT_HL},
	[0x3B] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_SP},
	[0x3C] = {.type = IN_INC, .addr_mode = AM_R, .reg1 = RT_A},
	[0x3D] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_A},
	[0x3E] = {.type = IN_LD, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0x3F] = {.type = IN_CCF},
	[0x40] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_B, .reg2 = RT_B},
	[0x41] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_B, .reg2 = RT_C},
	[0x42] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_B, .reg2 = RT_D},
	[0x43] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_B, .reg2 = RT_E},
	[0x44] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_B, .reg2 = RT_H},
	[0x45] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_B, .reg2 = RT_L},
	[0x46] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_B, .reg2 = RT_HL},
	[0x47] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_B, .reg2 = RT_A},
	[0x48] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_C, .reg2 = RT_B},
	[0x49] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_C, .reg2 = RT_C},
	[0x4A] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_C, .reg2 = RT_D},
	[0x4B] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_C, .reg2 = RT_E},
	[0x4C] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_C, .reg2 = RT_H},
	[0x4D] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_C, .reg2 = RT_L},
	[0x4E] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_C, .reg2 = RT_HL},
	[0x4F] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_C, .reg2 = RT_A},
	[0x50] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_D, .reg2 = RT_B},
	[0x51] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_D, .reg2 = RT_C},
	[0x52] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_D, .reg2 = RT_D},
	[0x53] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_D, .reg2 = RT_E},
	[0x54] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_D, .reg2 = RT_H},
	[0x55] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_D, .reg2 = RT_L},
	[0x56] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_D, .reg2 = RT_HL},
	[0x57] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_D, .reg2 = RT_A},
	[0x58] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_E, .reg2 = RT_B},
	[0x59] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_E, .reg2 = RT_C},
	[0x5A] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_E, .reg2 = RT_D},
	[0x5B] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_E, .reg2 = RT_E},
	[0x5C] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_E, .reg2 = RT_H},
	[0x5D] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_E, .reg2 = RT_L},
	[0x5E] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_E, .reg2 = RT_HL},
	[0x5F] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_E, .reg2 = RT_A},
	[0x60] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_H, .reg2 = RT_B},
	[0x61] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_H, .reg2 = RT_C},
	[0x62] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_H, .reg2 = RT_D},
	[0x63] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_H, .reg2 = RT_E},
	[0x64] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_H, .reg2 = RT_H},
	[0x65] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_H, .reg2 = RT_L},
	[0x66] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_H, .reg2 = RT_HL},
	[0x67] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_H, .reg2 = RT_A},
	[0x68] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_L, .reg2 = RT_B},
	[0x69] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_L, .reg2 = RT_C},
	[0x6A] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_L, .reg2 = RT_D},
	[0x6B] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_L, .reg2 = RT_E},
	[0x6C] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_L, .reg2 = RT_H},
	[0x6D] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_L, .reg2 = RT_L},
	[0x6E] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_L, .reg2 = RT_HL},
	[0x6F] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_L, .reg2 = RT_A},
	[0x70] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_HL, .reg2 = RT_B},
	[0x71] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_HL, .reg2 = RT_C},
	[0x72] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_HL, .reg2 = RT_D},
	[0x73] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_HL, .reg2 = RT_E},
	[0x74] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_HL, .reg2 = RT_H},
	[0x75] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_HL, .reg2 = RT_L},
	[0x76] = {.type = IN_HALT},
	[0x77] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_HL, .reg2 = RT_A},
	[0x78] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0x79] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0x7A] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0x7B] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0x7C] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0x7D] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0x7E] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0x7F] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_A},
	[0x80] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0x81] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0x82] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0x83] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0x84] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0x85] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0x86] = {.type = IN_ADD, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0x87] = {.type = IN_ADD, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_A},
	[0x88] = {.type = IN_ADC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0x89] = {.type = IN_ADC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0x8A] = {.type = IN_ADC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0x8B] = {.type = IN_ADC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0x8C] = {.type = IN_ADC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0x8D] = {.type = IN_ADC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0x8E] = {.type = IN_ADC, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0x8F] = {.type = IN_ADC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_A},
	[0x90] = {.type = IN_SUB, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0x91] = {.type = IN_SUB, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0x92] = {.type = IN_SUB, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0x93] = {.type = IN_SUB, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0x94] = {.type = IN_SUB, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0x95] = {.type = IN_SUB, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0x96] = {.type = IN_SUB, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0x97] = {.type = IN_SUB, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_A},
	[0x98] = {.type = IN_SBC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0x99] = {.type = IN_SBC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0x9A] = {.type = IN_SBC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0x9B] = {.type = IN_SBC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0x9C] = {.type = IN_SBC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0x9D] = {.type = IN_SBC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0x9E] = {.type = IN_SBC, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0x9F] = {.type = IN_SBC, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_A},
	[0xA0] = {.type = IN_AND, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0xA1] = {.type = IN_AND, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0xA2] = {.type = IN_AND, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0xA3] = {.type = IN_AND, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0xA4] = {.type = IN_AND, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0xA5] = {.type = IN_AND, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0xA6] = {.type = IN_AND, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0xA7] = {.type = IN_AND, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_A},
	[0xA8] = {.type = IN_XOR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0xA9] = {.type = IN_XOR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0xAA] = {.type = IN_XOR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0xAB] = {.type = IN_XOR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0xAC] = {.type = IN_XOR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0xAD] = {.type = IN_XOR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0xAE] = {.type = IN_XOR, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0xAF] = {.type = IN_XOR, .addr_mode = AM_R, .reg1 = RT_A},
	[0xB0] = {.type = IN_OR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0xB1] = {.type = IN_OR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0xB2] = {.type = IN_OR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0xB3] = {.type = IN_OR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0xB4] = {.type = IN_OR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0xB5] = {.type = IN_OR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0xB6] = {.type = IN_OR, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0xB7] = {.type = IN_OR, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_A},
	[0xB8] = {.type = IN_CP, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_B},
	[0xB9] = {.type = IN_CP, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_C},
	[0xBA] = {.type = IN_CP, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_D},
	[0xBB] = {.type = IN_CP, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_E},
	[0xBC] = {.type = IN_CP, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_H},
	[0xBD] = {.type = IN_CP, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_L},
	[0xBE] = {.type = IN_CP, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_HL},
	[0xBF] = {.type = IN_CP, .addr_mode = AM_R_R, .reg1 = RT_A, .reg2 = RT_A},
	[0xC0] = {.type = IN_RET, .cnd = CT_NZ},
	[0xC1] = {.type = IN_POP, .addr_mode = AM_R, .reg1 = RT_BC},
	[0xC2] = {.type = IN_JP, .addr_mode = AM_D16, .cnd = CT_NZ},
	[0xC3] = {.type = IN_JP, .addr_mode = AM_D16},
	[0xC4] = {.type = IN_CALL, .addr_mode = AM_D16, .cnd = CT_NZ},
	[0xC5] = {.type = IN_PUSH, .addr_mode = AM_R, .reg1 = RT_BC},
	[0xC6] = {.type = IN_ADD, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0xC7] = {.type = IN_RST, .param = 0x00},
	[0xC8] = {.type = IN_RET, .cnd = CT_Z},
	[0xC9] = {.type = IN_RET},
	[0xCA] = {.type = IN_JP, .addr_mode = AM_D16, .cnd = CT_Z},
	[0xCB] = {.type = IN_CB, .addr_mode = AM_D8},
	[0xCC] = {.type = IN_CALL, .addr_mode = AM_D16, .cnd = CT_Z},
	[0xCD] = {.type = IN_CALL, .addr_mode = AM_D16},
	[0xCE] = {.type = IN_ADC, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0xCF] = {.type = IN_RST, .param = 0x08},
	[0xD0] = {.type = IN_RET, .cnd = CT_NC},
	[0xD1] = {.type = IN_POP, .addr_mode = AM_R, .reg1 = RT_DE},
	[0xD2] = {.type = IN_JP, .addr_mode = AM_D16, .cnd = CT_NC},
	[0xD4] = {.type = IN_CALL, .addr_mode = AM_D16, .cnd = CT_NC},
	[0xD5] = {.type = IN_PUSH, .addr_mode = AM_R, .reg1 = RT_DE},
	[0xD6] = {.type = IN_SUB, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0xD7] = {.type = IN_RST, .param = 0x10},
	[0xD8] = {.type = IN_RET, .cnd = CT_C},
	[0xD9] = {.type = IN_RETI},
	[0xDA] = {.type = IN_JP, .addr_mode = AM_D16, .cnd = CT_C},
	[0xDC] = {.type = IN_CALL, .addr_mode = AM_D16, .cnd = CT_C},
	[0xDE] = {.type = IN_SBC, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0xDF] = {.type = IN_RST, .param = 0x18},
	[0xE0] = {.type = IN_LDH, .addr_mode = AM_A8_R, .reg2 = RT_A},
	[0xE1] = {.type = IN_POP, .addr_mode = AM_R, .reg1 = RT_HL},
	[0xE2] = {.type = IN_LD, .addr_mode = AM_MR_R, .reg1 = RT_C, .reg2 = RT_A},
	[0xE5] = {.type = IN_PUSH, .addr_mode = AM_R, .reg1 = RT_HL},
	[0xE6] = {.type = IN_AND, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0xE7] = {.type = IN_RST, .param = 0x20},
	[0xE8] = {.type = IN_ADD, .addr_mode = AM_R_D8, .reg1 = RT_SP},
	[0xE9] = {.type = IN_JP, .addr_mode = AM_R, .reg1 = RT_HL},
	[0xEA] = {.type = IN_LD, .addr_mode = AM_A16_R, .reg2 = RT_A},
	[0xEE] = {.type = IN_XOR, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0xEF] = {.type = IN_RST, .param = 0x28},
	[0xF0] = {.type = IN_LDH, .addr_mode = AM_R_A8, .reg1 = RT_A},
	[0xF1] = {.type = IN_POP, .addr_mode = AM_R, .reg1 = RT_AF},
	[0xF2] = {.type = IN_LD, .addr_mode = AM_R_MR, .reg1 = RT_A, .reg2 = RT_C},
	[0xF3] = {.type = IN_DI},
	[0xF5] = {.type = IN_PUSH, .addr_mode = AM_R, .reg1 = RT_AF},
	[0xF6] = {.type = IN_OR, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0xF7] = {.type = IN_RST, .param = 0x30},
	[0xF8] = {.type = IN_LD, .addr_mode = AM_HL_SPR, .reg1 = RT_HL, .reg2 = RT_SP},
	[0xF9] = {.type = IN_LD, .addr_mode = AM_R_R, .reg1 = RT_SP, .reg2 = RT_HL},
	[0xFA] = {.type = IN_LD, .addr_mode = AM_R_A16, .reg1 = RT_A},
	[0xFB] = {.type = IN_EI},
	[0xFE] = {.type = IN_CP, .addr_mode = AM_R_D8, .reg1 = RT_A},
	[0xFF] = {.type = IN_RST, .param = 0x38}
};

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

// Registers reading functions
uint16_t _R_RT_A() { return cpu.regs.A; }
uint16_t _R_RT_B() { return cpu.regs.B; }
uint16_t _R_RT_C() { return cpu.regs.C; }
uint16_t _R_RT_D() { return cpu.regs.D; }
uint16_t _R_RT_E() { return cpu.regs.E; }
uint16_t _R_RT_F() { return cpu.regs.F; }
uint16_t _R_RT_H() { return cpu.regs.H; }
uint16_t _R_RT_L() { return cpu.regs.L; }
uint16_t _R_RT_SP() { return cpu.regs.SP; }
uint16_t _R_RT_PC() { return cpu.regs.PC; }
uint16_t _R_RT_HL() { return ((uint16_t)cpu.regs.L | (((uint16_t)cpu.regs.H) << 8)); }
uint16_t _R_RT_AF() { return ((uint16_t)cpu.regs.F | (((uint16_t)cpu.regs.A) << 8)); }
uint16_t _R_RT_BC() { return ((uint16_t)cpu.regs.C | (((uint16_t)cpu.regs.B) << 8)); }
uint16_t _R_RT_DE() { return ((uint16_t)cpu.regs.E | (((uint16_t)cpu.regs.D) << 8)); }
rfunc_t reg_read_funcs[] = {
	[RT_A] = _R_RT_A,
	[RT_B] = _R_RT_B,
	[RT_C] = _R_RT_C,
	[RT_D] = _R_RT_D,
	[RT_E] = _R_RT_E,
	[RT_F] = _R_RT_F,
	[RT_H] = _R_RT_H,
	[RT_L] = _R_RT_L,
	[RT_SP] = _R_RT_SP,
	[RT_PC] = _R_RT_PC,
	[RT_HL] = _R_RT_HL,
	[RT_BC] = _R_RT_BC,
	[RT_AF] = _R_RT_AF,
	[RT_DE] = _R_RT_DE,
};
#define cpu_read_reg(r) reg_read_funcs[r]()

// Registers writing functions
void _RT_A(uint16_t val) { cpu.regs.A = val; }
void _RT_B(uint16_t val) { cpu.regs.B = val; }
void _RT_C(uint16_t val) { cpu.regs.C = val; }
void _RT_D(uint16_t val) { cpu.regs.D = val; }
void _RT_E(uint16_t val) { cpu.regs.E = val; }
void _RT_F(uint16_t val) { cpu.regs.F = val; }
void _RT_H(uint16_t val) { cpu.regs.H = val; }
void _RT_L(uint16_t val) { cpu.regs.L = val; }
void _RT_SP(uint16_t val) { cpu.regs.SP = val; }
void _RT_PC(uint16_t val) { cpu.regs.PC = val; }
void _RT_HL(uint16_t val) {
	cpu.regs.L = val & 0xFF;
	cpu.regs.H = (val >> 8) & 0xFF;
}
void _RT_AF(uint16_t val) {
	cpu.regs.F = val & 0xFF;
	cpu.regs.A = (val >> 8) & 0xFF;
}
void _RT_BC(uint16_t val) {
	cpu.regs.C = val & 0xFF;
	cpu.regs.B = (val >> 8) & 0xFF;
}
void _RT_DE(uint16_t val) {
	cpu.regs.E = val & 0xFF;
	cpu.regs.D = (val >> 8) & 0xFF;
}
wfunc_t reg_write_funcs[] = {
	[RT_A] = _RT_A,
	[RT_B] = _RT_B,
	[RT_C] = _RT_C,
	[RT_D] = _RT_D,
	[RT_E] = _RT_E,
	[RT_F] = _RT_F,
	[RT_H] = _RT_H,
	[RT_L] = _RT_L,
	[RT_SP] = _RT_SP,
	[RT_PC] = _RT_PC,
	[RT_HL] = _RT_HL,
	[RT_BC] = _RT_BC,
	[RT_AF] = _RT_AF,
	[RT_DE] = _RT_DE,
};
#define cpu_write_reg(r, v) reg_write_funcs[r](v);

// Instruction fecting functions
void _AM_R() {
	cpu.fetched_data = cpu_read_reg(cpu.instr->reg1);
}
void _AM_R_R() {
	cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
}
void _AM_R_D8() {
	cpu.fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
}
void _AM_D16() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.fetched_data = low | (high << 8);
	cpu.regs.PC += 2;
}
void _AM_MR_R() {
	cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
	cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
	cpu.use_mem_dest = 1;
	if (cpu.instr->reg1 == RT_C) {
		cpu.mem_dest |= 0xFF00;
	}
}
void _AM_R_MR() {
	if (cpu.instr->reg2 == RT_C) {
		cpu.fetched_data = bus_read(((uint16_t)cpu.regs.C) | 0xFF00);
	} else {
		cpu.fetched_data = bus_read(cpu_read_reg(cpu.instr->reg2));
	}
	emu_incr_cycles(1);
}
void _AM_R_HLI() {
	cpu.fetched_data = bus_read(cpu_read_reg(cpu.instr->reg2));
	emu_incr_cycles(1);
	cpu_write_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
}
void _AM_R_HLD() {
	cpu.fetched_data = bus_read(cpu_read_reg(cpu.instr->reg2));
	emu_incr_cycles(1);
	cpu_write_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
}
void _AM_HLI_R() {
	cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
	cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
	cpu.use_mem_dest = 1;
	cpu_write_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
}
void _AM_HLD_R() {
	cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
	cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
	cpu.use_mem_dest = 1;
	cpu_write_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
}
void _AM_A8_R() {
	cpu.mem_dest = bus_read(cpu.regs.PC) | 0xFF00;
	cpu.use_mem_dest = 1;
	emu_incr_cycles(1);
	cpu.regs.PC++;
}
void _AM_A16_R() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.mem_dest = low | (high << 8);
	cpu.use_mem_dest = 1;
	cpu.regs.PC += 2;
	cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
}
void _AM_MR_D8() {
	cpu.fetched_data = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	cpu.regs.PC++;
	cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
	cpu.use_mem_dest = 1;
}
void _AM_MR() {
	cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
	cpu.use_mem_dest = 1;
	cpu.fetched_data = bus_read(cpu.mem_dest);
	emu_incr_cycles(1);
}
void _AM_R_A16() {
	uint16_t low = bus_read(cpu.regs.PC);
	emu_incr_cycles(1);
	uint16_t high = bus_read(cpu.regs.PC + 1);
	emu_incr_cycles(1);
	cpu.regs.PC += 2;
	cpu.fetched_data = bus_read(low | (high << 8));
	emu_incr_cycles(1);
}
func_t fetch_funcs[] = {
	[AM_IMP] = _NOP,
	[AM_R] = _AM_R,
	[AM_R_R] = _AM_R_R,
	[AM_R_D8] = _AM_R_D8,
	[AM_HL_SPR] = _AM_R_D8,
	[AM_D8] = _AM_R_D8,
	[AM_R_A8] = _AM_R_D8,
	[AM_D16] = _AM_D16,
	[AM_R_D16] = _AM_D16,
	[AM_MR_R] = _AM_MR_R,
	[AM_R_MR] = _AM_R_MR,
	[AM_R_HLI] = _AM_R_HLI,
	[AM_R_HLD] = _AM_R_HLD,
	[AM_HLI_R] = _AM_HLI_R,
	[AM_HLD_R] = _AM_HLD_R,
	[AM_A8_R] = _AM_A8_R,
	[AM_A16_R] = _AM_A16_R,
	[AM_D16_R] = _AM_A16_R,
	[AM_MR_D8] = _AM_MR_D8,
	[AM_MR] = _AM_MR,
	[AM_R_A16] = _AM_R_A16,
};
#define cpu_fetch_data() fetch_funcs[cpu.instr->addr_mode]()

// Condition checking functions
uint8_t _CT_NZ() { return (cpu.regs.F & FLAG_Z) == 0; }
uint8_t _CT_Z() { return (cpu.regs.F & FLAG_Z) == FLAG_Z; }
uint8_t _CT_NC() { return (cpu.regs.F & FLAG_C) == 0; }
uint8_t _CT_C() { return (cpu.regs.F & FLAG_C) == FLAG_C; }
ufunc_t cnd_funcs[] = {
	[CT_NONE] = _RET1,
	[CT_NZ] = _CT_NZ,
	[CT_Z] = _CT_Z,
	[CT_NC] = _CT_NC,
	[CT_C] = _CT_C,
};
#define cpu_check_cond() cnd_funcs[cpu.instr->cnd]()

static void cpu_goto(uint16_t addr, uint8_t push_pc) {
	if (cpu_check_cond()) {
		if (push_pc) {
			emu_incr_cycles(2);
			stack_push16(cpu.regs.PC);
		}
		cpu.regs.PC = addr;
		emu_incr_cycles(1);
	}
}

// Instruction exection functions
void _IN_LD() {
	if (cpu.use_mem_dest) {
		if (cpu.instr->reg2 >= RT_SP) {
			emu_incr_cycles(1);
			bus_write16(cpu.mem_dest, cpu.fetched_data);
		} else {
			bus_write(cpu.mem_dest, cpu.fetched_data);
		}
		emu_incr_cycles(1);
	} else if (cpu.instr->addr_mode == AM_HL_SPR) {
		uint16_t r2 = cpu_read_reg(cpu.instr->reg2);
		CPU_SET_FLAG(FLAG_Z, 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, (r2 & 0x0F) + (cpu.fetched_data & 0x0F) >= 0x10);
		CPU_SET_FLAG(FLAG_C, (r2 & 0xFF) + (cpu.fetched_data & 0xFF) >= 0x100);
		cpu_write_reg(cpu.instr->reg1, r2 + (int8_t)cpu.fetched_data);
	} else {
		cpu_write_reg(cpu.instr->reg1, cpu.fetched_data);
	}
}
void _IN_LDH() {
	if (cpu.instr->reg1 == RT_A) {
		cpu_write_reg(cpu.instr->reg1, bus_read(cpu.fetched_data | 0xFF00));
	} else {
		bus_write(cpu.mem_dest, cpu.regs.A);
	}
	emu_incr_cycles(1);
}
void _IN_DI() {
	cpu.master_interrupts = 0;
}
void _IN_EI() {
	cpu.enable_interrupts = 1;
}
void _IN_XOR() {
	cpu.regs.A ^= cpu.fetched_data & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_POP() {
	uint16_t low = stack_pop();
	emu_incr_cycles(1);
	uint16_t high = stack_pop();
	emu_incr_cycles(1);
	uint16_t val = (high << 8) | low;
	if (cpu.instr->reg1 == RT_AF) {
		cpu_write_reg(cpu.instr->reg1, val & 0xFFF0);
	} else {
		cpu_write_reg(cpu.instr->reg1, val);
	}
}
void _IN_PUSH() {
	uint16_t val = cpu_read_reg(cpu.instr->reg1);
	emu_incr_cycles(1);
	stack_push((val >> 8) & 0xFF);
	emu_incr_cycles(1);
	stack_push(val & 0xFF);
	emu_incr_cycles(1);
}
void _IN_CALL() {
	cpu_goto(cpu.fetched_data, 1);
}
void _IN_JP() {
	cpu_goto(cpu.fetched_data, 0);
}
void _IN_JR() {
	int8_t val = (int8_t)(cpu.fetched_data & 0xFF);
	cpu_goto(cpu.regs.PC + val, 0);
}
void _IN_RST() {
	emu_incr_cycles(2);
	stack_push16(cpu.regs.PC);
	cpu.regs.PC = cpu.instr->param;
	emu_incr_cycles(1);
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
	if (cpu.instr->cnd) {
		emu_incr_cycles(1);
	}
	if (cpu_check_cond()) {
		uint16_t low = stack_pop();
		emu_incr_cycles(1);
		uint16_t high = stack_pop();
		emu_incr_cycles(1);
		cpu.regs.PC = (high << 8) | low;
		emu_incr_cycles(1);
	}
}
void _IN_INC() {
	uint16_t val;
	if (cpu.instr->reg1 >= RT_SP) {
		emu_incr_cycles(1);
	}
	if (cpu.instr->reg1 == RT_HL && cpu.instr->addr_mode == AM_MR) {
		uint16_t hl = cpu_read_reg(RT_HL);
		val = (bus_read(hl) + 1) & 0xFF;
		bus_write(hl, val);
	} else {
		val = cpu_read_reg(cpu.instr->reg1) + 1;
		cpu_write_reg(cpu.instr->reg1, val);
		val = cpu_read_reg(cpu.instr->reg1); 
	}
	if ((cpu.opcode & 0x03) != 0x03) {
		CPU_SET_FLAG(FLAG_Z, val == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, (val & 0x0F) == 0);
	}
}
void _IN_DEC() {
	uint16_t val;
	if (cpu.instr->reg1 >= RT_SP) {
		emu_incr_cycles(1);
	}
	if (cpu.instr->reg1 == RT_HL && cpu.instr->addr_mode == AM_MR) {
		uint16_t hl = cpu_read_reg(RT_HL);
		val = bus_read(hl) - 1;
		bus_write(hl, val);
	} else {
		val = cpu_read_reg(cpu.instr->reg1) - 1;
		cpu_write_reg(cpu.instr->reg1, val);
		val = cpu_read_reg(cpu.instr->reg1);
	}
	if ((cpu.opcode & 0x0B) != 0x0B) {
		CPU_SET_FLAG(FLAG_Z, val == 0);
		CPU_SET_FLAG(FLAG_N, 1);
		CPU_SET_FLAG(FLAG_H, (val & 0x0F) == 0x0F);
	}
}
void _IN_ADD() {
	uint32_t val;
	if (cpu.instr->reg1 >= RT_SP) {
		emu_incr_cycles(1);
	}
	uint32_t reg = cpu_read_reg(cpu.instr->reg1);
	if (cpu.instr->reg1 == RT_SP) {
		val = reg + (int8_t)cpu.fetched_data;
		CPU_SET_FLAG(FLAG_Z, 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (cpu.fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
		CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (cpu.fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
	} else {
		val = reg + cpu.fetched_data;
		if (cpu.instr->reg1 > RT_SP) {
			CPU_SET_FLAG(FLAG_N, 0);
			CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (cpu.fetched_data & 0x0FFF)) >= 0x1000 ? 1 : 0);
			CPU_SET_FLAG(FLAG_C, val >= 0x10000 ? 1 : 0);
		} else if (cpu.instr->reg1 < RT_SP) {
			CPU_SET_FLAG(FLAG_Z, ((val) & 0xFF) == 0);
			CPU_SET_FLAG(FLAG_N, 0);
			CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (cpu.fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
			CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (cpu.fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
		}
	}
	cpu_write_reg(cpu.instr->reg1, val & 0xFFFF);
}
void _IN_SUB() {
	uint16_t val = cpu_read_reg(cpu.instr->reg1) - cpu.fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu_read_reg(cpu.instr->reg1) & 0x0F) - ((int)cpu.fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, ((int)cpu_read_reg(cpu.instr->reg1)) - ((int)cpu.fetched_data) < 0);
	cpu_write_reg(cpu.instr->reg1, val);
}
void _IN_ADC() {
	uint16_t reg = cpu.regs.A;
	cpu.regs.A = (reg + cpu.fetched_data + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (cpu.fetched_data & 0x0F) + CPU_FLAG_C_SET > 0x0F);
	CPU_SET_FLAG(FLAG_C, reg + cpu.fetched_data + (uint16_t)CPU_FLAG_C_SET > 0xFF);
}
void _IN_SBC() {
	uint8_t val = cpu.fetched_data + CPU_FLAG_C_SET;
	int reg = cpu_read_reg(cpu.instr->reg1);
	cpu_write_reg(cpu.instr->reg1, reg - val);
	CPU_SET_FLAG(FLAG_Z, reg - val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) - ((int)cpu.fetched_data & 0x0F) - (int)CPU_FLAG_C_SET) < 0);
	CPU_SET_FLAG(FLAG_C, (reg - (int)cpu.fetched_data - (int)CPU_FLAG_C_SET) < 0);
}
void _IN_OR() {
	cpu.regs.A |= cpu.fetched_data & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_CP() {
	int val = (int)cpu.regs.A - (int)cpu.fetched_data;
	CPU_SET_FLAG(FLAG_Z, val == 0);
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, ((int)cpu.regs.A & 0x0F) - ((int)cpu.fetched_data & 0x0F) < 0);
	CPU_SET_FLAG(FLAG_C, val < 0);
}
void _IN_CB() {
	uint8_t val = cpu.fetched_data;
	uint8_t reg = rt_lookup[val & 0x07];
	uint8_t rval;
	uint8_t bit = (val >> 3) & 0x07;
	uint16_t hl; 
	if (reg == RT_HL) {
		hl = cpu_read_reg(RT_HL);
		rval = bus_read(hl);
		emu_incr_cycles(3);
	} else {
		rval = cpu_read_reg(reg);
		emu_incr_cycles(1);
	}
	
	switch ((val >> 6) & 0x03) {
	case 1: // BIT
		CPU_SET_FLAG(FLAG_Z, (rval & (1 << bit)) == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 1);
		return;
	case 2: // RST
		rval &= ~(1 << bit);
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
		return;
	case 3: // SET
		rval |= (1 << bit);
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
		return;
	default:
		break;
	}
	
	uint8_t res;
	uint8_t set;
	switch (bit) {
	case 0: // RLC
		set = 0;
		res = (rval << 1) & 0xFF;
		if ((rval & 0x80) == 0x80) {
			res |= 1;
			set = 1;
		}
		if (reg == RT_HL) {
			bus_write(hl, res);
		} else {
			cpu_write_reg(reg, res);
		}
		CPU_SET_FLAG(FLAG_Z, res == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, set);
		break;
	case 1: // RRC
		res = rval;
		rval = (rval >> 1) | (res << 7);
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
		CPU_SET_FLAG(FLAG_Z, rval == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, res & 1);
		break;
	case 2: // RL
		res = rval;
		rval = (rval << 1) | CPU_FLAG_C_SET;
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
		CPU_SET_FLAG(FLAG_Z, !rval);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
		break;
	case 3: // RR
		res = rval;
		rval = (rval >> 1) | (CPU_FLAG_C_SET << 7);
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
		CPU_SET_FLAG(FLAG_Z, rval == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, res & 1);
		break;
	case 4: // SLA
		res = rval;
		rval <<= 1;
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
		CPU_SET_FLAG(FLAG_Z, rval == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, (res & 0x80) == 0x80);
		break;
	case 5: // SRA
		res = (int8_t)rval >> 1;
		if (reg == RT_HL) {
			bus_write(hl, res);
		} else {
			cpu_write_reg(reg, res);
		}
		CPU_SET_FLAG(FLAG_Z, res == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, rval & 1);
		break;
	case 6: // SWAP
		rval = ((rval & 0xF0) >> 4) | ((rval & 0x0F) << 4);
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
		CPU_SET_FLAG(FLAG_Z, rval == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, 0);
		break;
	case 7: // SRL
		res = rval >> 1;
		if (reg == RT_HL) {
			bus_write(hl, res);
		} else {
			cpu_write_reg(reg, res);
		}
		CPU_SET_FLAG(FLAG_Z, res == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, rval & 1);
		break;
	}
}
void _IN_AND() {
	cpu.regs.A &= cpu.fetched_data & 0xFF;
	CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 1);
	CPU_SET_FLAG(FLAG_C, 0);
}
void _IN_RLCA() {
	uint8_t val = (cpu.regs.A >> 7) & 1;
	cpu.regs.A = val | (cpu.regs.A << 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}
void _IN_RRCA() {
	uint8_t val = cpu.regs.A & 1;
	cpu.regs.A = (val << 7) | (cpu.regs.A >> 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}
void _IN_RLA() {
	uint8_t val = (cpu.regs.A >> 7) & 1;
	cpu.regs.A = CPU_FLAG_C_SET | (cpu.regs.A << 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}
void _IN_RRA() {
	uint8_t val = cpu.regs.A & 1;
	cpu.regs.A = (CPU_FLAG_C_SET << 7) | (cpu.regs.A >> 1);
	CPU_SET_FLAG(FLAG_Z, 0);
	CPU_SET_FLAG(FLAG_N, 0);
	CPU_SET_FLAG(FLAG_H, 0);
	CPU_SET_FLAG(FLAG_C, val);
}
void _IN_STOP() {
	sceClibPrintf("IN_STOP: NOIMPL\n");
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
void _IN_CPL() {
	cpu.regs.A = ~cpu.regs.A;
	CPU_SET_FLAG(FLAG_N, 1);
	CPU_SET_FLAG(FLAG_H, 1);
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
func_t instr_funcs[] = {
	[IN_LD] = _IN_LD,
	[IN_LDH] = _IN_LDH,
	[IN_DI] = _IN_DI,
	[IN_EI] = _IN_EI,
	[IN_XOR] = _IN_XOR,
	[IN_POP] = _IN_POP,
	[IN_PUSH] = _IN_PUSH,
	[IN_CALL] = _IN_CALL,
	[IN_JP] = _IN_JP,
	[IN_JR] = _IN_JR,
	[IN_RST] = _IN_RST,
	[IN_RETI] = _IN_RETI,
	[IN_RET] = _IN_RET,
	[IN_INC] = _IN_INC,
	[IN_DEC] = _IN_DEC,
	[IN_ADD] = _IN_ADD,
	[IN_SUB] = _IN_SUB,
	[IN_ADC] = _IN_ADC,
	[IN_SBC] = _IN_SBC,
	[IN_OR] = _IN_OR,
	[IN_CP] = _IN_CP,
	[IN_CB] = _IN_CB,
	[IN_AND] = _IN_AND,
	[IN_RLCA] = _IN_RLCA,
	[IN_RRCA] = _IN_RRCA,
	[IN_RLA] = _IN_RLA,
	[IN_RRA] = _IN_RRA,
	[IN_STOP] = _IN_STOP,
	[IN_DAA] = _IN_DAA,
	[IN_CPL] = _IN_CPL,
	[IN_SCF] = _IN_SCF,
	[IN_CCF] = _IN_CCF,
	[IN_NOP] = _NOP,
	[IN_HALT] = _IN_HALT,
};

#define cpu_exec_instr() instr_funcs[cpu.instr->type]()

// Register names lookup table
static const char *reg_names[] = {
	[RT_A]  = "A",
	[RT_F]  = "F",
	[RT_B]  = "B",
	[RT_C]  = "C",
	[RT_D]  = "D",
	[RT_E]  = "E",
	[RT_H]  = "H",
	[RT_L]  = "L",
	[RT_SP] = "SP",
	[RT_PC] = "PC",	
	[RT_AF] = "AF",
	[RT_BC] = "BC",	
	[RT_DE] = "DE",
	[RT_HL] = "HL",	
};

// Instructions logging functions
static const char *cpu_name_funcs[] = {
	[IN_NOP] = "NOP",
	[IN_LD] = "LD",
	[IN_LDH] = "LDH",
	[IN_JP] = "JP",
	[IN_DI] = "DI",
	[IN_EI] = "EI",
	[IN_XOR] = "XOR",
	[IN_POP] = "POP",
	[IN_PUSH] = "PUSH",
	[IN_CALL] = "CALL",
	[IN_JP] = "JP",
	[IN_JR] = "JR",
	[IN_RST] = "RST",
	[IN_RET] = "RET",
	[IN_RETI] = "RETI",
	[IN_INC] = "INC",
	[IN_DEC] = "DEC",
	[IN_ADD] = "ADD",
	[IN_SUB] = "SUB",
	[IN_ADC] = "ADC",
	[IN_SBC] = "SBC",
	[IN_OR] = "OR",
	[IN_CP] = "CP",
	[IN_CB] = "CB",
	[IN_AND] = "AND",
	[IN_RLCA] = "RLCA",
	[IN_RRCA] = "RRCA",
	[IN_RLA] = "RLA",
	[IN_RRA] = "RRA",
	[IN_STOP] = "STOP",
	[IN_DAA] = "DAA",
	[IN_CPL] = "CPL",
	[IN_SCF] = "SCF",
	[IN_CCF] = "CCF",
	[IN_HALT] = "HALT",
};
static char _instr_str[32];
void _LOG_AM_R_D16() {
	sprintf(_instr_str, "%s %s,0x%04X", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], cpu.fetched_data);
}
void _LOG_AM_R_R() {
	sprintf(_instr_str, "%s %s,%s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
}
void _LOG_AM_MR_R() {
	sprintf(_instr_str, "%s (%s),%s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
}
void _LOG_AM_R() {
	sprintf(_instr_str, "%s %s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1]);
}
void _LOG_AM_R_D8() {
	sprintf(_instr_str, "%s %s,0x%02X", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], cpu.fetched_data & 0xFF);
}
void _LOG_AM_R_MR() {
	sprintf(_instr_str, "%s %s,(%s)", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
}
void _LOG_AM_R_HLI() {
	sprintf(_instr_str, "%s %s,(%s+)", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
}
void _LOG_AM_R_HLD() {
	sprintf(_instr_str, "%s %s,(%s-)", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
}
void _LOG_AM_HLI_R() {
	sprintf(_instr_str, "%s (%s+),%s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
}
void _LOG_AM_HLD_R() {
	sprintf(_instr_str, "%s (%s-),%s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
}
void _LOG_AM_A8_R() {
	sprintf(_instr_str, "%s 0x%02X,%s", cpu_name_funcs[cpu.instr->type], bus_read(cpu.regs.PC - 1), reg_names[cpu.instr->reg2]);
}
void _LOG_AM_HL_SPR() {
	sprintf(_instr_str, "%s (%s),SP+%d", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], cpu.fetched_data & 0xFF);
}
void _LOG_AM_D16() {
	sprintf(_instr_str, "%s 0x%04X", cpu_name_funcs[cpu.instr->type], cpu.fetched_data);
}
void _LOG_AM_D8() {
	sprintf(_instr_str, "%s 0x%02X", cpu_name_funcs[cpu.instr->type], cpu.fetched_data & 0xFF);
}
void _LOG_AM_D16_R() {
	sprintf(_instr_str, "%s (0x%04X),%s", cpu_name_funcs[cpu.instr->type], cpu.fetched_data, reg_names[cpu.instr->reg2]);
}
void _LOG_AM_MR_D8() {
	sprintf(_instr_str, "%s (%s),0x%02X", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], cpu.fetched_data & 0xFF);
}
void _LOG_AM_MR() {
	sprintf(_instr_str, "%s (%s)", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1]);
}
void _LOG_AM_IMP() {
	strcpy(_instr_str, cpu_name_funcs[cpu.instr->type]);
}
func_t instr_log_funcs[] = {
	[AM_IMP] = _LOG_AM_IMP,
	[AM_R_D16] = _LOG_AM_R_D16,
	[AM_R_A16] = _LOG_AM_R_D16,
	[AM_R_R] = _LOG_AM_R_R,
	[AM_MR_R] = _LOG_AM_MR_R,
	[AM_R] = _LOG_AM_R,
	[AM_R_D8] = _LOG_AM_R_D8,
	[AM_R_A8] = _LOG_AM_R_D8,
	[AM_R_MR] = _LOG_AM_R_MR,
	[AM_R_HLI] = _LOG_AM_R_HLI,
	[AM_R_HLD] = _LOG_AM_R_HLD,
	[AM_HLI_R] = _LOG_AM_HLI_R,
	[AM_HLD_R] = _LOG_AM_HLD_R,
	[AM_A8_R] = _LOG_AM_A8_R,
	[AM_HL_SPR] = _LOG_AM_HL_SPR,
	[AM_D16] = _LOG_AM_D16,
	[AM_D8] = _LOG_AM_D8,
	[AM_D16_R] = _LOG_AM_D16_R,
	[AM_A16_R] = _LOG_AM_D16_R,
	[AM_MR_D8] = _LOG_AM_MR_D8,
	[AM_MR] = _LOG_AM_MR,
};
#define cpu_stringify_instr() instr_log_funcs[cpu.instr->addr_mode]()

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
		cpu.instr = &instrs[cpu.opcode];
		emu_incr_cycles(1);
		
		// Fetching any required data for the given instruction
		cpu.mem_dest = 0;
		cpu.use_mem_dest = 0;
		cpu_fetch_data();
		
		// Interpreter debugger
		if (emu.opts.debug_log) {
			char c = CPU_FLAG_C_SET ? 'C' : '-';
			char z = CPU_FLAG_Z_SET ? 'Z' : '-';
			char n = CPU_FLAG_N_SET ? 'N' : '-';
			char h = CPU_FLAG_H_SET ? 'H' : '-';
			cpu_stringify_instr();
			sceClibPrintf("%04X: %-16s (%02X) A: %02X F: %c%c%c%c BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
				instr_PC, _instr_str, cpu.opcode, cpu.regs.A, z, n, h, c, cpu.regs.B, cpu.regs.C, cpu.regs.D, cpu.regs.E, cpu.regs.H, cpu.regs.L);
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