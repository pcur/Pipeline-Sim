#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/helpers.h"

int debug;
bool halted;
unsigned int instrQ[10];

int main(){
    debug = 1;
    printDebug("Setting up Pipeline test...", 0);
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
    cpu1.run();

    return 0;
}
