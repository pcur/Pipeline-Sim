#include "../sim/memory.h"

Memory::Memory(int size) {
    mem_size = size;
    memory = new uint8_t[mem_size];
    for (int i = 0; i < mem_size; ++i) {
        memory[i] = 0;
    }
    printDebug("Memory initialized with size " + std::to_string(mem_size), 1);
}

Memory::Memory() {
    memory = new uint8_t[mem_size];
    for (int i = 0; i < mem_size; ++i) {
        memory[i] = 0;
    }
    printDebug("Memory initialized with size " + std::to_string(mem_size), 1);
}

Memory::~Memory() {
    delete[] memory;
    printDebug("Memory deallocated", 1);
}

void Memory::storeWord(uint32_t address, uint32_t data) {
    if (address % 4 != 0 || address + 3 >= mem_size) {
        {
            std::ostringstream oss;
            oss << "0x" << std::hex << std::uppercase << address;
            printDebug("Error: Unaligned or out-of-bounds memory access at address " + oss.str(), 0);
        }
        return;
    }
    memory[address] = (data >> 24) & 0xFF;
    memory[address + 1] = (data >> 16) & 0xFF;
    memory[address + 2] = (data >> 8) & 0xFF;
    memory[address + 3] = data & 0xFF;
}

void Memory::storeHalfWord(uint32_t address, uint16_t data) {
    if (address % 2 != 0 || address + 1 >= mem_size) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("Error: Unaligned or out-of-bounds memory access at address " + oss.str(), 0);
        return;
    }
    memory[address] = (data >> 8) & 0xFF;
    memory[address + 1] = data & 0xFF;
}

void Memory::storeByte(uint32_t address, uint8_t data) {
    if (address >= mem_size) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("Error: Unaligned or out-of-bounds memory access at address " + oss.str(), 0);
        return;
    }
    memory[address] = data;
}

uint32_t Memory::loadWord(uint32_t address) {
    if (address % 4 != 0 || address + 3 >= mem_size) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("Error: Unaligned or out-of-bounds memory access at address " + oss.str(), 0);
        return 0;
    }
    return (static_cast<uint32_t>(memory[address]) << 24) |
           (static_cast<uint32_t>(memory[address + 1]) << 16) |
           (static_cast<uint32_t>(memory[address + 2]) << 8) |
           (static_cast<uint32_t>(memory[address + 3]));
}

uint16_t Memory::loadHalfWord(uint32_t address) {
    if (address % 2 != 0 || address + 1 >= mem_size) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("Error: Unaligned or out-of-bounds memory access at address " + oss.str(), 0);
        return 0;
    }
    return (static_cast<uint16_t>(memory[address]) << 8) |
           (static_cast<uint16_t>(memory[address + 1]));
}

uint8_t Memory::loadByte(uint32_t address) {
    if (address >= mem_size) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("Error: Unaligned or out-of-bounds memory access at address " + oss.str(), 0);
        return 0;
    }
    return memory[address];
}

MemoryBus::MemoryBus(uint32_t bank0endaddr, uint32_t bank1endaddr, uint32_t bank2endaddr) {
    
    bank_offsets.push_back(0); // bank 0 offset
    bank_offsets.push_back(bank0endaddr + 1); // bank 1 offset
    bank_offsets.push_back(bank1endaddr + 1); // bank 2 offset

    bank_locks.resize(3, false); // Initialize all bank locks to false (unlocked)
    ticks_until_free.resize(3, 0); // Initialize all ticks until free to 0

    mem_size = bank2endaddr + 1;
    memory = new uint8_t[mem_size];
    for (int i = 0; i < mem_size; ++i) {
        memory[i] = 0;
    }
    printDebug("MemoryBus initialized with size " + std::to_string(mem_size), 1);
}

MemoryBus::~MemoryBus() {
    printDebug("MemoryBus deallocated", 1);
}

void MemoryBus::lock(uint8_t bank, bool readLatency){
    if (readLatency) {
        if (LOAD_LATENCY > 0){ ticks_until_free[bank] = LOAD_LATENCY;}
        else {
            unlock(bank);
            printDebug("Bank " + std::to_string(bank) +" not locking due to no load latency set.", 1);
            return;
        }
    }
    else {
        if (STORE_LATENCY > 0){ ticks_until_free[bank] = LOAD_LATENCY;}
        else {
            unlock(bank);
            printDebug("Bank " + std::to_string(bank) +" not locking due to no store latency set.", 1);
            return;
        }
    }
    printDebug("Locking bank " + std::to_string(bank)+ ".", 2);
    bank_locks[bank] = true;
}

void MemoryBus::unlock(uint8_t bank){
    printDebug("Unocking bank " + std::to_string(bank)+ ".", 2);
    ticks_until_free[bank] = 0;
    bank_locks[bank] = false;
}

bool MemoryBus::is_locked(uint8_t bank) {
    printDebug("Bank " + std::to_string(bank) + " lock status: " + (bank_locks[bank] ? "locked" : "unlocked"), 1);
    return bank_locks[bank];
}

