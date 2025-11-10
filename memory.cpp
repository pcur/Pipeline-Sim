#include "memory.h"

class Memory {
public:
    Memory(); // Default constructor
    Memory(int size); //custom size constructor, may want an additional for initializing data
    ~Memory();
    int mem_size = MEM_SIZE; // number of addressable locations in memory
    uint32_t loadWord(uint32_t address);
    void storeWord(uint32_t address, uint32_t data);
    uint16_t loadHalfWord(uint32_t address);
    void storeHalfWord(uint32_t address, uint16_t data);
    uint8_t loadByte(uint32_t address);
    void storeByte(uint32_t address, uint8_t data);
private:
    uint8_t* memory; // Simple byte-addressable memory
};

Memory::Memory(int size) {
    mem_size = size;
    memory = new uint8_t[mem_size];
    for (int i = 0; i < mem_size; ++i) {
        memory[i] = 0;
    }
}

Memory::Memory() {
    memory = new uint8_t[mem_size];
    for (int i = 0; i < mem_size; ++i) {
        memory[i] = 0;
    }
}

Memory::~Memory() {
    delete[] memory;
}

uint32_t Memory::loadWord(uint32_t address) {
    if (address % 4 != 0 || address + 3 >= mem_size) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return 0;
    }
    return (static_cast<uint32_t>(memory[address]) << 24) |
           (static_cast<uint32_t>(memory[address + 1]) << 16) |
           (static_cast<uint32_t>(memory[address + 2]) << 8) |
           (static_cast<uint32_t>(memory[address + 3]));
}

void Memory::storeWord(uint32_t address, uint32_t data) {
    if (address % 4 != 0 || address + 3 >= mem_size) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return;
    }
    memory[address] = (data >> 24) & 0xFF;
    memory[address + 1] = (data >> 16) & 0xFF;
    memory[address + 2] = (data >> 8) & 0xFF;
    memory[address + 3] = data & 0xFF;
}

uint16_t Memory::loadHalfWord(uint32_t address) {
    if (address % 2 != 0 || address + 1 >= mem_size) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return 0;
    }
    return (static_cast<uint16_t>(memory[address]) << 8) |
           (static_cast<uint16_t>(memory[address + 1]));
}

void Memory::storeHalfWord(uint32_t address, uint16_t data) {
    if (address % 2 != 0 || address + 1 >= mem_size) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return;
    }
    memory[address] = (data >> 8) & 0xFF;
    memory[address + 1] = data & 0xFF;
}

uint8_t Memory::loadByte(uint32_t address) {
    if (address >= mem_size) {
        std::cout << "Error: Out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return 0;
    }
    return memory[address];
}

void Memory::storeByte(uint32_t address, uint8_t data) {
    if (address >= mem_size) {
        std::cout << "Error: Out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return;
    }
    memory[address] = data;
}
