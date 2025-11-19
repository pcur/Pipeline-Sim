#include "../sim/pipeline.h"

void fill_queue(const std::string& filename, unsigned int* instructionQueue, size_t queueSize) {
    std::ifstream infile(filename);
    std::string line;
    size_t index = 0;

    while (std::getline(infile, line) && index < queueSize) {
        unsigned int instruction = std::bitset<32>(line).to_ulong();
        instructionQueue[index] = instruction;  // Store instruction in array
        ++index;  // Move to the next index
    }
}

int main(){
    uint32_t tempQ[38];
    uint32_t i;
    uint32_t = memAddr;

    pipelineSimulation cpuSim;
    cpuSim = pipelineSimulation();

    //First we populate the cpu0 instructions
    fill_queue("cpu0_instructions.txt", tempQ, 38);
    for(i=0;i<38;i++){
        memAddr = i * 4;
        cpuSim.simMemory.tryStoreWord(memAddr, tempQ[i]);
    }
    
    //Next we populate the cpu1 instructions
    fill_queue("cpu1_instructions.txt", tempQ, 38);
    for(i=0;i<38;i++){
        memAddr = 0x100 + (i * 4); //Gotta offset it to the correct memory location
        cpuSim.simMemory.tryStoreWord(memAddr, tempQ[i]);
    }
    //Need to tell it to run the sim
    cpuSim.run();

    //Print some stuff here if necessary

    return 0;
}