#include "memory.h"

class Memory {
public:
    Memory();
    uint32_t loadWord(uint32_t address);
    void storeWord(uint32_t address, uint32_t data);
    uint16_t loadHalfWord(uint32_t address);
    void storeHalfWord(uint32_t address, uint16_t data);
private:
    uint32_t memory[MEM_SIZE];
};

Memory::Memory() {
    for (int i = 0; i < MEM_SIZE; ++i) {
        memory[i] = 0;
    }
}

uint32_t Memory::loadWord(uint32_t address) {
    if (address % 4 != 0 || address / 4 >= MEM_SIZE) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return 0;  // Invalid access
    }
    return memory[address / 4];
}

void Memory::storeWord(uint32_t address, uint32_t data) {
    if (address % 4 != 0 || address / 4 >= MEM_SIZE) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return;  // Invalid access
    }
    memory[address / 4] = data;
}

uint16_t Memory::loadHalfWord(uint32_t address) {
    if (address % 2 != 0 || address / 2 >= MEM_SIZE * 4) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return 0;  // Invalid access
    }
    return static_cast<uint16_t>(memory[address / 2]);
}

void Memory::storeHalfWord(uint32_t address, uint16_t data) {
    if (address % 2 != 0 || address / 2 >= MEM_SIZE * 4) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return;  // Invalid access
    }
    memory[address / 2] = static_cast<uint32_t>(data);
}
