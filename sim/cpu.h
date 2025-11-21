#pragma once
#include "../sim/memory.h"
#include "../sim/alu.h"
#include "../sim/helpers.h"
#include <bit>
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
        CpuSim(MemoryBus& memBus, uint32_t offset, uint32_t sp) : pipelineBusy(0), scalar(0), simMemory(memBus), pc(offset), pc_offset(offset) {int_reg_bank[2] = sp;}
        bool notStalled();
        void fetch();
        void decode();
        void execute();
        void store();
        void initializeRegisters();
        void flush();
        int alu(int reg1_val, int reg2_val, int function_code, int imm);
        float alu(float reg1_val, float reg2_val, int function_code);

        uint32_t pc;
        uint32_t pc_offset;
        uint32_t instr_decode_pc;  // PC value when instruction was fetched
        uint32_t instr_execute_pc;
        pipelineState   state;
        uint32_t        instruction;
        uint32_t        jumpval;
        uint32_t        stallTime;
        bool            pipelineBusy;
        bool            shouldHalt = false;
        float           scalar;
        unsigned int    array[160];
        riscvInstr      assemblyCode;
        executeData     exeData;
        uint32_t int_reg_bank[32] = {0,0, };
        float   float_reg_bank[32];
        MemoryBus& simMemory;
        uint32_t instrCt = 0;
};