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


#define MAINHEIGHT 25
#define MAINWIDTH 80

#define IOHEIGHT 10
#define IOWIDTH 80

#define RegHeight 9
#define RegWidth 12

#define MemHeight 17
#define MemWidth 15

#define CPUHeight 8
#define CPUWidth 21

void displayTitle() {
    mvwprintw(MainWindow, 0, 28, "Welcome to LC3 Simulator");
    mvwprintw(IOWindow, 0, 37, " I/O ");
}


void mainWindow(){
    box(MainWindow, 0, 0);
    displayTitle();
    wrefresh(MainWindow);
}


void IOwindow () {
    box(IOWindow, 0, 0);
    displayTitle();
    wrefresh(IOWindow);

}

void RegWindow(CPU_p cpu){
    //box(RegisterWindow, 0, 0);
    mvwprintw(RegisterWindow, 0, 1, "Registers:");
    for(int i = 0; i < 8; i++) {
        mvwprintw(RegisterWindow, 1 + i, 1, "R%d: x%04X", i, cpu->r[i]);
    }
    wrefresh(RegisterWindow);
}

void MemWindow(memShift) {
    //box(MemoryWindow, 0, 0);
    mvwprintw(MemoryWindow, 0, 4, "Memory:");
    for (int i = 0; i < 16; i++) {
        mvwprintw(MemoryWindow, 1 + i, 1, "x%X: x%04X", START_MEM + i + memShift, memory[i + memShift]);
    }
    wrefresh(MemoryWindow);
}

void CPUwindow(CPU_p cpu) {
    //box(CPUWindow, 0, 0);
    mvwprintw(CPUWindow, 0, 1, "CPU Data:");
    //mvwprintw(CPUWindow, 1, 1, "PC:x%04X   IR:x%04X", cpu->PC, cpu->ir);
    mvwprintw(CPUWindow, 1,1, "PC:x%04X", cpu->PC);
    mvwprintw(CPUWindow, 2,1, "IR:x%04X", cpu->ir);

    mvwprintw(CPUWindow, 3,1, "A: x%04X", cpu->A);
    mvwprintw(CPUWindow, 4,1, "B: x%04X", cpu->B);
    //mvwprintw(CPUWindow, 2, 1, "A: x%04X   B: x%04X", cpu->A, cpu->B);

    //mvwprintw(CPUWindow, 3, 1, "MAR:x%04X  MDR:x%04X", cpu->MAR, cpu->MDR);
    mvwprintw(CPUWindow, 5,1, "MAR:x%04X", cpu->MAR);
    mvwprintw(CPUWindow, 6,1, "MDR:x%04X", cpu->MDR);

    mvwprintw(CPUWindow, 7,1, "N%d Z:%d P:%d", cpu->N, cpu->Z, cpu->P);

    //mvwprintw(CPUWindow, 4, 1, "N:%d   Z:%d   P:%d", cpu->N, cpu->Z, cpu->P);
    wrefresh(CPUWindow);
}

void MainInputWindow() {
    box(MainInput, 0, 0);
    mvwprintw(MainInput, 1, 1, "  Select: 1)Load, 3)Step, 5)Display Mem, 7)Run, 9)Exit");
    mvwprintw(MainInput, 2, 1, "  Input: ");
    wrefresh(MainInput);
    int value = getch();
    mvwprintw(MainInput, 2, 9, " %c", value);
    wrefresh(MainInput);
}

void display(CPU_p cpu, int mem) {
    refresh();
    mainWindow();
    RegWindow(cpu);
    MemWindow(mem);
    CPUwindow(cpu);
    IOwindow();
    MainInputWindow();
    getch();
}

void initializeWindow() {
    initscr();
    MainWindow = newwin(MAINHEIGHT, MAINWIDTH, 0, 0);
    IOWindow = newwin(IOHEIGHT, IOWIDTH, 25, 0);
    RegisterWindow = newwin(RegHeight, RegWidth, 1, 1);
    MemoryWindow = newwin(MemHeight, MemWidth, 1, 64);
    CPUWindow = newwin(CPUHeight, CPUWidth, 11, 1);
    MainInput = newwin(4, 78, 20, 1);
}


