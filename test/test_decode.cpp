#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <random>
#include <bitset>
#include <cstdint>
#include "../test/config.h"

#define FLD     0b0000111
#define FADD    0b1010011
#define FSD     0b0100111
#define ADDI    0b0010011
#define BNE     0b1100011
            
#define RTYPE   0b0110011
#define RTYPE2  0b1010011
#define ITYPE   0b0010011
#define ITYPE3  0b0000011
#define BTYPE   0b1100011
#define STYPE   0b0100111
#define UTYPE   0b0110111
#define UTYPE2  0b0010111
#define JTYPE   0b1101111
#define JTYPE2  0b1100111

#define NOP     0b0000000
unsigned int instrQ[10];
uint32_t pc;
bool halted;
int debug;

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
};
struct pipelineState{
    std::string fetchState;
    std::string decodeState;
    std::string executeState;
    std::string storeState;
};

pipelineState state;

int32_t decodeBTypeImm(uint32_t instr) {
    uint32_t imm12   = (instr >> 31) & 0x1;       // bit 31 -> imm[12]
    uint32_t imm11   = (instr >> 7)  & 0x1;       // bit 7  -> imm[11]
    uint32_t imm10_5 = (instr >> 25) & 0x3F;      // bits 30..25 -> imm[10:5]
    uint32_t imm4_1  = (instr >> 8)  & 0xF;       // bits 11..8  -> imm[4:1]

    uint32_t imm = (imm12 << 12) |
                   (imm11 << 11) |
                   (imm10_5 << 5) |
                   (imm4_1 << 1);

    // Sign extend 13-bit immediate
    return (int32_t)((imm ^ 0x1000) - 0x1000);
}

int32_t decodeJALImmediate(uint32_t instruction) {
    // Extract relevant bits
    int32_t imm20   = (instruction >> 31) & 0x1;       // bit 31
    int32_t imm10_1 = (instruction >> 21) & 0x3FF;     // bits 30–21
    int32_t imm11   = (instruction >> 20) & 0x1;       // bit 20
    int32_t imm19_12 = (instruction >> 12) & 0xFF;     // bits 19–12

    // Combine them into a 21-bit immediate (note positions)
    int32_t imm = (imm20 << 20) |
                  (imm19_12 << 12) |
                  (imm11 << 11) |
                  (imm10_1 << 1);

    // Sign-extend to 32 bits (if imm20 is set, negative offset)
    if (imm20)
        imm |= 0xFFF00000;

    return imm;
}

