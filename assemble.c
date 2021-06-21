/* Assembler code fragment for LC-2K */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAXLINE 50
#define MAXLINELENGTH 1000
#define MAXOFFSETFIELD (1 << 16)

int saveLabelList(FILE *, char [][MAXLINELENGTH], int *, int *, int *);
int readAndParse(FILE *inFilePtr, FILE *outFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2, char labelList[][MAXLINELENGTH], int labelCount, int *labelAddress, int *lineCount);
int isNumber(char *);
int findLabel(char *label, char labelList[][MAXLINELENGTH], int labelCount);

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    int labelCount = 0, lineCount = 0;
    int labelAddress[MAXLINE];
    char labelList[MAXLINE][MAXLINELENGTH];
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }
    inFileString = argv[1];
    outFileString = argv[2];
    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL)
    {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL)
    {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /* save labelList */
    while (saveLabelList(inFilePtr, labelList, &labelCount, labelAddress, &lineCount)) {
        lineCount++;
    }

    /* this is how to rewind the file ptr so that you start reading from the beginning of the file */
    rewind(inFilePtr);
    lineCount = 0;

    /* here is an example for how to use readAndParse to read a line from inFilePtr */
    while (readAndParse(inFilePtr, outFilePtr, label, opcode, arg0, arg1, arg2, labelList, labelCount, labelAddress, &lineCount)) {
        lineCount++;
    }

    return (0);
} 

int saveLabelList(FILE *inFilePtr, char labelList[][MAXLINELENGTH], int *labelCount, int *labelAddress, int *lineCount) {
    char line[MAXLINELENGTH];
    char label[MAXLINELENGTH];
    char *ptr = line;
    int i;
    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
    {
        /* reached end of file */
        return (0);
    }
    
    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL)
    {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }
    /* is there a label? */
    if (sscanf(ptr, "%[^\t\n\r ]", label))
    {
        /* successfully read label; advance pointer over the label */
        for (i = 0; i < (*labelCount); i++) {
            if (strcmp(labelList[i], label) == 0) {
                printf("error: duplicate labels\n");
                exit(1);
            }
        }
        labelAddress[(*labelCount)] = (*lineCount);
        strcpy(labelList[(*labelCount)], label);
        (*labelCount)++;
    }
    return (1);
}

/*
 * Read and parse a line of the assembly-language file. Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 * 0 if reached end of file
 * 1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, FILE *outFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2, char labelList[][MAXLINELENGTH], int labelCount, int *labelAddress, int *lineCount)
{
    char line[MAXLINELENGTH], blank[MAXLINELENGTH];
    char *ptr = line;
    long long inst;
    int op, regA, regB, destReg;
    long long offsetField, fillNum;
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
    {
        /* reached end of file */
        return (0);
    }
    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL)
    {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }
    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label))
    {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }
    /*
 * Parse the rest of the line. Would be nice to have real regular
 * expressions, but scanf will suffice.
 */
    sscanf(ptr, "%[\t\n\r ]%[^\t\n\r ]", blank, opcode);
    ptr = ptr + strlen(blank) + strlen(opcode);
    /* R-Type */
    if(!strcmp(opcode, "add") || !strcmp(opcode, "nor")) {
        if(strcmp(opcode, "add") == 0) op = 0b000;
        else op = 0b001;
        sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
                arg0, arg1, arg2);
        regA = atoi(arg0);
        regB = atoi(arg1);
        destReg = atoi(arg2);
        inst = (op << 22) + (regA << 19) + (regB << 16) + destReg;
    }
    /* I-Type */
    else if(!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq")) {
        if(strcmp(opcode, "lw") == 0) op = 0b010;
        else if(strcmp(opcode, "sw") == 0) op = 0b011;
        else op = 0b100;

        sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
                arg0, arg1, arg2);
        regA = atoi(arg0);
        regB = atoi(arg1);
        if(isNumber(arg2)) {
            offsetField = atoi(arg2);
            if(offsetField < -MAXOFFSETFIELD || offsetField >= MAXOFFSETFIELD) {
                /* offsetFields that don't fit in 16 bits */
                printf("error: offsetFields that don't fit in 16 bits\n");
                exit(1);
            }
        }
        else {
            offsetField = labelAddress[findLabel(arg2, labelList, labelCount)];
            if(!strcmp(opcode, "beq")) { 
                offsetField -= ((*lineCount) + 1);
            }
        } 
        if(offsetField < 0) {
           offsetField = offsetField & 0xFFFF;
        }
        inst = (op << 22) + (regA << 19) + (regB << 16) + offsetField;
    }
    /* J-Type */
    else if(!strcmp(opcode, "jalr")) {
        op = 0b101;
        sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
                arg0, arg1);
        regA = atoi(arg0);
        regB = atoi(arg1);
        inst = (op << 22) + (regA << 19) + (regB << 16);
    }
    /* O-Type */
    else if(!strcmp(opcode, "halt") || !strcmp(opcode, "noop")) {
        if(strcmp(opcode, "halt") == 0) op = 0b110;
        else op = 0b111;
        inst = op << 22;
    }
    /* .fill */
    else if(!strcmp(opcode, ".fill")) {
        sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]", arg0);
        if(isNumber(arg0)) {
            fillNum = atoi(arg0);
        }
        else {
            fillNum = labelAddress[findLabel(arg0, labelList, labelCount)];
        }
        inst = fillNum;
    }
    /* unrecognized opcodes */
    else {
        printf("error: unrecognized opcodes\n");
        printf("%s\n", opcode);
        exit(1);
    }
    fprintf(outFilePtr, "%lld\n", inst);
    return (1);
}

int isNumber(char *string)
{ /* return 1 if string is a number */
    long long i;
    return ((sscanf(string, "%lld", &i)) == 1);
}

/* return label offset */
int findLabel(char *label, char labelList[][MAXLINELENGTH], int labelCount) {
    int i;
    for(i = 0; i < labelCount; i++) {
        if(strcmp(label, labelList[i]) == 0) {
            /* found label */
            return i;
        }
    }

    /* use of undefined labels */
    printf("error: use of undefined labels\n");
    exit(1);
}