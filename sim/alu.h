#include <cstdint>

// For the arithmetic int alu operations
#define ADD     0b0000000000
#define SUB     0b0100000000
#define SLL     0b0000000001
#define SLT     0b0000000010
#define SLTU    0b0000000011
#define XOR     0b0000000100
#define SRL     0b0000000101
#define SRA     0b0100000101
#define OR      0b0000000110
#define AND     0b0000000111

// For the logical int alu operations
#define BEQ     0b1100011000
#define BNE     0b1100011001
#define BLT     0b1100011100
#define BGE     0b1100011101
#define BLTU    0b1100011110
#define BGEU    0b1100011111

// For the Jump stuff
#define JAL     0b1101111000
#define JALR    0b1100111000

// For lui stuff
#define LUI     0b0110111000

// For the floating point alu
#define FADDS   0b0000000000
#define FSUBS   0b0000100000

int alu(int reg1_val, int reg2_val, int function_code, uint32_t progCount, int imm);

float alu(float reg1_val, float reg2_val, int function_code);