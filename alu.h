#define ADD     0b0000000000
#define SUB     0b0100000000
#define SLL     0b0000000001
#define SLT     0b0000000010
#define SLTU    0b0000000011
#define XOR     0b0000000100
#define SRL     0b0000000101
#define SRA     0b0100000101
#define OR      0b0000000110
#define AND     0b0000000111


int alu(int reg1_val, int reg2_val, int function_code);