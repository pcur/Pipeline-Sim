#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/helpers.h"

int debug;

int main(){
    debug = 1;
    uint32_t instrQ0[38];
    printDebug("Setting up Pipeline test...", 0);
    printDebug("Initializing MemoryBus", 1);
    MemoryBus memBus = MemoryBus(0x00FF, 0x03FF, 0x13FF);
    printDebug("Creating CpuSim instance", 1);
    CpuSim cpu1 = CpuSim(memBus,0, 0x0300);
    cpu1.initializeRegisters();
    // Load instruction queue from file and into memory
    printDebug("Setting up instruction queue", 1);
    fill_queue("instructions/cpu0_instructions.txt", instrQ0, 38);
    load_mem_array(memBus, 0x0000, 0x0093, instrQ0);

    //Populate data vectors in memory with random FP32 values
    //0x400-0x7FF (ARRAY_A)
    initialize_mem_array(memBus, 0x400, 0x7FF, 123);
    //0x800-0xBFF (ARRAY_B)
    initialize_mem_array(memBus, 0x800, 0xBFF, 456);

    // Initialize pipeline simulations
    printDebug("Creating pipeline simulations", 1);
    pipelineSimulation pipeline1 = pipelineSimulation(&cpu1, "CPU1");

    //begin cpu simulation, driving clock externally
    printDebug("Starting CPU simulation loop", 0);
    printDebug("============================================================", 0);
    pipeline1.start();
    int tick0 = 0;
    while(!pipeline1.halted){
        if(!pipeline1.halted) pipeline1.tick();
        memBus.tick(); //advance memory bus arbitration
        tick0++;
        }

    unsigned int cycles0 = tick0 / 10;
    printDebug("Cycles: " + std::to_string(cycles0), 1);
    unsigned int instructionCt0 = sizeof(instrQ0) / sizeof(instrQ0[0]);
    printDebug("Instructions: " + std::to_string(instructionCt0), 1);
    float calculated_cpi0 = cycles0 / instructionCt0;
    printDebug("CPU simulation complete.", 0);
    printDebug("CPU0 CPI: " + std::to_string(calculated_cpi0), 0);
    return 0;
}