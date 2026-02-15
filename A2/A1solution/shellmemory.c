#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

#define Max_Length 1000



struct memory_struct {
    char *var;
    char *value;
};

typedef struct  {
    char *lines[Max_Length];
    int next_free;
} code_memory;

struct memory_struct shellmemory[MEM_SIZE];
code_memory codememory;

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else
        return 0;
}

// Shell memory functions

void mem_init() {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return NULL;
}


int code_load(char *filename, int *start, int *end) {
    FILE *fp;
    char line[Max_Length];
    int lineCount = 0;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }
    *start = codememory.next_free;
    while (fgets(line, Max_Length, fp) != NULL) {
        line[strcspn(line, "\n")] = 0; // remove newline character
        codememory.lines[lineCount] = strdup(line);
        lineCount++;
        codememory.next_free++;

    }

    codememory.next_free = lineCount;
    *end = codememory.next_free - 1;
    fclose(fp);
    return lineCount;
}

char* code_get_line(int line_num) {
    if (line_num < 0 || line_num >= codememory.next_free || codememory.lines[line_num] == NULL) {
        return NULL;
    }
    return codememory.lines[line_num];
}

void free_code_memory(int start, int end) {
    for (int i = start; i <= end && i < codememory.next_free; i++) {
        free(codememory.lines[i]);
        codememory.lines[i] = NULL;
    }
}

void mem_reset() {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        free(shellmemory[i].var);
        free(shellmemory[i].value);
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

