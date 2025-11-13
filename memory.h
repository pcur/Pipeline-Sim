#include <cstdint>
#include <iostream>
#include <vector>
#include <tuple>

#define MEM_SIZE 1024

// Size specifiers for dynamic store/load (in bits)
#define SIZE_BYTE      8
#define SIZE_HALFWORD  16
#define SIZE_WORD      32

#define LOAD_LATENCY 20 //in ticks
#define STORE_LATENCY 20 //in ticks

// Base Memory class providing basic load/store functionality of a memory system.
// Can be constructed with a default size or a custom size.
// Allows for storing and loading in byte, half-word, and word sizes.
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
protected:  // Changed from private to protected for derived access
    uint8_t* memory; // Simple byte-addressable memory
};

// Subclass of Memory that allows for multi-bank memory with arbitration for bus-style access.
// Each bank is defined by its end address, and each bank is individually arbitrated.
class MemoryBus : public Memory {
public:
    //Memory Bus Constructor
    MemoryBus(uint32_t bank1endaddr, uint32_t bank2endaddr, uint32_t bank3endaddr, uint32_t bank4endaddr);
    //Memory Bus Destructor
    ~MemoryBus();
    
    // Stores a word of data at the specified address, with arbitration
    bool tryStoreWord(uint32_t address, uint32_t data);
    // Stores a half-word of data at the specified address, with arbitration
    bool tryStoreHalfWord(uint32_t address, uint16_t data);
    // Stores a byte of data at the specified address, with arbitration
    bool tryStoreByte(uint32_t address, uint8_t data);
    // Loads a word of data from the specified address, with arbitration
    std::tuple<uint32_t, bool> tryLoadWord(uint32_t address);
    // Loads a half-word of data from the specified address, with arbitration
    std::tuple<uint16_t, bool> tryLoadHalfWord(uint32_t address);
    // Loads a byte of data from the specified address, with arbitration
    std::tuple<uint8_t, bool> tryLoadByte(uint32_t address);
    // Tick: Advances the state of the memory bus by one tick (for arbitration timing)
    void tick();

private:
// Stores the start address of each bank
    std::vector<uint32_t> bank_offsets;
    //Returns which bank an address belongs to (0 to n)
    uint8_t bank(uint32_t address);
    std::vector<bool> bank_locks; // Stores the lock status of each bank for arbitration
    // Returns true if the port of a memory address is locked (for arbitration)
    bool is_locked(uint32_t address); 
    std::vector<uint8_t> ticks_until_free; // Tracks ticks until each bank is free
};
