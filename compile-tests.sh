#!/bin/bash

echo -e "\033[1;34mCompiling test_memory...\033[0m"
g++ -std=c++23 sim/memory.cpp test/test_memory.cpp -o test_memory.o

echo -e "\033[1;34mCompiling test_pipeline...\033[0m"
g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp test/test_pipeline.cpp -o test_pipeline.o

echo -e "\033[1;34mCompiling single-cpu...\033[0m"
g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp test/single-cpu.cpp -o single.o

echo -e "\033[1;34mCompiling dual-cpu...\033[0m"
g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp test/dual-cpu.cpp -o dual.o

echo -e "\033[1;32mCompilation complete.\033[0m"