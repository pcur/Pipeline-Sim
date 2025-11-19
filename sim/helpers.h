#pragma once
#include "../test/config.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstring>
#include <bitset>

inline void printDebug(const std::string &msg, int debugLevel) {
    if (debugLevel == 0){
    std::cout << msg << std::endl;
    return;
    }
    #ifdef DEBUG
    if (debugLevel <= DEBUG)
    std::cout << "DEBUG: " << msg << std::endl;
    #endif
}

inline void fill_queue(const std::string& filename, unsigned int* instructionQueue, size_t queueSize) {
    std::ifstream infile(filename);
    std::string line;
    size_t index = 0;
    printDebug("Filling instruction queue from file: " + filename, 1);
    while (std::getline(infile, line) && index < queueSize) {
        unsigned int instruction = std::bitset<32>(line).to_ulong();
        instructionQueue[index] = instruction;  // Store instruction in array
        printDebug("Loaded instruction: " + line + " into queue index " + std::to_string(index), 3);
        ++index;  // Move to the next index
    }
}