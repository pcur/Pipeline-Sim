#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <random>
#include <bitset>
#include <cstdint>
#include "../sim/alu.h"
#include "../sim/memory.h"


/*
#define FLD     0b0000111
#define FADD    0b1010011
#define FSD     0b0100111
#define ADDI    0b0010011
#define BNE     0b1100011
*/

#define RTYPE   0b0110011
#define RTYPE2  0b1010011
#define ITYPE   0b0010011
#define ITYPE2  0b0000111
#define ITYPE3  0b0000011
#define BTYPE   0b1100011
#define STYPE   0b0100011
#define STYPE2  0b0100111
#define UTYPE   0b0110111
#define UTYPE2  0b0010111
#define JTYPE   0b1101111
#define JTYPE2  0b1100111

#define NOP     0b0000000

#define Byte        8
#define HalfWord    16
#define Word        32

uint32_t int_reg_bank[32];
float   float_reg_bank[32];

extern unsigned int instrQ[10];
extern uint32_t pc;
extern bool halted;
extern int debug;

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
    int float_regs;
};

struct executeData{
    int alu_val1;
    int alu_val2;
    float alu_float1;
    float alu_float2;
    int wb_int_val;
    float wb_float_val;
    uint32_t wb_reg;
    uint8_t wb;
};

Memory simMemory;

struct pipelineState{
    std::string fetchState;
    std::string decodeState;
    std::string executeState;
    std::string storeState;
};

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

class event {
  public:
      // Construct sets time of event.
    event (float t, std::string n) : time (t), name(n) { }
    
      // Execute event by invoking this method.
    virtual void processEvent () = 0;
    
    const float time;
    std::string name;
};

struct eventComparator {
  bool operator() (const event * left, const event * right) const {
    return left->time > right->time;
  }
};

class simulation {
  public:
    simulation () : clk (0.0), eventQueue () { }
    void run ();
    void scheduleEvent (event * newEvent) {
      eventQueue.push(newEvent);
    }
    float clk;
    pipelineState state;
  protected:
    std::priority_queue<event*, std::vector<event *, std::allocator<event*> >, eventComparator> eventQueue;
};

class pipelineSimulation : public simulation {
    public:
        pipelineSimulation() : simulation(), threads(1), pipelineBusy(0), scalar(0), x1(160), x2(0), f2(1) {}
        bool notStalled();
        void fetch();
        void decode();
        void execute();
        void store();
        void halt();

        unsigned int    threads;
        uint32_t        instruction;
        uint32_t        jumpval;
        uint16_t        f0;
        uint16_t        f2;
        uint16_t        f4;
        uint16_t        x1;
        uint16_t        x2;
        uint32_t        stallTime;
        bool            pipelineBusy;
        float           scalar;
        unsigned int    array[160];
        riscvInstr      assemblyCode;
        executeData     exeData;
} pipelineSimulation;


bool pipelineSimulation::notStalled(){
    if(pipelineBusy) return false;
    else return true;
}

void pipelineSimulation::fetch(){
    instruction = instrQ[pc];
    pc++;
}

void pipelineSimulation::decode(){
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
            assemblyCode.float_regs = 0;
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
            assemblyCode.float_regs = 0;
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
            assemblyCode.float_regs = 0;
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
            break;

        case NOP: // no op
            state.decodeState    = "NO_OP";
            break;

        default:
            state.decodeState    = "???";
            break;

    }
}

