#include "../sim/pipeline.h"
#include <cstring>
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

int debug;
bool halted;
unsigned int instrQ[10];

//Store random FP32 values into memory arrays between specified addresses
void initialize_mem_array(MemoryBus& memBus, uint32_t startAddr, uint32_t endAddr, unsigned int seed = 42) {
    std::srand(seed); // Seed for reproducibility
    uint32_t numFloats = (endAddr - startAddr + 1) / 4;
    for(uint32_t i = 0; i < numFloats; i++){
        float randomValue = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 100.0f; // Random float between 0 and 100
        uint32_t binaryValue;
        std::memcpy(&binaryValue, &randomValue, sizeof(float)); // Copy float bits to uint32_t
        uint32_t memAddr = startAddr + (i * 4);
        memBus.storeWord(memAddr, binaryValue);
    }
}

int main(){
    uint32_t tempQ[38];
    uint32_t i;
    uint32_t memAddr;

    MemoryBus memBus = MemoryBus(0x00FF, 0x01FF, 0x13FF);
    pipelineSimulation cpuSim = pipelineSimulation(memBus);

    //First we populate the cpu0 instructions
    fill_queue("cpu0_instructions.txt", tempQ, 38);
    for(i=0;i<38;i++){
        memAddr = i * 4;
        cpuSim.simMemory.storeWord(memAddr, tempQ[i]);
    }
    
    //Next we populate the cpu1 instructions
    fill_queue("cpu1_instructions.txt", tempQ, 38);
    for(i=0;i<38;i++){
        memAddr = 0x100 + (i * 4); //Gotta offset it to the correct memory location
        cpuSim.simMemory.storeWord(memAddr, tempQ[i]);
    }

    //Populate data vectors in memory with random FP32 values
    //0x400-0x7FF (ARRAY_A)
    initialize_mem_array(cpuSim.simMemory, 0x400, 0x7FF, 123);
    //0x800-0xBFF (ARRAY_B)
    initialize_mem_array(cpuSim.simMemory, 0x800, 0xBFF, 456);
    //0xC00-0xFFF (ARRAY_C)
    initialize_mem_array(cpuSim.simMemory, 0xC00, 0xFFF, 789);
    //0x1000-0x13FF (ARRAY_D)
    initialize_mem_array(cpuSim.simMemory, 0x1000, 0x13FF, 987);
    
    //Need to tell it to run the sim
    cpuSim.run();

    //Print some stuff here if necessary

    return 0;
}