#!/bin/bash

echo -e "\033[1;34mBacking up old object files...\033[0m"
cp *.o ./old

echo -e "\033[1;34mCompiling test_memory...\033[0m"
g++ -std=c++23 sim/memory.cpp test/test_memory.cpp -o test_memory.o

echo -e "\033[1;34mCompiling test_alu...\033[0m"
#g++ -std=c++23 sim/alu.cpp test/test_alu.cpp -o test_alu.o

echo -e "\033[1;34mCompiling test_cpu...\033[0m"
g++ -std=c++23 sim/alu.cpp sim/memory.cpp sim/cpu.cpp test/test_cpu.cpp -o test_cpu.o

echo -e "\033[1;34mCompiling test_pipelined...\033[0m"
g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp sim/cpu.cpp test/test_pipeline.cpp -o test_pipeline.o

echo -e "\033[1;34mCompiling single-cpu...\033[0m"
#g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp sim/cpu.cpp test/single-cpu.cpp -o single.o

echo -e "\033[1;34mCompiling dual-cpu...\033[0m"
#g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp sim/cpu.cpp test/dual-cpu.cpp -o dual.o

echo -e "\033[1;32mCompilation complete.\033[0m"