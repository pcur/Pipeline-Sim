#include "alu.h"


int alu(int reg1_val, int reg2_val, int function_code){
    switch(function_code){
        case ADD:
            return reg1_val + reg2_val;
            break;
        case SUB:
            return reg1_val - reg2_val;  
            break;
        case SLL:
            return reg1_val << reg2_val;
            break;
        case SLT:
            return (reg1_val < reg2_val) ? 1 : 0;
            break;
        case SLTU: (uint32_t(reg1_val) < uint32_t(reg2_val)) ? 1 : 0;
            break;
        case XOR:
            return reg1_val ^ reg2_val;
            break;
        case SRL: 
            return reg1_val >> reg2_val;
            break;
        case SRA:
            return reg1_val >> reg2_val;
            break;
        case OR:
            return reg1_val | reg2_val;
            break;
        case AND:
            return reg1_val & reg2_val;
            break;
        default:
            break;
    }
}