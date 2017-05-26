/*
 * Authors: Connor Lundberg, Daniel Ivanov
 * Date: 5/3/2017
 */
#include <curses.h>
#include "view.c"

void setFlags (CPU_p, unsigned int, unsigned int, unsigned int);

void writeMemory(char * fileToWriteToName);


// you can define a simple memory module here for this program
//unsigned short memory[MAX_MEMORY];   // 500 words of memory enough to store simple program
int isLoaded;
int memShift;


/*
	This function is the offset6 sign extender.
*/
int sext6(int offset6) {
	if (HIGH_ORDER_BIT_VALUE6 & offset6) return (offset6 | SEXT6_SIGN_EXTEND);
	else return offset6 & SEXT6_MASK;
}


/*
	This function is the offset9 sign extender.
*/
int sext9(int offset9) {
	if (HIGH_ORDER_BIT_VALUE9 & offset9) return (offset9 | SEXT9_SIGN_EXTEND);
	else return offset9;
}


// This is the trap function that handles trap vectors. Acts as 
// the trap vector table for now. Currently exits the HALT trap command.
int trap(CPU_p cpu, int trap_vector) {
	int value = 0;
	int i = 0;
	char temp;
	switch (trap_vector) {
		case GETC:
			value = (int) getch();
			break;
		case OUT:
			printf("%c", cpu->gotC);
			break;
		case PUTS:
			i = 0;
			temp = (char ) memory[(cpu->r[0] - CONVERT_TO_DECIMAL + i)];
			while ((temp)) {  
			  printf("%c", (temp));
			  i++;
			  temp = memory[(cpu->r[0] - CONVERT_TO_DECIMAL + i)];
			}
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
void chooseFlag (CPU_p cpu, int cc) {
	if (cc < 0){
		setFlags(cpu, 1, 0, 0);
	}
	if (cc == 0){
		setFlags(cpu, 0, 1, 0);
	}
	if (cc > 0){
		setFlags(cpu, 0, 0, 1);
	}
}
	

/*
	This function sets the appropriate flags.
*/
void setFlags (CPU_p cpu, unsigned int neg, unsigned int zero, unsigned int pos) {
	cpu->N = neg;
	cpu->Z = zero;
	cpu->P = pos;
}


/*
	This is the dialog function that provides the functionality to the choices shown in
	the displayScreen.
*/
int dialog(CPU_p cpu) {
	int opNum = 0, isRunning = 0;
	unsigned int placeInMemory;
	unsigned short newValue; 
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
						break;
					}
					int i = 0;
					while (fscanf(inputFile, "%04X", &memory[i]) != EOF) {
						if (i == 0) {
							cpu->PC = memory[0];
						}
						i++;
					}
					isLoaded = 1;
					//displayScreen(cpu, 0);
					fclose(inputFile);
					break;
				case SAVE:
					printf("Enter a file name to save to: ");
					scanf("%s", &fileName);
					writeMemory(fileName);
					//displayScreen(cpu, 0);
					break;
				case STEP:
					if (isLoaded == 1) {
						controller(cpu, 0);
						opNum = 0;
					} else {
						printf("No file loaded!");
					}
					break;
				case DISP_MEM:
					printf("Position to move to? (in hex): ");
					scanf("%4x", &memShift);
					if(memShift - START_MEM > MAX_MEMORY - DISP_BOUNDARY) {
						printf("Error: out of memory");
						memShift = 0;
						break;
					} else {
						//displayScreen(cpu, memShift - START_MEM);
					}
					break;
				case EDIT:
					printf("What memory address would you like to edit: ");
					scanf("%04x", &placeInMemory);
					printf("The contents of location %04x is  %04x\n", placeInMemory, memory[placeInMemory - START_MEM + 1]);
					printf("What would you like the new value in location %04x to be: ", placeInMemory);
					scanf("%s", &newMemoryValue);
					printf("%s\n", newMemoryValue);
					newValue = (short)strtol(newMemoryValue, &charPtr, 16);
					memory[placeInMemory - START_MEM + 1] = newValue;
					//displayScreen(cpu, placeInMemory - START_MEM - 7);
					break;
				case RUN:
					controller(cpu, 1);
					//displayScreen(cpu, 0);
					break;
				case EXIT:
					printf("Simulation Terminated.");
					break;
			}
		}
}

