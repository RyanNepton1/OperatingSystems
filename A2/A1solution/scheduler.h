#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "readystruct.h"

#define Max_Length 1000

void run_scheduler(ready_queue* rq);
void clean_PCB(PCB* pcb);

// Scheduler runtime helpers used by `exec`/interpreter to interact
// with the currently-running PCB and request yielding.
PCB* scheduler_get_current_pcb();
void scheduler_request_yield();


#endif