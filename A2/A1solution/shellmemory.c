#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

#define Max_Length 1000


// Define the structure for shell memory
struct memory_struct {
    char *var;
    char *value;
};

// Define the structure for code memory with an array of lines and a pointer to the next free line
typedef struct  {
    char *lines[Max_Length];
    int next_free;
} code_memory;

// Initialize the shell memory and code memory
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
    codememory.next_free = 0;
    for (i = 0; i < Max_Length; i++) {
        codememory.lines[i] = NULL;
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

// All functions related to code memory are below

// Function to load code from a file into code memory at start and end locations, returns number of lines loaded or -1 if error
// Also updates the start and end pointers to indicate where the code was loaded in memory
int code_load(char *filename, int *start, int *end) {
    FILE *fp;
    char line[Max_Length];
    int lineCount = 0;
    // Open the file for reading
    fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }
    // Update PCB start value for the next open location in memory
    *start = codememory.next_free;
    // Read the file line by line and store it in code memory
    while (fgets(line, Max_Length, fp) != NULL) {
        line[strcspn(line, "\n")] = 0; // remove newline character
        codememory.lines[codememory.next_free] = strdup(line);
        lineCount++;
        codememory.next_free++;

    }

    // Update PCB end value for the last line in memory
    *end = codememory.next_free - 1;
    fclose(fp);
    return lineCount;
}

// Function to get a line of code from code memory based on the line number
char* code_get_line(int line_num) {
    if (line_num < 0 || line_num >= codememory.next_free || codememory.lines[line_num] == NULL) {
        return NULL;
    }
    return codememory.lines[line_num];
}

// Function to free the memory allocated for code lines between start and end line numbers
void free_code_memory(int start, int end) {
    for (int i = start; i <= end && i < codememory.next_free; i++) {
        free(codememory.lines[i]);
        codememory.lines[i] = NULL;
    }
}

// Function to reset the shell memory by freeing all allocated memory and resetting values to "none"
void mem_reset() {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        free(shellmemory[i].var);
        free(shellmemory[i].value);
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

