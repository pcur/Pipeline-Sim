
#define DEBUG 3
#include "../sim/cpu.h"

int debug;
bool halted;
unsigned int instrQ[10];

int main() {
    printDebug("Setting up CPU test...", 0);
    printDebug("Initializing MemoryBus", 1);
    MemoryBus memBus = MemoryBus(0x00FF, 0x01FF, 0x13FF);
    printDebug("Creating CpuSim instance", 1);
    CpuSim cpu1 = CpuSim(memBus);

    printDebug("Setting up instruction queue", 0);
    fill_queue("instructions.txt", instrQ, 10);
    load_mem_array(memBus, 0x0000, 0x0027, instrQ);

     // Load instructions into memory
    printDebug("Starting CPU simulation loop", 0);
    printDebug("============================================================", 0);
    cpu1.pc = 0;
    halted = false;

    printDebug("FETCH STAGE:", 0);
    cpu1.fetch();
    std::string printStr = "state: " + cpu1.state.fetchState + ", instruction: " + std::to_string(cpu1.instruction);
    printDebug(printStr, 0);

    printDebug("\nDECODE STAGE:", 0);
    cpu1.decode();
    printStr = "state: " + cpu1.state.decodeState + ", opcode: " + std::to_string(cpu1.assemblyCode.opcode);
    printDebug(printStr, 0);

    printDebug("\nEXECUTE STAGE:", 0);
    cpu1.execute();
    printStr = "state: " + cpu1.state.executeState + ", alu_val1: " + std::to_string(cpu1.exeData.alu_val1);
    //printDebug(printStr, 0);

    printDebug("\nSTORE STAGE:", 0);
    cpu1.store();
    printStr = "state: " + cpu1.state.storeState;
    printDebug(printStr, 0);

    return 0;
}