void pipelineSimulation::execute(){
    state.executeState = "NO_OP";
    if(pipelineBusy) {
        state.executeState = "STALL";
    }
    float float_alu_val;
    float float_demux_store_line;
    float float_demux_wb_line;
    int int_alu_val;  
    int int_demux_store_line;
    int int_demux_wb_line;

    switch(assemblyCode.float_regs){
        case 0: // Case 0, entirely INT based operations
            exeData.alu_val1 = int_reg_bank[assemblyCode.rs1];
            if(assemblyCode.imm_sel)
                // May need to sign extend here. Not sure
                exeData.alu_val2 = assemblyCode.imm;
            else
                exeData.alu_val2 = int_reg_bank[assemblyCode.rs2];
            int_alu_val = alu(exeData.alu_val1, exeData.alu_val2, assemblyCode.alucode, pc, assemblyCode.imm);
            if(assemblyCode.store_sel){
                // Store stuff goes here
                int_demux_store_line = int_alu_val;
                if(assemblyCode.rw_enable){ // rw_enable high means store 
                    //STORE FUNCTION HERE
                    switch(assemblyCode.bit_len){
                        case Byte:
                            simMemory.storeByte(int_alu_val, uint8_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                            break;
                        case HalfWord:
                            simMemory.storeHalfWord(int_alu_val, uint16_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                            break;
                        case Word:
                            simMemory.storeWord(int_alu_val, uint32_t(int_reg_bank[assemblyCode.rs2] << (32 - assemblyCode.bit_len)) >> (32 - assemblyCode.bit_len));
                            break;
                        default:
                            break;
                    }
                }
                else{
                    //LOAD FUNCTION HERE
                    switch(assemblyCode.bit_len){
                        case Byte:
                            exeData.wb_int_val = simMemory.loadByte(int_alu_val);
                            break;
                        case HalfWord:
                            exeData.wb_int_val = simMemory.loadHalfWord(int_alu_val);
                            break;
                        case Word:
                            exeData.wb_int_val = simMemory.loadWord(int_alu_val);
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
            break;
        case 1: // Case 1, float value in Reg2 we need to store or load somewhere
            exeData.alu_val1 = int_reg_bank[assemblyCode.rs1];
            exeData.alu_val2 = assemblyCode.imm;
            int_alu_val = alu(exeData.alu_val1, exeData.alu_val2, assemblyCode.alucode, pc, assemblyCode.imm);
            if(assemblyCode.store_sel){
                // Store stuff goes here
                int_demux_store_line = int_alu_val;
                if(assemblyCode.rw_enable){ // rw_enable high means store 
                    //STORE FUNCTION HERE
                    simMemory.storeWord(int_alu_val, float(float_reg_bank[assemblyCode.rs2]));
                }
                else{
                    //LOAD FUNCTION HERE
                    exeData.wb_int_val = simMemory.loadWord(int_alu_val);
                }
            }
            else{
                exeData.wb_int_val = int_alu_val;
            }
            if(assemblyCode.wb_enable){
                exeData.wb = 1;
                exeData.wb_reg = assemblyCode.rd;
            }
            break;
        case 2: // Case 2, float ALU math
            if(assemblyCode.imm_sel)
            // May need to sign extend here. Not sure
                exeData.alu_val2 = assemblyCode.imm;
            else
                exeData.alu_val2 = float_reg_bank[assemblyCode.rs2];
            float_alu_val = alu(float_reg_bank[assemblyCode.rs1], exeData.alu_val2, assemblyCode.alucode);
            // No need for STORE or LOAD, this option should only be reached for Float ALU ops
            exeData.wb_float_val = float_alu_val;
            if(assemblyCode.wb_enable){
                exeData.wb = 2;
                exeData.wb_reg = assemblyCode.rd;
            }
            break;
        default:
            break;
    }
}

void pipelineSimulation::store(){
// No store stage actions needed for this simulation
    state.storeState = state.executeState;
    switch(exeData.wb){
        case 1:
            int_reg_bank[exeData.wb_reg] = exeData.wb_int_val;
            exeData.wb = 0;
            break;
        case 2:
            float_reg_bank[exeData.wb_reg] = exeData.wb_float_val;
            exeData.wb = 0;
            break;
        default:
            break;
    }
        
}

void pipelineSimulation::halt(){
    while(!eventQueue.empty()){
        event * nextEvent = eventQueue.top();
        eventQueue.pop();
        delete nextEvent;
    }
    pc = 5;
    halted = true;
}

class fetchEvent : public event {
    public:
        fetchEvent(float t) : event(t, "fetch"){}
        virtual void processEvent();
};

class decodeEvent : public event {
    public:
        decodeEvent(float t) : event(t, "decode"){}
        virtual void processEvent();
};

class executeEvent : public event {
    public:
        executeEvent(float t) : event(t, "execute"){}
        virtual void processEvent();
};

class storeEvent : public event {
    public:
        storeEvent(float t) : event(t, "store"){}
        virtual void processEvent();
};

void simulation::run(){
    pipelineSimulation.scheduleEvent(new fetchEvent(clk));

    while(!eventQueue.empty()){ //clock cycle loop
        while(eventQueue.top()->time <= clk + 1) {
            event * nextEvent = eventQueue.top();
            nextEvent->processEvent();
            if(halted) break;
            eventQueue.pop();
            delete nextEvent;
        }
        if(debug == 1){
            printf("Clock Cycle: %04.1f", clk);
            std::cout << " Fetch: " << state.fetchState;
            std::cout << " Decode: " << state.decodeState << " Execute: " << state.executeState;
            std::cout << " Store: " << state.storeState << " Value in x1: " << pipelineSimulation.x1 << std::endl;
        }
        if(debug == 2){
            std::priority_queue<event*, std::vector<event *, std::allocator<event*> >, eventComparator> queueCopy = eventQueue;
            while(!queueCopy.empty()){
                event* ev = queueCopy.top();
                std::cout << "Event: " << ev->name << ", time: " << ev->time << std::endl;
                queueCopy.pop();
            }
        }
        if(clk == 4) halted = 1;
        if(halted){
            state.fetchState = "HALTED";
            state.decodeState = "HALTED";
            state.executeState = "HALTED";
            state.storeState = "HALTED";
            printf("Halting simulation\n");
            printf("Clock Cycle: %04.1f", clk);
            std::cout << " Fetch: " << state.fetchState;
            std::cout << " Decode: " << state.decodeState << " Execute: " << state.executeState;
            std::cout << " Store: " << state.storeState << " Value in x1: " << pipelineSimulation.x1 << std::endl;
            return;
        }
        else{
            if(pipelineSimulation.stallTime > 0) pipelineSimulation.pipelineBusy = 1;
            if(pipelineSimulation.notStalled()){
                pipelineSimulation.scheduleEvent(new fetchEvent(clk + 1.4));
            }
            else { 
                pipelineSimulation.stallTime--;
                state.executeState = "STALL";
                //pipelineSimulation.scheduleEvent(new executeEvent(clk + 1.1));
                if(pipelineSimulation.stallTime <= 0) pipelineSimulation.pipelineBusy = 0;
            }
            clk++;
        }
    }
}

void fetchEvent::processEvent(){
    pipelineSimulation.fetch();
    pipelineSimulation.scheduleEvent(new decodeEvent(pipelineSimulation.clk + 1.3 + pipelineSimulation.stallTime));
}

void decodeEvent::processEvent(){
    pipelineSimulation.decode();
    pipelineSimulation.scheduleEvent(new executeEvent(pipelineSimulation.clk + 1.2 + pipelineSimulation.stallTime));
}

void executeEvent::processEvent(){
    pipelineSimulation.execute();
    pipelineSimulation.scheduleEvent(new storeEvent(pipelineSimulation.clk + 1.1));
}
void storeEvent::processEvent(){
    pipelineSimulation.store();
}
