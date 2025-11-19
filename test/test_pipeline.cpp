#include "../test/config.h"
#include "../sim/pipeline.h"
#include "../sim/helpers.h"

int debug;
bool halted;
unsigned int instrQ[10];

int main(){
    debug = 1;
    halted = 0;
    std::cout << "Pipeline Simulation, assignment 2\n";
    fill_queue("instructions.txt", instrQ, 9);
    MemoryBus memBus = MemoryBus(0x00FF, 0x01FF, 0x13FF);
    for(int i = 0; i < 8; i++){
        std::cout << instrQ[i] << std::endl;
    }
    pipelineSimulation cpuSim = pipelineSimulation(memBus);
    cpuSim.run();

    return 0;
}
