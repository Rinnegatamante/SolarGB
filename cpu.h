#ifndef _CPU_H_
#define _CPU_H_

#ifdef __cplusplus
extern "C" {
#endif

// Instructions types
enum {
	IN_NOP  = 0x01,
	IN_LD   = 0x02,
	IN_INC  = 0x03,
	IN_DEC  = 0x04,
	IN_RLCA = 0x05,
	IN_ADD  = 0x06,
	IN_RRCA = 0x07,
	IN_STOP = 0x08,
	IN_RLA  = 0x09,
	IN_JR   = 0x0A,
	IN_RRA  = 0x0B,
	IN_DAA  = 0x0C,
	IN_CPL  = 0x0D,
	IN_SCF  = 0x0E,
	IN_CCF  = 0x0F,
	IN_HALT = 0x10,
	IN_ADC  = 0x11,
	IN_SUB  = 0x12,
	IN_SBC  = 0x13,
	IN_AND  = 0x14,
	IN_XOR  = 0x15,
	IN_OR   = 0x16,
	IN_CP   = 0x17,
	IN_POP  = 0x18,
	IN_JP   = 0x19,
	IN_PUSH = 0x1A,
	IN_RET  = 0x1B,
	IN_CB   = 0x1C,
	IN_CALL = 0x1D,
	IN_RETI = 0x1E,
	IN_LDH  = 0x1F,
	IN_JPHL = 0x20,
	IN_DI   = 0x21,
	IN_EI   = 0x22,
	IN_RST  = 0x23,
	IN_ERR  = 0x24,
	IN_RLC  = 0x25,
	IN_RRC  = 0x26,
	IN_RL   = 0x27,
	IN_RR   = 0x28,
	IN_SLA  = 0x29,
	IN_SRA  = 0x2A,
	IN_SWAP = 0x2B,
	IN_SRL  = 0x2C,
	IN_BIT  = 0x2D,
	IN_RES  = 0x2E,
	IN_SET  = 0x2F,	
};

// Address mode types
enum {
	AM_IMP    = 0x00,
	AM_R_D16  = 0x01,
	AM_R_R    = 0x02,
	AM_MR_R   = 0x03,
	AM_R      = 0x04,
	AM_R_D8   = 0x05,
	AM_R_MR   = 0x06,
	AM_R_HLI  = 0x07,
	AM_R_HLD  = 0x08,
	AM_HLI_R  = 0x09,
	AM_HLD_R  = 0x0A,
	AM_R_A8   = 0x0B,
	AM_A8_R   = 0x0C,
	AM_HL_SPR = 0x0D,
	AM_D16    = 0x0E,
	AM_D8     = 0x0F,
	AM_D16_R  = 0x10,
	AM_MR_D8  = 0x11,
	AM_MR     = 0x12,
	AM_A16_R  = 0x13,
	AM_R_A16  = 0x14,	
};

// Register access types
enum {
	RT_NONE = 0x00,
	RT_A    = 0x01,
	RT_F    = 0x02,
	RT_B    = 0x03,
	RT_C    = 0x04,
	RT_D    = 0x05,
	RT_E    = 0x06,
	RT_H    = 0x07,
	RT_L    = 0x08,
	RT_SP   = 0x09,
	RT_PC   = 0x0A,
	RT_AF   = 0x0B,
	RT_BC   = 0x0C,
	RT_DE   = 0x0D,
	RT_HL   = 0x0E,	
};

// Condition types
enum {
	CT_NONE = 0x00,
	CT_NZ   = 0x01,
	CT_Z    = 0x02,
	CT_NC   = 0x03,
	CT_C    = 0x04,
};

// Register F flags bitmask
enum {
	FLAG_C = 0x10,
	FLAG_H = 0x20,
	FLAG_N = 0x40,
	FLAG_Z = 0x80,
};

// Interrupt types
enum {
	IT_VBLANK    = 0x01,
	IT_LCD_STAT  = 0x02,
	IT_TIMER     = 0x04,
	IT_SERIAL    = 0x08,
	IT_JOYPAD    = 0x10,
};

typedef struct {
	uint8_t type;
	uint8_t addr_mode;
	uint8_t reg1;
	uint8_t reg2;
	uint8_t cnd;
	uint8_t param;
} instr_t;

typedef struct {
	uint8_t A;
	uint8_t F;
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
	uint16_t PC;
	uint16_t SP;
	uint8_t IE;
} reg_t;

typedef struct {
	uint8_t debug_log; // Log debug info on system console
	uint8_t debug_ppu; // Show PPU data on screen
	uint8_t serial_port_enabled; // Log serial port output to system console
	uint8_t emu_state;
	reg_t regs;
	uint8_t interrupts;
	uint8_t halted;
	uint8_t master_interrupts;
	uint8_t enable_interrupts;
} cpu_t;

extern cpu_t cpu;

void cpu_init();
void cpu_step();

uint16_t cpu_read_reg(uint8_t reg);
void cpu_write_reg(uint8_t reg, uint16_t val);

#define CPU_FLAG_C_SET ((cpu.regs.F & FLAG_C) == FLAG_C)
#define CPU_FLAG_H_SET ((cpu.regs.F & FLAG_H) == FLAG_H)
#define CPU_FLAG_N_SET ((cpu.regs.F & FLAG_N) == FLAG_N)
#define CPU_FLAG_Z_SET ((cpu.regs.F & FLAG_Z) == FLAG_Z)

#define CPU_SET_FLAG(f, x) ((x) ? (cpu.regs.F |= f) : (cpu.regs.F &= ~f))
#define CPU_SET_INTERRUPT(x) cpu.interrupts |= x;
#define CPU_MASTER_INTR_SET(x) (((cpu.interrupts & x) == x) && ((cpu.regs.IE & x) == x))

// Stack functions
static inline __attribute__((always_inline)) void stack_push(uint8_t val) {
	cpu.regs.SP--;
	bus_write(cpu.regs.SP, val);
}
static inline __attribute__((always_inline)) void stack_push16(uint16_t val) {
	stack_push((val >> 8) & 0xFF);
	stack_push(val & 0xFF);
}
static inline __attribute__((always_inline)) uint8_t stack_pop() {
	return bus_read(cpu.regs.SP++);
}
static inline __attribute__((always_inline)) uint16_t stack_ppop16() {
	uint16_t low = stack_pop();
	uint16_t high = stack_pop();
	return (high << 8) | low;
}

#ifdef __cplusplus
}
#endif

#endif
