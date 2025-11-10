uint32_t int_reg_bank[32];
float   float_reg_bank[32];

void pipeline_execute(void){
    exeData.alu_val1 = assemblyCode.rs1;
    if(assemblyCode.imm_sel)
        exeData.alu_val2 = assemblyCode.imm;
    else
        exeData.alu_val2 = assemblyCode.rs2;

    alu(exeData.alu_val1, exeData.alu_val2,)
}