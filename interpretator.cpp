#include <stdio.h>
#include <cstdint>
#include <stdint.h>
#include <stdbool.h>

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
/из cpp, чтобы перечисляемые элементы были не типа int
enum class Opcode : std::uint8_t {
        kUnknown = 0,
        kAdd = 0b0110011,
        kSub =  0b0110011,
        kAddi = 0b0010011,
        kJal = 0b1101111,
        kJalr = 0b1100111,
        kBne = 0b1100011,
        kBeq = 0b1100011,
        kLoad = 0b0000011,      //lb, lh, lw
        kStore = 0b0100011,     //sw, sh, sw
        kEBreak = 0b1110011,
        kLui = 0b0110111,
        kAuipc = 0b0010111,
};

struct Memory {
        Register data[kMemSize]; 
};

struct CpuState{
    Register pc;
    Register regs[kNumRegisters];
    struct Memory *memory;
    bool is_finished;
}; 

Register load(struct Memory *memory, Addr addr);
void store(struct Memory *memory, Addr addr, Register value);

void CpuState_init(CpuState *cpu, struct Memory *memory);

Register fetch(struct CpuState *cpu);
Opcode get_opcode(Register bytes);
Register get_dst(Register bytes);
Register get_src1(Register bytes);
Register get_src2(Register bytes);

int main()
{       
        return 0;
}

Register load(struct Memory *memory, Addr addr)
{
        //проверка...
         return memory->data[addr];
}

void store(struct Memory *memory, Addr addr, Register value) 
{       
        //проверка на выход на пределы 
         memory->data[addr] = value; 
}

void CpuState_init(CpuState *cpu, struct Memory *mem)
{
        cpu->pc = 0;
        cpu->memory = mem;
        cpu->is_finished = false;
}

Register fetch(struct CpuState *cpu) 
{
    return load(cpu->memory, cpu->pc);
}

Opcode get_opcode(Register bytes) 
{
    return (Opcode)((bytes >> 24U) & 0xFFU);
}

Register get_dst(Register bytes)  
{
    return (bytes >> 16U) & 0xFFU;
}

Register get_src1(Register bytes) 
{
    return (bytes >> 8U) & 0xFFU;
}

Register get_src2(Register bytes)  
{
    return bytes & 0xFFU;
}
