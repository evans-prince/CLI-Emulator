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



int main(int argc, const char * argv[])
{
    
    k = 1;
    
    //Code for opening the file and computing the size of the program
    long int size;
    FILE* f = fopen(argv[1], "r");        // argv[1] is the name of the file in which the program is written
    if(f == 0)
    {
        printf("Could not read file!!!\n");
        exit(0);
    }
    else
    {
        fseek(f, 0, SEEK_END);    // fseek takes f to EOF
        size = ftell(f);        // ftell gives the position of f which is EOF and hence returns the length of the file
    }
    size = size + 2;            // We add 2 to accomodate for an extra '\n' and a '\0'
    
    
    
    // Code for storing the whole program into the string str
    str = malloc(size*(sizeof(char)));
    rewind(f);                    // Brings f to the beginning of the file
    int colon_count = 0;        // colon_count will contain the number of colons in the program which decides the maximum number of labels
    int line_count = 0;            // line_count will contain the number of lines in the program which decides the maximum number of instructions
    int i = 0;
    int x = fgetc(f);
    while(x != EOF)
    {
        if(x != 13)         // We neglect the carriage returns '/r' (ASCII 13) if present
        {
            if(x == '/')        // We neglect the multiline comments and donot add them to the string str
            {
                x = fgetc(f);
                if(x == '*')
                {
                    while(1)
                    {
                        x = fgetc(f);
                        if(x == EOF )
                        {
                            printf("Error: Unterminated Comment!\n");
                            exit(0);
                        }
                        else if (x == '*')
                        {
                            x = fgetc(f);
                            if( x == '/')
                            {
                                x=fgetc(f);
                                break;
                            }
                        }
                    }
                }
                else if(x == EOF)
                {
                    str[i++] = '/';
                    break;
                }
                else
                    str[i++] = '/';
            }
            if(x == EOF)
                break;
            str[i++] = x;
            if(x == '\n')
                line_count++;
            else if(x == ':')
                colon_count++;
        }
        x = fgetc(f);
    }
    str[i++] = '\n';
    str[i] = '\0';

    
    
    instructions = malloc(line_count*sizeof(struct instruction));
    labels = malloc(colon_count*sizeof(struct label));

    
    // Skipping empty lines
    x = 0;
    while(str[x] == '\n')
    {
        x++;
        k++;    // Incrementing the line number
    }
    i = x;        // Beginnning of the first non-empty line
    
    int j, t;
    pc = 0;
    lab_no = 0;
    while(str[x] != '\0')
    {
        if(str[x] == ':')    // to obtain the instruction which the label is identifying
        {
            t = x--;        // t stores the position of the colon
            while(str[x] != ' ' && str[x] != '\t' && str[x] != '\n')
            {
                x--;
                if(x < 0)
                    break;
            }
            labels[lab_no].i = x+1;
            labels[lab_no].j = t;
            j = x;
            x = t+1;
            while(str[x] == ' ' || str[x] == '\t')
            {
                ++x;
            }
            if(str[x] == '\n')    // label is pointing to the instruction in the next line
            {
                while(j > 0)
                {
                    if(str[j] != ' ' && str[j] != '\t')
                        break;
                    --j;
                }
                if(str[j] == '\n' || j == -1)
                    labels[lab_no++].inst_no = pc;        // There is no instruction in the line of this label
                else
                {
                    labels[lab_no++].inst_no = pc+1;    // There is an instruction before this label in the same line but label points to the next line
                    instructions[pc].i = i;
                    instructions[pc].j = j+1;
                    instructions[pc].line = k;
                    pc++;
                }
            }
            else
            {
                labels[lab_no++].inst_no = pc;        // Label points to an instruction in the same line
                instructions[pc].i = x;
                while(str[x] != '\n')
                    x++;
                instructions[pc].j = x;
                instructions[pc].line = k;
                pc++;
            }
            while(str[x] == '\n')    // Neglecting all blank lines after this label
            {
                x++;
                k++;    // Incrementing the line number
            }
            i = x;
        }
        else if(str[x] == '\n')            // This is the end of an instruction
        {
            j=x;
            instructions[pc].i = i;
            instructions[pc].j = j;
            instructions[pc].line=k;
            pc++;
            
            while(str[x] == '\n')        // Neglecting all blank lines after this instruction
            {
                x++;
                k++;    // Incrementing the line number
            }
            i = x;
        }
        else
            x++;
    }
    int inst_count = pc;
    lab_count = lab_no;
    setPcForMain();
    reg[14] = 4092;        // setting the stack pointer to 0xFFC (the end of the memory)
    while(pc < inst_count)
    {
        k = instructions[pc].line;    // Line number of the current pc
        executeInstruction();        // Executes the instruction corresponding to the current pc
        pc++;
    }
    
    return 0;
}


