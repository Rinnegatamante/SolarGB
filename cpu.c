#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "cpu.h"
#include "emu.h"

cpu_t cpu;
static char serial_out[1024] = {};
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
	[0x35] = {.type = IN_DEC, .addr_mode = AM_R, .reg1 = RT_HL},
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
	[0xC4] = {.type = IN_CALL, .addr_mode = AM_D16},
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
	cpu.regs.IE = 0x00;
	cpu.fetched_data = 0;
	cpu.mem_dest = 0;
	cpu.use_mem_dest = 0;
	cpu.master_interrupts = 0;
	cpu.interrupts = 0x00;
	cpu.enable_interrupts = 0;
	cpu.halted = 0;
}

// Registers reading function
uint16_t cpu_read_reg(uint8_t reg) {
	switch (reg) {
	case RT_A:
		return cpu.regs.A;
	case RT_F:
		return cpu.regs.F;
	case RT_B:
		return cpu.regs.B;
	case RT_C:
		return cpu.regs.C;
	case RT_D:
		return cpu.regs.D;
	case RT_E:
		return cpu.regs.E;
	case RT_H:
		return cpu.regs.H;
	case RT_L:
		return cpu.regs.L;
	case RT_SP:
		return cpu.regs.SP;
	case RT_PC:
		return cpu.regs.PC;
	case RT_HL:
		return ((uint16_t)cpu.regs.L | (((uint16_t)cpu.regs.H) << 8));
	case RT_AF:
		return ((uint16_t)cpu.regs.F | (((uint16_t)cpu.regs.A) << 8));
	case RT_BC:
		return ((uint16_t)cpu.regs.C | (((uint16_t)cpu.regs.B) << 8));
	case RT_DE:
		return ((uint16_t)cpu.regs.E | (((uint16_t)cpu.regs.D) << 8));
	default:
		sceClibPrintf("cpu_read_reg: Invalid register\n");
		break;
	}
}

// Registers writing function
void cpu_write_reg(uint8_t reg, uint16_t val) {
	switch (reg) {
	case RT_A:
		cpu.regs.A = val;
		break;
		break;
	case RT_F:
		cpu.regs.F = val;
		break;
	case RT_B:
		cpu.regs.B = val;
		break;
	case RT_C:
		cpu.regs.C = val;
		break;
	case RT_D:
		cpu.regs.D = val;
		break;
	case RT_E:
		cpu.regs.E = val;
		break;
	case RT_H:
		cpu.regs.H = val;
		break;
	case RT_L:
		cpu.regs.L = val;
		break;
	case RT_SP:
		cpu.regs.SP = val;
		break;
	case RT_PC:
		cpu.regs.PC = val;
		break;
	case RT_HL:
		cpu.regs.L = val & 0xFF;
		cpu.regs.H = (val >> 8) & 0xFF;
		break;
	case RT_AF:
		cpu.regs.F = val & 0xFF;
		cpu.regs.A = (val >> 8) & 0xFF;
		break;
	case RT_BC:
		cpu.regs.C = val & 0xFF;
		cpu.regs.B = (val >> 8) & 0xFF;
		break;
	case RT_DE:
		cpu.regs.E = val & 0xFF;
		cpu.regs.D = (val >> 8) & 0xFF;
	default:
		break;
	}	
}

