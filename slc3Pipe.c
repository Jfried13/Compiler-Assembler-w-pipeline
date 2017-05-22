/*
 * Authors: Connor Lundberg, Daniel Ivanov
 * Date: 5/3/2017
 */
#include "tempslc3.h"
#include "view.c"

int controller (CPU_p, int);

//int display (CPU_p, int);

int dialog (CPU_p cpu);

void setCC(CPU_p cpu, Register Rd);

void writeMemory(char * fileToWriteToName);


// you can define a simple memory module here for this program
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
	This is the dialog function that provides the functionality to the choices shown in
	the display.
*/
int dialog(CPU_p cpu) {
    int opNum = 0;
    char fileName[MAX_FILE_NAME];
    FILE* inputFile;
    while (opNum != EXIT) {
        opNum = getch();
        scanw("%d", &opNum);
        switch (opNum) {
            case LOAD:
                printw("File Name: ");
                scanw("%s", &fileName);
                inputFile = fopen(fileName, "r");
                if (inputFile == NULL) {
                    printw("DIDN'T OPEN");
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
                display(cpu, 0);
                fclose(inputFile);
                break;
            case STEP:
                if (isLoaded == 1) {
                    opNum = 0;
                } else {
                    printw("No file loaded!");
                }
                break;
            case DISP_MEM:
                printw("Position to move to? (in decimal): ");

                scanw("%d", &memShift);
                if(memShift > MAX_MEMORY - DISP_BOUNDARY) {
                    printw("Error: out of memory");
                    memShift = 0;
                    break;
                } else {
                    display(cpu, memShift);
                }
                break;
            case RUN:
                display(cpu, 0);
                break;
            case EXIT:
                printw("Simulation Terminated.");
                endwin();
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
			case STORE: // Look at ST. Microstate 16 is the store to memory
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
                //state = FETCH;
			case MEM:
				switch (cpu->buffers[2].Opcode) {
					case ST:
					case STR:
					case STI:
					case LD:
					case LDR:
					case LDI:
					case LEA:
						break;
				}
				//Do Mem work with EBuff (buffers[2]) here first.
				cpu->buffers[3] = cpu->buffers[2];
				cpu->buffers[3].A = cpu->alu.A;
				cpu->buffers[3].B = cpu->alu.B;
			case EXECUTE: // Note that ST does not have an execute microstate
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
				}
				cpu->buffers[2] = cpu->buffers[1];
				cpu->buffers[2].A;
				cpu->buffers[2].B;
			case IDRR:
				cpu->buffers[1].Opcode = cpu->buffers[0].IR >> OPCODE_SHIFT;			//Decode Stage
				cpu->buffers[1].Rd = cpu->buffers[0].IR & DR_MASK;
				cpu->buffers[1].Rd = (short)cpu->buffers[1].Rd >> DR_SHIFT;
				cpu->buffers[1].A = cpu->buffers[0].IR & SR_MASK;
				cpu->buffers[1].A = (short)cpu->buffers[1].A >> SR_SHIFT;
				cpu->buffers[1].B = cpu->buffers[1].B & SR2_MASK;
				immed_offset = cpu->buffers[0].IR & SR_MASK;
				BaseR = (cpu->buffers[0].IR & BASE_MASK) >> SR_SHIFT;
				
				//IDRR Buffer
				
                switch (cpu->buffers[1].Opcode) {							//Evaluate Address Stage
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

                switch (cpu->buffers[1].Opcode) {							//Fetch Operand Stage
					case LDR:
					case LD:
					 cpu->MDR = memory[cpu->MAR];
						break;
					case ADD:
						if(HIGH_ORDER_BIT_VALUE6 & cpu->ir){ //0000|0000|0010|0000
							cpu->buffers[2].A = cpu->r[cpu->buffers[1].A];
							cpu->buffers[2].B = (immed_offset & SEXT5_MASK);
						} else{
							cpu->A = cpu->r[Rs1];
							cpu->B = cpu->r[Rs2];
						}

						break;
					case AND:
					if(HIGH_ORDER_BIT_VALUE6 & cpu->ir){ //0000|0000|0010|0000
							cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
							cpu->buffers[1].B = (immed_offset & SEXT5_MASK);
						} else{
							cpu->buffers[1].A = cpu->r[cpu->buffers[1].A];
							cpu->buffers[1].B = cpu->r[cpu->buffers[1].B];
						}
						break;
					case NOT:
						cpu->A = cpu->r[cpu->buffers[1].A];
						break;
					case STR:
					case ST:
						cpu->MDR = cpu->r[cpu->buffers[1].Rd];
						break;
					case TRAP:
						cpu->MDR = memory[cpu->MAR];
						cpu->r[7] = cpu->buffers[1].PC;
                }
				cpu->buffers[1] = cpu->buffers[0];
				cpu->buffers[1].A; //might need to take these out
				cpu->buffers[1].B;
                state = EXECUTE;
            case FETCH: // microstates 18, 33, 35 in the book
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
            
                break;
        }
		if (!isRunning) {
			display(cpu, 0);
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

int main(int argc, char* argv[]){

	setvbuf(stdout, NULL, _IONBF, 0);
	isLoaded = 0;
	memShift = 0;
	CPU_p cpu = malloc(sizeof(CPU_s));
	cpuInit(cpu);
	display(cpu, memShift);
	dialog(cpu);
	return 0;
}*/

int main() {
	setvbuf(stdout, NULL, _IONBF, 0);
	isLoaded = 0;
    memShift = 0;
    CPU_p cpu = malloc(sizeof(CPU_s));
    cpuInit(cpu);
    initializeWindow();
    display(cpu, memShift);
    endwin();
	return 0;
}