/*
	Authors: Connor Lundberg, Daniel Ivanov, James Roberts, Joe Freed? Fried? Who the hell cares...
*/

#include "../slc3.h"


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
	printf("  Select: 1)Load, 3)Step, 5)Display Mem, 7)Run, 9)Exit\n");
	return 0;
}


/*
	This is the dialog function that provides the functionality to the choices shown in
	the displayScreen.
*/
int dialog(CPU_p cpu) {
	int opNum = 0, isRunning = 0;
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
				case STEP:
					if (isLoaded == 1) {
						controller(cpu, 0);
						opNum = 0;
					} else {
						printf("No file loaded!");
					}
					break;
				case DISP_MEM:
					printf("Position to move to? (in decimal): ");

					scanf("%d", &memShift);
					if(memShift > MAX_MEMORY - DISP_BOUNDARY) {
						printf("Error: out of memory");
						memShift = 0;
						break;
					} else {
						displayScreen(cpu, memShift);
					}
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