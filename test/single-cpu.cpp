#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/helpers.h"

int debug;

int main(){
    debug = 1;
    uint32_t instrQ0[38];
    printDebug("Setting up Pipeline test...", 0);
    printDebug("Initializing MemoryBus", 1);
    MemoryBus memBus = MemoryBus(0x00FF, 0x01FF, 0x13FF);
    printDebug("Creating CpuSim instance", 1);
    CpuSim cpu1 = CpuSim(memBus,0);

    // Load instruction queue from file and into memory
    printDebug("Setting up instruction queue", 1);
    fill_queue("instructions/cpu0_instructions.txt", instrQ0, 38);
    load_mem_array(memBus, 0x0000, 0x0093, instrQ0);

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
        if (tick0 >= 100) break;
        }

    unsigned int cycles0 = tick0 / 10;
    unsigned int instructionCt0 = sizeof(instrQ0) / sizeof(instrQ0[0]);
    float calculated_cpi0 = cycles0 / instructionCt0;
    printDebug("CPU simulation complete.", 0);
    printDebug("CPU0 CPI: " + std::to_string(calculated_cpi0), 0);
    return 0;
}