void cpu_fetch_data() {
	uint16_t low, high;
	switch (cpu.instr->addr_mode) {
	case AM_R:
		cpu.fetched_data = cpu_read_reg(cpu.instr->reg1);
		break;
	case AM_R_R:
		cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
		break;
	case AM_R_D8:
	case AM_HL_SPR:
	case AM_D8:
	case AM_R_A8:
		cpu.fetched_data = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		cpu.regs.PC++;
		break;
	case AM_D16:
	case AM_R_D16:
		low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.fetched_data = low | (high << 8);
		cpu.regs.PC += 2;
		break;
	case AM_MR_R:
		cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
		cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
		cpu.use_mem_dest = 1;
		if (cpu.instr->reg1 == RT_C) {
			cpu.mem_dest |= 0xFF00;
		}
		break;
	case AM_R_MR:
		if (cpu.instr->reg2 == RT_C) {
			cpu.fetched_data = bus_read(((uint16_t)cpu.regs.C) | 0xFF00);
		} else {
			cpu.fetched_data = bus_read(cpu_read_reg(cpu.instr->reg2));
		}
		emu_incr_cycles(1);
		break;
	case AM_R_HLI:
		cpu.fetched_data = bus_read(cpu_read_reg(cpu.instr->reg2));
		emu_incr_cycles(1);
		cpu_write_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
		break;
	case AM_R_HLD:
		cpu.fetched_data = bus_read(cpu_read_reg(cpu.instr->reg2));
		emu_incr_cycles(1);
		cpu_write_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
		break;
	case AM_HLI_R:
		cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
		cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
		cpu.use_mem_dest = 1;
		cpu_write_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
		break;
	case AM_HLD_R:
		cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
		cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
		cpu.use_mem_dest = 1;
		cpu_write_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
		break;
	case AM_A8_R:
		cpu.mem_dest = bus_read(cpu.regs.PC) | 0xFF00;
		cpu.use_mem_dest = 1;
		emu_incr_cycles(1);
		cpu.regs.PC++;
		break;
	case AM_A16_R:
	case AM_D16_R:
		low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.mem_dest = low | (high << 8);
		cpu.use_mem_dest = 1;
		cpu.regs.PC += 2;
		cpu.fetched_data = cpu_read_reg(cpu.instr->reg2);
		break;
	case AM_MR_D8:
		cpu.fetched_data = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		cpu.regs.PC++;
		cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
		cpu.use_mem_dest = 1;
		break;
	case AM_MR:
		cpu.mem_dest = cpu_read_reg(cpu.instr->reg1);
		cpu.use_mem_dest = 1;
		cpu.fetched_data = bus_read(cpu.mem_dest);
		emu_incr_cycles(1);
		break;
	case AM_R_A16:
		low = bus_read(cpu.regs.PC);
		emu_incr_cycles(1);
		high = bus_read(cpu.regs.PC + 1);
		emu_incr_cycles(1);
		cpu.regs.PC += 2;
		cpu.fetched_data = bus_read(low | (high << 8));
		emu_incr_cycles(1);
		break;
	default:
		break;
	}
}

static uint8_t cpu_check_cond() {
	switch (cpu.instr->cnd) {
	case CT_NZ:
		return (cpu.regs.F & FLAG_Z) == 0;
	case CT_Z:
		return (cpu.regs.F & FLAG_Z) == FLAG_Z;
	case CT_NC:
		return (cpu.regs.F & FLAG_C) == 0;
	case CT_C:
		return (cpu.regs.F & FLAG_C) == FLAG_C;
	default:
		return 1;
	}
}

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

