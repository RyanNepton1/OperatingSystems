#include <stdio.h>

#ifndef FRAME_STORE_SIZE
#define FRAME_STORE_SIZE 18
#endif

#ifndef VAR_STORE_SIZE
#define VAR_STORE_SIZE 10
#endif

#define PAGE_SIZE 3

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);


size_t mem_get_frame_store_size(void);
size_t mem_get_variable_store_size(void);


void reset_frame_store(void);
int frame_store_num_frames(void);
int frame_find_free(void);
int frame_pick_lru(void);
void frame_touch(int frame_number);
int frame_is_allocated(int frame_number);

void frame_set_page_owner(int frame_number, void *owner, int page_number);
void *frame_get_page_owner(int frame_number);
int frame_get_page_number(int frame_number);

void frame_write_line(int frame_number, int offset, const char *line);
const char *frame_read_line(int frame_number, int offset);
void frame_clear(int frame_number);

