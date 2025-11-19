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

extern unsigned int instrQ[10];
uint32_t pc;
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

struct pipelineState{
    std::string fetchState;
    std::string decodeState;
    std::string executeState;
    std::string storeState;
};

class pipelineSimulation; //forward declaration

class event {
  public:
      // Construct sets time of event.
    event (float t, std::string n, pipelineSimulation* cI) : time (t), name(n), cpuInstance(cI) { }

      // Execute event by invoking this method.
    virtual void processEvent () = 0;
    pipelineSimulation* cpuInstance;
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
        void run ();

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
};

class fetchEvent : public event {
    public:
        fetchEvent(float t, pipelineSimulation* cpuInstance) : event(t, "fetch", cpuInstance){}
        void processEvent();
};

class decodeEvent : public event {
    public:
        decodeEvent(float t, pipelineSimulation* cpuInstance) : event(t, "decode", cpuInstance){}
        void processEvent();
};

class executeEvent : public event {
    public:
        executeEvent(float t, pipelineSimulation* cpuInstance) : event(t, "execute", cpuInstance){}
        void processEvent();
};

class storeEvent : public event {
    public:
        storeEvent(float t, pipelineSimulation* cpuInstance) : event(t, "store", cpuInstance){}
        void processEvent();
};