void getReg3Add(char* inst, int i) {
    
    
    // Code to extract rd registor number
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    if(inst[i] == 's') {
        i++;
        
        if(inst[i+1] == 'p') {
            rd = 14;
            i++;
        }else {
            invalidInst();
        }
        
    }else if ( inst[i] == 'r') {
        i++;
        if( inst[i] == 'a') {
            rd = 15;
            i++;
        }else if ( isdigit(inst[i])) {
            rd = inst[i] - '0';
            i++;
            if(isdigit(inst[i])) {
                rd = ((rd *10) + (inst[i] - '0' ));
                i++;
            }
            
            if(rd <0 || rd > 15 ) {
                invalidInst();
            }
        }else {
            invalidInst();
        }
    }else {
        invalidInst();
    }
    
    
    
   // Code to extract rs1 registor number
    
    
     while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    if(inst[i] != ','){
        invalidInst();
    }
    i++;
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
     
    if(inst[i] == 's') {
        i++;
        
        if(inst[i+1] == 'p') {
            rs1 = 14;
            i++;
        }else {
            invalidInst();
        }
        
    }else if ( inst[i] == 'r') {
        i++;
        if( inst[i] == 'a') {
            rs1 = 15;
            i++;
        }else if ( isdigit(inst[i])) {
            rs1 = inst[i] - '0';
            i++;
            if(isdigit(inst[i])) {
                rs1 = ((rs1 *10) + (inst[i] - '0' ));
                i++;
            }
            
            if(rs1 <0 || rs1 > 15 ) {
                invalidInst();
            }
        }else {
            invalidInst();
        }
    }else {
        invalidInst();
    }
    

    
    // Code to extract rs2 / imm
    
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    if(inst[i] != ','){
        invalidInst();
    }
    i++;
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    
    // if it is rs2
    if(inst[i] == 's') {
        i++;
        
        if(inst[i+1] == 'p') {
            rs2 = 14;
            isImm = 0; // This instruction donot contain any immediate
            i++;
        }else {
            invalidInst();
        }
        
    }else if ( inst[i] == 'r') {
        i++;
        if( inst[i] == 'a') {
            rs2 = 15;
            isImm =0;
            i++;
        }else if ( isdigit(inst[i])) {
            rs2 = inst[i] - '0';
            i++;
            if(isdigit(inst[i])) {
                rs2 = ((rs2 *10) + (inst[i] - '0' ));
                i++;
            }
            isImm = 0;
            
            if(rd <0 || rd > 15 ) {
                invalidInst();
            }
        }else {
            invalidInst();
        }
    }else if (inst[i] == '0' && inst[i+1] == 'x'){ // It is a immediate in hexadeximal format
        i+=2;
        // We can have 16 bits signed immediate
        // Hence we will only have upto 4 places in hex format as each digit can be written in 4 bits in binary
        
        while(inst[i] == ' ' || inst[i] == '\t') {
            i++;
        }
        
        if(inst[i] == '\0'){
            invalidInst();
        }
        
        int hexIndex = 0;
        
        while(inst[i] != '\0' && hexIndex < 4) {
            
            hexImm[hexIndex++] = dec(inst[i++]);
            while(inst[i] == ' ' || inst[i] == '\t'){
                i++;
            }
            
        }
        
        if(inst[i] != '\0'){
            invalidInst();
        }
        
        imm = 0;
        int q = 0;
        while(q < hexIndex){
            imm = 16*imm + hexImm[q++];
        }
        
        if(hexIndex == 4) {
          // m =0 for default , 1 for u (unsigned) , 2 for h ( half upper )
            if(m != 1 && hexImm[0] >= 8)
                // If m is not unsigned that is signed and if hexadecimal starts with 8 ( 4 word hex) then it is negative
                imm -= 65536; // So we subtract 2^16 = 65536 from it to make it signed
        }
        
        if(m == 2){
           // Because in C if we do << behaviour of signed number can change so it is not safe so we will do multiply
            imm *= 65536; // Multiplying with 2^16 is same as imm << 16
        }
        isImm = 1;
    }else if (isdigit(inst[i])) { // Immediate is a positive number
        imm = 0;
        
        while(isdigit(inst[i])) {
            
            imm = imm*10 + (inst[i] - '0');
            i++;
        }
        if(imm > 65535){
            
            invalidInst();         // The immediate cannot be greater than 16 bits and largest 16 bit unsigned number is 0xFFFF = 65535
        }
        
        if(m == 0 || m == 2) {
            
            if(imm > 32767)
                invalidInst();    //The largest positive number in 16 bit signed numbers is 0x7FFF = 32767 ( 2^15  -1  = 32768-1)
        }
        
        if(m == 2) {
            
            imm *= 65536;
        }
        
        isImm = 1;
    }else if ( inst[i] == '-'){ // Immediate is a negative number
        
        i++;
        
        while( inst[i] == ' ' || inst[i] == '\t') {
            i++;
        }
        
        imm =0;
        while(isdigit(inst[i])) {
            
            imm = imm *10 + ( inst[i] - '0' );
            i++;
        }
        
        imm*=-1;
        
        if(imm < -32768){
            invalidInst(); // Largest negative number in 16 bit signed number is 32768
        }
        
        if(m==1){
            imm += 65536; // adding 2^16 to treat it as unsigned integer
        }
       
        
        if( m == 2 ) {
            
            imm *=65536;
        }
        
        isImm = 1;
    }else {
        invalidInst();
    }
    


}


