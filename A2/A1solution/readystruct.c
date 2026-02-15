#include "readystruct.h"
#include <stdlib.h>


void ready_queue_init(ready_queue* rq, SchedulingAlgorithm algorithm) {
    rq->head = NULL;
    rq->tail = NULL;
    rq->algorithm = algorithm;
}

void ready_queue_enqueue(ready_queue* rq, PCB* new_pcb) {
    new_pcb ->next_pcb = NULL;
    if (rq->algorithm == FCFS || rq->algorithm == RR ) {
        if (rq->tail == NULL) {
            rq->head = new_pcb;
            rq->tail = new_pcb;
        } else {
            rq->tail->next_pcb = new_pcb;
            rq->tail = new_pcb;
        }
    }
    else if (rq->algorithm == SJF || rq->algorithm == AGING) {
        if (rq->head == NULL) {
            rq->head = new_pcb;
            rq->tail = new_pcb;
        } else if (rq->head->job_length_score > new_pcb->job_length_score) {
            new_pcb->next_pcb = rq->head;
            rq->head = new_pcb;
        } else {
            PCB* checking_pcb = rq->head;
            while ((checking_pcb->next_pcb != NULL) && (checking_pcb->next_pcb->job_length_score <= new_pcb->job_length_score)) {
                checking_pcb = checking_pcb->next_pcb;
            }
            new_pcb->next_pcb = checking_pcb->next_pcb;
            checking_pcb->next_pcb = new_pcb;
            if (new_pcb->next_pcb == NULL) {
                rq->tail = new_pcb;
            }
        }
    }
}

PCB* ready_queue_dequeue(ready_queue* rq) {
    if (rq->head == NULL) {
        return NULL;
    }
    PCB* old_pcb = rq->head;
    rq->head = rq->head->next_pcb;
    if (rq->head == NULL) {
        rq->tail = NULL;
    }
    old_pcb->next_pcb = NULL;
    return old_pcb;
}

int ready_queue_is_empty(ready_queue* rq) {
    if (rq->head == NULL) {
        return 1;
    } else {
        return 0;
    }
}

void ready_queue_age(ready_queue* rq) {
    if (rq->algorithm != AGING || rq->head == NULL) {
        return;
    }
    
    PCB* curr = rq->head->next_pcb;
    while (curr != NULL) {
        if (curr->job_length_score > 0) {
            curr->job_length_score--;
            curr = curr->next_pcb;
        }
        else {
            curr = curr->next_pcb;
        }
    }
}


