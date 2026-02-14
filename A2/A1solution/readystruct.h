#include "pcb.h"


typedef struct {
    PCB* head;
    PCB* tail;
    SchedulingAlgorithm algorithm;
} ready_queue;

typedef enum {
    FCFS,
    SJF,
    RR,
    AGING
} SchedulingAlgorithm;

void ready_queue_init(ready_queue* rq);
void ready_queue_enqueue(ready_queue* rq, PCB* new_pcb);
PCB* ready_queue_dequeue(ready_queue* rq);
int ready_queue_is_empty(ready_queue* rq);
void ready_queue_age(ready_queue* rq);