void cpu_exec_bitwise_instr() {
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
		break;
	case 2: // RST
		rval &= ~(1 << bit);
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
		break;
	case 3: // SET
		rval |= (1 << bit);
		if (reg == RT_HL) {
			bus_write(hl, rval);
		} else {
			cpu_write_reg(reg, rval);
		}
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

void cpu_exec_instr() {
	uint16_t low, high, val, reg;
	uint32_t large_val;
	uint8_t uint_val;
	int8_t int_val;
	
	switch (cpu.instr->type) {
	case IN_LD:
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
		} else {
			cpu_write_reg(cpu.instr->reg1, cpu.fetched_data);
		}
		break;
	case IN_LDH:
		if (cpu.instr->reg1 == RT_A) {
			cpu_write_reg(cpu.instr->reg1, bus_read(cpu.fetched_data | 0xFF00));
		} else {
			bus_write(cpu.mem_dest, cpu.regs.A);
		}
		emu_incr_cycles(1);
		break;
	case IN_DI:
		cpu.master_interrupts = 0;
		break;
	case IN_EI:
		cpu.enable_interrupts = 1;
		break;
	case IN_XOR:
		cpu.regs.A ^= cpu.fetched_data & 0xFF;
		CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, 0);
		break;
	case IN_POP:
		low = stack_pop();
		emu_incr_cycles(1);
		high = stack_pop();
		emu_incr_cycles(1);
		val = (high << 8) | low;
		if (cpu.instr->reg1 == RT_AF) {
			cpu_write_reg(cpu.instr->reg1, val & 0xFFF0);
		} else {
			cpu_write_reg(cpu.instr->reg1, val);
		}
		break;
	case IN_PUSH:
		val = cpu_read_reg(cpu.instr->reg1);
		emu_incr_cycles(1);
		stack_push((val >> 8) & 0xFF);
		emu_incr_cycles(1);
		stack_push(val & 0xFF);
		emu_incr_cycles(1);
		break;
	case IN_CALL:
		cpu_goto(cpu.fetched_data, 1);
		break;
	case IN_JP:
		cpu_goto(cpu.fetched_data, 0);
		break;
	case IN_JR:
		int_val = (int8_t)(cpu.fetched_data & 0xFF);
		cpu_goto(cpu.regs.PC + int_val, 0);
		break;
	case IN_RST:
		cpu_goto(cpu.instr->param, 1);
		break;
	case IN_RETI:
		cpu.master_interrupts = 1;
	case IN_RET:
		if (cpu.instr->cnd) {
			emu_incr_cycles(1);
		}
		if (cpu_check_cond()) {
			low = stack_pop();
			emu_incr_cycles(1);
			high = stack_pop();
			emu_incr_cycles(1);
			cpu.regs.PC = (high << 8) | low;
			emu_incr_cycles(1);
		}
		break;
	case IN_INC:
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
		break;
	case IN_DEC:
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
		break;
	case IN_ADD:
		if (cpu.instr->reg1 >= RT_SP) {
			emu_incr_cycles(1);
		}
		reg = cpu_read_reg(cpu.instr->reg1);
		if (cpu.instr->reg1 == RT_SP) {
			large_val = reg + (int8_t)cpu.fetched_data;
			CPU_SET_FLAG(FLAG_Z, 0);
			CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (cpu.fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
			CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (cpu.fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
		} else {
			large_val = reg + cpu.fetched_data;
			if (cpu.instr->reg1 > RT_SP) {
				CPU_SET_FLAG(FLAG_H, ((reg & 0x0FFF) + (cpu.fetched_data & 0x0FFF)) >= 0x1000 ? 1 : 0);
				CPU_SET_FLAG(FLAG_C, (large_val) >= 0x10000 ? 1 : 0);
			} else if (cpu.instr->reg1 < RT_SP) {
				CPU_SET_FLAG(FLAG_Z, ((large_val) & 0xFF) == 0);
				CPU_SET_FLAG(FLAG_H, ((reg & 0x0F) + (cpu.fetched_data & 0x0F)) >= 0x10 ? 1 : 0);
				CPU_SET_FLAG(FLAG_C, ((reg & 0xFF) + (cpu.fetched_data & 0xFF)) >= 0x100 ? 1 : 0);
			}
		}
		cpu_write_reg(cpu.instr->reg1, large_val & 0xFFFF);
		break;
	case IN_SUB:
		val = cpu_read_reg(cpu.instr->reg1) - cpu.fetched_data;;
		CPU_SET_FLAG(FLAG_Z, val == 0);
		CPU_SET_FLAG(FLAG_H, ((int)cpu_read_reg(cpu.instr->reg1) & 0x0F) - ((int)cpu.fetched_data & 0x0F) < 0);
		CPU_SET_FLAG(FLAG_C, ((int)cpu_read_reg(cpu.instr->reg1)) - ((int)cpu.fetched_data) < 0);
		cpu_write_reg(cpu.instr->reg1, val);
		break;
	case IN_ADC:
		reg = cpu.regs.A;
		cpu.regs.A = (reg + cpu.fetched_data + (uint16_t)CPU_FLAG_C_SET) & 0xFF;
		CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, (reg & 0x0F) + (cpu.fetched_data & 0x0F) + CPU_FLAG_C_SET > 0x0F);
		CPU_SET_FLAG(FLAG_C, reg + cpu.fetched_data + (uint16_t)CPU_FLAG_C_SET > 0xFF);
		break;
	case IN_SBC:
		uint_val = cpu.fetched_data + CPU_FLAG_C_SET;
		reg = cpu_read_reg(cpu.instr->reg1);
		CPU_SET_FLAG(FLAG_Z, (int)reg - uint_val == 0);
		CPU_SET_FLAG(FLAG_N, 1);
		CPU_SET_FLAG(FLAG_H, ((int)reg & 0x0F) - ((int)cpu.fetched_data & 0x0F) - (int)CPU_FLAG_C_SET < 0);
		CPU_SET_FLAG(FLAG_C, (int)reg - (int)cpu.fetched_data - (int)CPU_FLAG_C_SET < 0);
		break;
	case IN_OR:
		cpu.regs.A |= cpu.fetched_data & 0xFF;
		CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, 0);
		break;
	case IN_CP:
		cpu.regs.A = ~cpu.regs.A;
		CPU_SET_FLAG(FLAG_N, 1);
		CPU_SET_FLAG(FLAG_H, 1);
		break;
	case IN_CB:
		cpu_exec_bitwise_instr();
		break;
	case IN_AND:
		cpu.regs.A &= cpu.fetched_data & 0xFF;
		CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 1);
		CPU_SET_FLAG(FLAG_C, 0);
		break;
	case IN_RLCA:
		uint_val = (cpu.regs.A >> 7) & 1;
		cpu.regs.A = uint_val | (cpu.regs.A << 1);
		CPU_SET_FLAG(FLAG_Z, 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, uint_val);
		break;
	case IN_RRCA:
		uint_val = cpu.regs.A & 1;
		cpu.regs.A = (uint_val << 7) | (cpu.regs.A >> 1);
		CPU_SET_FLAG(FLAG_Z, 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, uint_val);
		break;
	case IN_RLA:
		uint_val = (cpu.regs.A >> 7) & 1;
		cpu.regs.A = CPU_FLAG_C_SET | (cpu.regs.A << 1);
		CPU_SET_FLAG(FLAG_Z, 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, uint_val);
		break;
	case IN_RRA:
		uint_val = cpu.regs.A & 1;
		cpu.regs.A = (CPU_FLAG_C_SET << 7) | (cpu.regs.A >> 1);
		CPU_SET_FLAG(FLAG_Z, 0);
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, uint_val);
		break;
	case IN_STOP:
		sceClibPrintf("IN_STOP: NOIMPL\n");
		break;
	case IN_DAA:
		uint_val = 0;
		int_val = 0;
		if (CPU_FLAG_H_SET || (((cpu.regs.A & 0x0F) > 9) && !CPU_FLAG_N_SET)) {
			uint_val = 6;
		}
		if (CPU_FLAG_C_SET || ((cpu.regs.A > 0x99) && !CPU_FLAG_N_SET)) {
			uint_val |= 0x60;
			int_val = 1;
		}
		cpu.regs.A += CPU_FLAG_N_SET ? -uint_val : uint_val;
		CPU_SET_FLAG(FLAG_Z, cpu.regs.A == 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, int_val);
		break;
	case IN_CPL:
		cpu.regs.A = ~cpu.regs.A;
		CPU_SET_FLAG(FLAG_N, 1);
		CPU_SET_FLAG(FLAG_H, 1);
		break;
	case IN_SCF:
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, 1);
		break;
	case IN_CCF:
		CPU_SET_FLAG(FLAG_N, 0);
		CPU_SET_FLAG(FLAG_H, 0);
		CPU_SET_FLAG(FLAG_C, CPU_FLAG_C_SET);
		break;
	case IN_HALT:
		cpu.halted = 1;
		break;
	case IN_NOP:
	default:
		break;
	}
}

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

