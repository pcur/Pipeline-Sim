#!/bin/bash

# Function to compile test_memory
compile_memory() {
    echo -e "\033[1;34mCompiling test_memory...\033[0m"
    g++ -std=c++23 sim/memory.cpp test/test_memory.cpp -o test_memory.o
}

# Function to compile test_alu
compile_alu() {
    echo -e "\033[1;34mCompiling test_alu...\033[0m"
    g++ -std=c++23 sim/alu.cpp test/test_alu.cpp -o test_alu.o
}

# Function to compile test_cpu
compile_cpu() {
    echo -e "\033[1;34mCompiling test_cpu...\033[0m"
    g++ -std=c++23 sim/alu.cpp sim/memory.cpp sim/cpu.cpp test/test_cpu.cpp -o test_cpu.o
}

# Function to compile test_pipeline
compile_pipeline() {
    echo -e "\033[1;34mCompiling test_pipelined...\033[0m"
    g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp sim/cpu.cpp test/test_pipeline.cpp -o test_pipeline.o
}

# Function to compile single-cpu
compile_single() {
    echo -e "\033[1;34mCompiling single-cpu...\033[0m"
    g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp sim/cpu.cpp test/single-cpu.cpp -o single.o
}

# Function to compile dual-cpu
compile_dual() {
    echo -e "\033[1;34mCompiling dual-cpu...\033[0m"
    g++ -std=c++23 sim/pipeline.cpp sim/alu.cpp sim/memory.cpp sim/cpu.cpp test/dual-cpu.cpp -o dual.o
}

# Display usage information
usage() {
    echo "Usage: ./compile-tests.sh [all|test|demo|memory|alu|cpu|pipeline|single|dual]"
    echo "  all      - Compile everything"
    echo "  test     - Compile only test_memory and test_cpu"
    echo "  demo     - Compile only single-cpu and dual-cpu"
    echo "  memory   - Compile only test_memory"
    echo "  alu      - Compile only test_alu"
    echo "  cpu      - Compile only test_cpu"
    echo "  pipeline - Compile only test_pipeline"
    echo "  single   - Compile only single-cpu"
    echo "  dual     - Compile only dual-cpu"
}

if [ -z "$1" ]; then
    echo -e "\033[1;31mError: No mode specified\033[0m"
    usage
    exit 1
fi

MODE="$1"

echo -e "\033[1;34mBacking up old object files...\033[0m"
cp *.o ./old 2>/dev/null || true

case "$MODE" in
    all)
        compile_memory
        compile_alu
        compile_cpu
        compile_pipeline
        compile_single
        compile_dual
        ;;
    test)
        compile_memory
        compile_cpu
        ;;
    demo)
        compile_single
        compile_dual
        ;;
    memory)
        compile_memory
        ;;
    alu)
        compile_alu
        ;;
    cpu)
        compile_cpu
        ;;
    pipeline)
        compile_pipeline
        ;;
    single)
        compile_single
        ;;
    dual)
        compile_dual
        ;;
    *)
        echo -e "\033[1;31mUnknown mode: $MODE\033[0m"
        usage
        exit 1
        ;;
esac

echo -e "\033[1;32mCompilation complete.\033[0m"