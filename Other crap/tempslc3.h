// lc3.h
#ifndef TEMPSLC3
#define TEMPSLC3 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <termios.h> 
#include <string.h>
//#include <ncurses.h>

// states
#define FETCH 0
#define IDRR 1
#define EXECUTE 2
#define MEM 3
#define STORE 4


// buffers
#define FBUFF 0
#define DBUFF 1
#define EBUFF 2
#define MBUFF 3

// instructions
#define ADD 1
#define AND 5
#define NOT 9
#define TRAP 15
#define LD 2
#define LDR 6
#define LDI 10
#define LEA 14
#define ST 3
#define STR 7
#define STI 11
#define JMP 12
#define JSRR 4
#define BR 0
#define PP 13
#define NOP 16

// traps
#define HALT 0x25
#define GETC 0x20
#define OUT 0x21
#define PUTS 0x22

// debug actions 
#define EXIT 8
#define RUN 6
#define EDIT 5
#define DISP_MEM 4
#define STEP 3
#define SAVE 2
#define LOAD 1
#define SET_BRKPTS 7

// etc.
#define SEXT6_SIGN_EXTEND 0xFFC0
#define SEXT6_MASK 0x003F
#define HIGH_ORDER_BIT_VALUE6 0x0020

#define HIGH_ORDER_BIT_VALUE5 0x0010
#define SEXT5_SIGN_EXTEND 0xFFE0
#define SEXT5_MASK 0x001F

#define SEXT11_SIGN_EXTEND 0xF800
#define SEXT11_MASK 0x07FF

//may need to change this
#define CONVERT_TO_DECIMAL 0x3000

#define START_MEM 0x3000
#define BOTTOM_HALF 8
#define DISP_BOUNDARY 17

#define OPCODE_SHIFT 12

#define DR_MASK 0x0FFF
#define DR_SHIFT 9

#define SR_MASK 0x01FF
#define SR_SHIFT 6

#define SR2_MASK 0x0007

#define BASE_MASK 0x01C0

#define TRAP_VECTOR_MASK 0x00FF

#define MAX_FILE_NAME 12
#define MAX_BREAKPOINTS 4
#define MAX_BUFFERS 4
#define PREFETCH_SIZE 8
#define AVAILABLE_BRKPT 0x9999
#define SEXT9_SIGN_EXTEND 0xFE00
#define HIGH_ORDER_BIT_VALUE8 0x0080    // 0000 0000 1000 0000
#define HIGH_ORDER_BIT_VALUE9 0x0100
#define HIGH_ORDER_BIT_VALUE15 0x8000
#define HIGH_ORDER_BIT_VALUE12 0x0800
#define HIGH_ORDER_BIT_VALUE11 0x0400
#define PUSH_POP_BIT_MASK 0x0020

#define ENTER_KEY 10

#define MAX_MEMORY 500

unsigned short memory[MAX_MEMORY];   // 500 words of memory enough to store simple program


typedef unsigned short Register;
//cpu a b res mar mdr
// lc3.c

typedef struct BUFFER {
	Register PC;
	Register IR;
	Register Rd;
	Register Opcode;
	Register A;		//16-bit value from Rs
	Register B;		//16-bit value from either Rs2 or SEXT(immed)
	Register SEXT;
	char stalled;
	int isStalled;
	int N;
	int Z;
	int P;
}BUFF;

typedef struct ALU {
	Register A;
	Register B;
	Register R;
} ALU_s;

typedef struct PREFETCH {
	Register values[PREFETCH_SIZE];
	int head;
	int nopCount;
	int stepCounter;
	Register PC;
	int collisionFound;
	int isEmpty;
} PRE;

typedef struct CPU_s{
	Register r[8];
	Register A, B, Res;
	Register PC, ir;
	Register MAR, MDR;
	int reachedInput, hasAccessedMem, memStepCount;
	ALU_s alu;
	unsigned int N;
	unsigned int P;
	unsigned int Z;
	char gotC;
	unsigned short breakPoints[MAX_BUFFERS];
	struct BUFFER buffers[MAX_BREAKPOINTS];
	struct PREFETCH prefetch;
} CPU_s, *CPU_p;


int controller(CPU_p, int);




#endif