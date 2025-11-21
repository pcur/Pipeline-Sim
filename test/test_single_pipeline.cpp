#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/helpers.h"

int debug;
bool halted;
unsigned int instrQ[38];

int main(){
    debug = 1;
    printDebug("Setting up Pipeline test...", 0);
    printDebug("Initializing MemoryBus", 1);
    MemoryBus memBus = MemoryBus(0x01FF, 0x03FF, 0x13FF);
    printDebug("Creating CpuSim instance", 1);
    CpuSim cpu1 = CpuSim(memBus,0,0x0300);

    // Load instruction queue from file and into memory
    printDebug("Setting up instruction queue", 1);
    fill_queue("instructions/cpu0_instructions.txt", instrQ, 38);
    load_mem_array(memBus, 0x0000, 0x0093, instrQ);

    //Filling memory array
    initialize_mem_array(memBus, 0x400, 0x7FF, 123);
    initialize_mem_array(memBus, 0x800, 0xBFF, 456);

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
        if(loop > 2000){ //safety break
            printDebug("ERROR: Simulation loop exceeded 1000 cycles, forcing halt.", 0);
            pipeline1.halt();
        }
    }
    printDebug("CPU simulation complete", 0);
    return 0;
}