#include "../sim/cpu.h"

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
    printDebug("Decoded B-type immediate: " + std::to_string(imm), 3);
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
    printDebug("Decoded JAL immediate: " + std::to_string(imm), 3);
    return imm;
}

// Sign extend 'value', whose actual bit-width is 'bits' (e.g. 12 for I-Type)
static inline int32_t signExtend(uint32_t value, int bits) {
    // If the sign bit is set...
    if (value & (1u << (bits - 1))) {
        // extend with 1s from bit 'bits' upward
        return value | (~((1u << bits) - 1));
    }
    return value;
}

void CpuSim::fetch(){
    bool loadSuccess;
     std::tie(instruction, loadSuccess) = simMemory.tryLoadWord(pc);
     if (!loadSuccess) {
         printDebug("Fetch failed: Memory bank is locked at PC " + std::to_string(pc), 1);
         // Handle stall or retry logic as needed
         return;
     }
     printDebug("Fetched instruction 0x" + std::to_string(instruction) + " at PC " + std::to_string(pc), 1);
    pc+=4;
}

void CpuSim::decode(){
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
            assemblyCode.float_regs = 0;
            printDebug("Decoded R-type instruction", 1);
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
            assemblyCode.float_regs = 2;
            printDebug("Decoded R-type floating point instruction", 1);
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
            assemblyCode.float_regs = 0;
            printDebug("Decoded I-type instruction", 1);
            break;

        case ITYPE2: // I type, loads, float
            state.decodeState    = "ITYPE-LF"; 
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
            assemblyCode.rw_enable = 0;
            assemblyCode.float_regs = 1;
            printDebug("Decoded I-type floating point instruction", 1);
            break;

        case ITYPE3: // I type, loads, int
            state.decodeState    = "ITYPE-L"; 
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
            assemblyCode.rw_enable = 0;
            assemblyCode.float_regs = 0;
            // Size control
            assemblyCode.bit_len = 8 << assemblyCode.funct3;
            printDebug("Decoded I-type load instruction with bit length: " + std::to_string(assemblyCode.bit_len), 1);
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
            assemblyCode.float_regs = 0;
            // Size control
            assemblyCode.bit_len = 8 << assemblyCode.funct3;
            printDebug("Decoded S-type store instruction", 1);
            break;

        case STYPE2:
            state.decodeState    = "STYPE-F";
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
            assemblyCode.float_regs = 1;
            printDebug("Decoded S-type floating point store instruction", 1);
            break;

        case BTYPE: // branching
            state.decodeState    = "BTYPE";
            // B-type instruction decoding
            assemblyCode.imm     = decodeBTypeImm(instruction);
            assemblyCode.rs2     = (instruction & 0x01F00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            // ALU control
            assemblyCode.alucode = (assemblyCode.opcode << 3) + assemblyCode.funct3;             
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 0;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 0;
            assemblyCode.rw_enable = 0;
            assemblyCode.float_regs = 0;
            printDebug("Decoded B-type instruction", 1);
            break;

        case JTYPE: // jal
            state.decodeState    = "JTYPE";
            // J-type instruction decoding
            assemblyCode.imm     = decodeJALImmediate(instruction);
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            assemblyCode.funct3  = 0;
            assemblyCode.funct7  = 0;
            // ALU control
            assemblyCode.alucode = (assemblyCode.opcode << 3) + assemblyCode.funct3;
            assemblyCode.pc_enable = 1;
            assemblyCode.imm_sel = 0;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            assemblyCode.float_regs = 0;
            printDebug("Decoded J-type instruction", 1);
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
            assemblyCode.alucode = (assemblyCode.opcode << 3) + assemblyCode.funct3;
            assemblyCode.pc_enable = 1;
            assemblyCode.imm_sel = 0;
            // Memory mux control
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            assemblyCode.float_regs = 0;
            printDebug("Decoded J-type instruction", 1);
            break;
            
        case UTYPE: // lui - load upper immediate
            state.decodeState   = "UTYPE";
            // U-type instruction decoding
            assemblyCode.imm    = (instruction & 0xFFFFF000) >> 12;
            assemblyCode.rd     = (instruction & 0x00000F80) >> 7;
            assemblyCode.rs1    = 0;
            assemblyCode.funct3 = 0;
            assemblyCode.funct7 = 0;
            // ALU control
            assemblyCode.alucode = (assemblyCode.opcode << 3) + assemblyCode.funct3;
            assemblyCode.pc_enable = 0;
            assemblyCode.imm_sel = 1;
            // Memory mux control
            assemblyCode.store_sel = 1;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.wb_enable = 1;
            assemblyCode.rw_enable = 0;
            assemblyCode.float_regs = 0;
            printDebug("Decoded U-type instruction", 1);
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
            assemblyCode.float_regs = 0;
            printDebug("Decoded U-type instruction", 1);
            break;

        case NOP: // no op
            state.decodeState    = "NO_OP";
            printDebug("Decoded NOP instruction", 1);
            break;

        default:
            state.decodeState    = "???";
            printDebug("ERR: Decoded unknown instruction with opcode: " + std::to_string(assemblyCode.opcode), 0);
            break;
    }
    if(assemblyCode.opcode != UTYPE) assemblyCode.imm = signExtend(assemblyCode.imm, 12);
}

