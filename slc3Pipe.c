/*
	This is our very own LC-3 Pipeline Simulator! 

	Authors: Connor Lundberg, Daniel Ivanov, James Roberts, Joe Fried
    Date: 5/30/2017
*/
#include "slc3Pipe.h"

int controller (CPU_p, int);

int displayScreen (CPU_p, int, int, int, int, int, char *);

int dialog (CPU_p cpu);

char getch ();

void setFlags (CPU_p, unsigned int, unsigned int, unsigned int);

int encounteredBreakPoint (CPU_p, Register);

void editBreakPoint (CPU_p);

void writeMemory(char * fileToWriteToName);


int isLoaded;
int memShift;
int startMem;


/*
	This function is the offset6 sign extender.
*/
int sext6(int offset6) {
	if (HIGH_ORDER_BIT_VALUE6 & offset6) return (offset6 | SEXT6_SIGN_EXTEND);
	else return offset6 & SEXT6_MASK;
}


/*
	This function is the offset11 sign extender.
*/
int sext11(int offset11) {
	if (HIGH_ORDER_BIT_VALUE11 & offset11) return (offset11 | SEXT11_SIGN_EXTEND);
	else return offset11 & SEXT11_MASK;
}


/*
	This function is the offset9 sign extender.
*/
int sext9(int offset9) {
	if (HIGH_ORDER_BIT_VALUE9 & offset9) return (offset9 | SEXT9_SIGN_EXTEND);
	else return offset9;
}

/*
	This fucntion is the immed5 sign extender.
*/
int sext5(int immed5) {
	if (HIGH_ORDER_BIT_VALUE5 & immed5) return (immed5 | SEXT5_SIGN_EXTEND);
	else return (immed5 & SEXT5_MASK);
}


// This is the trap function that handles trap vectors. Acts as 
// the trap vector table for now. Currently exits the HALT trap command.
int trap(CPU_p cpu, int trap_vector) {
	int value = 0;
	int i = 0;
	char temp;
	char charToPrint, carriageReturn;
	switch (trap_vector) {
		case GETC:
			if (!cpu->reachedInput) {
				printf("Input:  ");
				cpu->reachedInput = 1;
			}
			
			value = (int) getch();
			break;
		case OUT:
			printf("%c", cpu->gotC);
			//putchar(cpu->gotC);
			break;
		case PUTS:
			cpu->reachedInput = 0;
			i = 0;
			temp = (char ) memory[(cpu->r[0] - startMem + i)];
			printf("\n\nOutput:  ");
			while ((temp)) {  
			  printf("%c", (temp));
			  i++;
			  temp = memory[(cpu->r[0] - startMem + i)];
			}
			//printf("\n");
			break;
		case HALT:
			value = 1;
			break;
	}
	return value;
}


/*
	This is a helper function to choose which CC values to set (N, Z, or, P)
*/
void setCC(CPU_p cpu, Register Rd) {
	if (Rd == 0) {
		cpu->N = 0;
		cpu->Z = 1;
		cpu->P = 0;
	} else if (Rd & HIGH_ORDER_BIT_VALUE8) { // 0000 0000 1000 0000
		cpu->N = 1;
		cpu->Z = 0;
		cpu->P = 0;
	} else {
		cpu->N = 0;
		cpu->Z = 0;
		cpu->P = 1;
	}
}


/*
	This function simulates the GETC trap command in assembly.
*/
char getch() {
	char buf = 0;         
	struct termios old = {0};         
	if (tcgetattr(0, &old) < 0)                 
		perror("tcsetattr()");         
	old.c_lflag &= ~ICANON;         
	old.c_lflag &= ~ECHO;         
	old.c_cc[VMIN] = 1;         
	old.c_cc[VTIME] = 0;         
	if (tcsetattr(0, TCSANOW, &old) < 0)                 
		perror("tcsetattr ICANON");        
	if (read(0, &buf, 1) < 0)                 
		perror ("read()");         
	old.c_lflag |= ICANON;         
	old.c_lflag |= ECHO;         
	if (tcsetattr(0, TCSADRAIN, &old) < 0)                 
		perror ("tcsetattr ~ICANON");         
	return (buf); 
}


/*
	A helper function to print the breakpoint symbol. This is used in displayScreen to keep
	from having to use the same if else statement every time we choose to print the * character.
*/
void printBreakpoint(CPU_p cpu, Register pc) {
	if (encounteredBreakPoint(cpu, pc)) {
		printf("*\n");
	} else {
		printf("\n");
	}
}

