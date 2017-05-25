#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "../slc3.h"

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

void refreshALlMain

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



void DisplayPiplineWindow() {
    currentWindow = 0;
    box(Pipeline, 0, 0);
    mvwprintw(Pipeline, 0, 15, "PipeLine Info:");
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
    DisplayPiplineWindow();
    IOwindow();
    MainInputWindow();
    refresh();
}

void Loadfile(){
    FILE * inputfile;
    char fileName[MAX_FILE_NAME];
    mvwprintw(MainInput, 2, 1, "File Name:");
    mvwscanw(MainInput, 2, 12, "%s", &fileName);
//    mvwprintw(MainInput, 2, 35, "File name is:");
//    mvwprintw(MainInput, 2, 49, fileName);

    inputfile = fopen(fileName, "rw");
    if (inputfile == NULL) {
        mvwprintw(MainInput, 2, 35, "Couldnt open file!");
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
    refreshAllMain();
    fclose(inputFile);
    break;
}
void Exit(){
    clear();
    endwin();
    exit(0);
}

void saveFile(){
    mvwprintw(MainInput, 2, 1, "Cannot save without Loading Assembly Code First!");
}

void Edit(){
    mvwprintw(MainInput, 2, 1, "Cannot edit without Loading Assembly Code First!");
}

void Step(){
    mvwprintw(MainInput, 2, 1, "Cannot step without Loading Assembly Code First!");
}

void displayMem(){
    mvwprintw(MainInput, 2, 1, "Cannot Display Memory without Loading Assembly Code First!");
}

void switchView(){
    if (currentWindow == 0) {
        DisplayCacheWindow();
        mvwprintw(MainInput, 2, 1, "Cache Info Window Displayed!");
    } else {
        DisplayPiplineWindow();
        mvwprintw(MainInput, 2, 1, "Pipeline Info Window Displayed!");
    }
}

void Run(){
    mvwprintw(MainInput, 2, 1, "Cannot Run without Loading Assembly Code First!");
}

void breakPoint(){
    mvwprintw(MainInput, 2, 1, "Cannot use break points without Loading Assembly Code First!");
}

void MainInputWindow() {

    keypad(MainInput, true);
    box(MainInput, 0, 0);
    char* choices[9] = {"Load", "Save", "Step", "Display Mem", "(Un)set Brkpt", "Switch View", "Edit", "Run", "Exit"};
    int choice;
    int highlight = 0;
    while (1){
        mvwprintw(MainInput, 1, 1, "Select: ");
        for (int i = 0; i < 9; i++) {
            if (i == highlight)
                wattron(MainInput, A_REVERSE);

            if (i == 0)
                mvwprintw(MainInput, 1, 9, choices[i]);
            if (i == 1)
                mvwprintw(MainInput, 1, 14, choices[i]);
            if (i == 2)
                mvwprintw(MainInput, 1, 19, choices[i]);
            if (i == 3)
                mvwprintw(MainInput, 1, 24, choices[i]);
            if (i == 4)
                mvwprintw(MainInput, 1, 36, choices[i]);
            if (i == 5)
                mvwprintw(MainInput, 1, 50, choices[i]);
            if (i == 6)
                mvwprintw(MainInput, 1, 62, choices[i]);
            if (i == 7)
                mvwprintw(MainInput, 1, 67, choices[i]);
            if (i == 8)
                mvwprintw(MainInput, 1, 71, choices[i]);
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
                    Loadfile();
                }
                if (choices[highlight] == "Exit") {
                    Exit();
                }

                if (choices[highlight] == "Save") {
                    saveFile();
                }
                if (choices[highlight] == "Edit") {
                    Edit();
                }
                if (choices[highlight] == "Step") {
                    Step();
                }
                if (choices[highlight] == "Display Mem") {
                    displayMem();
                }
                if (choices[highlight] == "Switch View") {
                    switchView();
                }
                if (choices[highlight] == "Run") {
                    Run();
                }
                if (choices[highlight] ==  "(Un)set Brkpt") {
                    breakPoint();
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


