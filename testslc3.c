/*
 * Authors: Connor Lundberg, Daniel Ivanov
 * Date: 5/3/2017
 */
#include "tempslc3.h"

int controller (CPU_p, int);

int displayScreen (CPU_p, int);

int dialog (CPU_p cpu);

char getch ();

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
	This function sets the appropriate flags.
*/
void setFlags (CPU_p cpu, unsigned int neg, unsigned int zero, unsigned int pos) {
	cpu->N = neg;
	cpu->Z = zero;
	cpu->P = pos;
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
	This function displays the debug screen with LOAD, STEP, DISPLAY MEM, RUN, or EXIT
	commands to use.
*/
int displayScreen(CPU_p cpu, int mem) {
  printf("\n\n\n");
	printf("\t\tWelcome to the LC-3 Simulator Simulator\n\n");
	printf("\t\tRegisters \t\t    Memory\n");
	int i = START_MEM + mem;
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 0, cpu->r[0], i, memory[1 + mem]);

	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 1, cpu->r[1], i+1, memory[2 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 2, cpu->r[2], i+2, memory[3 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 3, cpu->r[3], i+3, memory[4 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 4, cpu->r[4], i+4, memory[5 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 5, cpu->r[5], i+5, memory[6 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 6, cpu->r[6], i+6, memory[7 + mem]);
	printf("\t\tR%d: x%04X \t\t x%X: x%04X\n", 7, cpu->r[7], i+7, memory[8 + mem]);

	i = BOTTOM_HALF + mem; // replace i with the mem dump number if you want.
	printf("\t\t\t\t\t x%X: x%04X\n",i, memory[9 + mem]);
	printf("\t\t\t\t\t x%X: x%04X\n",i+1, memory[10 + mem]);
	printf("\t\t\t\t\t x%X: x%04X\n",i+2, memory[11 + mem]);
	printf("\t\tPC:x%0.4X    IR:x%04X     x%X: x%04X\n",cpu->PC,cpu->ir,i+3, memory[12 + mem]);
	printf("\t\tA: x%04X    B: x%04X     x%X: x%04X\n",cpu->A,cpu->B,i+4, memory[13 + mem]);
	printf("\t\tMAR:x%04X  MDR:x%04X     x%X: x%04X\n",cpu->MAR + CONVERT_TO_DECIMAL,cpu->MDR,i+5, memory[14 + mem]);
	printf("\t\tCC: N: %d  Z: %01d P: %d      x%X: x%04X\n",cpu->N,cpu->Z,cpu->P,i+6, memory[15 + mem]);
	printf("\t\t\t\t\t x%X: x%04X\n",i+7, memory[16 + mem]);
	printf("\nPipeLine Info:\n");
	printf("FBUFF: PC: %04x  IR: %04x\n", cpu->buffers[0].PC, cpu->buffers[0].IR);
	printf("DBUFF: Op: %04x  DR: %04x  SR1: %04x  SEXT/SR2: %04x\n", cpu->buffers[1].Opcode, cpu->buffers[1].Rd, cpu->buffers[1].A, cpu->buffers[1].B);
	printf("EBUFF: Op: %04x DR: %04x  RESULT: %04x\n", cpu->buffers[2].Opcode, cpu->buffers[2].Rd, cpu->buffers[2].B);
	printf("MBUFF: Op: %04x DR: %04x RESULT: %04x\n", cpu->buffers[3].Opcode, cpu->buffers[3].Rd, cpu->buffers[3].B);
	printf("  Select: 1)Load,2)Save, 3)Step, 5)Display Mem 6)Edit, 7)Run, 9)Exit\n");
	return 0;
}


/*
	This is the dialog function that provides the functionality to the choices shown in
	the displayScreen.
*/
int dialog(CPU_p cpu) {
	int opNum = 0, isRunning = 0;
	//long newValue;
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
					displayScreen(cpu, 0);
					fclose(inputFile);
					break;
				case SAVE:
					printf("Enter a file name to save to: ");
					scanf("%s", &fileName);
					writeMemory(fileName);
					displayScreen(cpu, 0);
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
						displayScreen(cpu, memShift - START_MEM);
					}
					break;
				case EDIT:
					printf("What memory address would you like to edit: ");
					scanf("%04x", &placeInMemory);
					printf("The contents of location %04x is  %04x\n", placeInMemory - START_MEM + 1, memory[placeInMemory - START_MEM + 1]);
					printf("What would you like the new value in location %04x to be: ", placeInMemory);
					scanf("%s", &newMemoryValue);
					printf("%s\n", newMemoryValue);
					newValue = (short)strtol(newMemoryValue, &charPtr, 16);
					memory[placeInMemory - START_MEM + 1] = newValue;
					displayScreen(cpu, placeInMemory - START_MEM - 7);
					break;
				case RUN:
					controller(cpu, 1);
					displayScreen(cpu, 0);
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


void printBuffer(struct BUFFER buff) {
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
}


void printAllBuffers(CPU_p cpu) {
	for (int i = 0; i < MAX_BUFFERS; i++) {
		printBuffer(cpu->buffers[i]);
		printf("\n");
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
    state = STORE;
	int j;
	
	if(isRunning && encounteredBreakPont(cpu)) {
		isRunning = 0;
	}
    for (;;) {
		//printf("here\n");
		
        switch (state) {
			case STORE: // Look at ST. Microstate 16 is the store to memory
				printf("STORE\n");
                switch (cpu->buffers[3].Opcode) {
					case ADD:
						cpu->r[Rd] = cpu->buffers[3].A;

						break;
					case AND:
						cpu->r[Rd] = cpu->buffers[3].A;
						break;
					case NOT:
						cpu->r[Rd] = cpu->buffers[3].A;
						break;
					case LDR:
					case LD:
						cpu->r[Rd] = cpu->MDR;
						setCC(cpu, cpu->r[Rd]);
						//cc = cpu->r[Rd];
						//chooseFlag (cpu, cc);
						break;
					case LEA:
						cpu->r[Rd] = cpu->buffers[3].PC + sext9(immed_offset);
						setCC(cpu, cpu->r[Rd]);
						//cc = cpu->r[Rd];
						//chooseFlag (cpu, cc);
						break;
					case STR:
					case ST:
						memory[cpu->MAR] = cpu->MDR;
						break;
	                }
				state = MEM;
				break;
                //state = FETCH;
			case MEM:
				printf("MEM\n");
				switch (cpu->buffers[2].Opcode) {
					case ST:
					case STR:
					case STI:
						//cpu->buffers[2].MAR = memory[cpu->buffers[2].MDR];
						
					case LD:
					case LDR:
					case LDI:
					case LEA:
						break;
				}
				//Do Mem work with EBuff (buffers[2]) here first.
				cpu->buffers[3] = cpu->buffers[2];
				//cpu->buffers[3].A = cpu->alu.A;
				//cpu->buffers[3].B = cpu->alu.B;
				state = EXECUTE;
				break;
			case EXECUTE: // Note that ST does not have an execute microstate
				printf("EXECUTE\n");
				cpu->buffers[2].PC = cpu->buffers[1].PC;
				cpu->buffers[2].Rd = cpu->buffers[1].Rd;
				switch (cpu->buffers[1].Opcode) {
					case ADD:
						if (cpu->alu.A & HIGH_ORDER_BIT_VALUE15) {
							cpu->buffers[2].A = -(cpu->alu.A) + (cpu->alu.B);
						} else if (cpu->B & HIGH_ORDER_BIT_VALUE15) {
							cpu->buffers[2].A = (cpu->alu.A) -(cpu->alu.B);
						} else if ((cpu->alu.A & HIGH_ORDER_BIT_VALUE15) && (cpu->alu.B & HIGH_ORDER_BIT_VALUE15)) {
							cpu->buffers[2].A = -(cpu->alu.A) -(cpu->alu.B);
						} else {
							cpu->buffers[2].A = (cpu->alu.A) + (cpu->alu.B);
						}
						setCC(cpu, cpu->Res);
						//cc = (short int) cpu->Res;
						//chooseFlag (cpu, cc);
						break;
					case AND:
						cpu->buffers[2].A = cpu->alu.A & cpu->alu.B;
						cpu->N = 0;
						cpu->Z = 0;
						cpu->P = 0;
						setCC(cpu, cpu->buffers[2].A);
						//cc = cpu->Res;
						//chooseFlag (cpu, cc);
						break;
					case NOT:
						cpu->buffers[2].A = ~(cpu->alu.A);
						cpu->N = 0;
						cpu->Z = 0;
						cpu->P = 0;
						setCC(cpu, cpu->buffers[2].A);
						//cc = (short) cpu->Res;
						//chooseFlag (cpu, cc);
						break;
					case TRAP:
						cpu->buffers[2].PC = cpu->MDR;
						value = trap(cpu, cpu->MAR);
						cpu->buffers[2].PC = cpu->r[7];
						//start NOP stall
						if (value == 1) {
							return 0;
						} else if (value > 1) {
							cpu->r[0] = (char) value;
							cpu->gotC = (char) value;
							cpu->r[Rd] = value;
						}
						break;
					case JSRR:
						cpu->r[7] = cpu->buffers[2].PC;
						cpu->buffers[2].PC = cpu->r[BaseR];
						//start NOP stall
						break;
					case JMP:
						cpu->r[7] = cpu->buffers[2].PC;
						cpu->buffers[2].PC = cpu->r[cpu->buffers[2].A];
						break;
					case BR:
						if (cpu->N && (cpu->buffers[2].Rd & 4)) {
							cpu->buffers[2].PC = cpu->buffers[2].PC + sext9(immed_offset);
							break;
						}
						if (cpu->Z && (cpu->buffers[2].Rd & 2)) {
							cpu->buffers[2].PC = cpu->buffers[2].PC + sext9(immed_offset);
							break;
						}
						if (cpu->P && (cpu->buffers[2].Rd & 1)) {
							cpu->buffers[2].PC = cpu->buffers[2].PC + sext9(immed_offset);
							break;
						}
					case NOP:
						break;
					case LEA:
						cpu->buffers[2].A = cpu->buffers[1].PC + sext9(cpu->buffers[1].SEXT);
						break;
				}
				//cpu->buffers[3] = cpu->buffers[2];
				//cpu->buffers[2] = cpu->buffers[1];
				//cpu->buffers[2].A;
				//cpu->buffers[2].B;
				state = IDRR;
				break;
			case IDRR:
				printf("IDRR\n");
				cpu->buffers[1].PC = cpu->buffers[0].PC;
				cpu->buffers[1].Opcode = cpu->buffers[0].IR >> OPCODE_SHIFT;			//Decode Stage
				cpu->buffers[1].Rd = cpu->buffers[0].IR & DR_MASK;
				cpu->buffers[1].Rd = (short)cpu->buffers[1].Rd >> DR_SHIFT;
				cpu->buffers[1].A = cpu->buffers[0].IR & SR_MASK;
				cpu->buffers[1].A = (short)cpu->buffers[1].A >> SR_SHIFT;
				cpu->buffers[1].B = cpu->buffers[1].B & SR2_MASK;
				cpu->buffers[1].SEXT = cpu->buffers[0].IR & SR_MASK;
				//immed_offset = cpu->buffers[0].IR & SR_MASK;
				BaseR = (cpu->buffers[0].IR & BASE_MASK) >> SR_SHIFT;
				//printf("here\n");
				//IDRR Buffer
				//printBuffer(cpu->buffers[1]);
                switch (cpu->buffers[1].Opcode) {							//Evaluate Address Stage
					case LDR:
						printf("LDR\n");
						//printf("%04X = (%04X + %04X) - %d\n", (cpu->r[BaseR] + sext6(cpu->buffers[1].SEXT)) - CONVERT_TO_DECIMAL, cpu->r[BaseR], sext6(cpu->buffers[1].SEXT), CONVERT_TO_DECIMAL);
						cpu->MAR = (cpu->r[BaseR] + sext6(cpu->buffers[1].SEXT)) - CONVERT_TO_DECIMAL;
						break;
					case LD:
						//printf("LD\n");
						cpu->MAR = (cpu->PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
						break;
					case LDI:
						//printf("LDI\n");
						cpu->MAR = (cpu->PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
						break;
					case ST:
						//printf("ST\n");
						cpu->MAR = (cpu->PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
						break;
					case STR:
						//printf("STR\n");
						cpu->MAR = (cpu->r[BaseR] - CONVERT_TO_DECIMAL) + sext6(cpu->buffers[1].SEXT);
						break;
					case STI:
						//printf("STI\n");
						cpu->MAR = (cpu->PC - CONVERT_TO_DECIMAL) + sext9(cpu->buffers[1].SEXT);
						break;
					case TRAP:
						//printf("TRAP\n");
						cpu->MAR = cpu->buffers[1].SEXT & TRAP_VECTOR_MASK;
						break;
					case NOP:
						//printf("NOP\n");
						break;
                }

                switch (cpu->buffers[1].Opcode) {							//Fetch Operand Stage
					case LDR:
					case LD:
						printf("LD/LDR\n");
						printf("%04X\n", cpu->MAR);
						cpu->MDR = memory[cpu->MAR];
						break;
					case LDI:
						//printf("LDI\n");
						cpu->MDR = memory[cpu->MAR];
						break;
					case ADD:
						//printf("ADD\n");
						if(HIGH_ORDER_BIT_VALUE6 & cpu->buffers[1].IR){ //0000|0000|0010|0000
							cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
							cpu->buffers[1].B = (cpu->buffers[1].SEXT & SEXT5_MASK);
							
						} else{
							cpu->A = cpu->r[Rs1];
							cpu->B = cpu->r[Rs2];
						}

						break;
					case AND:
						//printf("AND\n");
						if(HIGH_ORDER_BIT_VALUE6 & cpu->buffers[1].IR){ //0000|0000|0010|0000
							cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
							cpu->buffers[1].B = (cpu->buffers[1].SEXT & SEXT5_MASK);
						} else{
							cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
							cpu->buffers[1].B = cpu->r[cpu->buffers[1].B];
						}
						break;
					case NOT:
						//printf("NOT\n");
						cpu->A = cpu->r[cpu->buffers[1].A];
						break;
					case STR:
					case ST:
						//printf("ST/STR\n");
						cpu->MDR = cpu->r[cpu->buffers[1].Rd];
						break;
					case STI:
						//printf("STI\n");
						cpu->MDR = memory[cpu->MAR];
						break;
					case TRAP:
						//printf("TRAP\n");
						cpu->MDR = memory[cpu->MAR];
						cpu->r[7] = cpu->buffers[1].PC;
						break;
					case NOP:
						//printf("NOP\n");
						break;
                }
				//cpu->buffers[2] = cpu->buffers[1];
				//cpu->buffers[1] = cpu->buffers[0];
				//cpu->buffers[1].A; //might need to take these out
				//cpu->buffers[1].B;
                state = FETCH;
				break;
            case FETCH: // microstates 18, 33, 35 in the book
				printf("FETCH\n");
				cpu->MAR = (cpu->PC - CONVERT_TO_DECIMAL);
				cpu->PC++;	// increment PC
				cpu->MDR = memory[cpu->MAR];
				cpu->ir = cpu->MDR;
				cc = 0;
				cpu->buffers[0].PC = cpu->PC;		//IF Buffer
				cpu->buffers[0].IR = cpu->ir;
                state = IDRR;     
					//break;
                
                //state = STORE;
				//cpu->buffers[1] = cpu->buffers[0];
				state = STORE;
                break;
        }
		printAllBuffers(cpu);
		if (!isRunning) {
			//display(cpu, 0);
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
	for (int i = 0; i < MAX_BUFFERS; i++) {
		cpu->buffers[i].PC = NOP;
		cpu->buffers[i].IR = NOP;
		cpu->buffers[i].Rd = NOP;
		cpu->buffers[i].Opcode = NOP;
		cpu->buffers[i].A = NOP;
		cpu->buffers[i].B = NOP;
		cpu->buffers[i].SEXT = NOP;
	}
	printAllBuffers(cpu);
}

//returns 1 if true 0 if false
int checkIfFileExists(char* fileToCheckIfExists) {
	FILE* filePtr;
	filePtr = fopen(fileToCheckIfExists, "r+");
	if(filePtr != NULL) {
		fclose(filePtr);
		return 1;
	} else {
		return 0;
		fclose(filePtr);
	}
}

//need to #define TRAP25 61477;
void writeMemory(char * fileToWriteToName) {
	FILE * filePtr;
	int TRAP25 = 61477;
	unsigned int memoryStart, memoryEnd; 
	//the file exists so promt the user to see if they 
	//are ok with overwritting the preexisting file right here
	//include if/else statement to check user decision for overwriting
	if(checkIfFileExists(fileToWriteToName)) {

		filePtr = fopen(fileToWriteToName, "w");
		for(int i=memoryStart + 1; i <= memoryEnd; i++) {
			printf("i = %i i = x%04x\n", i, memory[i - START_MEM]);
			fprintf(filePtr, "%04x\n", memory[i - START_MEM]);
		}
		fclose(filePtr);

	//the file doesn't exist so create the new file and write to it
	} else {
		FILE * filePtr;
		filePtr = fopen(fileToWriteToName, "w");
		printf("Enter the beginning and end of the memory to save: ");
		scanf("%4x %4x", &memoryStart, &memoryEnd);
		printf("start = %4x end = %4x\n", memoryStart, memoryEnd);
		for(int i=memoryStart + 1; i <= memoryEnd; i++) {
			printf("i = %i i = x%04x\n", i, memory[i - START_MEM]);
			fprintf(filePtr, "%04x\n", memory[i - START_MEM]);
		}
		fclose(filePtr);
	}
}


/*
	This is the main function that starts the program off.
*/
int main(int argc, char* argv[]){

	//setvbuf(stdout, NULL, _IONBF, 0);
	isLoaded = 0;
	memShift = 0;
	CPU_p cpu = malloc(sizeof(CPU_s));
	cpuInit(cpu);
	displayScreen(cpu, memShift);
	dialog(cpu);
	return 0;
}