void CpuSim::execute(){
    state.executeState = "NO_OP";
    if(pipelineBusy) {
        state.executeState = "STALL";
        printDebug("Pipeline is busy, stalling execute stage", 1);
    }
    float float_alu_val;
    float float_demux_store_line;
    float float_demux_wb_line;
    int int_alu_val;  
    int int_demux_store_line;
    int int_demux_wb_line;

    switch(assemblyCode.float_regs){
        case 0: // Case 0, entirely INT based operations
            printDebug("Executing INT based operation", 2);
            exeData.alu_val1 = int_reg_bank[assemblyCode.rs1];
            if(assemblyCode.imm_sel){
                printDebug("Using immediate value for ALU operand 2: " + std::to_string(assemblyCode.imm), 2);
                // May need to sign extend here. Not sure
                exeData.alu_val2 = assemblyCode.imm;
            }
            else {
                printDebug("Using register value for ALU operand 2: " + std::to_string(int_reg_bank[assemblyCode.rs2]), 2);
                exeData.alu_val2 = int_reg_bank[assemblyCode.rs2];
            }
            int_alu_val = alu(exeData.alu_val1, exeData.alu_val2, assemblyCode.alucode, pc, assemblyCode.imm);
            if(assemblyCode.store_sel){
                printDebug("Store select is enabled", 3);
                // Store stuff goes here
                int_demux_store_line = int_alu_val;
                if(assemblyCode.rw_enable){ // rw_enable high means store 
                    printDebug("Store enable is high, performing store operation", 2);
                    //STORE FUNCTION HERE
                    switch(assemblyCode.bit_len){
                        case Byte:
                            simMemory.tryStoreByte(int_alu_val, uint8_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                            break;
                        case HalfWord:
                            simMemory.tryStoreHalfWord(int_alu_val, uint16_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                            break;
                        case Word:
                            simMemory.tryStoreWord(int_alu_val, uint32_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                            break;
                        default:
                            break;
                    }
                }
                else{
                    //LOAD FUNCTION HERE
                    printDebug("Store enable is low, performing load operation", 2);
                    bool load_success;
                    unsigned int data;
                    switch(assemblyCode.bit_len){
                        case Byte:
                            std::tie(data, load_success) = simMemory.tryLoadByte(int_alu_val);
                            exeData.wb_int_val = data;
                            break;
                        case HalfWord:
                            std::tie(data, load_success) = simMemory.tryLoadHalfWord(int_alu_val);
                            exeData.wb_int_val = data;
                            break;
                        case Word:
                            std::tie(data, load_success) = simMemory.tryLoadWord(int_alu_val);
                            exeData.wb_int_val = data;
                            break;
                        default:
                            break;
                    }
                }
            }
            else{
                printDebug("Store select is disabled, writing ALU result to write-back integer value", 3);
                exeData.wb_int_val = int_alu_val;
            }
            if(assemblyCode.wb_enable){
                printDebug("Write-back is enabled, preparing to write to register " + std::to_string(assemblyCode.rd), 3);
                exeData.wb = 1;
                exeData.wb_reg = assemblyCode.rd;
            }
            break;
        case 1: // Case 1, float value in Reg2 we need to store or load somewhere
            printDebug("Executing FLOAT based operation", 2);
            exeData.alu_val1 = int_reg_bank[assemblyCode.rs1];
            exeData.alu_val2 = assemblyCode.imm;
            int_alu_val = alu(exeData.alu_val1, exeData.alu_val2, assemblyCode.alucode, pc, assemblyCode.imm);
            if(assemblyCode.store_sel){
                printDebug("Store select is enabled for FLOAT operation", 3);
                // Store stuff goes here
                int_demux_store_line = int_alu_val;
                if(assemblyCode.rw_enable){ // rw_enable high means store 
                    printDebug("Store enable is high, performing FLOAT store operation", 2);
                    //STORE FUNCTION HERE
                    simMemory.tryStoreWord(int_alu_val, float(float_reg_bank[assemblyCode.rs2]));
                }
                else{
                    printDebug("Store enable is low, performing FLOAT load operation", 2);
                    //LOAD FUNCTION HERE
                    unsigned int data;
                    bool load_success;
                    std::tie(data, load_success) = simMemory.tryLoadWord(int_alu_val);
                    exeData.wb_int_val = data;
                }
            }
            else{
                printDebug("Store select is disabled, writing ALU result to write-back integer value for FLOAT operation", 3);
                exeData.wb_int_val = int_alu_val;
            }
            if(assemblyCode.wb_enable){
                printDebug("Write-back is enabled, preparing to write to register " + std::to_string(assemblyCode.rd), 3);
                exeData.wb = 1;
                exeData.wb_reg = assemblyCode.rd;
            }
            break;
        case 2: // Case 2, float ALU math
            printDebug("Executing FLOAT ALU operation", 2);
            if(assemblyCode.imm_sel){
                printDebug("Using immediate value for ALU operand 2: " + std::to_string(assemblyCode.imm), 2);
            // May need to sign extend here. Not sure
                exeData.alu_val2 = assemblyCode.imm;
            }
            else{
                printDebug("Using register value for ALU operand 2: " + std::to_string(float_reg_bank[assemblyCode.rs2]), 2);
                exeData.alu_val2 = float_reg_bank[assemblyCode.rs2];
            }
            float_alu_val = alu(float_reg_bank[assemblyCode.rs1], exeData.alu_val2, assemblyCode.alucode);
            // No need for STORE or LOAD, this option should only be reached for Float ALU ops
            exeData.wb_float_val = float_alu_val;
            if(assemblyCode.wb_enable){
                printDebug("Write-back is enabled, preparing to write to register " + std::to_string(assemblyCode.rd), 3);
                exeData.wb = 2;
                exeData.wb_reg = assemblyCode.rd;
            }
            break;
        default:
            printDebug("ERR: Default case reached in execute stage", 0);
            break;
    }
}

void CpuSim::store(){
// No store stage actions needed for this simulation
    state.storeState = state.executeState;
    printDebug("Store stage processing write-back for register " + std::to_string(exeData.wb_reg), 1);
    switch(exeData.wb){
        case 1:
            if(exeData.wb_reg == 0){ // not allowed to write things to register 0, its forever 0
                printDebug("Attempted to write to register 0, operation ignored", 1);
            }
            else{
                printDebug("Writing value " + std::to_string(exeData.wb_int_val) + " to integer register " + std::to_string(exeData.wb_reg), 2);
                int_reg_bank[exeData.wb_reg] = exeData.wb_int_val;
            }
            exeData.wb = 0;
            break;
        case 2:
            printDebug("Writing value " + std::to_string(exeData.wb_float_val) + " to float register " + std::to_string(exeData.wb_reg), 2);
            float_reg_bank[exeData.wb_reg] = exeData.wb_float_val;
            exeData.wb = 0;
            break;
        default:
            printDebug("No write back required for instruction", 0);
            break;
    }
        
}