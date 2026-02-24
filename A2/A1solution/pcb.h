#ifndef PCB_H       // If PCB_H is not defined
#define PCB_H

typedef struct PCB{
    int pid;
    int start;
    int end;
    int pc;
    struct PCB *next_pcb;
    int job_length_score;
    int should_free_code;  // flag: 1 if this PCB should free its code, 0 if not
} PCB;

void pcb_init();
PCB* pcb_create(int start, int end);
PCB* pcb_create_nofree(int start, int end);  // Create PCB without code ownership
void pcb_destroy(PCB* new_pcb);
int pcb_get_next_pid();

#endif