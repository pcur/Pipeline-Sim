#include "../test/config.h"
#include "../sim/pipeline.h"
#include <iostream>
#include <fstream>

    int debug;
    bool halted;
    unsigned int instrQ[10];
    
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
    debug = 1;
    halted = 0;
    std::cout << "Pipeline Simulation, assignment 2\n";
    fill_queue("instructions.txt", instrQ, 9);

    for(int i = 0; i < 8; i++){
        std::cout << instrQ[i] << std::endl;
    }
    pipelineSimulation cpuSim;
    cpuSim = pipelineSimulation();
    cpuSim.run();

    return 0;
}
