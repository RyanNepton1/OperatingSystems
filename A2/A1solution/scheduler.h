#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "readystruct.h"

#define Max_Length 1000

void run_scheduler(ready_queue* rq);
void clean_PCB(PCB* pcb);


#endif