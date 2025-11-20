#include "../sim/alu.h"


int alu(int reg1_val, int reg2_val, int function_code, uint32_t* progCount, int imm){
    int val;
    std::string state;
    std::string debugStr;
    switch(function_code){
        // Arithmetic ALU ops
        // Add
        case ADD:
            val = reg1_val + reg2_val;
            state = "ADD";
            break;
        // Subtract
        case SUB:
            val = reg1_val - reg2_val;  
            state = "SUB";
            break;
        // Shift Left Logical
        case SLL:
            val = reg1_val << reg2_val;
            state = "SLL";
            break;
        // Set Less Than
        case SLT:
            val = (reg1_val < reg2_val) ? 1 : 0;
            state = "SLT";
            break;
        // Set Less Than Unsigned
        case SLTU: (uint32_t(reg1_val) < uint32_t(reg2_val)) ? 1 : 0;
            state = "SLTU";
            break;
        // XOR
        case XOR:
            val = reg1_val ^ reg2_val;
            state = "XOR";
            break;
        // Shift Right Logical
        case SRL: 
            val = reg1_val >> reg2_val;
            state = "SRL";
            break;
        // Shift Right Arthmetic. NOT CORRECT
        case SRA:
            val = reg1_val >> reg2_val;
            state = "SRA";
            break;
        // Or
        case OR:
            val = reg1_val | reg2_val;
            state = "OR";
            break;
        // AND
        case AND:
            val = reg1_val & reg2_val;
            state = "AND";
            break;
        // Branching ALU ops
        // Branch Equal to
        case BEQ:
            if(reg1_val == reg2_val) *progCount = *progCount + imm - 1; // -1 Because we add it already in fetch
            val = *progCount;
            state = "BEQ";
            break;
        // Branch Not Equal to
        case BNE:
            if(reg1_val != reg2_val) *progCount = *progCount + imm - 1;
            val = *progCount;
            state = "BNE";
            break;
        // Branch less than
        case BLT:
            if(reg1_val < reg2_val) *progCount = *progCount + imm - 1;
            val = *progCount;
            state = "BTL";
            break;
        // Branch greater than
        case BGE:
            if(reg1_val > reg2_val) *progCount = *progCount + imm - 1;
            val = *progCount;
            state = "BGE";
            break;
        // Branch Less than or Equal to
        case BLTU:
            if(reg1_val <= reg2_val) *progCount = *progCount + imm - 1;
            val = *progCount;
            state = "BLTU";
            break;
        // Branch Greater than or Equal to
        case BGEU:
            if(reg1_val >= reg2_val) *progCount = *progCount + imm - 1;
            val = *progCount;
            state = "BGEU";
            break;
        case JAL:
            *progCount = *progCount + imm - 1;
            val = *progCount;
            state = "JAL";
            break;
        case JALR:
            *progCount = *progCount + imm - 1;
            val = *progCount;
            state = "JALR";
            break;
        case LUI:
            val = reg2_val << 12;
            state = "LUI";
            break;
        default:
            val = *progCount;
            state = "????";
            break;
    }
    debugStr = "state: " + state + ", alu_val1: " + std::to_string(val);
    printDebug(debugStr, 0);
    return val;
}

float alu(float reg1_val, float reg2_val, int function_code){
    float val;
    std::string debugStr;
    std::string state;
    switch(function_code){
        // FADD.S
        case FADDS:
            val = reg1_val + reg2_val;
            state = "FADD";
            break;
        // FSUB.S
        case FSUBS:
            val = reg1_val - reg2_val;
            state = "FSUB";
            break;
        default:
            val = 0;
            break;
    }
    debugStr = "state: " + state + ", alu_val1: " + std::to_string(val);
    printDebug(debugStr, 0);
    return val;
}