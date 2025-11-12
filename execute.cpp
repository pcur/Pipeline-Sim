#define Byte        8
#define HalfWord    16
#define Word        32

uint32_t int_reg_bank[32];
float   float_reg_bank[32];

void pipeline_execute(void){
    float float_alu_val;
    float float_demux_store_line;
    float float_demux_wb_line;
    int int_alu_val;  
    int int_demux_store_line;
    int int_demux_wb_line;

    if(assemblyCode.float_sel){
        exeData.alu_val1 = float_reg_bank[assemblyCode.rs1];
        if(assemblyCode.imm_sel)
            // May need to sign extend here. Not sure
            exeData.alu_val2 = assemblyCode.imm;
        else
            exeData.alu_val2 = float_reg_bank[assemblyCode.rs2];
        float_alu_val = alu(exeData.alu_val1, exeData.alu_val2, assemblyCode.alucode);
        if(assemblyCode.store_sel){
            // Store stuff goes here
            float_demux_store_line = int_alu_val;
            if(assemblyCode.rw_enable){ // rw_enable high means store 
                //STORE FUNCTION HERE
                storeFloatWord(); // TODO: Fill this out
            }
            else{
                //LOAD FUNCTION HERE
                exeData.wb_float_val = loadFloatWord(); // TODO: Fill this out
            }
        }
        else{
            exeData.wb_float_val = float_alu_val;
        }
        if(assemblyCode.wb_enable){
            exeData.wb = 2;
            exeData.wb_reg = assemblyCode.rd;
        }
    }
    else{
        exeData.alu_val = int_reg_bank[assemblyCode.rs1];
        if(assemblyCode.imm_sel)
            // May need to sign extend here. Not sure
            exeData.alu_val2 = assemblyCode.imm;
        else
            exeData.alu_val2 = int_reg_bank[assemblyCode.rs2];
        int_alu_val = alu(exeData.alu_val1, exeData.alu_val2, assemblyCode.alucode);
        if(assemblyCode.store_sel){
            // Store stuff goes here
            int_demux_store_line = int_alu_val;
            if(assemblyCode.rw_enable){ // rw_enable high means store 
                //STORE FUNCTION HERE
                switch(assemblyCode.bit_len){
                    case Byte:
                        storeByte(int_alu_val, uint8_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                        break;
                    case HalfWord:
                        storeHalfWord(int_alu_val, uint16_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                        break;
                    case Word:
                        storeWord(int_alu_val, uint32_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                        break;
                    default:
                        break;
                }
            }
            else{
                //LOAD FUNCTION HERE
                switch(assemblyCode.bit_len){
                    case Byte:
                        exeData.wb_int_val = loadByte(int_alu_val);
                        break;
                    case HalfWord:
                        exeData.wb_int_val = loadHalfWord(int_alu_val);
                        break;
                    case Word:
                        exeData.wb_int_val = loadWord(int_alu_val);
                        break;
                    default:
                        break;
                }
            }
        }
        else{
            exeData.wb_int_val = int_alu_val;
        }
        if(assemblyCode.wb_enable){
            exeData.wb = 1;
            exeData.wb_reg = assemblyCode.rd;
        }
    }

}