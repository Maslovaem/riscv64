#include <stdio.h>
#include <cstdint>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int32_t Register;
typedef uint32_t Addr;
const size_t kNumRegisters = 32;
const size_t kMemSize = 0x400000;

/**
 * ISA description
 * 4 bytes for each instruction
 * first byte - opcode
 * second byte - dest (if present)
 * third, fourth - two sources (if present)
 */

//из cpp, чтобы перечисляемые элементы были не типа int
enum class Opcode : std::uint8_t {
	kUnknown = 0,
	kAdd = 0b0110011,	//add, sub
	kAddi = 0b0010011,
	kJal = 0b1101111,
	kJalr = 0b1100111,
	kBeq = 0b1100011,	//beq, bne
	kLoad = 0b0000011,	//lb, lh, lw
	kStore = 0b0100011,	//sw, sh, sw
  	kEBreak = 0b1110011,
	kLui = 0b0110111,
  	kAuipc = 0b0010111,
};

struct Memory {
	Register data[kMemSize]; 
};

struct CpuState {
    Register pc;
    Register regs[kNumRegisters];
    struct Memory *memory;
    bool is_finished;
}; 

typedef struct {
	Opcode opcode;
	Register rs1, rs2, rd;
	uint8_t func3;
	uint8_t func7;
	uint32_t imm;
} Instruction;

Register load(struct Memory *memory, Addr addr);
void store(struct Memory *memory, Addr addr, Register value);

void cpu_state_init(CpuState *cpu, struct Memory *memory);
void memory_init(struct Memory *mem);

Register fetch(struct CpuState *cpu);
Instruction decode(Register bytes);
void dump_cpu(struct CpuState *cpu);

Register get_reg(struct CpuState *cpu, size_t id);
void set_reg(struct CpuState *cpu, size_t id, Register value);

Opcode get_opcode(Register bytes);
Register get_rd(Register bytes);
uint8_t get_func3(Register bytes);
uint8_t get_func7(Register bytes);
Register get_rs1(Register bytes);
Register get_rs2(Register bytes);
uint32_t get_imm_I(Register bytes);
uint32_t get_imm_B(Register bytes);
uint32_t get_imm_S(Register bytes);
uint32_t get_imm_U(Register bytes);
uint32_t get_imm_J(Register bytes);

int main()
{	
	return 0;
}

Register load(struct Memory *memory, Addr addr)
{
	//проверка
	 return memory->data[addr];
}

void store(struct Memory *memory, Addr addr, Register value) 
{	
	//проверка на выход на пределы 
	 memory->data[addr] = value; 
}

void cpu_state_init(CpuState *cpu, struct Memory *mem)
{
	cpu->pc = 0;
	cpu->memory = mem;
	cpu->is_finished = false;
}

void memory_init(struct Memory *mem) 
{
	for (size_t i = 0; i < kMemSize; i++)
	{
		mem->data[i] = 0;
	}
  
	if (mem->data == NULL) 
	{
        	printf("Failed to initialize memory\n");
        	exit(-1);
    	}
}

Register get_reg(struct CpuState *cpu, size_t id) 
{
	return cpu->regs[id];
}

void set_reg(struct CpuState *cpu, size_t id, Register value) 
{
    cpu->regs[id] = value;
}

void dump_cpu(struct CpuState *cpu) 
{
	printf("CpuState:\n");
	printf("PC: %u\n", cpu->pc);
	for (size_t i = 0; i < kNumRegisters; i++) 
	{
		 printf("[%lu] = %u\n", i, cpu->regs[i]);
	}
}

Register fetch(struct CpuState *cpu) 
{
    return load(cpu->memory, cpu->pc);
}

Opcode get_opcode(Register bytes) 
{
	//0 - 6 bits
	return (Opcode)(bytes & 0x7F);
}

Register get_rd(Register bytes)  
{
	//7 - 11 bits
	return (bytes >> 7) & 0x1F;
}

Register get_rs1(Register bytes) 
{
	//15 - 19 bits
	return (bytes >> 15) & 0x1F;
}

Register get_rs2(Register bytes)  
{
	//20 - 24 bits
	return (bytes >> 20) & 0x1F;
}

uint8_t get_func3(Register bytes)
{
	//12 - 14 bits
	return (bytes >> 12) & 0x7;
}

uint8_t get_func7(Register bytes)
{
	//25 - 31 bits
	return (bytes >> 25) & 0x7F;
}

