#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/helpers.h"

int debug;

int main(){
    debug = 1;
    unsigned int instrQ0[38];
    unsigned int instrQ1[38];
    printDebug("Setting up Pipeline test...", 0);
    printDebug("Initializing MemoryBus", 1);
    MemoryBus memBus = MemoryBus(0x00FF, 0x01FF, 0x13FF);
    printDebug("Creating CpuSim instances", 1);
    CpuSim cpu1 = CpuSim(memBus,0x000,0x2FF);
    CpuSim cpu2 = CpuSim(memBus,0x100,0x3FF);

    // Load instruction queue from file and into memory
    printDebug("Setting up instruction queue", 1);
    fill_queue("instructions/cpu0_instructions.txt", instrQ0, 38);
    load_mem_array(memBus, 0x0000, 0x0093, instrQ0);
    fill_queue("instructions/cpu1_instructions.txt", instrQ1, 38);
    load_mem_array(memBus, 0x0100, 0x0193, instrQ1);

    //Populate data vectors in memory with random FP32 values
    //0x400-0x7FF (ARRAY_A)
    initialize_mem_array(memBus, 0x400, 0x7FF, 123);
    //0x800-0xBFF (ARRAY_B)
    initialize_mem_array(memBus, 0x800, 0xBFF, 456);

    // Initialize pipeline simulations
    printDebug("Creating pipeline simulations", 1);
    pipelineSimulation pipeline1 = pipelineSimulation(&cpu1, "CPU1");
    pipelineSimulation pipeline2 = pipelineSimulation(&cpu2, "CPU2");

    //begin cpu simulation, driving clock externally
    printDebug("Starting CPU simulation loop", 0);
    printDebug("============================================================", 0);
    pipeline1.start();
    pipeline2.start();
    unsigned int tick0 = 0;
    unsigned int tick1 = 0;
    while(!pipeline1.halted || !pipeline2.halted){
        if(!pipeline1.halted) {
            pipeline1.tick();
            tick0++;
        }
        if(!pipeline2.halted) {
            pipeline2.tick();
            tick1++;
        }
        memBus.tick(); //advance memory bus arbitration
    }
    unsigned int cycles0 = tick0 / 10;
    unsigned int cycles1 = tick1 / 10;
    unsigned int instructionCt0 = sizeof(instrQ0) / sizeof(instrQ0[0]);
    unsigned int instructionCt1 = sizeof(instrQ1) / sizeof(instrQ1[0]);
    float calculated_cpi0 = cycles0 / instructionCt0;
    float calculated_cpi1 = cycles1 / instructionCt1;
    printDebug("CPU simulation complete.", 0);
    printDebug("CPU0 CPI: " + std::to_string(calculated_cpi0), 0);
    printDebug("CPU1 CPI: " + std::to_string(calculated_cpi1), 0);
    
    return 0;
}
