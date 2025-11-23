# Pipeline-Sim
Pipelined CPU Sim for comp arch taught by Dr. Mukerjee, by Team 2-2
Allows for single cpu-memory cacheless systems to be simulated with a limited RISC-V instruction set including most of RV32I, and selective instructions from RV32M and RV32F. 

Tests can be compiled with `compile-tests.sh`, may require you to run `chmod +x compile-tests.sh` to make executable.

### Contributors and Contributions
Patrick Cur, Team Member
- Priority queue for pipelined cpu sim including halts, stalls pre final project
- Decode for U, I types
- Fetch stage of pipeline
- Memory class
- Memory bus & arbitration
- Separation of old pipelinesim class into CpuSim and PipelineSim (event scheduler) classes
- Dynamic debug printouts
- Assignment adaptation (assignment specific runners/main functions)
- Test scripting and compile scripts
- Testing & debugging code
- Presenting Demos

Joshua Kauffman, Team Member
- Schematic and architecture of decoder
- RISC-V knower
- instruction loading
- Pipeline architecture setup
- ALU functions
- Execute, Store stages of pipeline
- Decode for R, S, B, J types
- Translation of instructions from C or Asm to binaries
- Testing & debugging
- Presenting Demos
- PipelineSim Stall functionality

Matthew Montano, Team Member
- Presentation of Ice Cream Sim
- literally nothing else

Specific contribution history (commits) can also be found on the github.
