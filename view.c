#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "tempslc3.h"

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
    keypad(MainInput, true);
    box(MainInput, 0, 0);
    char* choices[6] = {"Load", "Step", "Display Mem", "Switch View", "Run", "Exit"};
    int choice, i = 0, garbage = 0;
    int highlight = 0;
    while (1){
        mvwprintw(MainInput, 1, 1, "Select: ");
        for (int i = 0; i < 6; i++) {
            if (i == highlight)
                wattron(MainInput, A_REVERSE);

            if (i == 0)
                mvwprintw(MainInput, 1, 9, choices[i]);
            if (i == 1)
                mvwprintw(MainInput, 1, 16, choices[i]);
            if (i == 2)
                mvwprintw(MainInput, 1, 23, choices[i]);
            if (i == 3)
                mvwprintw(MainInput, 1, 37, choices[i]);
            if (i == 4)
                mvwprintw(MainInput, 1, 51, choices[i]);
            if (i == 5)
                mvwprintw(MainInput, 1, 57, choices[i]);
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
                if (highlight == 6)
                    highlight = 5;
                break;
            case ENTER_KEY:
                wmove(MainInput, 2, 1);
                wclrtoeol(MainInput);
                mvwprintw(MainInput, 2, 77, "|");
                if (choices[highlight] == "Load") {
                    mvwprintw(MainInput, 2, 1, "File Name:");
                    mvwscanw(MainInput, 2, 12, "%s", &fileName);
					inputFile = fopen(fileName, "r");
					fscanf(inputFile, "%04X", &garbage);
					while (fscanf(inputFile, "%04X", &memory[i]) != EOF) {
						if (!i) cpu->PC = memory[0];
						i++;
					}
                    mvwprintw(MainInput, 2, 35, "File name is:");
                    mvwprintw(MainInput, 2, 49, fileName);
					MemWindow(0);
                }
                if (choices[highlight] == "Exit") {
                    clear();
                    endwin();
                    exit(0);
                }
                if (choices[highlight] == "Step") {
                    mvwprintw(MainInput, 2, 1, "Cannot step without Loading Assembly Code First!");
					display(cpu, 0);
                }
                if (choices[highlight] == "Display Mem") {
                    mvwprintw(MainInput, 2, 1, "Cannot Display Memory without Loading Assembly Code First!");
					display(cpu, 0);

                }
                if (choices[highlight] == "Switch View") {
                    if (currentWindow == 0) {
                        DisplayCacheWindow();
                        mvwprintw(MainInput, 2, 1, "Cache Info Window Displayed!");
                    } else {
                        DisplayPipelineWindow(cpu);
                        mvwprintw(MainInput, 2, 1, "Pipeline Info Window Displayed!");
                    }
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


