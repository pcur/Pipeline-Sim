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
    CpuSim cpu1 = CpuSim(memBus,0,0x02FF);

    // Load instruction queue from file and into memory
    printDebug("Setting up instruction queue", 1);
    fill_queue("../instructions/instructions.txt", instrQ, 10);
    load_mem_array(memBus, 0x0000, 0x0027, instrQ);

    // Initialize pipeline simulations
    printDebug("Creating pipeline simulations", 1);
    pipelineSimulation pipeline1 = pipelineSimulation(&cpu1, "CPU1");

    //begin cpu simulation, driving clock externally
    printDebug("Starting CPU simulation loop", 0);
    printDebug("============================================================", 0);
    pipeline1.start();
    int loop = 0;
    while(!pipeline1.halted){
        if(!pipeline1.halted) pipeline1.tick();
        memBus.tick(); //advance memory bus arbitration
        loop++;
        if(loop > 51){ //safety break
            printDebug("ERROR: Simulation loop exceeded 1000 cycles, forcing halt.", 0);
            pipeline1.halt();
        }
    }
    printDebug("CPU simulation complete", 0);
    return 0;
}