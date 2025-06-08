#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

// reg will contain the value of the sixteen registers r0 - r15
int reg[16];

// flags contains flags(E) and flags(GT)
int flags[2];

// Data Memory of 4096 bytes
int Mem[1024];

// rd stores destination register, rs1 and rs2 store the source registers and imm is the immediate
int rd, rs1, rs2, imm;

// m stores the modifier, isImm stores whether the instruction contains an immediate
int m, isImm;

// str is the string containing the whole program
char* str;

// Executes the instruction depending upon the value of pc
void executeInstruction(void);

// Stores the register numbers of a 3-address instruction in rd, rs1 and rs2 or imm
void getReg3Add(char* inst, int i);

// Stores the register numbers of a 2-address instruction in rd and rs2 or imm
void getReg2Add(char* inst, int i);

// Stores the register numbers of a ld/st instruction in rd, rs1 and imm
void getLdSt(char* inst, int i);

// Prints the line number where an error has occured and exits the program
void invalidInst(void);    

// Returns decimal value of a hexadecimal digit
int dec(char ch);

// sets pc for the main function
void setPcForMain(void);

// gets pc for a given label str[i,j]
int getPcForLabel(char* str, int i, int j);

// k stores the line number of the instruction
int k;

// pc stores the instruction number and therefore pc will be the (program counter)/4
int pc;

// lab_no stores the label number
int lab_no;

// lab_count stores the number of labels in the program
int lab_count;

// An array used to convert a hexadecimal immediate to decimal
int hexImm[4];

// encodedInst will contain the encoding of an instruction
unsigned int encodedInst;

// A struct for each instruction
struct instruction
{
    int i;        // starting index of inst
    int j;      // ending index of inst
    int line;    // line in which the instruction appears
};

// An array of all instruction structs of the program (behaves somewhat like the instruction memory)
struct instruction *instructions;

// A struct for each label
struct label
{
    int i;        // starting index of label
    int j;        // ending index of label
    int inst_no;  // inst_no this label points to
};

// An array of label structs of the program
struct label *labels;



int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}

int dec(char ch)
{
    switch(ch)
    {
        case '0': return 0;
                   break;
        case '1': return 1;
                   break;
        case '2': return 2;
                   break;
        case '3': return 3;
                   break;
        case '4': return 4;
                   break;
        case '5': return 5;
                   break;
        case '6': return 6;
                   break;
        case '7': return 7;
                   break;
        case '8': return 8;
                   break;
        case '9': return 9;
                   break;
        case 'a':
        case 'A': return 10;
                   break;
        case 'b':
        case 'B': return 11;
                   break;
        case 'c':
        case 'C': return 12;
                   break;
        case 'd':
        case 'D': return 13;
                   break;
        case 'e':
        case 'E': return 14;
                   break;
        case 'f':
        case 'F': return 15;
                   break;
    }
    invalidInst();
    assert(0);
    return -1;
}

void setPcForMain(void)
{
    int lab_c = 0;
    int i, j;
    while(lab_c < lab_count)
    {
        i = labels[lab_c].i;
        j = labels[lab_c].j;
        if(j-i == 5)
        {
            if(strncmp(&str[i],".main",5) == 0)
            {
                pc = labels[lab_c].inst_no;
                return;
            }
        }
        lab_c++;
    }
    printf("There is no .main label in the program !!!\n");
    exit(0);
}

int getPcForLabel(char* str, int i, int j)
{
    int lab_c = 0;
    int li, lj;
    // Search for that label which has the same name as the label in the instruction
    while(lab_c < lab_count)
    {
        li = labels[lab_c].i;
        lj = labels[lab_c].j;
        if((j-i) == (lj-li))
        {
            if(strncmp(&str[i],&str[li],j-i) == 0)
                return labels[lab_c].inst_no;
        }
        lab_c++;
    }
    printf("The label does not exist !!!\n");
    invalidInst();
    assert(0);
    return -1;
}


void invalidInst(void)
{
    printf("The instruction in line number %d is INVALID.\n", k);
    exit(0);    // EXIT the interpreter
}