uint32_t get_imm_I(Register bytes)
{
	//20 - 31 bits
	//mb smth with sign extenshion ??
	return (bytes >> 20) & 0xFFF;
}

uint32_t get_imm_S(Register bytes)
{
	return ((bytes >> 7) & 0x1F) | (((bytes >> 25) & 0x7F) << 5);
}

uint32_t get_imm_B(Register bytes)
{
	return (((bytes >> 8) & 0xF) << 1) | (((bytes >> 7) & 0x1) << 11) | (((bytes >> 25) & 0x3F) << 5) | (((bytes >> 31) & 0x1) << 31);
}

uint32_t get_imm_U(Register bytes)
{
	return (bytes & 0xFFFFF000);
}

uint32_t get_imm_J(Register bytes)
{
        return (((bytes >> 31) & 0x1) << 20) | (((bytes >> 21) & 0x3FF) << 1) | (((bytes >> 20) & 0x1) << 11) | (((bytes >> 12) & 0xFF) << 12);
}


Instruction decode(Register bytes) 
{
	Instruction instruction;
	instruction.opcode = get_opcode(bytes);
	switch (instruction.opcode) 
	{	
		//R_type
        	case Opcode::kAdd:
			instruction.rd = get_rd(bytes);
			instruction.func3 = get_func3(bytes);
			instruction.rs1 = get_rs1(bytes);
			instruction.rs2 = get_rs2(bytes);
			instruction.func7 = get_func7(bytes);
			break;
		//I_type
		case Opcode::kAddi:
		case Opcode::kJalr:
		case Opcode::kEBreak:
		case Opcode::kLoad:
			instruction.rd = get_rd(bytes);
                        instruction.func3 = get_func3(bytes);
                        instruction.rs1 = get_rs1(bytes);
			instruction.imm = get_imm_I(bytes);
			break;
		//J_type
		case Opcode::kJal:
			instruction.rd = get_rd(bytes);
                        instruction.imm = get_imm_J(bytes);
			break;
		//S_type
        	case Opcode::kStore:
			instruction.func3 = get_func3(bytes);
                        instruction.rs1 = get_rs1(bytes);
                        instruction.rs2 = get_rs2(bytes);
			instruction.imm = get_imm_S(bytes);
			break;
		//U_type
		case Opcode::kLui:
		case Opcode::kAuipc:
			instruction.rd = get_rd(bytes);
			instruction.imm = get_imm_U(bytes);
			break;
		//B-type
        	case Opcode::kBeq:
			instruction.func3 = get_func3(bytes);
                        instruction.rs1 = get_rs1(bytes);
                        instruction.rs2 = get_rs2(bytes);
                        instruction.imm = get_imm_U(bytes);
            		break;
        	default:
            		printf("Unknown instruction");
	}
	return instruction;
}

void execute(CpuState *cpu, Instruction *instruction) {
	switch (instruction->opcode) 
	{
		case Opcode::kAdd:
		{
			Register res = 0; 
			if (instruction->func7 == 0b0000000)
			{
				res = get_reg(cpu, instruction->rs1) + get_reg(cpu, instruction->rs2);
			}
			else if (instruction->func7 == 0b0100000)
			{
				res = get_reg(cpu, instruction->rs1) - get_reg(cpu, instruction->rs2);
			}
			set_reg(cpu, instruction->rd, res);
			cpu->pc += 4;
			break;
		}
		case Opcode::kAddi:
		{
			Register res = get_reg(cpu, instruction->rs1) + instruction->imm;
			set_reg(cpu, instruction->rd, res);
			cpu->pc += 4;
			break;
		}
		case Opcode::kEBreak: 
		{
			cpu->is_finished = true;
			break;
		}
		case Opcode::kJal:
		{ 
			set_reg(cpu, instruction->rd, cpu->pc + 4);
			Register jump_target_address = cpu->pc + instruction->imm;
        		cpu->pc = jump_target_address;
			break;
		}
		case Opcode::kJalr:
		{
			set_reg(cpu, instruction->rd, cpu->pc + 4);
                        Register jump_target_address = (cpu->pc + instruction->imm) & 0xFFFFFFFE;
                        cpu->pc = jump_target_address; 
        		break;
		}
		case Opcode::kStore: 
        		cpu->pc += 4;
			break;
		case Opcode::kBeq: 
        		break;
		default:
			printf("Unknown instruction");
    }
}
