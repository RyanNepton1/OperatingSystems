#include "scheduler.h"
#include "pcb.h"
#include "readystruct.h"
#include <stdio.h>
#include "interpreter.h"
#include "shell.h"
#include "shellmemory.h"
#include <pthread.h>
#include <time.h>

static PCB* scheduler_current_pcb = NULL;
static int scheduler_yield_requested = 0;

// Mutex for shared PCB state
typedef struct {
    pthread_mutex_t mutex;
    PCB* pcb;
} SharedPCB;

static SharedPCB shared_pcb = {PTHREAD_MUTEX_INITIALIZER, NULL};

// Multithreading worker pool variables
static int mt_initialized = 0;
static pthread_mutex_t mt_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mt_queue_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t mt_done_cond = PTHREAD_COND_INITIALIZER;
static int mt_shutdown = 0;
static pthread_t worker_threads[2];

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
            // Dequeue and run
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

                /* decrement job length score so that remaining work is
                   reflected; this makes preemption decisions easier and
                   keeps the queue sorted correctly if we reenqueue. */
                if (curr_pcb->job_length_score > 0) {
                    curr_pcb->job_length_score--;
                }

                /* For SJF we are now preemptive.  After each executed line
                   compare the remaining length of the currently running
                   PCB against the next PCB in the ready queue.  If our
                   remaining work is less-than-or-equal-to the head of the
                   queue, yield and reenqueue ourselves so the other process
                   can run.  ("<=" ensures we switch when lengths tie, which
                   is what the background‑batch test expects.) */
                if (rq->algorithm == SJF && rq->head != NULL) {
                    int remaining = curr_pcb->job_length_score;
                    if (remaining <= rq->head->job_length_score) {
                        yielded = 1;
                        ready_queue_enqueue(rq, curr_pcb);
                        break;
                    }
                }

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
                // Check if process has finished
                if (curr_pcb->pc > curr_pcb->end) {
                    break;
                }
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

                /* decrement remaining work so aging and enqueue decisions
                   stay correct if we put this PCB back on the queue */
                if (curr_pcb->job_length_score > 0) {
                    curr_pcb->job_length_score--;
                }

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
            scheduler_current_pcb = curr_pcb;
            int yielded = 0;
            // Set # of lines to run for this time slice
            int time_slice = 30;
            for (int i = 0; i < time_slice; i++) {
                // Check if process has finished
                if (curr_pcb->pc > curr_pcb->end) {
                    break;
                }
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
}

// Worker thread function for multithreading
void* worker_thread(void* arg) {
    ready_queue* rq = (ready_queue*)arg;
    
    while (1) {
        pthread_mutex_lock(&mt_queue_mutex);
        
        // Check if shutdown with empty queue
        if (mt_shutdown && ready_queue_is_empty(rq)) {
            pthread_mutex_unlock(&mt_queue_mutex);
            break;
        }
        
        // Dequeue a PCB if available
        PCB* curr_pcb = ready_queue_dequeue(rq);
        pthread_mutex_unlock(&mt_queue_mutex);
        
        if (curr_pcb == NULL) {
            // Queue is empty, wait with timeout and retry
            pthread_mutex_lock(&mt_queue_mutex);
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 10000000;  // 10ms timeout
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }
            pthread_cond_timedwait(&mt_queue_cond, &mt_queue_mutex, &ts);
            pthread_mutex_unlock(&mt_queue_mutex);
            continue;
        }
        
        // Execute the PCB based on the algorithm (RR/RR30)
        int time_slice = (rq->algorithm == RR30) ? 30 : 2;
        
        // Execute the PCB for one time slice
        for (int i = 0; i < time_slice && curr_pcb->pc <= curr_pcb->end; i++) {
            char* line = code_get_line(curr_pcb->pc);
            if (line != NULL) {
                parseInput(line);
            }
            curr_pcb->pc++;
        }
        
        // Check if the PCB is done or needs to be re-enqueued
        pthread_mutex_lock(&mt_queue_mutex);
        
        if (curr_pcb->pc <= curr_pcb->end) {
            // PCB not done, re-enqueue for more execution
            ready_queue_enqueue(rq, curr_pcb);
        } else {
            // PCB is complete, clean up
            if (curr_pcb->should_free_code) {
                free_code_memory(curr_pcb->start, curr_pcb->end);
            }
            pcb_destroy(curr_pcb);
            // Signal that work has been completed to the main thread
            pthread_cond_broadcast(&mt_done_cond);
        }
        pthread_mutex_unlock(&mt_queue_mutex);
    }
    
    return NULL;
}

// Initialize the multithreading worker pool
void scheduler_init_mt(ready_queue* mt_queue, SchedulingAlgorithm algo) {
    // If MT is already initialized, wait for the previous work to finish first
    if (mt_initialized) {
        scheduler_wait_mt(mt_queue);
    }
    
    mt_initialized = 1;
    mt_shutdown = 0;
    
    // Create two worker threads
    pthread_create(&worker_threads[0], NULL, worker_thread, (void*)mt_queue);
    pthread_create(&worker_threads[1], NULL, worker_thread, (void*)mt_queue);
}

// Wait for worker threads to complete all work
void scheduler_wait_mt(ready_queue* mt_queue) {
    if (!mt_initialized || !mt_queue) return;
    
    pthread_mutex_lock(&mt_queue_mutex);
    
    // Keep waiting while there's work to do
    while (!ready_queue_is_empty(mt_queue)) {
        pthread_cond_wait(&mt_done_cond, &mt_queue_mutex);
    }
    
    // Queue is empty, signal shutdown
    mt_shutdown = 1;
    pthread_cond_broadcast(&mt_queue_cond);
    pthread_mutex_unlock(&mt_queue_mutex);
    
    // Wait for both threads to finish
    pthread_join(worker_threads[0], NULL);
    pthread_join(worker_threads[1], NULL);
    
    mt_initialized = 0;
}

// Shutdown the multithreading worker pool
void scheduler_shutdown_mt() {
    if (!mt_initialized) return;
    pthread_mutex_lock(&mt_queue_mutex);
    mt_shutdown = 1;
    pthread_cond_broadcast(&mt_queue_cond);
    pthread_mutex_unlock(&mt_queue_mutex);
    pthread_join(worker_threads[0], NULL);
    pthread_join(worker_threads[1], NULL);
    mt_initialized = 0;
}

// Set the shared PCB with lock
void set_shared_pcb(PCB* pcb) {
    pthread_mutex_lock(&shared_pcb.mutex);
    shared_pcb.pcb = pcb;
    pthread_mutex_unlock(&shared_pcb.mutex);
}

// Clear shared PCB after execution
void clear_shared_pcb() {
    pthread_mutex_lock(&shared_pcb.mutex);
    shared_pcb.pcb = NULL;
    pthread_mutex_unlock(&shared_pcb.mutex);
}