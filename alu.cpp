#include "alu.h"


int alu(int reg1_val, int reg2_val, int function_code){
    switch(function_code){
        // Add
        case ADD:
            return reg1_val + reg2_val;
            break;
        // Subtract
        case SUB:
            return reg1_val - reg2_val;  
            break;
        // Shift Left Logical
        case SLL:
            return reg1_val << reg2_val;
            break;
        // Set Less Than
        case SLT:
            return (reg1_val < reg2_val) ? 1 : 0;
            break;
        // Set Less Than Unsigned
        case SLTU: (uint32_t(reg1_val) < uint32_t(reg2_val)) ? 1 : 0;
            break;
        // XOR
        case XOR:
            return reg1_val ^ reg2_val;
            break;
        // Shift Right Logical
        case SRL: 
            return reg1_val >> reg2_val;
            break;
        // Shift Right Arthmetic. NOT CORRECT
        case SRA:
            return reg1_val >> reg2_val;
            break;
        // Or
        case OR:
            return reg1_val | reg2_val;
            break;
        // AND
        case AND:
            return reg1_val & reg2_val;
            break;
        default:
            break;
    }
}