#include <stdio.h>
#include <cstdint>
#include <stdint.h>

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
        Register data[kMemSize] = {}; 
} memory; 

Register load(Addr addr);
void store(Addr addr, Register value);

int main()
{       
        return 0;
}

Register load(Addr addr)
{
         return memory.data[addr];
}

void store(Addr addr, Register value) 
{
         memory.data[addr] = value; 
}
