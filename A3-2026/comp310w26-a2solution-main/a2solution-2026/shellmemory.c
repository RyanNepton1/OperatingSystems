#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "shellmemory.h"


#define true 1
#define false 0

#define MEM_SIZE (FRAME_STORE_SIZE + VAR_STORE_SIZE)


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

struct memory_struct { // block or line
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

struct frame_slot {
    int allocated;
    char *lines[PAGE_SIZE];
    unsigned long lru_tick;
    void *owner;
    int page_number;
};

static struct frame_slot frame_store[FRAME_STORE_SIZE / PAGE_SIZE];
static unsigned long global_lru_tick = 1;


size_t mem_get_frame_store_size(void) {
    return FRAME_STORE_SIZE;
}

size_t mem_get_variable_store_size(void) {
    return VAR_STORE_SIZE;
}

int frame_store_num_frames(void) {
    return (int)(FRAME_STORE_SIZE / PAGE_SIZE);
}

static void init_frame_store(void) {
    for (int i = 0; i < frame_store_num_frames(); ++i) {
        frame_store[i].allocated = false;
        frame_store[i].lru_tick = 0;
        frame_store[i].owner = NULL;
        frame_store[i].page_number = -1;
        for (int j = 0; j < PAGE_SIZE; ++j) {
            frame_store[i].lines[j] = NULL;
        }
    }
}

void reset_frame_store(void) {
    for (int i = 0; i < frame_store_num_frames(); ++i) {
        frame_clear(i);
    }
    global_lru_tick = 1;
}

int frame_find_free(void) {
    for (int i = 0; i < frame_store_num_frames(); ++i) {
        if (!frame_store[i].allocated) {
            return i;
        }
    }
    return -1;
}

int frame_pick_lru(void) {
    int victim = -1;
    unsigned long best_tick = 0;
    for (int i = 0; i < frame_store_num_frames(); ++i) {
        if (!frame_store[i].allocated) {
            continue;
        }
        if (victim < 0 || frame_store[i].lru_tick < best_tick) {
            victim = i;
            best_tick = frame_store[i].lru_tick;
        }
    }
    return victim;
}

void frame_touch(int frame_number) {
    assert(frame_number >= 0 && frame_number < frame_store_num_frames());
    frame_store[frame_number].lru_tick = global_lru_tick++;
}

int frame_is_allocated(int frame_number) {
    assert(frame_number >= 0 && frame_number < frame_store_num_frames());
    return frame_store[frame_number].allocated;
}

void frame_set_page_owner(int frame_number, void *owner, int page_number) {
    assert(frame_number >= 0 && frame_number < frame_store_num_frames());
    frame_store[frame_number].owner = owner;
    frame_store[frame_number].page_number = page_number;
}

void *frame_get_page_owner(int frame_number) {
    assert(frame_number >= 0 && frame_number < frame_store_num_frames());
    return frame_store[frame_number].owner;
}

int frame_get_page_number(int frame_number) {
    assert(frame_number >= 0 && frame_number < frame_store_num_frames());
    return frame_store[frame_number].page_number;
}

void frame_write_line(int frame_number, int offset, const char *line) {
    assert(frame_number >= 0 && frame_number < frame_store_num_frames());
    assert(offset >= 0 && offset < PAGE_SIZE);
    struct frame_slot *slot = &frame_store[frame_number];
    if (slot->lines[offset]) {
        free(slot->lines[offset]);
        slot->lines[offset] = NULL;
    }
    if (line) {
        slot->lines[offset] = strdup(line);
    }
    slot->allocated = true;
}

const char *frame_read_line(int frame_number, int offset) {
    assert(frame_number >= 0 && frame_number < frame_store_num_frames());
    assert(offset >= 0 && offset < PAGE_SIZE);
    return frame_store[frame_number].lines[offset];
}

void frame_clear(int frame_number) {
    assert(frame_number >= 0 && frame_number < frame_store_num_frames());
    struct frame_slot *slot = &frame_store[frame_number];
    for (int i = 0; i < PAGE_SIZE; ++i) {
        free(slot->lines[i]);
        slot->lines[i] = NULL;
    }
    slot->allocated = false;
    slot->lru_tick = 0;
    slot->owner = NULL;
    slot->page_number = -1;
}



void mem_init() {
    init_frame_store();

    int i;
    for (i = 0; i < VAR_STORE_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < VAR_STORE_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < VAR_STORE_SIZE; i++) {
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

    for (i = 0; i < VAR_STORE_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return NULL;
}
