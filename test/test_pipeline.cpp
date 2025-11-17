#include "../test/config.h"
#include <iostream>

int main(){
    debug = 1;
    halted = 0;
    std::cout << "Pipeline Simulation, assignment 2\n";
    fill_queue("instructions.txt", instrQ, 9);

    for(int i = 0; i < 8; i++){
        std::cout << instrQ[i] << std::endl;
    }
    pipelineSimulation cpuSim  = new pipelineSimulation();
    cpuSim.run();

    return 0;
}