riscvInstr decode(uint32_t instruction) {
    riscvInstr      assemblyCode;
    assemblyCode.opcode = instruction & 0x7F;

    switch(assemblyCode.opcode){
        case RTYPE: // register arithmetic
            state.decodeState    = "RTYPE";
            // R-type instruction decoding
            assemblyCode.funct7  = (instruction & 0xFE000000) >> 25;
            assemblyCode.rs2     = (instruction & 0x01F00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            // ALU control
            assemblyCode.alucode = (assemblyCode.funct7 << 3) + assemblyCode.funct3;
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 0;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            break;

        case RTYPE2: // register arithmetic
            state.decodeState    = "RTYPE-F";
            // R-type instruction decoding
            assemblyCode.funct7  = (instruction & 0xFE000000) >> 25;
            assemblyCode.rs2     = (instruction & 0x01F00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            // ALU control
            assemblyCode.alucode = (assemblyCode.funct7 << 3) + assemblyCode.funct3;
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 0;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            break;

        case ITYPE: // immediate arithmetic
            state.decodeState    = "ITYPE";
            // I-type instruction decoding
            assemblyCode.imm     = (instruction & 0xFFF00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            assemblyCode.funct7 = 0;
            if(assemblyCode.funct3 == 5){
                assemblyCode.funct7 = assemblyCode.imm & 0xFE0;
            }  
            // ALU control
            assemblyCode.alucode = (assemblyCode.funct7 << 3) | assemblyCode.funct3;
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 1;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            break;

        case ITYPE3: // I type, loads
            state.decodeState    = "ITYPE"; 
            // I-type instruction decoding
            assemblyCode.imm     = (instruction & 0xFFF00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            assemblyCode.funct7 = 0;
            // ALU control
            assemblyCode.alucode = 0; // add
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 1;
            // Memory mux control
            assemblyCode.store_sel = 1;
            assemblyCode.mem_load_sel = 1;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 1;
            break;

        case STYPE: // stores
            state.decodeState    = "STYPE";
            // S-type instruction decoding
            assemblyCode.imm     = ((instruction & 0xFE000000) >> 25) + ((instruction & 0x00000F80) >> 7);
            assemblyCode.rs2     = (instruction & 0x01F00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            // ALU control
            assemblyCode.alucode = 0; // add
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 1;
            // Memory mux control
            assemblyCode.store_sel = 1;
            assemblyCode.mem_load_sel = 1;
            assemblyCode.wb_enable = 0;
            assemblyCode.rw_enable = 1;
            // Size control
            assemblyCode.bit_len = 8 << assemblyCode.funct3;
            break;

        case BTYPE: // branching
            state.decodeState    = "BTYPE";
            // B-type instruction decoding
            assemblyCode.imm     = decodeBTypeImm(instruction);
            assemblyCode.rs2     = (instruction & 0x01F00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            // ALU control
            assemblyCode.alucode = assemblyCode.funct3 << 3; //TODO: Need some way to keep it unique from other op codes            
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 0;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 0;
            assemblyCode.rw_enable = 0;
            break;

        case JTYPE: // jal
            state.decodeState    = "JTYPE";
            // J-type instruction decoding
            assemblyCode.imm     = decodeJALImmediate(instruction);
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            assemblyCode.funct3  = 0;
            assemblyCode.funct7  = 0;
            // ALU control
            assemblyCode.alucode = 0;
            assemblyCode.pc_enable = 1;
            assemblyCode.imm_sel = 0;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            break;
        
        case JTYPE2: // jalr
            state.decodeState    = "JTYPE";
            // J-type instruction decoding
            assemblyCode.imm     = (instruction & 0xFFF00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            assemblyCode.funct3  = 0;
            assemblyCode.funct7  = 0;
            // ALU control
            assemblyCode.alucode = 0;
            assemblyCode.pc_enable = 1;
            assemblyCode.imm_sel = 0;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            break;
            

        case UTYPE: // lui - load upper immediate
            state.decodeState   = "UTYPE";
            // U-type instruction decoding
            assemblyCode.imm    = (instruction & 0xFFFFF000) >> 12;
            assemblyCode.rd     = (instruction & 0x00000F80) >> 7;
            // ALU control
            assemblyCode.alucode = 0x11111111; //TODO: keep the same
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 1;
            // Memory mux control
            assemblyCode.store_sel = 1;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            break;

        case UTYPE2: // auipc - add upper immediate to pc
            state.decodeState    = "UTYPE";
            // U-type instruction decoding
            assemblyCode.imm    = (instruction & 0xFFFFF000) >> 12;
            assemblyCode.rd     = (instruction & 0x00000F80) >> 7;
            // ALU control
            assemblyCode.alucode = 0; //add
            assemblyCode.pc_enable = 1;
            assemblyCode.imm_sel = 1;
            // Memory mux control
            assemblyCode.store_sel = 1;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            break;

        case NOP: // no op
            state.decodeState    = "NO_OP";
            break;

        default:
            state.decodeState    = "???";
            break;

    }
    return assemblyCode;
}

void fill_queue(const std::string& filename, unsigned int* instructionQueue, size_t queueSize) {
    std::ifstream infile(filename);
    std::string line;
    size_t index = 0;

    while (std::getline(infile, line) && index < queueSize) {
        unsigned int instruction = std::bitset<32>(line).to_ulong();
        instructionQueue[index] = instruction;  // Store instruction in array
        ++index;  // Move to the next index
    }
}

int main() {
    std::cout << "Decode Simulation, assignment 3\n";
    fill_queue("instructions.txt", instrQ, 9);

    for(int i = 0; i < 4; i++){
        std::bitset<32> binaryRepresentation(instrQ[i]);
        std::cout << "Input: " << binaryRepresentation << std::endl;
        riscvInstr values = decode(instrQ[i]);
            std::cout << "Type: " << state.decodeState << std::endl;
            std::cout << "Opcode: " << std::bitset<7>(values.opcode)
                    << " rd: " << std::bitset<5>(values.rd)
                    << " funct7: " << std::bitset<7>(values.funct7)
                    << " funct3: " << std::bitset<3>(values.funct3)
                    << " rs1: " << std::bitset<5>(values.rs1)
                    << " rs2: " << std::bitset<5>(values.rs2)
                    << " imm: " << std::bitset<13>(values.imm)
                    << " alucode: " << std::bitset<10>(values.alucode)
                    << " pc_enable: " << values.pc_enable
                    << " imm_sel: " << values.imm_sel
                    << " store_sel: " << values.store_sel
                    << " mem_load_sel: " << values.mem_load_sel
                    << " wb_enable: " << values.wb_enable
                    << " rw_enable: " << values.rw_enable
                    << "\n\n";
    }

    return 0;
}