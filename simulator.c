/* LC-2K Instruction-level simulator */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8       /* number of machine registers */
#define MAXLINELENGTH 1000

/* NO CHANGE */
typedef struct stateStruct
{
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
int convertNum(int num);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    long long inst;
    int opcode, regA, regB, destReg, offsetField;
    int i, countInst = 0;
    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL)
    {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }
    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
         state.numMemory++)
    {
        if (sscanf(line, "%d", state.mem + state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    state.pc = 0;
    for(i = 0; i < NUMREGS; i++) {
        state.reg[i] = 0;
    }
    while (1) {
        printState(&state);
        countInst++;
        inst = state.mem[state.pc];
        opcode = (inst >> 22) & 0b111;
        state.pc++;
        /* R-Type */
        if (opcode == 0b000 || opcode == 0b001) {
            regA = (inst >> 19) & 0b111;
            regB = (inst >> 16) & 0b111;
            destReg = inst & 0b111;
            /* add */
            if(opcode == 0b000) {
                state.reg[destReg] = state.reg[regA] + state.reg[regB];
            }
            /* nor */
            else {
                state.reg[destReg] = state.reg[regA] | state.reg[regB];
            }
        }
        /* I-Type */
        else if (opcode == 0b010 || opcode == 0b011 || opcode == 0b100) {
            regA = (inst >> 19) & 0b111;
            regB = (inst >> 16) & 0b111;
            offsetField = inst & 0xFFFF;
            /* lw */
            if(opcode == 0b010) {
                state.reg[regB] = state.mem[state.reg[regA] + convertNum(offsetField)];
            }
            /* sw */
            else if(opcode == 0b011) {
printf("%d %d\n", state.reg[regA], convertNum(offsetField));
                state.mem[state.reg[regA] + convertNum(offsetField)] = state.reg[regB];
            }
            /* beq */
            else {
                if(state.reg[regA] == state.reg[regB]) {
                    //branch to the address
                    state.pc += convertNum(offsetField);
                }
            }
        }
        /* J-Type */
        /* jalr */
        else if (opcode == 0b101) {
            regA = (inst >> 19) & 0b111;
            regB = (inst >> 16) & 0b111;
            state.reg[regB] = state.pc;
            if(regA != regB) {
                state.pc = state.reg[regA];
            }
        }
        /* O-Type */
        /* halt */
        else if (opcode == 0b110) {
            printf("machine halted\n");
            printf("total of %d instructions executed\n", countInst);
            printf("final state of machine:\n");
            printState(&state);
            break;
        }
        /* noop */
        /* Do nothing */
    }
    
    return (0);
}

/* NO CHANGE */
void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++)
    {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++)
    {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1 << 15))
    {
        num -= (1 << 16);
    }
    return (num);
}