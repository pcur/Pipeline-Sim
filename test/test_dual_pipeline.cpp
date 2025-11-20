#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/helpers.h"

int debug;
unsigned int instrQ[10];

int main(){
    debug = 1;
    printDebug("Setting up Pipeline test...", 0);
    printDebug("Initializing MemoryBus", 1);
    MemoryBus memBus = MemoryBus(0x00FF, 0x01FF, 0x13FF);
    printDebug("Creating CpuSim instances", 1);
    CpuSim cpu1 = CpuSim(memBus,0x000);
    CpuSim cpu2 = CpuSim(memBus,0x100);

    // Load instruction queue from file and into memory
    printDebug("Setting up instruction queue", 1);
    fill_queue("instructions.txt", instrQ, 10);
    load_mem_array(memBus, 0x0000, 0x0027, instrQ);

    // Initialize pipeline simulations
    printDebug("Creating pipeline simulations", 1);
    pipelineSimulation pipeline1 = pipelineSimulation(&cpu1, "CPU1");
    pipelineSimulation pipeline2 = pipelineSimulation(&cpu2, "CPU2");

    //begin cpu simulation, driving clock externally
    printDebug("Starting CPU simulation loop", 0);
    printDebug("============================================================", 0);
    pipeline1.start();
    pipeline2.start();
    int loop = 0;
    while(!pipeline1.halted && !pipeline2.halted){
        if(!pipeline1.halted) pipeline1.tick();
        if(!pipeline2.halted) pipeline2.tick();
        memBus.tick(); //advance memory bus arbitration
        loop++;
        if(loop > 21){ //safety break
            printDebug("ERROR: Simulation loop exceeded 1000 cycles, forcing halt.", 0);
            break;
        }
    }
    printDebug("CPU simulation complete", 0);
    return 0;
}
