#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "readystruct.h"

#define Max_Length 1000

void run_scheduler(ready_queue* rq);
void clean_PCB(PCB* pcb);

PCB* scheduler_get_current_pcb();
void scheduler_request_yield();
void scheduler_init_mt(ready_queue* mt_queue, SchedulingAlgorithm algo);
void scheduler_wait_mt(ready_queue* mt_queue);

// Shutdown the multithreading worker pool
void scheduler_shutdown_mt();

#endif