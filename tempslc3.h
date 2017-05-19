// lc3.h
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <termios.h> 
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
#define LEA 14
#define ST 3
#define STR 7
#define JMP 12
#define JSRR 4
#define BR 0

// traps
#define HALT 0x25
#define GETC 0x20
#define OUT 0x21
#define PUTS 0x22

// debug actions
#define EXIT 9
#define RUN 7
#define EDIT 6
#define DISP_MEM 5
#define STEP 3
#define SAVE 2
#define LOAD 1
#define SET_BRKPTS 8

// etc.
#define SEXT6_SIGN_EXTEND 0xFFC0
#define SEXT6_MASK 0x003F
#define HIGH_ORDER_BIT_VALUE6 0x0020

#define CONVERT_TO_DECIMAL 0x2FFF

#define START_MEM 0x3000
#define BOTTOM_HALF 0x3008
#define DISP_BOUNDARY 17

#define OPCODE_SHIFT 12

#define DR_MASK 0x0FFF
#define DR_SHIFT 9

#define SR_MASK 0x01FF
#define SR_SHIFT 6

#define SR2_MASK 0x0007

#define BASE_MASK 0x01C0

#define TRAP_VECTOR_MASK 0x00FF

#define MAX_FILE_NAME 100
#define MAX_BREAKPOINTS 4
#define AVAILABLE_BRKPT 9999
#define SEXT9_SIGN_EXTEND 0xFE00
#define HIGH_ORDER_BIT_VALUE8 0x0080    // 0000 0000 1000 0000
#define HIGH_ORDER_BIT_VALUE9 0x0100

#define SEXT5_MASK 0x001F

#define MAX_MEMORY 500


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
} BUFFER, *BUFFER_p;

typedef struct CPU_s{
	Register r[8];
	Register A, B, Res;
	Register PC, ir;
	Register MAR, MDR;
	unsigned int N;
	unsigned int P;
	unsigned int Z;
	char gotC;
	unsigned short breakPoints[MAX_BREAKPOINTS];
	struct BUFFER buffers[MAX_BREAKPOINTS];
} CPU_s, *CPU_p;