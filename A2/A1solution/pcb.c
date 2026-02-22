#include <stdlib.h>
#include <stdio.h>
#include "pcb.h"


// Static variable to keep track of the next PID to assign
static int next_pid = 1;
void pcb_init() {
    next_pid = 1;
}

// Function to get the next PID
int pcb_get_next_pid() {
    return next_pid++;
}

// Function to create a new PCB with the given start and end times (instructions)
PCB* pcb_create(int start, int end) {
    PCB* new_pcb = malloc(sizeof(PCB));
    if (new_pcb == NULL) {
        return NULL;
    }
    new_pcb->pid = pcb_get_next_pid();
    new_pcb->start = start;
    new_pcb->end = end;
    new_pcb->pc = start;
    new_pcb->next_pcb = NULL;
    new_pcb->job_length_score = end - start + 1;
    return new_pcb;
}

// Function to free the memory allocated for a PCB  
void pcb_destroy(PCB* old_pcb) {
    if (old_pcb == NULL) {
        return;
    }
    free(old_pcb);
}



