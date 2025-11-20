#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/helpers.h"

int debug;

int main(){
    debug = 1;
    unsigned int instrQ[10];
    printDebug("Setting up Pipeline test...", 0);
    printDebug("Initializing MemoryBus", 1);
    MemoryBus memBus = MemoryBus(0x00FF, 0x01FF, 0x13FF);
    printDebug("Creating CpuSim instances", 1);
    CpuSim cpu1 = CpuSim(memBus,0x000);
    CpuSim cpu2 = CpuSim(memBus,0x100);

    // Load instruction queue from file and into memory
    printDebug("Setting up instruction queue", 1);
    fill_queue("instructions/cpu0_instructions.txt", instrQ, 38);
    load_mem_array(memBus, 0x0000, 0x0093, instrQ);
    fill_queue("instructions/cpu1_instructions.txt", instrQ, 38);
    load_mem_array(memBus, 0x0100, 0x0193, instrQ);

        //Populate data vectors in memory with random FP32 values
    //0x400-0x7FF (ARRAY_A)
    initialize_mem_array(memBus, 0x400, 0x7FF, 123);
    //0x800-0xBFF (ARRAY_B)
    initialize_mem_array(memBus, 0x800, 0xBFF, 456);
    //0xC00-0xFFF (ARRAY_C)
    initialize_mem_array(memBus, 0xC00, 0xFFF, 789);
    //0x1000-0x13FF (ARRAY_D)
    initialize_mem_array(memBus, 0x1000, 0x13FF, 987);
    
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
    while(!pipeline1.halted || !pipeline2.halted){
        if(!pipeline1.halted) pipeline1.tick();
        if(!pipeline2.halted) pipeline2.tick();
        memBus.tick(); //advance memory bus arbitration
        loop++;
    }
    printDebug("CPU simulation complete", 0);
    return 0;
}
