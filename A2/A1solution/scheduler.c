#include "scheduler.h"
#include "pcb.h"
#include "readystruct.h"
#include <stdio.h>
#include "interpreter.h"
#include "shell.h"
#include "shellmemory.h"

// Track the currently executing PCB so `exec` can manipulate scheduling
static PCB* scheduler_current_pcb = NULL;
// When set, the scheduler should stop executing the current PCB and
// allow it to be handled by another ready queue.
static int scheduler_yield_requested = 0;

PCB* scheduler_get_current_pcb() {
    return scheduler_current_pcb;
}

void scheduler_request_yield() {
    scheduler_yield_requested = 1;
}

void run_scheduler(ready_queue* rq) {
    int errorCode = 0;
    // Check if preemptive algo
    if (rq->algorithm == SJF || rq->algorithm == FCFS) {
        while (ready_queue_is_empty(rq) == 0) {
            // Dequeu and run
            PCB* curr_pcb = ready_queue_dequeue(rq);
            if (curr_pcb == NULL) {
                break;
            }
            // mark current PCB
            scheduler_current_pcb = curr_pcb;
            int yielded = 0;
            // Iterate and run each line
            while (curr_pcb->pc <= curr_pcb->end) {
                char* line = code_get_line(curr_pcb->pc);
                if (line != NULL) {
                    errorCode += parseInput(line);
                }
                curr_pcb->pc++;
                if (scheduler_yield_requested) {
                    yielded = 1;
                    scheduler_yield_requested = 0;
                    break;
                }
            }
            // clear current marker
            scheduler_current_pcb = NULL;
            // Free space
            if (yielded) {
                // current PCB has been transferred to another ready queue;
                // don't free or destroy it here.
                continue;
            }
            if (curr_pcb->should_free_code) {
                free_code_memory(curr_pcb->start, curr_pcb->end);
            }
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
            scheduler_current_pcb = curr_pcb;
            int yielded = 0;
            // Set # of lines to run for this time slice
            int time_slice = 2;
            for (int i = 0; i < time_slice; i++) {
                char* line = code_get_line(curr_pcb->pc);
                if (line != NULL) {
                    errorCode += parseInput(line);
                }
                curr_pcb->pc++;
                if (scheduler_yield_requested) {
                    yielded = 1;
                    scheduler_yield_requested = 0;
                    break;
                }
            }
            scheduler_current_pcb = NULL;
            // Check if done and enqueue or destroy
            if (yielded) {
                continue;
            }
            if (curr_pcb->pc <= curr_pcb->end) {
                ready_queue_enqueue(rq, curr_pcb);
            } else {
                if (curr_pcb->should_free_code) {
                    free_code_memory(curr_pcb->start, curr_pcb->end);
                }
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
            scheduler_current_pcb = curr_pcb;
            int yielded = 0;
            for (int i = 0; i < time_slice; i++) {
                char* line = code_get_line(curr_pcb->pc);
                if (line != NULL) {
                    errorCode += parseInput(line);
                }
                curr_pcb->pc++;
                if (scheduler_yield_requested) {
                    yielded = 1;
                    scheduler_yield_requested = 0;
                    break;
                }
            }
            scheduler_current_pcb = NULL;
            // Age the ready queue after each time slice
            ready_queue_age(rq);
            // Check if done and enqueue or destroy
            if (yielded) {
                continue;
            }
            if (curr_pcb->pc <= curr_pcb->end) {
                ready_queue_enqueue(rq, curr_pcb);
            } else {
                if (curr_pcb->should_free_code) {
                    free_code_memory(curr_pcb->start, curr_pcb->end);
                }
                pcb_destroy(curr_pcb);
            }
        }
    }
    else if (rq->algorithm == RR30) {
        while (ready_queue_is_empty(rq) == 0) {
            // Dequeue
            PCB* curr_pcb = ready_queue_dequeue(rq);
            if (curr_pcb == NULL) {
                break;
            }
            // Set # of lines to run for this time slice
            int time_slice = 30;
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
                if (curr_pcb->should_free_code) {
                    free_code_memory(curr_pcb->start, curr_pcb->end);
                }
                pcb_destroy(curr_pcb);
            }
        }
    }
}