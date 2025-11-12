#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <iostream>
#include <vector>

#define MEM_SIZE 1024

// Size specifiers for dynamic store/load (in bits)
#define SIZE_BYTE      8
#define SIZE_HALFWORD  16
#define SIZE_WORD      32

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

class MemoryBus : public Memory {
public:
    MemoryBus(uint32_t bank1endaddr, uint32_t bank2endaddr, uint32_t bank3endaddr, uint32_t bank4endaddr);
    ~MemoryBus();
private:
    uint8_t* memory; // Simple byte-addressable memory
    std::vector<uint32_t> bank_offsets; // Stores the start address of each bank
};
