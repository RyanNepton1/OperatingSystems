#define MEM_SIZE 1000
#define Max_Length 1000
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

int code_load(char *filename, int *start, int *end);
char* code_get_line(int line_num);
void free_code_memory(int start, int end);
void free_shell_memory();

// Load remaining lines from stdin into code memory (batch mode).
// Returns number of lines loaded, and sets *start and *end.
int code_load_from_stdin(int *start, int *end);
