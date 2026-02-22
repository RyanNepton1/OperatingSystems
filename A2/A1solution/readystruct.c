#include "readystruct.h"
#include <stdlib.h>
#include <stdio.h>


// Function to initialize the ready queue
void ready_queue_init(ready_queue* rq, SchedulingAlgorithm algorithm) {
    rq->head = NULL;
    rq->tail = NULL;
    rq->algorithm = algorithm;
}

// Function to enqueue a PCB into the ready queue based on the scheduling algorithm
void ready_queue_enqueue(ready_queue* rq, PCB* new_pcb) {
    new_pcb ->next_pcb = NULL;
    // Add to end if FCFS or RR
    if (rq->algorithm == FCFS || rq->algorithm == RR ) {
        if (rq->tail == NULL) {
            rq->head = new_pcb;
            rq->tail = new_pcb;
        } else {
            rq->tail->next_pcb = new_pcb;
            rq->tail = new_pcb;
        }
    }
    // Add based on job length score if SJF or AGING
    else if (rq->algorithm == SJF || rq->algorithm == AGING) {
        if (rq->head == NULL) {
            rq->head = new_pcb;
            rq->tail = new_pcb;
        } else if (rq->head->job_length_score > new_pcb->job_length_score) {
            new_pcb->next_pcb = rq->head;
            rq->head = new_pcb;
        } else {
            // Iterate through LL and insert when appropriate
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

// Dequeue the head of the PCB LL
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

// Check if the ready queue is empty
int ready_queue_is_empty(ready_queue* rq) {
    if (rq->head == NULL) {
        return 1;
    } else {
        return 0;
    }
}

// Function to age the PCBs in the ready queue (only for AGING algorithm)
void ready_queue_age(ready_queue* rq) {
    if (rq->algorithm != AGING || rq->head == NULL) {
        return;
    }
    
    PCB* curr = rq->head;
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

// Enqueue a PCB at the front of the ready queue (force to front)
void ready_queue_enqueue_front(ready_queue* rq, PCB* new_pcb) {
    new_pcb->next_pcb = NULL;
    if (rq->head == NULL) {
        rq->head = new_pcb;
        rq->tail = new_pcb;
    } else {
        new_pcb->next_pcb = rq->head;
        rq->head = new_pcb;
    }
}


