#include <stdlib.h>
#include <stdio.h>
#include "pcb.h"

static int next_pid = 1;
void pcb_init() {
    next_pid = 1;
}

int pcb_get_next_pid() {
    return next_pid++;
}

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

void pcb_destroy(PCB* old_pcb) {
    if (old_pcb == NULL) {
        return;
    }
    free(old_pcb);
}



