#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "Other crap/tempslc3.h"
//#include "slc3Pipe.c"

// Created by Daniel Ivanov on 5/12/2017.
// this class if for the NCurses Gui.
//

WINDOW *MainWindow;
WINDOW *IOWindow;
WINDOW *RegisterWindow;
WINDOW *MemoryWindow;
WINDOW *CPUWindow;
WINDOW *MainInput;
WINDOW *Pipeline;
WINDOW *CacheWindow;

// 0 for pipline window, 1 for cache window
int currentWindow = 0;
int loaded = 0;

#define PHEIGHT 18
#define PWIDTH 48

#define CHEIGHT 18
#define CWIDTH 48

#define INPUTHEIGHT 4
#define INPUTWIDTH 78

#define MAINHEIGHT 25
#define MAINWIDTH 80

#define IOHEIGHT 10
#define IOWIDTH 80

#define RegHeight 9
#define RegWidth 12

#define MemHeight 17
#define MemWidth 15

#define CPUHeight 8
#define CPUWidth 13

void MainInputWindow();
int checkIfFileExists(char* fileToCheckIfExists);
void writeMemory(char * nameOfFileToWriteTo);

void displayTitle() {
    mvwprintw(MainWindow, 0, 28, "Welcome to LC3 Simulator");
    mvwprintw(IOWindow, 0, 37, " I/O ");
}


void mainWindow(){
    box(MainWindow, 0, 0);
    displayTitle();
    wrefresh(MainWindow);
}


void IOwindow() {
    box(IOWindow, 0, 0);
    displayTitle();
    wrefresh(IOWindow);

}

void RegWindow(CPU_p cpu){
    box(RegisterWindow, 0, 0);
    mvwprintw(RegisterWindow, 0, 1, "Registers:");
    for(int i = 0; i < 8; i++) {
        mvwprintw(RegisterWindow, 1 + i, 1, "R%d: x%04X", i, cpu->r[i]);
    }
    wrefresh(RegisterWindow);
}

void MemWindow(int memShift) {
    box(MemoryWindow, 0, 0);
    mvwprintw(MemoryWindow, 0, 4, "Memory:");
    for (int i = 0; i < 16; i++) {
        mvwprintw(MemoryWindow, 1 + i, 1, "x%X: x%04X", START_MEM + i + memShift, memory[i + memShift]);
    }
    wrefresh(MemoryWindow);
}

void CPUwindow(CPU_p cpu) {
    box(CPUWindow, 0, 0);
    mvwprintw(CPUWindow, 0, 1, "CPU Data:");
    mvwprintw(CPUWindow, 1,1, "PC:x%04X", cpu->PC);
    mvwprintw(CPUWindow, 2,1, "IR:x%04X", cpu->ir);
    mvwprintw(CPUWindow, 3,1, "A: x%04X", cpu->A);
    mvwprintw(CPUWindow, 4,1, "B: x%04X", cpu->B);
    mvwprintw(CPUWindow, 5,1, "MAR:x%04X", cpu->MAR);
    mvwprintw(CPUWindow, 6,1, "MDR:x%04X", cpu->MDR);
    mvwprintw(CPUWindow, 7,1, "N:%d Z:%d P:%d", cpu->N, cpu->Z, cpu->P);
    wrefresh(CPUWindow);
}



void DisplayPipelineWindow(CPU_p cpu) {
    currentWindow = 0;
    box(Pipeline, 0, 0);
    mvwprintw(Pipeline, 0, 15, "PipeLine Info:");
	mvwprintw(Pipeline, 3, 4, "FBUFF: PC:  IR: ");
	mvwprintw(Pipeline, 4, 11, "0x%X  0x%X", cpu->buffers[0].PC, cpu->buffers[0].IR); 
	mvwprintw(Pipeline, 6, 4, "DBUFF: Op:  DR:  SR1:  SEXT/SR2:");
	mvwprintw(Pipeline, 7, 11, "0x%X  0x%X  0x%X   0x%X", cpu->buffers[1].Opcode, cpu->buffers[1].Rd, cpu->buffers[1].A, cpu->buffers[1].B);
	mvwprintw(Pipeline, 9, 4, "EBUFF: Op:  DR:  RESULT:");
    printw("lhello");
	mvwprintw(Pipeline, 10, 11, "0x%X  0x%X  0x%X", cpu->buffers[2].Opcode, cpu->buffers[2].Rd, cpu->buffers[2].B);
	mvwprintw(Pipeline, 12, 4, "MBUFF: Op:  DR:  RESULT:");
	mvwprintw(Pipeline, 13, 11, "0x%X  0x%X  0x%X", cpu->buffers[3].Opcode, cpu->buffers[3].Rd, cpu->buffers[3].B);
    wrefresh(Pipeline);
}