// Instructions logging function
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
static void cpu_stringify_instr() {
	switch (cpu.instr->addr_mode) {
	case AM_R_D16:
	case AM_R_A16:
		sprintf(_instr_str, "%s %s,0x%04X", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], cpu.fetched_data);
		break;
	case AM_R_R:
		sprintf(_instr_str, "%s %s,%s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
		break;
	case AM_MR_R:
		sprintf(_instr_str, "%s (%s),%s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
		break;
	case AM_R:
		sprintf(_instr_str, "%s %s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1]);
		break;
	case AM_R_D8:
	case AM_R_A8:
		sprintf(_instr_str, "%s %s,0x%02X", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], cpu.fetched_data & 0xFF);
		break;
	case AM_R_MR:
		sprintf(_instr_str, "%s %s,(%s)", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
		break;
	case AM_R_HLI:
		sprintf(_instr_str, "%s %s,(%s+)", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
		break;
	case AM_R_HLD:
		sprintf(_instr_str, "%s %s,(%s-)", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
		break;
	case AM_HLI_R:
		sprintf(_instr_str, "%s (%s+),%s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
		break;
	case AM_HLD_R:
		sprintf(_instr_str, "%s (%s-),%s", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], reg_names[cpu.instr->reg2]);
		break;
	case AM_A8_R:
		sprintf(_instr_str, "%s 0x%02X,%s", cpu_name_funcs[cpu.instr->type], bus_read(cpu.regs.PC - 1), reg_names[cpu.instr->reg2]);
		break;
	case AM_HL_SPR:
		sprintf(_instr_str, "%s (%s),SP+%d", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], cpu.fetched_data & 0xFF);
		break;
	case AM_D16:
		sprintf(_instr_str, "%s 0x%04X", cpu_name_funcs[cpu.instr->type], cpu.fetched_data);
		break;
	case AM_D8:
		sprintf(_instr_str, "%s 0x%02X", cpu_name_funcs[cpu.instr->type], cpu.fetched_data & 0xFF);
		break;
	case AM_D16_R:
	case AM_A16_R:
		sprintf(_instr_str, "%s (0x%04X),%s", cpu_name_funcs[cpu.instr->type], cpu.fetched_data, reg_names[cpu.instr->reg2]);
		break;
	case AM_MR_D8:
		sprintf(_instr_str, "%s (%s),0x%02X", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1], cpu.fetched_data & 0xFF);
		break;
	case AM_MR:
		sprintf(_instr_str, "%s (%s)", cpu_name_funcs[cpu.instr->type], reg_names[cpu.instr->reg1]);
		break;
	default:
		strcpy(_instr_str, cpu_name_funcs[cpu.instr->type]);
		break;
	}
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
		cpu.opcode = bus_read(cpu.regs.PC);
		cpu.instr = &instrs[cpu.opcode];
		cpu.regs.PC++;
		
		// Fetching any required data for the given instruction
		cpu.mem_dest = 0;
		cpu.use_mem_dest = 0;
		cpu_fetch_data();
		
		// Interpreter debugger
		if (emu.debug_log) {
			char c = CPU_FLAG_C_SET ? 'C' : '-';
			char z = CPU_FLAG_Z_SET ? 'Z' : '-';
			char n = CPU_FLAG_N_SET ? 'N' : '-';
			char h = CPU_FLAG_H_SET ? 'H' : '-';
			cpu_stringify_instr();
			sceClibPrintf("%04X: %-16s (%02X) A: %02X F: %c%c%c%c BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
				instr_PC, _instr_str, cpu.opcode, cpu.regs.A, z, n, h, c, cpu.regs.B, cpu.regs.C, cpu.regs.D, cpu.regs.E, cpu.regs.H, cpu.regs.L);
		}
		
		// Serial data handling
		if (emu.serial_port_enabled) {
			if (bus_read(0xFF02) == 0x81) {
				char ch = bus_read(0xFF01);
				serial_out[serial_len++] = ch;
				serial_out[serial_len] = 0;
				bus_write(0xFF02, 0);
			}
			if (serial_len > 0) {
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
			cpu.interrupts &=  ~IT_VBLANK;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		} else if (CPU_MASTER_INTR_SET(IT_LCD_STAT)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &=  ~IT_LCD_STAT;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		} else if (CPU_MASTER_INTR_SET(IT_TIMER)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &=  ~IT_TIMER;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		} else if (CPU_MASTER_INTR_SET(IT_SERIAL)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &=  ~IT_SERIAL;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		} else if (CPU_MASTER_INTR_SET(IT_JOYPAD)) {
			stack_push16(cpu.regs.PC);
			cpu.regs.PC = 0x40;
			cpu.interrupts &=  ~IT_JOYPAD;
			cpu.halted = 0;
			cpu.master_interrupts = 0;
		}
		cpu.enable_interrupts = 0;
	} else if (cpu.enable_interrupts) {
		cpu.master_interrupts = 1;
	}
}