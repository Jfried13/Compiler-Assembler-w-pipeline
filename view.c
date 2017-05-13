#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "slc3.h"

// Created by Daniel Ivanov on 5/12/2017.
// this class if for the NCurses Gui.
//

#define MAINHEIGHT 80
#define MAINWIDTH 100

#define IOHEIGHT 20
#define IOWIDTH 100

void IOWindow (CPU_p cpu, memory) {
        WINDOW *IOWindow = newwin(IOHEIGHT, IOWIDTH, 3, 5);
}


void mainWindow(CPU_p cpu, memory){
    WINDOW *MainWindow = newwin(MAINHEIGHT, MAINWIDTH, 5, 5);
}

void displayWelcome() {
    int height = 5;
    int width = 200;
    int startx = 40;
    int starty = 5;
}

void displayRegisters(CPU_p cpu) {
    printw("\n\nRegisters:");
    for (int i = 0; i < 8; i++) {
        printw("\nR: %d x%04X", i, cpu->r[i]);
    }
}

void displayMemory(int mem) {
    printw("\n\n Memory:");
    for(int i = 0; i < 16; i++){
        printw("\nx%X: x%04X", i + START_MEM + mem, memory[(i+1) + mem]);
    }
}

void displayCPUConditions(CPU_p cpu) {
    printw("PC: x%0.4X    IR:x%04X", cpu->PC, cpu->ir);
    printw("A: x%04X    B: x%04X", cpu->A, cpu->B);
    printw("MAR: x%04X  MDR:x%04X", cpu->MAR, cpu->MDR);

}

void displayInput(CPU_p cpu) {
    printw("  Select: 1)Load, 3)Step, 5)Display Mem, 7)Run, 9)Exit\n");
    printw("=================================================================\n");
    printw("Input: ");
    dialog(cpu);
}

void displayOutput(CPU_p cpu) {
    printw("Output: ");
    dialog(cpu);
}

void display(CPU_p cpu, int mem) {
    displayWelcome();
    displayRegisters(cpu);
    displayMemory(mem);
    displayCPUConditions(cpu);
    displayInput(cpu);
    displayOutput(cpu);
    refresh();

};