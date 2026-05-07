#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct state_t {
	uint16_t pc;
	uint16_t psw;
	union {
		uint16_t regs[16];
		struct {
			uint16_t t0;
			uint16_t t1;
			uint16_t t2;
			uint16_t t3;
			uint16_t s0;
			uint16_t s1;
			uint16_t s2;
			uint16_t s3;
			uint16_t s4;
			uint16_t s5;
			uint16_t ra;
			uint16_t gp;
			uint16_t sp;
			uint16_t fp;
			uint16_t a0;
			uint16_t a1;
		};
	};
} state_t;

void show_cpu_state(state_t* cpu) {
	printf("pc=%i, psw=%i, [", cpu->pc, cpu->psw);
	for (size_t i = 0; i < 16; i++) {
		if (i) printf(", ");
		printf("%i", cpu->regs[i]);
	};
	printf("]\n");
};

uint8_t data_memory[65536];
uint32_t program_memory[65536];

typedef uint16_t opcode_t;
enum {
	OPCODE_ADD = 0,
	OPCODE_LW = 2,
	OPCODE_SW = 3,
	OPCODE_BEQ = 4,
	OPCODE_J = 14
};

#define MANGLE_ADD(rd, rs, rt) (OPCODE_ADD | ((rd) << 4) | ((rs) << 8) | ((rt) << 12))
#define MANGLE_LW(rd, rs, off) (OPCODE_LW | ((rd) << 4) | ((rs) << 8) | ((off) << 12))
#define MANGLE_SW(rd, rs, off) (OPCODE_SW | ((rd) << 4) | ((rs) << 8) | ((off) << 12))
#define MANGLE_BEQ(rd, rs, addr) (OPCODE_BEQ | ((rd) << 4) | ((rs) << 8) | ((off) << 12))
#define MANGLE_J(addr) (OPCODE_BEQ | ((addr) << 4))

int main() {
	printf("HOLA\n");

	data_memory[1] = 1;
	data_memory[3] = 2;
	program_memory[0] = MANGLE_LW(1, 0, 2);
	program_memory[1] = MANGLE_LW(0, 0, 0);
	program_memory[2] = 999;

	bool running = true;
	state_t cpu = {0};
	while (running) {
		uint32_t inst = program_memory[cpu.pc];
		cpu.pc++;

		switch(inst & 0xF) {
		case OPCODE_ADD: {
			const uint8_t rd = (inst >>  4) & 0xF;
			const uint8_t rs = (inst >>  8) & 0xF;
			const uint8_t rt = (inst >> 12) & 0xF;
			cpu.regs[rd] = cpu.regs[rs] + cpu.regs[rt];
			break;
		};

		case OPCODE_LW: {
			const uint8_t rd = (inst >> 4) & 0xF;
			const uint8_t rs = (inst >> 8) & 0xF;
			const uint8_t offset = (inst >> 12) & 0xFF;
			cpu.regs[rd] = data_memory[cpu.regs[rs] + offset];
			break;
		};

		case OPCODE_SW: {
			const uint8_t rd = (inst >> 4) & 0xF;
			const uint8_t rs = (inst >> 8) & 0xF;
			const uint8_t offset = (inst >> 12) & 0xFF;
			data_memory[cpu.regs[rs] + offset] = cpu.regs[rd];
			break;
		};

		case OPCODE_BEQ: {
			const uint8_t rd = (inst >> 4) & 0xF;
			const uint8_t rs = (inst >> 8) & 0xF;
			const uint8_t offset = (inst >> 12) & 0xFF;
			if (cpu.regs[rd] == cpu.regs[rs]) {
				cpu.pc += offset;
			};
			break;
		};

		case OPCODE_J: {
			const uint16_t offset = (inst >> 4) & 0xFFFF;
			cpu.pc = offset;
			break;
		};

		default:
			printf("illegal opcode %i at %i\n", inst & 0xF, cpu.pc);
			running = false;
			break;
		};
	};

	show_cpu_state(&cpu);
	return 0;
};