void getReg2Add(char* inst, int i){
        
    // Code to extract rd registor number
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    if(inst[i] == 's') {
        i++;
        
        if(inst[i+1] == 'p') {
            rd = 14;
            i++;
        }else {
            invalidInst();
        }
        
    }else if ( inst[i] == 'r') {
        i++;
        if( inst[i] == 'a') {
            rd = 15;
            i++;
        }else if ( isdigit(inst[i])) {
            rd = inst[i] - '0';
            i++;
            if(isdigit(inst[i])) {
                rd = ((rd *10) + (inst[i] - '0' ));
                i++;
            }
            
            if(rd <0 || rd > 15 ) {
                invalidInst();
            }
        }else {
            invalidInst();
        }
    }else {
        invalidInst();
    }
    
    
     
    // Code to extract rs2 / imm
    
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    if(inst[i] != ','){
        invalidInst();
    }
    i++;
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    
    // if it is rs2
    if(inst[i] == 's') {
        i++;
        
        if(inst[i+1] == 'p') {
            rs2 = 14;
            isImm = 0; // This instruction donot contain any immediate
            i++;
        }else {
            invalidInst();
        }
        
    }else if ( inst[i] == 'r') {
        i++;
        if( inst[i] == 'a') {
            rs2 = 15;
            isImm =0;
            i++;
        }else if ( isdigit(inst[i])) {
            rs2 = inst[i] - '0';
            i++;
            if(isdigit(inst[i])) {
                rs2 = ((rs2 *10) + (inst[i] - '0' ));
                i++;
            }
            isImm = 0;
            
            if(rd <0 || rd > 15 ) {
                invalidInst();
            }
        }else {
            invalidInst();
        }
    }else if (inst[i] == '0' && inst[i+1] == 'x'){ // It is a immediate in hexadeximal format
        i+=2;
        // We can have 16 bits signed immediate
        // Hence we will only have upto 4 places in hex format as each digit can be written in 4 bits in binary
        
        while(inst[i] == ' ' || inst[i] == '\t') {
            i++;
        }
        
        if(inst[i] == '\0'){
            invalidInst();
        }
        
        int hexIndex = 0;
        
        while(inst[i] != '\0' && hexIndex < 4) {
            
            hexImm[hexIndex++] = dec(inst[i++]);
            while(inst[i] == ' ' || inst[i] == '\t'){
                i++;
            }
            
        }
        
        if(inst[i] != '\0'){
            invalidInst();
        }
        
        imm = 0;
        int q = 0;
        while(q < hexIndex){
            imm = 16*imm + hexImm[q++];
        }
        
        if(hexIndex == 4) {
          // m =0 for default , 1 for u (unsigned) , 2 for h ( half upper )
            if(m != 1 && hexImm[0] >= 8)
                // If m is not unsigned that is signed and if hexadecimal starts with 8 ( 4 word hex) then it is negative
                imm -= 65536; // So we subtract 2^16 = 65536 from it to make it signed
        }
        
        if(m == 2){
           // Because in C if we do << behaviour of signed number can change so it is not safe so we will do multiply
            imm *= 65536; // Multiplying with 2^16 is same as imm << 16
        }
        isImm = 1;
    }else if (isdigit(inst[i])) { // Immediate is a positive number
        imm = 0;
        
        while(isdigit(inst[i])) {
            
            imm = imm*10 + (inst[i] - '0');
            i++;
        }
        if(imm > 65535){
            
            invalidInst();         // The immediate cannot be greater than 16 bits and largest 16 bit unsigned number is 0xFFFF = 65535
        }
        
        if(m == 0 || m == 2) {
            
            if(imm > 32767)
                invalidInst();    //The largest positive number in 16 bit signed numbers is 0x7FFF = 32767 ( 2^15  -1  = 32768-1)
        }
        
        if(m == 2) {
            
            imm *= 65536;
        }
        
        isImm = 1;
    }else if ( inst[i] == '-'){ // Immediate is a negative number
        
        i++;
        
        while( inst[i] == ' ' || inst[i] == '\t') {
            i++;
        }
        
        imm =0;
        while(isdigit(inst[i])) {
            
            imm = imm *10 + ( inst[i] - '0' );
            i++;
        }
        
        imm*=-1;
        
        if(imm < -32768){
            invalidInst(); // Largest negative number in 16 bit signed number is 32768
        }
        
        if(m==1){
            imm += 65536; // adding 2^16 to treat it as unsigned integer
        }
       
        
        if( m == 2 ) {
            
            imm *=65536;
        }
        
        isImm = 1;
    }else {
        invalidInst();
    }
    
 
}



