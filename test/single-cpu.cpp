#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/memory.h"


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
    uint32_t memAddr;
    //instance basic memory bank
    pipelineSimulation cpuSim;
    //instance pipeline sim
    cpuSim = pipelineSimulation();
    //load data/instructions into memory
    fill_queue("cpu0_instructions.txt", tempQ, 38);
    for(i=0;i<38;i++){
        memAddr = i * 4;
        cpuSim.simMemory.tryStoreWord(memAddr, tempQ[i]);
    }
    //Run the sim
    cpuSim.run();
    //run some tests/printouts
    //calculations?
    return 0;
}