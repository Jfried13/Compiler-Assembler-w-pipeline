#include <curses.h>
#include "view.c"

unsigned short memory[MAX_MEMORY];  // 500 words of memory enough to store simple program
int memShift;
int isLoaded;
int memShift;
char option;


int main() {

    memShift = 0;
    CPU_p cpu = malloc(sizeof(CPU_s));
    cpuInit(cpu);
    initializeWindow();
    display(cpu, memShift);
    endwin();
}

/*
	This function initializes the cpu fields
*/
void cpuInit(CPU_p cpu) {
    cpu->r[0] = 0x2350;
    cpu->r[1] = 0x0500;
    cpu->r[2] = 0x3040;
    cpu->r[3] = 0x0000;
    cpu->r[4] = 0x0400;
    cpu->r[5] = 0x0000;
    cpu->r[6] = 0x3000;
    cpu->r[7] = 0x0500;
    cpu->ir = 0x0000;
    cpu->PC = START_MEM;
    cpu->MAR = 0x0200;
    cpu->MDR = 0x1000;
    cpu->A = 0x0077;
    cpu->B = 0x00AD;
    cpu->N = 0;
    cpu->Z = 1;
    cpu->P = 0;

}

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