#include <stdio.h>
#include <cstdint>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int32_t Register;
typedef uint32_t Addr;
const size_t kNumRegisters = 32;
const size_t kMemSize = 0x100; //мало места под ubuntu

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
	int imm;
} Instruction;

Register load(struct Memory *memory, Addr addr);
void store(struct Memory *memory, Addr addr, Register value);

void cpu_state_init(CpuState *cpu, struct Memory *memory);
void memory_init(struct Memory *mem);
void init_regs(CpuState *cpu);

Register fetch(struct CpuState *cpu);
Instruction decode(Register bytes);
void execute(CpuState *cpu, Instruction *instruction);
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
int get_imm_B(Register bytes);
uint32_t get_imm_S(Register bytes);
uint32_t get_imm_U(Register bytes);
uint32_t get_imm_J(Register bytes);

int my_strlen(const char *str);
void asm_printf(const char *str);

int main()
{	
	CpuState cpu;
	Memory memory;

	cpu_state_init(&cpu, &memory);
	memory_init(&memory);
	
	uint32_t program[] = {
		0x00100093,  // addi a1, x0, 1
		0x00100113,  // addi a2, x0, 1
		0x00000013,  // addi a0, x0, 0       //for i
		0x00300193,  // addi a3, x0, 3       //for n, here n = 3

		0x00110233,  // add a4, a1, a2
		0x00010093,  // addi a1, a2, 0
		0x00020113,  // addi a2, a4, 0

		0x00100013,  // addi a0, x0, 1
		0xFE3018E3,  // bne a3, a0, -16

		0x00100073,
	};

	for (int i = 0; i < sizeof(program) / sizeof(program[0]); i++) 
	{
		store(&memory, i * 4, program[i]);
	}

	while (!cpu.is_finished) 
	{
		Register bytes = fetch(&cpu);              
		Instruction instruction = decode(bytes);   
		execute(&cpu, &instruction);               
		dump_cpu(&cpu);                            
	}
	
	return 0;
}

int my_strlen(const char *str)
{	
	int len = 0;
	
	for(int i = 0; str[i] != '\0'; i++)
	{
		len++;
	}
	return len;
}

/*void asm_printf(const char *str)
{
	int len = my_strlen(str);

	asm (
		"li a0, 1\n"         
		"la a1, %0\n"        
		"mv a2, %1\n"        
		"li a7, 64\n"        
		"ecall\n"            
		: //output
		: "r" (str), "r" (len)  //input
		: "a0", "a1", "a2", "a7"  //we need to spesify used registers
	);
}*/

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
	cpu->memory = mem;
	cpu->is_finished = false;
	init_regs(cpu);
}

void memory_init(struct Memory *mem) 
{
	if (mem == NULL) 
	{
		printf("Memory pointer is NULL\n");
		exit(1);
	}
    
	for (size_t i = 0; i < kMemSize; i++)
	{
		mem->data[i] = 0;
	}
}

void init_regs(CpuState *cpu)
{
	cpu->pc = 0;
	for(int i = 0; i < kNumRegisters; i++)
	{
		cpu->regs[i] = 0;
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

int get_imm_B(Register bytes)
{
	return ((bytes >> 20) & 0xFFFFF000) | ((bytes << 4) & 0x00000800) | ((bytes >> 20) & 0x000007E0) | ((bytes >> 7) & 0x0000001E);
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
			printf("Addi\n");
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
                        instruction.imm = get_imm_B(bytes);
            		break;
        	default:
            		printf("Unknown instruction, opcode: %hhu\n", (uint8_t)instruction.opcode);
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
			if (instruction->func3 == 0b010)
			{
				//sw
				//мб проверка на кратность адреса 4
				store(cpu->memory, instruction->rs1 + instruction->imm, instruction->rs2);
			} 
			else if (instruction->func3 == 0b001)
			{
				//sh
				//мб проверка на кратность адреса 2
				store(cpu->memory, instruction->rs1 + instruction->imm, instruction->rs2 & 0xFFFF);
			}
			else if (instruction->func3 == 0b000)
			{
				//sb
				store(cpu->memory, instruction->rs1 + instruction->imm, instruction->rs2 & 0xFF);
			}
			else
			{
				printf("Unknown instruction of type Store\n");
			}
        		cpu->pc += 4;
			break;
		case Opcode::kLoad:
			if (instruction->func3 == 0b010)
			{	
				//lw
				set_reg(cpu, instruction->rd, load(cpu->memory, instruction->rs1 + instruction->imm));
			}
			else if (instruction->func3 == 0b001)
			{
				//lh
				set_reg(cpu, instruction->rd, (load(cpu->memory, instruction->rs1 + instruction->imm)) & 0xFFFF);
			}
			else if (instruction->func3 == 0b000)
			{
				//lb
				set_reg(cpu, instruction->rd, (load(cpu->memory, instruction->rs1 + instruction->imm)) & 0xFF);
			}
			else
			{
				printf("Unknown instruction of type Load");
			}
			cpu->pc += 4;
			break;
		case Opcode::kBeq: 
			if (instruction->func3 == 0b000)
			{
				if (get_reg(cpu, instruction->rs1) == get_reg(cpu, instruction->rs2))
				{
					cpu->pc += instruction->imm;
				}
				else
				{
					cpu->pc += 4;
				}
			}
			else if (instruction->func3 == 0b001)
			{
				if (get_reg(cpu, instruction->rs1) != get_reg(cpu, instruction->rs2))
                                {
                                        cpu->pc += instruction->imm;
					printf("bne imm: %d", instruction->imm);
                                }
                                else
                                {
                                        cpu->pc += 4;
                                }

			}
			else 
			{
				printf("Unknown instruction of type Branch\n");
			}
        		break;
		case Opcode::kLui:
			set_reg(cpu, instruction->rd, instruction->imm & 0xFFFFF000);
			cpu->pc += 4;
			break;
		case Opcode::kAuipc:
			set_reg(cpu, instruction->rd, cpu->pc + (instruction->imm & 0xFFFFF000));
			cpu->pc += 4;
			break;
		default:
			printf("Unknown instruction execute\n");
    }
}
