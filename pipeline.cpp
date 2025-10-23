#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <random>
#include <bitset>
#include <cstdint>

#define FLD     0b0000111
#define FADD    0b1010011
#define FSD     0b0100111
#define ADDI    0b0010011
#define BNE     0b1100011

#define RTYPE   0b1010011
#define ITYPE   0b0010011
#define ITYPE3  0b0000011
#define ITYPE3  0b0000011
#define BTYPE   0b1100011
#define STYPE   0b0100111
#define UTYPE   0b0110111
#define UTYPE2  0b0010111
#define JTYPE   0b1101111

#define NOP     0b0000000
unsigned int instrQ[10];
uint32_t pc;
bool halted;
int debug;

struct riscvInstr{
    int funct3;
    int funct7;
    int rs1;
    int rs2;
    int imm;
    int rd;
    uint32_t opcode;
    int alucode;
    bool wb_enable;
    bool imm_sel;
    bool store_sel;
    bool mem_load_sel;
    bool rw_enable;
    int bit_len;
};

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
} pipelineSimulation;


bool pipelineSimulation::notStalled(){
    if(pipelineBusy) return false;
    else return true;
}

void pipelineSimulation::fetch(){
    uint32_t instructiondecode = instrQ[pc] & 0x7F;
    switch(instructiondecode){
        case FLD:
            state.fetchState = "FLD";
            instruction = instrQ[pc];
            pc++;
            break;
        case FSD:
            state.fetchState = "FSD";
            instruction = instrQ[pc];
            pc++;
            break;
        case FADD:
            state.fetchState = "FADD";
            instruction = instrQ[pc];
            pc++;
            break;
        case ADDI:
            state.fetchState = "ADDI";
            instruction = instrQ[pc];
            pc++;
            break;
        case BNE:
            state.fetchState = "BNE";
            instruction = instrQ[pc];
            pc = 0;
            break;
        case NOP:
            state.fetchState = "NO_OP";
            pc++;
            break;
        default:
            state.fetchState = "???";
            break;
    }    
}

void pipelineSimulation::decode(){
    assemblyCode.opcode = instruction & 0x7F;
    switch(assemblyCode.opcode){
        case RTYPE:
            assemblyCode.funct7  = (instruction & 0xFE000000) >> 25;
            assemblyCode.rs2     = (instruction & 0x01F00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            assemblyCode.alucode = (assemblyCode.funct7 << 3) + assemblyCode.funct3;
            assemblyCode.wb_enable = 1;
            assemblyCode.imm_sel = 0;
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.rw_enable = 0;
            state.decodeState    = "RTYPE";
            break;

        case ITYPE:         //ADDI stuff goes here
            assemblyCode.imm     = (instruction & 0xFFF00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            assemblyCode.funct7 = 0;

            if(assemblyCode.funct3 == 5){
                assemblyCode.funct7 = assemblyCode.imm & 0xFE0;
            }  
            assemblyCode.alucode = (assemblyCode.funct7 << 3) | assemblyCode.funct3;

            assemblyCode.wb_enable = 1;
            assemblyCode.imm_sel = 1;
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            state.decodeState    = "ITYPE";
            break;

        case ITYPE3: // I type, loads
            assemblyCode.imm     = (instruction & 0xFFF00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.rd      = (instruction & 0x00000F80) >> 7;
            assemblyCode.funct7 = 0;

            assemblyCode.alucode = 0; //add
            assemblyCode.wb_enable = 1;
            assemblyCode.imm_sel = 1;
            assemblyCode.store_sel = 1;
            assemblyCode.mem_load_sel = 1;
            assemblyCode.rw_enable = 1;
            state.decodeState    = "ITYPE"; 
            break;

        case STYPE:         //FSD stuff goes here
            assemblyCode.imm     = ((instruction & 0xFE000000) >> 25) + ((instruction & 0x00000F80) >> 7);
            assemblyCode.rs2     = (instruction & 0x01F00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.alucode = 0;
            assemblyCode.wb_enable = 0;
            assemblyCode.imm_sel = 1;
            assemblyCode.store_sel = 1;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.bit_len = 8 << assemblyCode.funct3;
            assemblyCode.rw_enable = 1;
            state.decodeState    = "STYPE";
            break;

        case BTYPE:         //BNE goes here
            assemblyCode.imm     = decodeBTypeImm(instruction);
            assemblyCode.rs2     = (instruction & 0x01F00000) >> 20;
            assemblyCode.rs1     = (instruction & 0x000F8000) >> 15;
            assemblyCode.funct3  = (instruction & 0x00007000) >> 12;
            assemblyCode.alucode = assemblyCode.funct3 << 3; //Need some way to keep it unique from other op codes            
            assemblyCode.imm_sel = 0;
            assemblyCode.wb_enable = 0;
            assemblyCode.store_sel = 0;
            assemblyCode.mem_load_sel = 0;
            assemblyCode.rw_enable = 0;
            state.decodeState    = "BNE";
            break;

        case JTYPE:
            state.decodeState    = "JTYPE";
            break;

        case UTYPE:
            state.decodeState    = "UTYPE";
            break;

        case UTYPE2:
            state.decodeState    = "UTYPE";
            break;

        case NOP:
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
    else {
        switch(assemblyCode.opcode){
            case FLD:

                //Stall time of 1 cycle
                //stallTime = 1;
                f0 = array[x1];
                state.executeState = "FLD";
                break;
            case FADD:
                //Stall time of 3 cycles
                stallTime = 2;
                f4 = f0 + f2;
                state.executeState = "FADD";
                break;
            case FSD:

                //Stall time of 2 cycles
                //stallTime = 2;
                array[x1] = f4;
                state.executeState = "FSD";
                break;
            case ADDI:
                x1 = x1 - 8;
                state.executeState = "ADDI";
                break;
            case BNE:
                if(x1 != x2){
                    //pc = assemblyCode.imm;
                }   
                else{
                    halt();
                }
                state.executeState = "BNE";
                break;
            default:
                break;
        
        }
    }
}

void pipelineSimulation::store(){
// No store stage actions needed for this simulation
    state.storeState = state.executeState;
}

void pipelineSimulation::halt(){
    //printf("Halting simulation\n");
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
    //printf("The time is: %f and this is fetch\n", pipelineSimulation.stallTime);
    pipelineSimulation.scheduleEvent(new decodeEvent(pipelineSimulation.clk + 1.3 + pipelineSimulation.stallTime));
}

void decodeEvent::processEvent(){
    pipelineSimulation.decode();
    //printf("The time is: %f and this is decode\n", pipelineSimulation.stallTime);
    pipelineSimulation.scheduleEvent(new executeEvent(pipelineSimulation.clk + 1.2 + pipelineSimulation.stallTime));
}

void executeEvent::processEvent(){
    pipelineSimulation.execute();
    //printf("The time is: %f and this is execute\n", time);
    pipelineSimulation.scheduleEvent(new storeEvent(pipelineSimulation.clk + 1.1));
}
void storeEvent::processEvent(){
    pipelineSimulation.store();
}

int main(){
    debug = 1;
    halted = 0;
    std::cout << "Pipeline Simulation, assignment 2\n";
    fill_queue("instructions.txt", instrQ, 9);
    for(int i = 0; i < 8; i++)
        std::cout << instrQ[i] << std::endl;
    pipelineSimulation.run();
    return 0;
}