/*
	This method compares the current value of PC against any breakpoints entered by the user. 
*/
int encounteredBreakPont(CPU_p cpu) {
		int encountered = 0;
		int i;
		for (i = 0; i < MAX_BREAKPOINTS; i++) {
			if(cpu->PC == cpu->breakPoints[i]) {
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
void editBreakPoint(CPU_p cpu, unsigned short breakPoint) {
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
	This is the main controller for the program. It takes a CPU struct and an int
	which is being used as a boolean for displaying the screen.
*/
int controller (CPU_p cpu, int isRunning) {
	unsigned int state;
	short cc;
	unsigned int opcode, Rd, Rs1, Rs2, immed_offset, BaseR;	// fields for the IR
	char charToPrint = ' ';
	char *temp;
	int value = 0;
    state = FETCH;
	int j;
	
	if(isRunning && encounteredBreakPont(cpu)) {
		isRunning = 0;
	}
    for (;;) {
        switch (state) {
            case FETCH: // microstates 18, 33, 35 in the book
            	 cpu->MAR = (cpu->PC - CONVERT_TO_DECIMAL);
            	 cpu->PC++;	// increment PC
            	 cpu->MDR = memory[cpu->MAR];
            	 cpu->ir = cpu->MDR;
            	 cc = 0;
               state = DECODE;
            case DECODE:
				opcode = cpu->ir >> OPCODE_SHIFT;
				Rd = cpu->ir & DR_MASK;
				Rd = (short)Rd >> DR_SHIFT;
				Rs1 = cpu->ir & SR_MASK;
				Rs1 = (short)Rs1 >> SR_SHIFT;
				Rs2 = cpu->ir & SR2_MASK;
				immed_offset = cpu->ir & SR_MASK;
				BaseR = (cpu->ir & BASE_MASK) >> SR_SHIFT;
                state = EVAL_ADDR;
            case EVAL_ADDR: // Look at the LD instruction to see microstate 2 example
                switch (opcode) {
					case LDR:
						cpu->MAR = (cpu->r[BaseR] + sext6(immed_offset)) - CONVERT_TO_DECIMAL;
						break;
					case LD:
						cpu->MAR = (cpu->PC - CONVERT_TO_DECIMAL) + sext9(immed_offset);
						break;
					case ST:
						cpu->MAR = (cpu->PC - CONVERT_TO_DECIMAL) + sext9(immed_offset);
						break;
					case STR:
						cpu->MAR = (cpu->r[BaseR] - CONVERT_TO_DECIMAL) + sext6(immed_offset);
						break;
					case TRAP:
						cpu->MAR = immed_offset & TRAP_VECTOR_MASK;
						break;
                }
                state = FETCH_OP;
            case FETCH_OP: // Look at ST. Microstate 23 example of getting a value out of a register
                switch (opcode) {
					case LDR:
					case LD:
					 cpu->MDR = memory[cpu->MAR];
						break;
					case ADD:
						if(HIGH_ORDER_BIT_VALUE6 & cpu->ir){ //0000|0000|0010|0000
							cpu->A = cpu->r[Rs1];
							cpu->B = (immed_offset & SEXT5_MASK);
						} else{
							cpu->A = cpu->r[Rs1];
							cpu->B = cpu->r[Rs2];
						}

						break;
					case AND:
					if(HIGH_ORDER_BIT_VALUE6 & cpu->ir){ //0000|0000|0010|0000
							cpu->A = cpu->r[Rs1];
							cpu->B = (immed_offset & SEXT5_MASK);
						} else{
							cpu->A = cpu->r[Rs1];
							cpu->B = cpu->r[Rs2];
						}
						break;
					case NOT:
						cpu->A = cpu->r[Rs1];
						break;
					case STR:
					case ST:
						cpu->MDR = cpu->r[Rd];
						break;
					case TRAP:
						cpu->MDR = memory[cpu->MAR];
						cpu->r[7] = cpu->PC;
                }
                state = EXECUTE;
            case EXECUTE: // Note that ST does not have an execute microstate
                switch (opcode) {
					case ADD:
						if (cpu->A < 0) {
							cpu->Res = -(cpu->A) + (cpu->B);
						} else if (cpu->B < 0) {
							cpu->Res = (cpu->A) -(cpu->B);
						} else if ((cpu->A < 0) & (cpu->B < 0)) {
							cpu->Res = -(cpu->A) -(cpu->B);
						} else {
							cpu->Res = (cpu->A) + (cpu->B);
						}
						cc = (short int) cpu->Res;
						chooseFlag (cpu, cc);
						break;
					case AND:
						cpu->Res = cpu->A & cpu->B;
						cpu->N = 0;
						cpu->Z = 0;
						cpu->P = 0;
						cc = cpu->Res;
						chooseFlag (cpu, cc);
						break;
					case NOT:
						cpu->Res = ~(cpu->A);
						cpu->N = 0;
						cpu->Z = 0;
						cpu->P = 0;
						cc = (short) cpu->Res;
						chooseFlag (cpu, cc);
						break;
					case TRAP:
						cpu->PC = cpu->MDR;
						value = trap(cpu, cpu->MAR);
						cpu->PC = cpu->r[7];
						
						if (value == 1) {
							return 0;
						} else if (value > 1) {
							cpu->r[0] = (char) value;
							cpu->gotC = (char) value;
							cpu->r[Rd] = value;
						}
						break;
					case JSRR:
						cpu->r[7] = cpu->PC;
						cpu->PC = cpu->r[BaseR];
						break;
					case JMP:
						cpu->r[7] = cpu->PC;
						cpu->PC = cpu->r[Rs1];
						break;
					case BR:
						if (cpu->N && (Rd & 4)) {
							cpu->PC = cpu->PC + sext9(immed_offset);
							break;
						}
						if (cpu->Z && (Rd & 2)) {
							cpu->PC = cpu->PC + sext9(immed_offset);
							break;
						}
						if (cpu->P && (Rd & 1)) {
							cpu->PC = cpu->PC + sext9(immed_offset);
							break;
						}
					break;
                }
                state = STORE;
            case STORE: // Look at ST. Microstate 16 is the store to memory
                switch (opcode) {
					case ADD:
						cpu->r[Rd] = cpu->Res;

						break;
					case AND:
						cpu->r[Rd] = cpu->Res;
						break;
					case NOT:
						cpu->r[Rd] = cpu->Res;
						break;
					case LDR:
					case LD:
						cpu->r[Rd] = cpu->MDR;
						cc = cpu->r[Rd];
						chooseFlag (cpu, cc);
						break;
					case LEA:
						cpu->r[Rd] = cpu->PC + sext9(immed_offset);
						cc = cpu->r[Rd];
						chooseFlag (cpu, cc);
						break;
					case STR:
					case ST:
						memory[cpu->MAR] = cpu->MDR;
						break;
	                }
                state = FETCH;
                break;
        }
		if (!isRunning) {
			//displayScreen(cpu, 0);
			scanf("%c", &charToPrint);
		}
    }
}


/*
	This function initializes the cpu fields
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
	cpu->MAR = 0x0000;
	cpu->MDR = 0x0000;
	cpu->A = 0x0000;
	cpu->B = 0x0000;
	cpu->N = 0;
	cpu->Z = 0;
	cpu->P = 0;
	cpu->breakPoints[0] = AVAILABLE_BRKPT;
	cpu->breakPoints[1] = AVAILABLE_BRKPT;
	cpu->breakPoints[2] = AVAILABLE_BRKPT;
	cpu->breakPoints[3] = AVAILABLE_BRKPT;
}

////returns 1 if true 0 if false
//int checkIfFileExists(char* fileToCheckIfExists) {
//	FILE* filePtr;
//	filePtr = fopen(fileToCheckIfExists, "r+");
//	if(filePtr != NULL) {
//		fclose(filePtr);
//		return 1;
//	} else {
//        fclose(filePtr);
//		return 0;
//	}
//}

////need to #define TRAP25 61477;
//void writeMemory(char * fileToWriteToName) {
//	FILE * filePtr;
//	int TRAP25 = 61477;
//	unsigned int memoryStart, memoryEnd;
//	//the file exists so promt the user to see if they
//	//are ok with overwritting the preexisting file right here
//	//include if/else statement to check user decision for overwriting
//	if(checkIfFileExists(fileToWriteToName)) {
//
//		filePtr = fopen(fileToWriteToName, "w");
//		for(int i=memoryStart + 1; i <= memoryEnd; i++) {
//			printf("i = %i i = x%04x\n", i, memory[i - START_MEM]);
//			fprintf(filePtr, "%04x\n", memory[i - START_MEM]);
//		}
//		fclose(filePtr);
//
//	//the file doesn't exist so create the new file and write to it
//	} else {
//		FILE * filePtr;
//		filePtr = fopen(fileToWriteToName, "w");
//		printf("Enter the beginning and end of the memory to save: ");
//		scanf("%4x %4x", &memoryStart, &memoryEnd);
//		printf("start = %4x end = %4x\n", memoryStart, memoryEnd);
//		for(int i=memoryStart + 1; i <= memoryEnd; i++) {
//			printf("i = %i i = x%04x\n", i, memory[i - START_MEM]);
//			fprintf(filePtr, "%04x\n", memory[i - START_MEM]);
//		}
//		fclose(filePtr);
//	}
//}

/*
	This is the main function that starts the program off.
*/
int main(int argc, char* argv[]){

	setvbuf(stdout, NULL, _IONBF, 0);
	isLoaded = 0;
	memShift = 0;
	CPU_p cpu = malloc(sizeof(CPU_s));
	cpuInit(cpu);
    initializeWindow();
    display(cpu, memShift);
	return 0;
}