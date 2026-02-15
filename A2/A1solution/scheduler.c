#include "scheduler.h"
#include "pcb.h"
#include "readystruct.h"
#include <stdio.h>
#include "interpreter.h"
#include "shell.h"
#include "shellmemory.h"

void run_scheduler(ready_queue* rq) {
    int errorCode = 0;
    // Check if preemptive algo
    if (rq->algorithm == SJF || rq->algorithm == FCFS || rq->algorithm == NULL) {
        while (ready_queue_is_empty(rq) == 0) {
            // Dequeu and run
            PCB* curr_pcb = ready_queue_dequeue(rq);
            if (curr_pcb == NULL) {
                break;
            }
            // Iterate and run each line
            while (curr_pcb->pc <= curr_pcb->end) {
                char* line = code_get_line(curr_pcb->pc);
                if (line != NULL) {
                    errorCode += parseInput(line);
                }
                curr_pcb->pc++;
            }
            // Free space
            free_code_memory(curr_pcb->start, curr_pcb->end);
            pcb_destroy(curr_pcb);
        }
    }
    // Check if RR
    else if (rq->algorithm == RR) {
        while (ready_queue_is_empty(rq) == 0) {
            // Dequeue
            PCB* curr_pcb = ready_queue_dequeue(rq);
            if (curr_pcb == NULL) {
                break;
            }
            // Set # of lines to run for this time slice
            int time_slice = 2;
            for (int i = 0; i < time_slice; i++) {
                char* line = code_get_line(curr_pcb->pc);
                if (line != NULL) {
                    errorCode += parseInput(line);
                }
                curr_pcb->pc++;
            }
            // Check if done and enqueue or destroy
            if (curr_pcb->pc <= curr_pcb->end) {
                ready_queue_enqueue(rq, curr_pcb);
            } else {
                free_code_memory(curr_pcb->start, curr_pcb->end);
                pcb_destroy(curr_pcb);
            }
        }
    }
    // Check if AGING
    else if (rq->algorithm == AGING) {
        // Set time slice to 1
        int time_slice = 1;
        while (ready_queue_is_empty(rq) == 0) {
            PCB* curr_pcb = ready_queue_dequeue(rq);
            if (curr_pcb == NULL) {
                break;
            }
            for (int i = 0; i < time_slice; i++) {
                char* line = code_get_line(curr_pcb->pc);
                if (line != NULL) {
                    errorCode += parseInput(line);
                }
                curr_pcb->pc++;
            }
            // Age the ready queue after each time slice
            ready_queue_age(rq);
            // Check if done and enqueue or destroy
            if (curr_pcb->pc <= curr_pcb->end) {
                ready_queue_enqueue(rq, curr_pcb);
            } else {
                free_code_memory(curr_pcb->start, curr_pcb->end);
                pcb_destroy(curr_pcb);
            }
        }
    }
}