/*
	This function displays the debug screen with LOAD, SAVE, STEP, DISPLAY MEM, EDIT, RUN, (UN)SET BREAKPTS, or EXIT
	commands to use.
*/
int displayScreen(CPU_p cpu, int mem, int isRunning, int stepCount, int nopCount, int collisionFound, char *stage) {
	for (int i = 0; i < 5; i++) {
		printf("\n");
	}
	
	if (!isRunning) {
		printf("\t\tWelcome to the LC-3 Pipeline Simulator\n\n");
	}
	
	int i = startMem + mem;
	if (cpu->buffers[0].IR != NOP) {
		printf("Registers          ______________\t\t\t\t    Memory\n");
		printf("R%d: x%04X         |              |\t\t\t\t x%X: x%04X", 0, cpu->r[0], i, memory[mem]);
		printBreakpoint(cpu, i);
		printf("R%d: x%04X    FBUFF|  PC:    IR:  | %c\t\t\t\t x%X: x%04X", 1, cpu->r[1], cpu->buffers[0].stalled, i+1, memory[1 + mem]);
		printBreakpoint(cpu, i+1);
		printf("R%d: x%04X         | x%04X  x%04X |\t\t\t\t x%X: x%04X", 2, cpu->r[2], cpu->buffers[0].PC, cpu->buffers[0].IR, i+2, memory[2 + mem]);
		printBreakpoint(cpu, i+2);
		printf("R%d: x%04X         |______________|\t\t\t\t x%X: x%04X", 3, cpu->r[3], i+3, memory[3 + mem]);
		printBreakpoint(cpu, i+3);
	} else {
		printf("Registers          _______\t\t\t\t\t    Memory\n");
		printf("R%d: x%04X         |       |\t\t\t\t\t x%X: x%04X", 0, cpu->r[0], i, memory[mem]);
		printBreakpoint(cpu, i);
		printf("R%d: x%04X    FBUFF|  NOP  | %c\t\t\t\t\t x%X: x%04X", 1, cpu->r[1], cpu->buffers[0].stalled, i+1, memory[1 + mem]);
		printBreakpoint(cpu, i+1);
		printf("R%d: x%04X         |       |\t\t\t\t\t x%X: x%04X", 2, cpu->r[2], i+2, memory[2 + mem]);
		printBreakpoint(cpu, i+2);
		printf("R%d: x%04X         |_______|\t\t\t\t\t x%X: x%04X", 3, cpu->r[3], i+3, memory[3 + mem]);
		printBreakpoint(cpu, i+3);
	
	}
	
	if (cpu->buffers[1].IR != NOP) {
		printf("R%d: x%04X          _______________________________________\t x%X: x%04X", 4, cpu->r[4], i+4, memory[4 + mem]);
		printBreakpoint(cpu, i+4);
		printf("R%d: x%04X         |                                       |\t x%X: x%04X", 5, cpu->r[5], i+5, memory[5 + mem]);
		printBreakpoint(cpu, i+5);
		printf("R%d: x%04X    DBUFF|  PC:    OP:    DR:    OPN1:    OPN2:  | %c\t x%X: x%04X", 6, cpu->r[6], cpu->buffers[1].stalled, i+6, memory[6 + mem]);
		printBreakpoint(cpu, i+6);
		printf("R%d: x%04X         | x%04X  x%04X  x%04X   x%04X    x%04X  |\t x%X: x%04X", 7, cpu->r[7], cpu->buffers[1].PC, cpu->buffers[1].Opcode, cpu->buffers[1].Rd, cpu->buffers[1].A, cpu->buffers[1].B, i+7, memory[7 + mem]);
		printBreakpoint(cpu, i+7);
		i = startMem + mem + BOTTOM_HALF; // replace i with the mem dump number if you want.
		printf("                  |_______________________________________|\t x%04X: x%04X", i, memory[8 + mem]);
		printBreakpoint(cpu, i);
	} else {
		printf("R%d: x%04X          _______\t\t\t\t\t x%X: x%04X", 4, cpu->r[4], i+4, memory[4 + mem]);
		printBreakpoint(cpu, i+4);
		printf("R%d: x%04X         |       |\t\t\t\t\t x%X: x%04X", 5, cpu->r[5], i+5, memory[5 + mem]);
		printBreakpoint(cpu, i+5);
		printf("R%d: x%04X    DBUFF|  NOP  | %c\t\t\t\t\t x%X: x%04X", 6, cpu->r[6], cpu->buffers[1].stalled, i+6, memory[6 + mem]);
		printBreakpoint(cpu, i+6);
		printf("R%d: x%04X         |       |\t\t\t\t\t x%X: x%04X", 7, cpu->r[7], i+7, memory[7 + mem]);
		printBreakpoint(cpu, i+7);
		i = startMem + mem + BOTTOM_HALF; // replace i with the mem dump number if you want.
		printf("                  |_______|\t\t\t\t\t x%04X: x%04X", i, memory[8 + mem]);
		printBreakpoint(cpu, i);
	}
	
	if (cpu->buffers[2].IR != NOP) {
		printf("                   _______________________________________\t x%04X: x%04X", i+1, memory[9 + mem]);
		printBreakpoint(cpu, i+1);
		printf("                  |                                       |\t x%04X: x%04X", i+2, memory[10 + mem]);
		printBreakpoint(cpu, i+2);
		printf("             EBUFF|  PC:    OP:    DR:    OPN1:    OPN2:  | %c\t x%04X: x%04X", cpu->buffers[2].stalled, i+3, memory[11 + mem]);
		printBreakpoint(cpu, i+3);
		printf("                  | x%04X  x%04X  x%04X   x%04X    x%04X  |\t x%04X: x%04X", cpu->buffers[2].PC, cpu->buffers[2].Opcode, cpu->buffers[2].Rd, cpu->buffers[2].A, cpu->buffers[2].B, i+4, memory[12 + mem]);
		printBreakpoint(cpu, i+4);
		printf("                  |_______________________________________|\t x%04X: x%04X", i+5, memory[13 + mem]);
		printBreakpoint(cpu, i+5);
	} else {
		printf("                   _______\t\t\t\t\t x%04X: x%04X", i+1, memory[9 + mem]);
		printBreakpoint(cpu, i+1);
		printf("                  |       |\t\t\t\t\t x%04X: x%04X", i+2, memory[10 + mem]);
		printBreakpoint(cpu, i+2);
		printf("             EBUFF|  NOP  | %c\t\t\t\t\t x%04X: x%04X", cpu->buffers[2].stalled, i+3, memory[11 + mem]);
		printBreakpoint(cpu, i+3);
		printf("                  |       |\t\t\t\t\t x%04X: x%04X", i+4, memory[12 + mem]);
		printBreakpoint(cpu, i+4);
		printf("                  |_______|\t\t\t\t\t x%04X: x%04X", i+5, memory[13 + mem]);
		printBreakpoint(cpu, i+5);
	}
	
	if (cpu->buffers[3].IR != NOP) {
		printf("                   _______________________________________\t x%04X: x%04X", i+6, memory[14 + mem]);
		printBreakpoint(cpu, i+6);
		printf("                  |                                       |\t x%04X: x%04X", i+7, memory[15 + mem]);
		printBreakpoint(cpu, i+7);
		printf("             MBUFF|  PC:    OP:    DR:    OPN1:    OPN2:  | %c\n", cpu->buffers[3].stalled);
		printf("                  | x%04X  x%04X  x%04X   x%04X    x%04X  |\n", cpu->buffers[3].PC, cpu->buffers[3].Opcode, cpu->buffers[3].Rd, cpu->buffers[3].A, cpu->buffers[3].B);
		printf("                  |_______________________________________|\n\n");
	} else {
		printf("                   _______\t\t\t\t\t x%04X: x%04X\n", i+6, memory[14 + mem]);
		printf("                  |       |\t\t\t\t\t x%04X: x%04X\n", i+7, memory[15 + mem]);
		printf("             MBUFF|  NOP  | %c\n", cpu->buffers[3].stalled);
		printf("                  |       |\n");
		printf("                  |_______|\n\n");
	
	}
	
	if (!cpu->hasAccessedMem) {
		printf("\t\tStep: %d  NOP Count: %d\n\n", stepCount, nopCount);
	} else {
		printf("\t\tStep: %d  Memory has been accessed! NOP Count: %d\n\n", stepCount, cpu->memStepCount);
	}
	
	if (collisionFound) {
		printf("Collision Detected!\n");
	} else {
		printf("\n");
	}
	
	if (isRunning) {
		printf("Current Stage: %s\n", stage);
	} else {
		printf("\n");
	}

	if (!isRunning) {
		printf("\n1) Load,  2) Save,  3) Step,  4) Display Memory,  5) Edit,  6) Run,  7) (Un)Set Breakpts,  8) Exit\n");
	} else {
		printf("\nPress enter to step or 'q' to quit ");
	}
	return 0;
}


