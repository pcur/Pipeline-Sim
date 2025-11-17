#include "alu.h"


int alu(int reg1_val, int reg2_val, int function_code, uint32_t pc, int imm){
    int val;
    switch(function_code){
        // Arithmetic ALU ops
        // Add
        case ADD:
            val = reg1_val + reg2_val;
            break;
        // Subtract
        case SUB:
            val = reg1_val - reg2_val;  
            break;
        // Shift Left Logical
        case SLL:
            val = reg1_val << reg2_val;
            break;
        // Set Less Than
        case SLT:
            val = (reg1_val < reg2_val) ? 1 : 0;
            break;
        // Set Less Than Unsigned
        case SLTU: (uint32_t(reg1_val) < uint32_t(reg2_val)) ? 1 : 0;
            break;
        // XOR
        case XOR:
            val = reg1_val ^ reg2_val;
            break;
        // Shift Right Logical
        case SRL: 
            val = reg1_val >> reg2_val;
            break;
        // Shift Right Arthmetic. NOT CORRECT
        case SRA:
            val = reg1_val >> reg2_val;
            break;
        // Or
        case OR:
            val = reg1_val | reg2_val;
            break;
        // AND
        case AND:
            val = reg1_val & reg2_val;
            break;
        // Branching ALU ops
        // Branch Equal to
        case BEQ:
            if(reg1_val == reg2_val) pc = pc + imm - 1; // -1 Because we add it already in fetch
            break;
        // Branch Not Equal to
        case BNE:
            if(reg1_val != reg2_val) pc = pc + imm - 1;
            break;
        // Branch less than
        case BLT:
            if(reg1_val < reg2_val) pc = pc + imm - 1;
            break;
        // Branch greater than
        case BGE:
            if(reg1_val > reg2_val) pc = pc + imm - 1;
            break;
        // Branch Less than or Equal to
        case BLTU:
            if(reg1_val <= reg2_val) pc = pc + imm - 1;
            break;
        // Branch Greater than or Equal to
        case BGEU:
            if(reg1_val >= reg2_val) pc = pc + imm - 1;
            break;
        case JAL:
            pc = pc + imm - 1;
            break;
        case JALR:
            pc = pc + imm - 1;
            break;
        case LUI:
            val = reg2_val << 12;
            break;
        default:
            val = 0;
            break;
    }
    return val;
}

float alu(float reg1_val, float reg2_val, int function_code){
    float val;
    switch(function_code){
        // FADD.S
        case FADDS:
            val = reg1_val + reg2_val;
            break;
        // FSUB.S
        case FSUBS:
            val = reg1_val - reg2_val;
            break;
        default:
            val = 0;
            break;
    }
    return val;
}