void getLdSt(char* inst, int i) { // instruction is of the form rd, imm[rs1]
   
    // Code to extract rd registor number
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    if(inst[i] == 's') {
        i++;
        
        if(inst[i+1] == 'p') {
            rd = 14;
            i++;
        }else {
            invalidInst();
        }
        
    }else if ( inst[i] == 'r') {
        i++;
        if( inst[i] == 'a') {
            rd = 15;
            i++;
        }else if ( isdigit(inst[i])) {
            rd = inst[i] - '0';
            i++;
            if(isdigit(inst[i])) {
                rd = ((rd *10) + (inst[i] - '0' ));
                i++;
            }
            
            if(rd <0 || rd > 15 ) {
                invalidInst();
            }
        }else {
            invalidInst();
        }
    }else {
        invalidInst();
    }
    
    
     // Code to extract registor number of immediate
    
    // Code to extract imm
    
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    if(inst[i] != ','){
        invalidInst();
    }
    i++;
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    
    if( inst[i] == '['){// if we are using just registor indirect mode of addressing
        
        imm =0;
        
    } else if (inst[i] == '0' && inst[i+1] == 'x'){ // It is a immediate in hexadeximal format
        i+=2;
        // We can have 16 bits signed immediate
        // Hence we will only have upto 4 places in hex format as each digit can be written in 4 bits in binary
        
        while(inst[i] == ' ' || inst[i] == '\t') {
            i++;
        }
        
        if(inst[i] == '[' || inst[i] == '\0') {
            invalidInst();
        }
        
        int hexIndex = 0;
        
        while(inst[i] != '[' && hexIndex < 4) {
            
            hexImm[hexIndex++] = dec(inst[i++]);
            while(inst[i] == ' ' || inst[i] == '\t'){
                i++;
            }
            
        }
        
        if(inst[i] != '['){
            invalidInst();
        }
        
        imm = 0;
        int q = 0;
        while(q < hexIndex){
            imm = 16*imm + hexImm[q++];
        }
        
        if(hexIndex == 4) {
            if(hexImm[0] >= 8)
                
                // we dont allow modifier in imm[rs1]
                
                // If m is not unsigned that is signed and if hexadecimal starts with 8 ( 4 word hex) then it is negative
                imm -= 65536; // So we subtract 2^16 = 65536 from it to make it signed
        }
        
        // we dont allow modifier in imm[rs1]
        
//        if(m == 2){
//           // Because in C if we do << behaviour of signed number can change so it is not safe so we will do multiply
//            imm *= 65536; // Multiplying with 2^16 is same as imm << 16
//        }
//        isImm = 1;
    }else if (isdigit(inst[i])) { // Immediate is a positive number
        imm = 0;
        
        while(isdigit(inst[i])) {
            
            imm = imm*10 + (inst[i] - '0');
            i++;
        }
        
        // We dont allow unsigned immediate in form imm[rs1]
        
//        if(imm > 65535){
//            
//            invalidInst();         // The immediate cannot be greater than 16 bits and largest 16 bit unsigned number is 0xFFFF = 65535
//        }
//        
//        if(m == 0 || m == 2) {
//            
        if(imm > 32767){
            
            invalidInst();    //The largest positive number in 16 bit signed numbers is 0x7FFF = 32767 ( 2^15  -1  = 32768-1)
        }
//        }
        
//        if(m == 2) {
//            
//            imm *= 65536;
//        }
        
//        isImm = 1;
    }else if ( inst[i] == '-'){ // Immediate is a negative number
        
        i++;
        
        while( inst[i] == ' ' || inst[i] == '\t') {
            i++;
        }
        
        imm =0;
        while(isdigit(inst[i])) {
            
            imm = imm *10 + ( inst[i] - '0' );
            i++;
        }
        
        imm*=-1;
        
        if(imm < -32768){
            invalidInst(); // Largest negative number in 16 bit signed number is 32768
        }
        
//        if(m==1){
//            imm += 65536; // adding 2^16 to treat it as unsigned integer
//        }
       
        
//        if( m == 2 ) {
//            
//            imm *=65536;
//        }
//        
//        isImm = 1;
    }else {
        invalidInst();
    }
    
    
    // Code to extract registor number of rs1
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
    if(inst[i] != '[') {
        invalidInst();
    }
    i++;
    
    // Code to extract rs1 registor number
     
     
    
     while(inst[i] == ' ' || inst[i] == '\t') {
         i++;
     }
      
     if(inst[i] == 's') {
         i++;
         
         if(inst[i+1] == 'p') {
             rs1 = 14;
             i++;
         }else {
             invalidInst();
         }
         
     }else if ( inst[i] == 'r') {
         i++;
         if( inst[i] == 'a') {
             rs1 = 15;
             i++;
         }else if ( isdigit(inst[i])) {
             rs1 = inst[i] - '0';
             i++;
             if(isdigit(inst[i])) {
                 rs1 = ((rs1 *10) + (inst[i] - '0' ));
                 i++;
             }
             
             if(rs1 <0 || rs1 > 15 ) {
                 invalidInst();
             }
         }else {
             invalidInst();
         }
     }else {
         invalidInst();
     }
    
    
    while(inst[i] == ' ' || inst[i] == '\t') {
        i++;
    }
     
    if(inst[i] != ']') {
        invalidInst();
    }
    
    return;
    
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