void MemoryBus::tick(int num_ticks) {
    printDebug("Ticking memory bus", 2);
    for (int t = 0; t < num_ticks; ++t) {
        for (size_t i = 0; i < ticks_until_free.size(); ++i) { //tick for each bank
            if(bank_locks[i] == true) {
                printDebug("Bank " + std::to_string(i) + " ticks until free before tick: " + std::to_string(ticks_until_free[i]), 5);
                if (ticks_until_free[i] > 0) {
                    --ticks_until_free[i];
                    printDebug("Bank " + std::to_string(i) + " ticks until free: " + std::to_string(ticks_until_free[i]), 3);
                    if (ticks_until_free[i] <= 0) {
                        unlock(static_cast<uint8_t>(i)); // Unlock the bank when ticks reach zero
                        printDebug("Bank " + std::to_string(i) + " unlocked", 1);
                    }
                }
            }
        }
    }
}

uint8_t MemoryBus::bank(uint32_t address) {
    if (address >= mem_size) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("Error: Address " + oss.str() + " is out of bounds for memory size " + std::to_string(mem_size), 0);
        return 0; // Default to bank 0 on error
    }
    for (size_t i = 0; i < bank_offsets.size() - 1; ++i) { //iterate through bank 0 to (n-1)
        if (address < bank_offsets[i+1]) { // if address is before beginning of bank i + 1 (located in)
            std::ostringstream oss;
            oss << "0x" << std::hex << std::uppercase << address;
            printDebug("Address " + oss.str() + " belongs to bank " + std::to_string(i), 2);
            return i;
        }
    }
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << address;
    printDebug("Address " + oss.str() + " belongs to bank " + std::to_string(bank_offsets.size() - 1), 2);
    return bank_offsets.size() - 1; // Address belongs to the last bank
}

// Wrapped methods - call parent's implementation with optional pre/post processing

bool MemoryBus::tryStoreWord(uint32_t address, uint32_t data) {
    uint8_t bankNum = bank(address);
    if (is_locked(bankNum)) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("StoreWord failed: Bank is locked at address " + oss.str(), 1);
        return false; // Bank is locked, cannot perform store
    }
    Memory::storeWord(address, data);
    lock(bankNum, false);
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << address;
    printDebug("StoreWord succeeded at address " + oss.str(),1);
    return true;
}

bool MemoryBus::tryStoreHalfWord(uint32_t address, uint16_t data) {
    uint8_t bankNum = bank(address);
    if (is_locked(bankNum)) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("StoreHalfWord failed: Bank is locked at address " + oss.str(), 1);
        return false; // Bank is locked, cannot perform store
    }
    Memory::storeHalfWord(address, data);

    lock(bankNum, false);
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << address;
    printDebug("StoreHalfWord succeeded at address " + oss.str(), 1);
    return true;
}
    
bool MemoryBus::tryStoreByte(uint32_t address, uint8_t data) {
    uint8_t bankNum = bank(address);
    if (is_locked(bankNum)) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("StoreByte failed: Bank is locked at address " + oss.str(), 1);
        return false; // Bank is locked, cannot perform store
    }
    Memory::storeByte(address, data);
    lock(bankNum, false);
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << address;
    printDebug("StoreByte succeeded at address " + oss.str(), 1);
    return true;
}

std::tuple<uint32_t, bool> MemoryBus::tryLoadWord(uint32_t address) {
    uint8_t bankNum = bank(address);
    if (is_locked(bankNum)) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("LoadWord failed: Bank is locked at address " + oss.str(), 1);
        return std::make_tuple(0, false); // Bank is locked, cannot perform load
    }
    lock(bankNum, true);
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << address;
    printDebug("LoadWord succeeded at address " + oss.str(), 1);
    return std::make_tuple(Memory::loadWord(address), true);
}

std::tuple<uint16_t, bool> MemoryBus::tryLoadHalfWord(uint32_t address) {
    uint8_t bankNum = bank(address);
    if (is_locked(bankNum)) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("LoadHalfWord failed: Bank is locked at address " + oss.str(), 1);
        return std::make_tuple(0, false); // Bank is locked, cannot perform load
    }
    lock(bankNum, true);
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << address;
    printDebug("LoadHalfWord succeeded at address " + oss.str(), 1);
    return std::make_tuple(Memory::loadHalfWord(address), true);
}

std::tuple<uint8_t, bool> MemoryBus::tryLoadByte(uint32_t address) {
    uint8_t bankNum = bank(address);
    if (is_locked(bankNum)) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << address;
        printDebug("LoadByte failed: Bank is locked at address " + oss.str(), 1);
        return std::make_tuple(0, false); // Bank is locked, cannot perform load
    }
    lock(bankNum, true);
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << address;
    printDebug("LoadByte succeeded at address " + oss.str(), 1);
    return std::make_tuple(Memory::loadByte(address), true);
}