void DisplayCacheWindow(){
    currentWindow = 1;
    box(Pipeline, 0, 0);
    mvwprintw(Pipeline, 0, 15, "Cache Info:");
    wrefresh(Pipeline);
}

void display(CPU_p cpu, int mem) {
    mainWindow();
    RegWindow(cpu);
    MemWindow(mem);
    CPUwindow(cpu);
    DisplayPipelineWindow(cpu);
    IOwindow();
    MainInputWindow(cpu);
    refresh();
}

void MainInputWindow(CPU_p cpu) {
	FILE *inputFile;
    char fileName[MAX_FILE_NAME];
    char saveFile[20];
    keypad(MainInput, true);
    box(MainInput, 0, 0);
    char *ptr;
    char* choices[9] = {"Load", "Save", "Step", "Dsply_Mem", "Switch_View", "Edit", "Run", "Set_Brkpts", "Exit"};
    int choice, i = 0, garbage = 0;
    int highlight = 0;
	//int loaded = 0;
    while (1){
        mvwprintw(MainInput, 1, 1, "Select: ");
        for (int i = 0; i < 9; i++) {
            if (i == highlight)
                wattron(MainInput, A_REVERSE);

            if (i == 0)
                mvwprintw(MainInput, 1, 9, choices[i]);
            if (i == 1)
                mvwprintw(MainInput, 1, 15, choices[i]);
            if (i == 2)
                mvwprintw(MainInput, 1, 21, choices[i]);
            if (i == 3)
                mvwprintw(MainInput, 1, 27, choices[i]);
            if (i == 4)
                mvwprintw(MainInput, 1, 38, choices[i]);
            if (i == 5)
                mvwprintw(MainInput, 1, 51, choices[i]);
            if (i == 6)
                mvwprintw(MainInput, 1, 57, choices[i]);
            if (i == 7)
                mvwprintw(MainInput, 1, 62, choices[i]);
            if (i == 8)
                mvwprintw(MainInput, 1, 73, choices[i]);
            wattroff(MainInput, A_REVERSE);
        }
        choice = wgetch(MainInput);
        switch(choice) {
            case KEY_LEFT:
                highlight--;
                if (highlight == -1)
                    highlight = 0;
                break;
            case KEY_RIGHT:
                highlight++;
                if (highlight == 9)
                    highlight = 8;
                break;
            case ENTER_KEY:
                wmove(MainInput, 2, 1);
                wclrtoeol(MainInput);
                mvwprintw(MainInput, 2, 77, "|");
                
				if (choices[highlight] == "Load") {
                    mvwprintw(MainInput, 2, 1, "File Name:");
                    mvwscanw(MainInput, 2, 12, "%s", &fileName);
					if(checkIfFileExists(fileName)) {
						inputFile = fopen(fileName, "r");
						fscanf(inputFile, "%04X", &garbage);
						while (fscanf(inputFile, "%04X", &memory[i]) != EOF) {
							if (!i) cpu->PC = memory[0];
							i++;
						}
					
                    mvwprintw(MainInput, 2, 35, "File name is:");
                    mvwprintw(MainInput, 2, 49, fileName);
					loaded = 1;
					MemWindow(0);
					} else {
						mvwprintw(MainInput, 2, 35, "Invalid File");
					}
						
                }
                if (choices[highlight] == "Exit") {
                    clear();
                    endwin();
                    exit(0);
                }
                //The only thing that this needs is for user to be able to input the memory location with x in front
                //and to also re display the memory so the new memory locations is centered in the displayed memory
				if (choices[highlight] == "Edit") {
                    char newMemoryValue[6];
                    unsigned short placeInMemory, newValue;
					mvwprintw(MainInput, 2, 1, "What memory address would you like to edit:");
                    mvwscanw(MainInput, 2, 44, "%04x", &placeInMemory);
                    mvwprintw(MainInput, 2, 1, "The contents of location x%04x is x%04x", placeInMemory, memory[placeInMemory - START_MEM]);
                    mvwprintw(MainInput, 2, 1, "What would you like the new value to be in location x%04x:", placeInMemory);
                    mvwscanw(MainInput, 2, 59, "%s", &newMemoryValue);
                    newValue = (short)strtol(newMemoryValue, &ptr, 16);
                    memory[placeInMemory - START_MEM] = newValue;
                    MemWindow(0);
					
                }
				if (choices[highlight] == "Save") {
                     //mvwprintw(MainInput, 2, 1, "Save Selected");
                     FILE *filePtr;
                     //char *ptr;
                     char begNum[4];
                     char endNum[4];
                     long beg, end;
                     unsigned int memoryStart;
                     mvwprintw(MainInput, 2, 1, "File Name:");
                     mvwscanw(MainInput, 2, 12, "%s", &fileName);
                     mvwprintw(MainInput, 2, 20, "File to save:");
                     writeMemory(fileName);
                }
				
                if (choices[highlight] == "Step") {
					if(!loaded) {
						mvwprintw(MainInput, 2, 1, "Cannot step without Loading Assembly Code First!");
						display(cpu, 0);
					} else {
						controller(cpu, 0);
						display(cpu, 0);
					}
					
                }
                if (choices[highlight] == "Dsply_Mem") {
                    mvwprintw(MainInput, 2, 1, "Cannot Display Memory without Loading Assembly Code First!");
					display(cpu, 0);

                }
                if (choices[highlight] == "Switch_View") {
                    if (currentWindow == 0) {
                        DisplayCacheWindow();
                        mvwprintw(MainInput, 2, 1, "Cache Info Window Displayed!");
                    } else {
                        DisplayPipelineWindow(cpu);
                        mvwprintw(MainInput, 2, 1, "Pipeline Info Window Displayed!");
                    }
                }
				
				 if (choices[highlight] == "Set_Brkpts") {
                    mvwprintw(MainInput, 2, 1, "Reached stbrk");
                }
				
                if (choices[highlight] == "Run") {
                    mvwprintw(MainInput, 2, 1, "Cannot Run without Loading Assembly Code First!");
					controller(cpu, 1);
					display(cpu, 0);


                }
                break;
            default:
                break;
        }
    }
}

