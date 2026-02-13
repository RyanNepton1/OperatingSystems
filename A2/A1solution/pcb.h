#define pcb.h
#include "shellmemory.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int pid;
    int start;
    int end;
    int pc;
} PCB;