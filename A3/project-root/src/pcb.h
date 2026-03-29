#pragma once
#include <stddef.h>
#include <stdio.h> 

typedef size_t pid;

struct Program;

struct PCB {
    pid pid;
    struct Program *program;
    size_t pc_line;
    size_t duration;
    struct PCB *next;
};

struct Program {
    char *name;
    char *backing_filename;
    int total_lines;
    int total_pages;
    int *page_table;
    int refcount;
    struct Program *next;
};


int pcb_has_next_instruction(struct PCB *pcb);
int pcb_current_page(struct PCB *pcb);
int pcb_current_offset(struct PCB *pcb);
const char *pcb_peek_instruction(struct PCB *pcb);
void pcb_advance(struct PCB *pcb);
struct PCB *create_process(const char *filename);
struct PCB *create_process_from_FILE(FILE *f);
void free_pcb(struct PCB *pcb);

void program_registry_reset(void);
void preload_program_pages(struct Program *program, int max_pages);
int ensure_page_loaded_for_pcb(struct PCB *pcb);

