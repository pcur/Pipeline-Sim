#include "../sim/pipeline.h"

#define BLOCK1      0x00FF
#define BLOCK2      0x01FF
#define BLOCK3      0x03FF
#define BLOCK4      0x13FF

int main(){
    uint32_t tempQ[38];
    uint32_t i;
    uint32_t = memAddr;

    MemoryBus memBus(BLOCK1, BLOCK2, BLOCK3, BLOCK4);
    //First we populate the cpu0 instructions
    fill_queue("cpu0_instructions.txt", tempQ, 38);
    for(i=0;i<38;i++){
        memAddr = i * 4;
        memBus.tryStoreWord(memAddr, tempQ[i]);
    }
    //Next we populate the cpu1 instructions
    fill_queue("cpu1_instructions.txt", tempQ, 38);
    for(i=0;i<38;i++){
        memAddr = 0x100 + (i * 4); //Gotta offset it to the correct memory location
        memBus.tryStoreWord(memAddr, tempQ[i]);
    }
    //Need a cpuSim
    pipelineSimulation cpuSim;
    cpuSim = pipelineSimulation();

    //Need to tell it to run the sim
    cpuSim.run();

    //Print some stuff here if necessary

    return 0;
}