void initializeWindow() {
    initscr();
    MainWindow = newwin(MAINHEIGHT, MAINWIDTH, 0, 0);
    IOWindow = newwin(IOHEIGHT, IOWIDTH, 25, 0);
    RegisterWindow = newwin(RegHeight, RegWidth, 1, 1);
    MemoryWindow = newwin(MemHeight, MemWidth, 1, 64);
    CPUWindow = newwin(CPUHeight, CPUWidth, 11, 1);
    MainInput = newwin(INPUTHEIGHT, INPUTWIDTH, 20, 1);
    Pipeline = newwin(PHEIGHT,PWIDTH, 1, 15);
    CacheWindow = newwin(PHEIGHT,PWIDTH, 1, 15);
}

/*
This function takes a string as a parameter and then attempts to
open a file to read with that name.  If it was successful then the 
file exists and we return a 1 else return 0.
*/
int checkIfFileExists(char* fileToCheckIfExists) {
	FILE* filePtr;
    if(filePtr = fopen(fileToCheckIfExists, "r")) {
		fclose(filePtr);
		return 1;
	} else {
        fclose(filePtr);
		return 0;
		
	}
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
//need to fix some magic numbers in here
//need to #define TRAP25 61477;
//needs to be broken down into two methods
/*
This file takes a string as a file name and checks to see if it exists,
if it does it prompts the user to see if they would like to overwrite the file
if they don't than return back to the user choices else ask for the part of memory
to write to a new file and copy it.
*/
void writeMemory(char * nameOfFileToWriteTo) {
    FILE * filePtr;
    char *ptr;
    char begNum[4];
    char endNum[4];
    
    if(checkIfFileExists(nameOfFileToWriteTo) == 1) {
        char userResponse[4];
        mvwprintw(MainInput, 2, 1, "This file already exists would you like to overwrite(yes/no):");
        mvwscanw(MainInput, 2, 62, "%s", &userResponse);
        if(strcmp(userResponse, "yes") == 0) {
            filePtr = fopen(nameOfFileToWriteTo, "w");
            mvwprintw(MainInput, 2, 1, "Enter the beginning of memory to save:");
            mvwscanw(MainInput, 2, 39, "%s", &begNum);
            mvwprintw(MainInput, 2, 43, "End of memory to save:");
            mvwscanw(MainInput, 2, 65, "%s", &endNum);
            mvwprintw(MainInput, 2, 1, endNum);
            int beg = strtol(begNum, &ptr, 10) - 3000;
            int end = strtol(endNum, &ptr, 10) - 3000;
            for(int i=beg; i <= end; i++) { 
                fprintf(filePtr, "%04x\n", memory[i]);
            }
        } 
    } else {
	    
	    filePtr = fopen(nameOfFileToWriteTo, "w");
        mvwprintw(MainInput, 2, 1, "Enter the beginning of memory to save:");
        mvwscanw(MainInput, 2, 39, "%s", &begNum);
        mvwprintw(MainInput, 2, 43, "End of memory to save:");
        mvwscanw(MainInput, 2, 65, "%s", &endNum);
        mvwprintw(MainInput, 2, 1, endNum);
        int beg = strtol(begNum, &ptr, 10) - 3000;
        int end = strtol(endNum, &ptr, 10) - 3000;
        for(int i=beg; i <= end; i++) { 
            fprintf(filePtr, "%04x\n", memory[i]);
        }
        
    }
    fclose(filePtr);
}






