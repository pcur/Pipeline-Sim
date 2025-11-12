#include "memory.h"

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

void Memory::storeHalfWord(uint32_t address, uint16_t data) {
    if (address % 2 != 0 || address + 1 >= mem_size) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return;
    }
    memory[address] = (data >> 8) & 0xFF;
    memory[address + 1] = data & 0xFF;
}

void Memory::storeByte(uint32_t address, uint8_t data) {
    if (address >= mem_size) {
        std::cout << "Error: Out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return;
    }
    memory[address] = data;
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

uint16_t Memory::loadHalfWord(uint32_t address) {
    if (address % 2 != 0 || address + 1 >= mem_size) {
        std::cout << "Error: Unaligned or out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return 0;
    }
    return (static_cast<uint16_t>(memory[address]) << 8) |
           (static_cast<uint16_t>(memory[address + 1]));
}

uint8_t Memory::loadByte(uint32_t address) {
    if (address >= mem_size) {
        std::cout << "Error: Out-of-bounds memory access at address 0x" << std::hex << address << std::endl;
        return 0;
    }
    return memory[address];
}

MemoryBus::MemoryBus(uint32_t bank1endaddr, uint32_t bank2endaddr, uint32_t bank3endaddr, uint32_t bank4endaddr) {
    
    bank_offsets.push_back(0);
    bank_offsets.push_back(bank1endaddr + 1);
    bank_offsets.push_back(bank2endaddr + 1);
    bank_offsets.push_back(bank3endaddr + 1);

    mem_size = bank4endaddr + 1;
    memory = new uint8_t[mem_size];
    for (int i = 0; i < mem_size; ++i) {
        memory[i] = 0;
    }
}
MemoryBus::~MemoryBus() {
    delete[] memory;
}
