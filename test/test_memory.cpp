// Simple test main function to verify functionality of memory bus module
#include "../sim/memory.h"
#include "../test/config.h"

int main(){
    std::cout << "Memory module test\n";
    MemoryBus memorybus = MemoryBus(255, 511, 767, 1023); // 4 banks of 256 bytes each
    uint32_t test_address = 0x100;
    uint32_t test_data = 0xDEADBEEF;

    if(memorybus.tryStoreWord(test_address, test_data)){
        std::cout << "Correctly stored word 0x" << std::hex << test_data << " at address 0x" << test_address << std::hex << "\n";
    } else {
        std::cout << " Incorrectly failed to store word at address 0x" << test_address << std::hex << "\n";
    }
    if(memorybus.tryStoreWord(test_address, test_data)){
        std::cout << "Incorrectly stored word 0x" << std::hex << test_data << " at address 0x" << test_address << std::hex << "\n";
    } else {
        std::cout << "Correctly failed to store word at address 0x" << test_address << std::hex << "\n";
    }
    for (int i = 0; i < STORE_LATENCY + 3; ++i) {
        memorybus.tick();
    }
    std::cout << "Lock should be removed\n";
    
    uint32_t loaded_data = 0x0;
    auto [data, success] = memorybus.tryLoadWord(test_address);
    if (success) {
        loaded_data = data;
        std::cout << " Correctly loaded word 0x" << std::hex << loaded_data << " from address 0x" << test_address << std::hex << "\n";
    } else {
        std::cout << " Incorrectly failed to load word from address 0x" << test_address << std::hex << "\n";
    }
    return 0;
}