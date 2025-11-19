#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/memory.h"

int main(){
    uint32_t tempQ[38];
    uint32_t i;
    uint32_t memAddr;
    //instance basic memory bank
    Memory memBank = Memory();
    pipelineSimulation cpuSim;
    //instance pipeline sim
    cpuSim = pipelineSimulation();
    //load data/instructions into memory
    fill_queue("cpu0_instructions.txt", tempQ, 38);
    for(i=0;i<38;i++){
        memAddr = i * 4;
        memBank.storeWord(memAddr, tempQ[i]);
    }
    //Run the sim
    cpuSim.run();
    //run some tests/printouts
    //calculations?
    return 0;
}