/*
	This is the dialog function that provides the functionality to the choices shown in
	the displayScreen.
*/
int dialog(CPU_p cpu) {
	int opNum = 0, isRunning = 0;
	unsigned int placeInMemory;
	unsigned short newValue; 
	int dontDeleteThisGarbage = 0;
	char newMemoryValue[4];
	char * charPtr;
	char fileName[MAX_FILE_NAME];
	FILE* inputFile;
		while (opNum != EXIT) {
			scanf("%d", &opNum);
			switch (opNum) {
				case LOAD:
					printf("File Name: ");
					scanf("%s", &fileName);
					inputFile = fopen(fileName, "r");
					if (inputFile == NULL) {
						printf("DIDN'T OPEN");
						displayScreen(cpu, 0, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
						break;
					}
					int i = 0;
					fscanf(inputFile, "%04X", &startMem);
					while (fscanf(inputFile, "%04X", &memory[i]) != EOF) {
						i++;
					}
					isLoaded = 1;
					displayScreen(cpu, 0, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
					fclose(inputFile);
					break;
				case SAVE:
					printf("Enter a file name to save to: ");
					scanf("%s", &fileName);
					writeMemory(fileName);
					displayScreen(cpu, 0, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);					
					break;
				case STEP:
					if (isLoaded == 1) {
						controller(cpu, 0);
						displayScreen(cpu, 0, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
						opNum = 0;
					} else {
						printf("No file loaded!");
						displayScreen(cpu, 0, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
					}
					break;
				case DISP_MEM:
					printf("Position to move to? (in hex): ");
					scanf("%4x", &memShift);
					if(memShift - startMem > MAX_MEMORY - DISP_BOUNDARY) {
						printf("Error: out of memory");
						memShift = 0;
						displayScreen(cpu, memShift - startMem, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
						break;
					} else {
						displayScreen(cpu,memShift - startMem, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
					}
					break;
				case EDIT:
					printf("What memory address would you like to edit: ");
					scanf("%04X", &placeInMemory);
					printf("The contents of location 0x%04X is  0x%04X\n", placeInMemory, memory[placeInMemory - startMem]);
					printf("What would you like the new value in location 0x%04X to be: ", placeInMemory);
					scanf("%s", &newMemoryValue);
					printf("%s\n", newMemoryValue);
					newValue = (short)strtol(newMemoryValue, &charPtr, 16);
					memory[placeInMemory - startMem] = newValue;
					displayScreen(cpu, 0, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
					break;
				case RUN:
					controller(cpu, 1);
					displayScreen(cpu, 0, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
					break;
				case EXIT:
					printf("Simulation Terminated.");
					break;
				case SET_BRKPTS:
					editBreakPoint(cpu);
					displayScreen(cpu, 0, 0, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, charPtr);
					break;
			}
		}
}


/*
	This method compares the current value of PC against any breakpoints entered by the user. 
*/
int encounteredBreakPoint(CPU_p cpu, Register pc) {
		int encountered = 0;
		int i;
		for (i = 0; i < MAX_BREAKPOINTS; i++) {
			if(pc == cpu->breakPoints[i]) {
				encountered = 1;
				i = MAX_BREAKPOINTS;
			}
		}
		return encountered; 
}


/*
	This function takes a passed breakPoint and searches the existing collection of breakpoints.
	If a match is found the breakPoint is removed from the collection. If a match isn't found the
	breakPoint is added. 
*/
void editBreakPoint(CPU_p cpu) {
	unsigned int breakPoint;
	printf("Enter Memory location of desired breakpoint: ");
	scanf("%04X", &breakPoint);
	int i;
	int found = 0;
	for(i = 0; i < MAX_BREAKPOINTS; i++) {
		//User wants to remove this breakpoint
		if (cpu->breakPoints[i] == breakPoint) {
			//Set spot to available value, set found variable and exit the loop;
			cpu->breakPoints[i] = AVAILABLE_BRKPT;
			i = MAX_BREAKPOINTS;
			found = 1;
		}		
	}
	
	//If this address doesn't exist find first available spot and add it to the collection of breakpoints
	if(!found) {
		for(i = 0; i < MAX_BREAKPOINTS; i++) {
			//found the first open spot
			if (cpu->breakPoints[i] == AVAILABLE_BRKPT) {
				//Add in new break point and exit the loop;
				cpu->breakPoints[i] = breakPoint;
				i = MAX_BREAKPOINTS;
			}		
		}
	}
}


/*
	A helper function to print a single buffer. Used in debugging.
*/
void printBuffer(CPU_p cpu, struct BUFFER buff) {
	if (buff.PC == NOP) printf("PC = NOP\n");
	else printf ("PC = %04X\n", buff.PC);
	
	if (buff.IR == NOP) printf("IR = NOP\n");
	else printf ("IR = %04X\n", buff.IR);
	
	if (buff.Rd == NOP) printf("Rd = NOP\n");
	else printf("Rd = %d\n", buff.Rd);
	
	if (buff.Opcode == NOP) printf("Opcode = NOP\n");
	else printf("Opcode = %d\n", buff.Opcode);
	
	if (buff.A == NOP) printf("A = NOP\n");
	else printf("A = %04X\n", buff.A);
	
	if (buff.B == NOP) printf("B = NOP\n");
	else printf("B = %04X\n", buff.B);
	
	if (buff.SEXT == NOP) printf("SEXT = NOP\n");
	else printf("SEXT = %04X\n", buff.SEXT);
	
	printf("General CCs:\n");
	printf("N = %d\n", cpu->N);
	printf("Z = %d\n", cpu->Z);
	printf("P = %d\n", cpu->P);
}


/*
	A helper function to print all of the buffers. Used in debugging.
*/
void printAllBuffers(CPU_p cpu) {
	for (int i = 0; i < MAX_BUFFERS; i++) {
        printf("Buffer #: %d\n", i);
		printBuffer(cpu, cpu->buffers[i]);
		printf("\n");
	}
}


/*
	This is the meat of our Predecoder. This will be deciding if the current head of the Prefetch is
	colliding with the passed in hex value. If so, it will return 4 minus the difference between the two. 
*/
int checkForCollision(Register headOfPrefetch, int headPos, Register collisionCheck, int collisionPos) {
	int nopCount = 0, Rd = 0, Rs1 = 0, Rs2 = -1;
	int opcode = collisionCheck >> OPCODE_SHIFT;
	if (opcode != ST && opcode != STR && opcode != STI) {
		Rd = (headOfPrefetch & DR_MASK) >> DR_SHIFT; //Gets destination register
		Rs1 = (collisionCheck & SR_MASK) >> SR_SHIFT; //Gets source register 1
		// if Rd or Rs1 are R6 then check to see if collisionCheck == PP
		Rs2 = -1;
		
		if (!((collisionCheck & HIGH_ORDER_BIT_VALUE5) || (collisionCheck & HIGH_ORDER_BIT_VALUE6) 
			|| (collisionCheck & HIGH_ORDER_BIT_VALUE8) || (collisionCheck & HIGH_ORDER_BIT_VALUE9))) {   //Checks if this value is a sext.
			Rs2 = collisionCheck & SR2_MASK; //Gets the source register 2 if not a sext.
		}

	} else {
		Rd = (collisionCheck & DR_MASK) >> DR_SHIFT; //Gets destination register
		Rs1 = (headOfPrefetch & SR_MASK) >> SR_SHIFT; //Gets source register 1

		Rs2 = -1;

		if (!((headOfPrefetch & HIGH_ORDER_BIT_VALUE5) || (headOfPrefetch & HIGH_ORDER_BIT_VALUE6) 
			|| (headOfPrefetch & HIGH_ORDER_BIT_VALUE8) || (headOfPrefetch & HIGH_ORDER_BIT_VALUE9))) {   //Checks if this value is a sext.
			Rs2 = headOfPrefetch & SR2_MASK; //Gets the source register 2 if not a sext.
		}
	}
	
	
	if (Rd == Rs1 || (Rs2 >= 0 && Rd == Rs2)) {  //Checks if Rd is Rs1 or that Rd is Rs2 as long as Rs2 is not a sext.
		nopCount = 4 - (collisionPos - headPos);
	}

	if(opcode == PP && (headOfPrefetch >> OPCODE_SHIFT == PP)) {
		nopCount = 4 - (collisionPos - headPos);		
	}
	if(opcode == PP && (Rd == 6 || Rs1 == 6)) {
		nopCount = collisionPos - headPos;		
	}
		
	if (opcode == BR) {
		nopCount = 2;
	}
	if (opcode == TRAP && (collisionCheck & SR_MASK) == HALT) {
		nopCount = 2;
	}
	return nopCount;
}


/*
	A helper method to print the contents of the Prefetch array for debugging.
*/
void printPrefetch (CPU_p cpu) {
	printf("\n");
	for (int i = 0; i < PREFETCH_SIZE; i++) {
		printf("\t 0x%04X\n", cpu->prefetch.values[i]);
	}
	printf("\n");
}


/*
	This is our predecode that determines the next value that will be passed into the IR.
*/
Register predecode (CPU_p cpu) {

	Register returnValue = 0;
	int collision = 0, i = cpu->PC, j = 0;
	cpu->prefetch.collisionFound = 0;
	if (cpu->prefetch.head >= 8) {
		for (; i < cpu->PC + 8; i++, j++) {
			
			cpu->prefetch.values[j] = memory[i - startMem];
		}		
		cpu->prefetch.head = 0;
		cpu->prefetch.nopCount = 79;
		returnValue = NOP;
	}	else if (cpu->prefetch.nopCount > 0) {
		returnValue = NOP;
	} else {
		for (int i = cpu->prefetch.head + 1; i < cpu->prefetch.head + 4 && i < PREFETCH_SIZE; i++) {
			collision = 5 * checkForCollision(cpu->prefetch.values[cpu->prefetch.head], 
						cpu->prefetch.head, cpu->prefetch.values[i], i);
            if (collision) {
				cpu->prefetch.collisionFound = 1;
				cpu->prefetch.nopCount = collision;
				break;
			}
		}
		returnValue = cpu->prefetch.values[cpu->prefetch.head];
		cpu->prefetch.head++;
	}
	
	return returnValue;
}


/*
	A helper function to initialize a Buffer struct and pass it back.
*/
struct BUFFER initBuffer() {
	struct BUFFER buff;
	
	buff.PC = NOP;
	buff.IR = NOP;
	buff.Rd = NOP;
	buff.Opcode = NOP;
	buff.A = NOP;
	buff.B = NOP;
	buff.SEXT = NOP;
	buff.isStalled = 0;
	buff.N = 0;
	buff.Z = 0;
	buff.P = 0;
	
	return buff;
}


/*
	This is the main controller for the program. It takes a CPU struct and an int
	which is being used as a boolean for displaying the screen.
*/
int controller (CPU_p cpu, int isRunning) {
	unsigned int state;
	short cc;
	unsigned int opcode, Rd, Rs1, Rs2, immed_offset, BaseR;	// fields for the IR
	char charToPrint = ' ';
	char *temp = malloc(sizeof(char) * 8);
	int value = 0;
    state = STORE;
	int j;
	struct BUFFER tempHolder;
	Register predecodeValue = 0;
	
    for (;;) {
		if(isRunning && encounteredBreakPoint(cpu, cpu->PC)) {
			isRunning = 0;
		}
		if (!isRunning) {
			displayScreen(cpu, 0, 1, cpu->prefetch.stepCounter, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, temp);
		}
        switch (state) {
			case STORE: // Here in STORE we simply are sending the information from 
						// the MBUFF into the DPRF. If the opcode from MBUFF is an 
						// LDI or STI it will just go back to MEM for a second go around.
				strcpy(temp, "STORE");
				if (cpu->buffers[3].Opcode == LDI || cpu->buffers[3].Opcode == STI) {
					cpu->buffers[2].isStalled = 1;
					cpu->buffers[2].stalled = 'S';
					if (cpu->buffers[3].Opcode == STI) {
						cpu->buffers[3].Opcode = ST;
					} else {
						cpu->buffers[3].Opcode = LD;
					}
					state = MEM;
				} else {
					switch (cpu->buffers[3].Opcode) {
						case ADD:
							cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
							setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
							break;
						case AND:
							cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
							setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
							break;
						case NOT:
							cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
							setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
							break;
						case LDI:
						case LDR:
						case LD:
							cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].B;
							//may need to change the buffer being passed here
							setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
							break;
						case LEA:
							cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
							setCC(cpu, cpu->r[cpu->buffers[3].Rd]);
							break;
							break;
						case TRAP:
							if (cpu->buffers[3].SEXT == GETC) {
								cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].A;
							}
							break;
						case NOP:
							break;
						case PP:
							//cpu->r[6] = cpu->buffers[3].A;
							
							if(cpu->buffers[3].IR & PUSH_POP_BIT_MASK) { // pop
								cpu->r[cpu->buffers[3].Rd] = cpu->buffers[3].B;
								cpu->r[6]++;
							} else { // push
								cpu->r[6]--;
							}
							break;
					}
					state = MEM;
				}
				break;
			case MEM:	// In MEM we simulate the act of going to Memory to Store or
						// Load items. It then "pauses" the previous buffers and reads
						// in the 10 NOPs it takes to go to Memory. This also handles cases
						// for when the buffers are stalled for BR, JMP, JSR, JSRR, and
						// STI and LDI instructions.
				strcpy(temp, "MEM");
				//may need to add in a check if the last buffer's PC is NOP (same as other states).
                if (cpu->buffers[1].isStalled) {
                    cpu->prefetch.head = PREFETCH_SIZE;
					
					cpu->buffers[0] = initBuffer();
					cpu->buffers[1] = initBuffer();
					cpu->buffers[3] = cpu->buffers[2];
                } else {
					if (!cpu->buffers[2].isStalled) {
						cpu->buffers[3] = cpu->buffers[2];
					}
                    switch (cpu->buffers[3].Opcode) {
                        case ST:
							memory[cpu->buffers[3].A - startMem] = cpu->r[cpu->buffers[3].Rd];
							cpu->buffers[2].isStalled = 1;
							cpu->buffers[2].stalled = 'S';
							cpu->hasAccessedMem = 1;
							cpu->memStepCount = 10;
							tempHolder = cpu->buffers[3];
							break;
                        case STR:
							memory[cpu->buffers[3].A - startMem] = cpu->buffers[3].B;
							cpu->buffers[2].isStalled = 1;
							cpu->buffers[2].stalled = 'S';
							cpu->hasAccessedMem = 1;
							cpu->memStepCount = 10;
							tempHolder = cpu->buffers[3];
							break;
                        case STI:  //not sure of how to handle STI yet
							cpu->buffers[3].A = memory[cpu->buffers[3].A - startMem];
							cpu->buffers[2].isStalled = 1;
							cpu->buffers[2].stalled = 'S';
							cpu->hasAccessedMem = 1;
							cpu->memStepCount = 10;
							tempHolder = cpu->buffers[3];
							break;
                        case LD:
							cpu->buffers[3].B = memory[cpu->buffers[3].A - startMem];
							cpu->buffers[2].isStalled = 1;
							cpu->buffers[2].stalled = 'S';
							cpu->hasAccessedMem = 1;
							cpu->memStepCount = 10;
							tempHolder = cpu->buffers[3];
							break;
                        case LDR:
							cpu->buffers[3].B = memory[cpu->buffers[3].A - startMem];
							cpu->buffers[2].isStalled = 1;
							cpu->buffers[2].stalled = 'S';
							cpu->hasAccessedMem = 1;
							cpu->memStepCount = 10;
							tempHolder = cpu->buffers[3];
							break;
                        case LDI:
							cpu->buffers[3].A = memory[cpu->buffers[3].A - startMem];
							cpu->buffers[2].isStalled = 1;
							cpu->buffers[2].stalled = 'S';
							cpu->hasAccessedMem = 1;
							cpu->memStepCount = 10;
							tempHolder = cpu->buffers[3];
							break;
						case PP:
							if(cpu->buffers[3].IR & PUSH_POP_BIT_MASK) { // pop
								cpu->buffers[3].B = memory[(cpu->buffers[3].A + 1) - startMem];
								memory[cpu->buffers[3].A - startMem] = 0x0; // clear this spot in memory
							} else { // push
								memory[(cpu->buffers[3].A) - startMem] = cpu->r[cpu->buffers[3].Rd];
							}
							cpu->buffers[2].isStalled = 1;
							cpu->buffers[2].stalled = 'S';
							cpu->hasAccessedMem = 1;
							cpu->memStepCount = 10;
							tempHolder = cpu->buffers[3];
							break;
                    }
                }
				
				for (; cpu->memStepCount > 0; cpu->memStepCount--) {
					cpu->buffers[3] = initBuffer();
					if (!isRunning) {
						displayScreen(cpu, 0, 1, cpu->prefetch.stepCounter++, cpu->prefetch.nopCount, cpu->prefetch.collisionFound, temp);
						scanf("%c", &charToPrint);
						if (charToPrint == 'q') {
							return 0;
						}
					}
				}
				
				if (cpu->memStepCount <= 0 && cpu->hasAccessedMem) {
					cpu->hasAccessedMem = 0;
					cpu->buffers[3] = tempHolder;
				}
				
				if (cpu->buffers[3].Opcode == STI || cpu->buffers[3].Opcode == LDI) {
					state = STORE;
					cpu->buffers[2].isStalled = 0;
					cpu->buffers[2].stalled = ' ';
				} else {
					state = EXECUTE;
				}
                break;
			case EXECUTE: 	// In EXECUTE the A and B values will be used together corresponding to what 
							// the opcode is. If it is some kind of LD or ST type opcode, it will make the
							// address that will be used to go into Memory. This also performs the TRAPs in 
							// this state. If a HALT is found it will end here.
				strcpy(temp, "EXECUTE");
				if (cpu->buffers[1].PC == NOP) {
					cpu->buffers[2] = cpu->buffers[1];
				} else {
					cpu->buffers[2] = cpu->buffers[1];
					switch (cpu->buffers[1].Opcode) {
						case ADD:
							if (cpu->buffers[2].A & HIGH_ORDER_BIT_VALUE15) {
								cpu->buffers[2].A = (cpu->buffers[2].A) + (cpu->buffers[2].B);//same reason as below
							} else if (cpu->buffers[1].B & HIGH_ORDER_BIT_VALUE15) {
								cpu->buffers[2].A = (cpu->buffers[2].A) + (cpu->buffers[2].B); //need to add here because B is negative
							} else if ((cpu->buffers[2].A & HIGH_ORDER_BIT_VALUE15) && (cpu->buffers[2].B & HIGH_ORDER_BIT_VALUE15)) {
								cpu->buffers[2].A = -(cpu->buffers[2].A) - (cpu->buffers[2].B);
							} else {
								cpu->buffers[2].A = (cpu->buffers[2].A) + (cpu->buffers[2].B);
							}
							break;
						case AND:
							cpu->buffers[2].A = cpu->buffers[2].A & cpu->buffers[2].B;
							break;
						case NOT:
							cpu->buffers[2].A = ~(cpu->buffers[2].A);
							break;
						case TRAP:
							cpu->r[7] = cpu->buffers[2].PC;
							value = trap(cpu, cpu->buffers[2].B);
							if (value == 1) {
								printf("\n\n\n    Program finished. Press any key to end ");
								getch();
								return 0;
							} else if (value > 1) {
								cpu->buffers[2].A = (char) value;
								cpu->gotC = (char) value;
							}
							break;
						case JSRR:
							cpu->r[7] = cpu->buffers[2].PC;
							if (cpu->buffers[2].IR & HIGH_ORDER_BIT_VALUE12) {
								cpu->PC = cpu->buffers[2].PC + cpu->buffers[2].A;
							} else {
								cpu->PC = cpu->buffers[2].A;
							}
							cpu->buffers[1].isStalled = 1;
							cpu->prefetch.head = 8;
							break;
						case JMP:
							cpu->PC = cpu->r[cpu->buffers[2].A];
							cpu->buffers[1].isStalled = 1;
							cpu->prefetch.head = 8;
							break;
						case BR:
							if (cpu->N && (cpu->buffers[2].Rd & 4)) {
								cpu->PC = cpu->buffers[2].PC + sext9(cpu->buffers[1].SEXT);
								cpu->buffers[1].isStalled = 1;
								cpu->prefetch.head = 8;
								break;
							}
							if (cpu->Z && (cpu->buffers[2].Rd & 2)) {
								cpu->PC = cpu->buffers[2].PC + sext9(cpu->buffers[1].SEXT);
								cpu->buffers[1].isStalled = 1;
								cpu->prefetch.head = 8;
								break;
							}
							if (cpu->P && (cpu->buffers[2].Rd & 1)) {
								cpu->PC = cpu->buffers[2].PC + sext9(cpu->buffers[1].SEXT);
								cpu->buffers[1].isStalled = 1;
								cpu->prefetch.head = 8;
								break;
							}
							break;
						case NOP:
							break;
						case LEA:
							cpu->buffers[2].A = cpu->buffers[2].PC + sext9(cpu->buffers[2].SEXT);
							break;
						case LDR:
							cpu->buffers[2].A = cpu->buffers[2].A + sext6(cpu->buffers[2].SEXT);
							break;
						case LD:
							cpu->buffers[2].A = cpu->buffers[2].PC + sext9(cpu->buffers[2].SEXT);
							break;
						case LDI:
							cpu->buffers[2].A = cpu->buffers[2].PC + sext9(cpu->buffers[2].SEXT);
							break;
						case ST:
							cpu->buffers[2].A = cpu->buffers[2].PC + sext9(cpu->buffers[2].SEXT);
							break;
						case STR:
							cpu->buffers[2].A = cpu->r[BaseR] + sext6(cpu->buffers[2].SEXT);
							break;
						case STI:
							cpu->buffers[2].A = cpu->buffers[2].PC + sext9(cpu->buffers[2].SEXT);
							break;
					}
				}
                if(cpu->buffers[1].isStalled) {
                    state = STORE;
                } else {
                    state = IDRR;
                }
                break;
			case IDRR:	// This is a combination of Decode, Eval_Addr, and Fetch_Op. It handles the simulation
						// of the DECODE state that pulls the values out of the Dual-Ported Register File.
				strcpy(temp, "IDRR");
				if (cpu->buffers[0].PC == NOP) {
					cpu->buffers[1] = cpu->buffers[0];
				} else {			
					cpu->buffers[1].PC = cpu->buffers[0].PC;
					cpu->buffers[1].IR = cpu->buffers[0].IR;
					cpu->buffers[1].Opcode = cpu->buffers[0].IR >> OPCODE_SHIFT;			
					cpu->buffers[1].Rd = cpu->buffers[0].IR & DR_MASK;
					cpu->buffers[1].Rd = (short)cpu->buffers[1].Rd >> DR_SHIFT;
					cpu->buffers[1].A = cpu->buffers[0].IR & SR_MASK;
					cpu->buffers[1].A = (short)cpu->buffers[1].A >> SR_SHIFT;
					cpu->buffers[1].B = cpu->buffers[0].B & SR2_MASK;
					cpu->buffers[1].SEXT = cpu->buffers[0].IR & SR_MASK;
					BaseR = (cpu->buffers[0].IR & BASE_MASK) >> SR_SHIFT;
					
					switch (cpu->buffers[1].Opcode) {							
						case ADD:
							if(HIGH_ORDER_BIT_VALUE6 & cpu->buffers[1].IR){ //0000|0000|0010|0000
								cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
								cpu->buffers[1].B = sext5(cpu->buffers[1].SEXT);
							} else{
								//change this
								cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
								cpu->buffers[1].B = cpu->r[cpu->buffers[1].B];
							}

							break;
						case AND:
							if(HIGH_ORDER_BIT_VALUE6 & cpu->buffers[1].IR){ //0000|0000|0010|0000
								cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
								cpu->buffers[1].B = (cpu->buffers[1].SEXT & SEXT5_MASK);
							} else{
								cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
								cpu->buffers[1].B = cpu->r[cpu->buffers[1].B];
							}
							break;
						case NOT:
							cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
							break;
						case JSRR:
							if (cpu->buffers[1].IR & HIGH_ORDER_BIT_VALUE12) {
								cpu->buffers[1].A = cpu->buffers[1].IR & SEXT11_MASK;
							} else {
								cpu->buffers[1].A = cpu->r[BaseR];
							}
							break;
						case LDR:
							cpu->buffers[1].A = cpu->r[BaseR];
							break;
						case STR:
						case ST:
							cpu->buffers[1].B = cpu->r[cpu->buffers[1].Rd];
							break;
						case STI:
							cpu->buffers[1].B = memory[cpu->buffers[1].A];
							break;
						case TRAP:
							cpu->buffers[1].B = cpu->buffers[1].SEXT;
							break;
						case NOP:
							break;
						case PP:
							cpu->buffers[1].A = cpu->r[6];
							break;
					}
				}
                state = FETCH;
				break;
            case FETCH: // In the FETCH state this will simply call the predecode and 
						// pass that into the FETCH which will pull it apart and send itoa
						// to IDRR.
				strcpy(temp, "FETCH");
				predecodeValue = predecode(cpu);
				if (predecodeValue != NOP) {
					cpu->buffers[0].A = (cpu->PC - startMem);
					cpu->PC++;	// increment PC
					cpu->buffers[0].B = memory[cpu->buffers[0].A];
					cpu->ir = cpu->buffers[0].B;
					cc = 0;
					cpu->buffers[0].PC = cpu->PC;  //IF Buffer
					cpu->buffers[0].IR = predecodeValue;
				} else {
                    cpu->buffers[0].A = NOP;
                    cpu->buffers[0].B = NOP;
                    cpu->ir = NOP;
                    cc = 0;
                    cpu->buffers[0].PC = NOP;  //IF Buffer
                    cpu->buffers[0].IR = NOP;
                }
				state = STORE;
                break;
        }
		//printAllBuffers(cpu);
		if (!isRunning) {
			scanf("%c", &charToPrint);
			if (charToPrint == 'q') {
				//cpu->prefetch.stepCounter++;
				//cpu->prefetch.nopCount--;
				return 0;
			}
		}
		cpu->prefetch.stepCounter++;
		cpu->prefetch.nopCount--;

    }
	
}

/*
	This function initializes the cpu fields, buffers, and the prefetch.
*/
void cpuInit(CPU_p cpu) {
	cpu->r[0] = 0x0000;
	cpu->r[1] = 0x0000;
	cpu->r[2] = 0x0000;
	cpu->r[3] = 0x0000;
	cpu->r[4] = 0x0000;
	cpu->r[5] = 0x0000;
	cpu->r[6] = 0x0000;
	cpu->r[7] = 0x0000;
	cpu->ir = 0x0000;
	cpu->PC = START_MEM;
	startMem = START_MEM;
	cpu->MAR = 0x0000;
	cpu->MDR = 0x0000;
	cpu->A = 0x0000;
	cpu->B = 0x0000;
	cpu->hasAccessedMem = 0; 
	cpu->memStepCount = 0;
	cpu->reachedInput = 0;
	cpu->N = 0;
	cpu->Z = 0;
	cpu->P = 0;
	cpu->breakPoints[0] = AVAILABLE_BRKPT;
	cpu->breakPoints[1] = AVAILABLE_BRKPT;
	cpu->breakPoints[2] = AVAILABLE_BRKPT;
	cpu->breakPoints[3] = AVAILABLE_BRKPT;
	cpu->prefetch.head = 8;
	cpu->prefetch.nopCount = 0;
	cpu->prefetch.stepCounter = 1;
	for (int i = 0; i < PREFETCH_SIZE; i++) {
		cpu->prefetch.values[i] = NOP;
	}
	
	for (int i = 0; i < MAX_MEMORY; i++) {
		memory[i] = 0x0;
	}
	
	for (int i = 0; i < MAX_BUFFERS; i++) {
		cpu->buffers[i] = initBuffer();
	}
}


/*
	This is a helper function that simply checks if the specified file name exists in the 
	current directory.
*/
int checkIfFileExists(char* fileToCheckIfExists) {
	FILE* filePtr;
	filePtr = fopen(fileToCheckIfExists, "r");
	if(filePtr != NULL) {
		fclose(filePtr);
		return 1;
	} else {
		fclose(filePtr);
		return 0;
	}
}


/*
	Writes the file specified out to the current directory.
*/
void writeMemory(char * fileToWriteToName) {
	FILE * filePtr;
	int TRAP25 = 61477;
	char shouldOverwrite;
	unsigned int memoryStart, memoryEnd; 
	//the file exists so prompt the user to see if they 
	//are ok with overwritting the preexisting file right here
	//include if/else statement to check user decision for overwriting
	if(checkIfFileExists(fileToWriteToName)) {
		printf("This file exists would you like to overwrite?(y/n): ");
		scanf("%c", &shouldOverwrite);
		if(shouldOverwrite == 'y') {
			filePtr = fopen(fileToWriteToName, "w");
			printf("Enter the beginning of memory to save from: x");
			scanf("%4x", &memoryStart);
			printf("Enter the end of memory to save to: x");
			scanf("%4x", &memoryEnd);
			for(int i=memoryStart; i <= memoryEnd; i++) {
				fprintf(filePtr, "%04X\r\n", memory[i - startMem]);
			}
			fclose(filePtr);
		}
	//the file doesn't exist so create the new file and write to it
	} else {
		FILE * filePtr;
		filePtr = fopen(fileToWriteToName, "w");
		printf("Enter the beginning of memory to save from: x");
		scanf("%4x", &memoryStart);
		printf("Enter the end of memory to save to: x");
		scanf("%4x", &memoryEnd);
		printf("start = %4x end = %4x\n", memoryStart, memoryEnd);
		for(int i=memoryStart; i <= memoryEnd; i++) {
			fprintf(filePtr, "%04X\r\n", memory[i - startMem]);
		}
		fclose(filePtr);
	}
}


/*
	This is the main function that starts the program off.
*/
int main(int argc, char* argv[]){
	//setvbuf needs to be used in order for getch to work correctly
	setvbuf(stdout, NULL, _IONBF, 0);
	isLoaded = 0;
	memShift = 0;
	char *temp;
	CPU_p cpu = malloc(sizeof(CPU_s));
	cpuInit(cpu);
	displayScreen(cpu, memShift, 0, 0, 0, 0, temp);
	dialog(cpu);
	return 0;
}