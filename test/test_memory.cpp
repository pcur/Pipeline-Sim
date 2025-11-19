// Simple test main function to verify functionality of memory bus module
#include "../sim/memory.h"

int main(){
    printDebug("Memory module test", 0);
    MemoryBus memorybus = MemoryBus(255, 511, 1023); // 3 banks of 256 bytes each
    uint32_t test_address = 0x100;
    uint32_t test_data = 0xDEADBEEF;

    if(memorybus.tryStoreWord(test_address, test_data)){
        std::stringstream ss;
        ss << "Correctly stored word 0x" << std::hex << test_data << " at address 0x" << test_address;
        printDebug(ss.str(), 0);
    } else {
        std::stringstream ss;
        ss << "Incorrectly failed to store word at address 0x" << std::hex << test_address;
        printDebug(ss.str(), 0);
    }
    if(memorybus.tryStoreWord(test_address, test_data)){
        std::stringstream ss2;
        ss2 << "Incorrectly stored word 0x" << std::hex << test_data << " at address 0x" << test_address;
        printDebug(ss2.str(), 0);
    } else {
        std::stringstream ss3;
        ss3 << "Correctly failed to store word at address 0x" << std::hex << test_address;
        printDebug(ss3.str(), 0);
    }
    for (int i = 0; i < STORE_LATENCY + 3; ++i) {
        memorybus.tick();
    }
    printDebug("Lock should be removed", 0);
    
    uint32_t loaded_data = 0x0;
    auto [data, success] = memorybus.tryLoadWord(test_address);
    if (success) {
        loaded_data = data;
        std::stringstream ss4;
        ss4 << "Correctly loaded word 0x" << std::hex << loaded_data << " from address 0x" << test_address;
        printDebug(ss4.str(), 0);
    } else {
        std::stringstream ss5;
        ss5 << "Incorrectly failed to load word from address 0x" << std::hex << test_address;
        printDebug(ss5.str(), 0);
    }
    printDebug("End of memory module test", 0);
    return 0;
}