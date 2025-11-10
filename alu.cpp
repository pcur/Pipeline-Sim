#include "alu.h"


int alu(int reg1_val, int reg2_val, int function_code){
    int val;
    switch(function_code){
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