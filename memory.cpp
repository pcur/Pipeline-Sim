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

MemoryBus::MemoryBus(uint32_t bank0endaddr, uint32_t bank1endaddr, uint32_t bank2endaddr, uint32_t bank3endaddr) {
    
    bank_offsets.push_back(0); // bank 0 offset
    bank_offsets.push_back(bank0endaddr + 1); // bank 1 offset
    bank_offsets.push_back(bank1endaddr + 1); // bank 2 offset
    bank_offsets.push_back(bank2endaddr + 1); // bank 3 offset

    bank_locks.resize(4, false); // Initialize all bank locks to false (unlocked)

    mem_size = bank3endaddr + 1;
    memory = new uint8_t[mem_size];
    for (int i = 0; i < mem_size; ++i) {
        memory[i] = 0;
    }
}

MemoryBus::~MemoryBus() {
    delete[] memory;
}

void MemoryBus::tick() {
    for (size_t i = 0; i < ticks_until_free.size(); ++i) {
        if (ticks_until_free[i] > 0) {
            --ticks_until_free[i];
            if (ticks_until_free[i] == 0) {
                bank_locks[i] = false; // Unlock the bank when ticks reach zero
            }
        }
    }
}

uint8_t MemoryBus::bank(uint32_t address) {
    for (size_t i = 0; i < bank_offsets.size() - 1; ++i) { //iterate through bank 0 to (n-1)
        if (address < bank_offsets[i+1]) { // if address is before beginning of bank i + 1 (located in)
            return i;
        }
    }
    return bank_offsets.size() - 1; // Address belongs to the last bank
}

bool MemoryBus::is_locked(uint32_t address) {
    uint8_t bank_index = bank(address);
    return bank_locks[bank_index];
}

// Wrapped methods - call parent's implementation with optional pre/post processing

bool MemoryBus::tryStoreWord(uint32_t address, uint32_t data) {
    if (is_locked(address)) {
        return false; // Bank is locked, cannot perform store
    }
    Memory::storeWord(address, data);
    return true;
}

bool MemoryBus::tryStoreHalfWord(uint32_t address, uint16_t data) {
    if (is_locked(address)) {
        return false; // Bank is locked, cannot perform store
    }
    Memory::storeHalfWord(address, data);
    return true;
}
    
bool MemoryBus::tryStoreByte(uint32_t address, uint8_t data) {
    if (is_locked(address)) {
        return false; // Bank is locked, cannot perform store
    }
    Memory::storeByte(address, data);
    
    return true;
}

std::tuple<uint32_t, bool> MemoryBus::tryLoadWord(uint32_t address) {
    if (is_locked(address)) {
        return std::make_tuple(0, false); // Bank is locked, cannot perform load
    }
    return std::make_tuple(Memory::loadWord(address), true);
}

std::tuple<uint16_t, bool> MemoryBus::tryLoadHalfWord(uint32_t address) {
    if (is_locked(address)) {
        return std::make_tuple(0, false); // Bank is locked, cannot perform load
    }
    return std::make_tuple(Memory::loadHalfWord(address), true);
}

std::tuple<uint8_t, bool> MemoryBus::tryLoadByte(uint32_t address) {
    if (is_locked(address)) {
        return std::make_tuple(0, false); // Bank is locked, cannot perform load
    }
    return std::make_tuple(Memory::loadByte(address), true);
}
