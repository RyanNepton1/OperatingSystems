typedef struct PCB{
    int pid;
    int start;
    int end;
    int pc;
    struct PCB *next_pcb;
    int job_length_score;
} PCB;

void pcb_init();
PCB* pcb_create(int start, int end);
void pcb_destroy(PCB* new_pcb);
int pcb_get_next_pid();