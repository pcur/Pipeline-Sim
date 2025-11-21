#include "../sim/alu.h"

int CpuSim::alu(int reg1_val, int reg2_val, int function_code, int imm){
    int val;
    std::stringstream temp_ss;
    temp_ss << std::hex << std::showbase << instr_execute_pc;
    std::string debugStr;
    switch(function_code){
        // Arithmetic ALU ops
        // Add
        case ADD:
            val = reg1_val + reg2_val;
            state.executeState = "ADD";
            break;
        // Subtract
        case SUB:
            val = reg1_val - reg2_val;  
            state.executeState = "SUB";
            break;
        // Shift Left Logical
        case SLL:
            val = reg1_val << reg2_val;
            state.executeState = "SLL";
            break;
        // Set Less Than
        case SLT:
            val = (reg1_val < reg2_val) ? 1 : 0;
            state.executeState = "SLT";
            break;
        // Set Less Than Unsigned
        case SLTU: (uint32_t(reg1_val) < uint32_t(reg2_val)) ? 1 : 0;
            state.executeState = "SLTU";
            break;
        // XOR
        case XOR:
            val = reg1_val ^ reg2_val;
            state.executeState = "XOR";
            break;
        // Shift Right Logical
        case SRL: 
            val = reg1_val >> reg2_val;
            state.executeState = "SRL";
            break;
        // Shift Right Arthmetic. NOT CORRECT
        case SRA:
            val = reg1_val >> reg2_val;
            state.executeState = "SRA";
            break;
        // Or
        case OR:
            val = reg1_val | reg2_val;
            state.executeState = "OR";
            break;
        // AND
        case AND:
            val = reg1_val & reg2_val;
            state.executeState = "AND";
            break;
        // Branching ALU ops
        // Branch Equal to
        case BEQ:
            if(reg1_val == reg2_val){
                 pc = instr_execute_pc + imm + FETCH_OFFSET; // -1 Because we add it already in fetch
                 flush();
            }
            val = pc;
            state.executeState = "BEQ";
            break;
        // Branch Not Equal to
        case BNE:
            if(reg1_val != reg2_val){
                pc = instr_execute_pc + imm + FETCH_OFFSET;
                flush();
            }
            val = pc;
            state.executeState = "BNE";
            break;
        // Branch less than
        case BLT:
            printDebug("EXECUTE - " + temp_ss.str() + ": " + "xs1: " + std::to_string(reg1_val) + ", xs2 " + std::to_string(reg2_val),1);
            if(reg1_val < reg2_val){
                pc = instr_execute_pc + imm + FETCH_OFFSET;
                flush();
            }
            
            val = pc;
            state.executeState = "BLT";
            break;
        // Branch greater than or equal
        case BGE:
            if(reg1_val > reg2_val){
                pc = instr_execute_pc + imm + FETCH_OFFSET;
                flush();
            }
            val = pc;
            state.executeState = "BGE";
            break;
        // Branch Less than or Equal to (unsigned)
        case BLTU:
            if(reg1_val <= reg2_val){
                pc = instr_execute_pc + imm + FETCH_OFFSET;
                flush();
            }
            val = pc;
            state.executeState = "BLTU";
            break;
        // Branch Greater than or Equal to (unsigned)
        case BGEU:
            if(reg1_val >= reg2_val){
                pc = instr_execute_pc + imm + FETCH_OFFSET;
                flush();
            }
            val = pc;
            state.executeState = "BGEU";
            break;
        case JAL:
            pc = instr_execute_pc + imm + FETCH_OFFSET;
            flush();
            val = pc;
            state.executeState = "JAL";
            break;
        case JALR:
            pc = instr_execute_pc + imm;
            flush();
            val = pc;
            state.executeState = "JALR";
            break;
        case LUI:
            val = reg2_val << 12;
            state.executeState = "LUI";
            break;
        default:
            val = instr_execute_pc;
            state.executeState = "????";
            break;
    }
    debugStr = "state: " + state.executeState + ", alu_val1: " + std::to_string(val);
    printDebug("EXECUTE - " + temp_ss.str() + ": " + debugStr, 1);
    return val;
}

float CpuSim::alu(float reg1_val, float reg2_val, int function_code){
    float val;
    std::string debugStr;
    switch(function_code){
        // FADD.S
        case FADDS:
            val = reg1_val + reg2_val;
            state.executeState = "FADD";
            break;
        // FSUB.S
        case FSUBS:
            val = reg1_val - reg2_val;
            state.executeState = "FSUB";
            break;
        default:
            val = 0;
            break;
    }
    debugStr = "state: " + state.executeState + ", alu_val1: " + std::to_string(val);
    printDebug(debugStr, 1);
    return val;
}