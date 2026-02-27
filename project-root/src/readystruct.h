#ifndef READYSTRUCT_H
#define READYSTRUCT_H

#include "pcb.h"

typedef enum {
    FCFS,
    SJF,
    RR,
    AGING, 
    RR30
} SchedulingAlgorithm;

typedef struct {
    PCB* head;
    PCB* tail;
    SchedulingAlgorithm algorithm;
} ready_queue;



void ready_queue_init(ready_queue* rq, SchedulingAlgorithm algo);
void ready_queue_enqueue(ready_queue* rq, PCB* new_pcb);
void ready_queue_enqueue_front(ready_queue* rq, PCB* new_pcb);
PCB* ready_queue_dequeue(ready_queue* rq);
int ready_queue_is_empty(ready_queue* rq);
void ready_queue_age(ready_queue* rq);
void ready_queue_enqueue_front(ready_queue* rq, PCB* new_pcb);


#endif