#pragma once
#include "../sim/memory.h"
#include "../sim/alu.h"

/*
#define FLD     0b0000111
#define FADD    0b1010011
#define FSD     0b0100111
#define ADDI    0b0010011
#define BNE     0b1100011
*/

#define RTYPE   0b0110011
#define RTYPE2  0b1010011
#define ITYPE   0b0010011
#define ITYPE2  0b0000111
#define ITYPE3  0b0000011
#define BTYPE   0b1100011
#define STYPE   0b0100011
#define STYPE2  0b0100111
#define UTYPE   0b0110111
#define UTYPE2  0b0010111
#define JTYPE   0b1101111
#define JTYPE2  0b1100111

#define NOP     0b0000000

#define Byte        8
#define HalfWord    16
#define Word        32

extern unsigned int instrQ[10];
extern uint32_t pc;
extern bool halted;
extern int debug;

struct riscvInstr{
    // RISC-V instruction fields
    uint32_t opcode;
    int funct3;
    int funct7;
    int rs1;
    int rs2;
    int imm;
    int rd;
    // ALU control
    int alucode;
    bool pc_enable;
    bool imm_sel;
    // Memory mux control
    bool store_sel;
    bool mem_load_sel;
    int bit_len;
    bool rw_enable;
    bool wb_enable;
    int float_regs;
};

struct executeData{
    int alu_val1;
    int alu_val2;
    float alu_float1;
    float alu_float2;
    int wb_int_val;
    float wb_float_val;
    uint32_t wb_reg;
    uint8_t wb;
};

struct pipelineState{
    std::string fetchState;
    std::string decodeState;
    std::string executeState;
    std::string storeState;
};

class CpuSim {
    public:
        CpuSim(MemoryBus memBus) : pipelineBusy(0), scalar(0), x1(160), x2(0), f2(1), simMemory(memBus) {}
        bool notStalled();
        void fetch();
        void decode();
        void execute();
        void store();

        pipelineState   state;
        uint32_t        instruction;
        uint32_t        jumpval;
        uint16_t        f0;
        uint16_t        f2;
        uint16_t        f4;
        uint16_t        x1;
        uint16_t        x2;
        uint32_t        stallTime;
        bool            pipelineBusy;
        float           scalar;
        unsigned int    array[160];
        riscvInstr      assemblyCode;
        executeData     exeData;
        uint32_t int_reg_bank[32] = {0,};
        float   float_reg_bank[32];
        MemoryBus simMemory;
};