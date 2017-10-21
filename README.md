# Compiler-Assembler-w-pipeline
Assembler that accepts hex assembly code and runs it!

To run this program, pull TCSS372_Final_Project.zip and run slc3Pipe.c

This program accepts hex assembly code and runs it. 
The simulated processor had 8 registers with a memory of 500 blocks and runs
based on a pipeline architecture. The defined opcodes are 
// instructions
ADD
AND
NOT
TRAP
LD
LDR
LDI
LEA
ST
STR
STI
JMP
JSRR
BR

// traps
HALT
GETC
OUT
PUTS

Some other notable features of this IDE are:
Load external .hex files
Ability to save
Set breakpoints
Step through code
Display Pipeline activity



Note:
To run with Ncurses, run view.c 
(This will also require modifications to slc3.c, slc3.h)
