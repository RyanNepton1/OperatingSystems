#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h" // MAX_USER_INPUT
#include "shellmemory.h"
#include "pcb.h"

static struct Program *program_registry = NULL;

static void trim_newline_if_missing(char *line) {
    size_t len = strlen(line);
    if (len == 0) {
        return;
    }
    if (line[len - 1] != '\n') {
        if (len + 1 < MAX_USER_INPUT) {
            line[len] = '\n';
            line[len + 1] = '\0';
        }
    }
}

static int count_lines_in_file(const char *filename) {
    FILE *f = fopen(filename, "rt");
    if (!f) {
        return -1;
    }
    int count = 0;
    char line[MAX_USER_INPUT];
    while (fgets(line, sizeof(line), f)) {
        count++;
    }
    fclose(f);
    return count;
}

static struct Program *find_program(const char *filename) {
    struct Program *it = program_registry;
    while (it) {
        if (strcmp(it->name, filename) == 0) {
            return it;
        }
        it = it->next;
    }
    return NULL;
}

static int read_page_lines(const char *filename, int page_number, char out[PAGE_SIZE][MAX_USER_INPUT]) {
    FILE *f = fopen(filename, "rt");
    if (!f) {
        return 0;
    }

    int start_line = page_number * PAGE_SIZE;
    int line_no = 0;
    int copied = 0;
    char line[MAX_USER_INPUT];

    while (fgets(line, sizeof(line), f)) {
        if (line_no >= start_line && copied < PAGE_SIZE) {
            strncpy(out[copied], line, MAX_USER_INPUT - 1);
            out[copied][MAX_USER_INPUT - 1] = '\0';
            trim_newline_if_missing(out[copied]);
            copied++;
        }
        line_no++;
        if (copied == PAGE_SIZE) {
            break;
        }
    }

    fclose(f);
    return copied;
}

static int install_page_into_frame(struct Program *program, int page_number, int frame_number) {
    char lines[PAGE_SIZE][MAX_USER_INPUT];
    int copied = read_page_lines(program->backing_filename, page_number, lines);
    if (copied <= 0) {
        return 0;
    }

    frame_clear(frame_number);
    for (int i = 0; i < PAGE_SIZE; ++i) {
        if (i < copied) {
            frame_write_line(frame_number, i, lines[i]);
        } else {
            frame_write_line(frame_number, i, NULL);
        }
    }

    frame_set_page_owner(frame_number, program, page_number);
    frame_touch(frame_number);
    program->page_table[page_number] = frame_number;
    return 1;
}

static void invalidate_evicted_mapping(int frame_number) {
    struct Program *owner = (struct Program *)frame_get_page_owner(frame_number);
    int page_number = frame_get_page_number(frame_number);

    if (owner && page_number >= 0 && page_number < owner->total_pages) {
        owner->page_table[page_number] = -1;
    }
}

static struct Program *create_program(const char *filename) {
    int total_lines = count_lines_in_file(filename);
    if (total_lines < 0) {
        return NULL;
    }

    struct Program *program = malloc(sizeof(struct Program));
    if (!program) {
        return NULL;
    }

    program->name = strdup(filename);
    program->backing_filename = strdup(filename);
    program->total_lines = total_lines;
    program->total_pages = (total_lines + PAGE_SIZE - 1) / PAGE_SIZE;
    program->refcount = 0;
    program->next = program_registry;

    if (program->total_pages > 0) {
        program->page_table = malloc(sizeof(int) * program->total_pages);
        if (!program->page_table) {
            free(program->backing_filename);
            free(program->name);
            free(program);
            return NULL;
        }
        for (int i = 0; i < program->total_pages; ++i) {
            program->page_table[i] = -1;
        }
    } else {
        program->page_table = NULL;
    }

    program_registry = program;
    return program;
}

int pcb_has_next_instruction(struct PCB *pcb) {
    return pcb->pc_line < (size_t)pcb->program->total_lines;
}

int pcb_current_page(struct PCB *pcb) {
    return (int)(pcb->pc_line / PAGE_SIZE);
}

int pcb_current_offset(struct PCB *pcb) {
    return (int)(pcb->pc_line % PAGE_SIZE);
}

const char *pcb_peek_instruction(struct PCB *pcb) {
    int page = pcb_current_page(pcb);
    int offset = pcb_current_offset(pcb);
    int frame = pcb->program->page_table[page];
    if (frame < 0) {
        return NULL;
    }
    return frame_read_line(frame, offset);
}

void pcb_advance(struct PCB *pcb) {
    pcb->pc_line++;
}

struct PCB *create_process(const char *filename) {
    struct Program *program = find_program(filename);
    if (!program) {
        program = create_program(filename);
    }
    if (!program) {
        perror("failed to open file for create_process");
        return NULL;
    }

    struct PCB *pcb = malloc(sizeof(struct PCB));
    static pid fresh_pid = 1;
    pcb->pid = fresh_pid++;
    pcb->program = program;
    pcb->pc_line = 0;
    pcb->duration = (size_t)program->total_lines;
    pcb->next = NULL;
    program->refcount++;

    return pcb;
}

struct PCB *create_process_from_FILE(FILE *script) {
    char template_name[] = "/tmp/mysh_stdin_XXXXXX";
    int fd = mkstemp(template_name);
    if (fd < 0) {
        return NULL;
    }

    FILE *tmp = fdopen(fd, "w");
    if (!tmp) {
        close(fd);
        return NULL;
    }

    char linebuf[MAX_USER_INPUT];
    while (fgets(linebuf, sizeof(linebuf), script)) {
        fputs(linebuf, tmp);
    }
    fclose(tmp);

    return create_process(template_name);
}

void free_pcb(struct PCB *pcb) {
    pcb->program->refcount--;
    if (pcb->program->refcount == 0) {
        struct Program **scan = &program_registry;
        while (*scan && *scan != pcb->program) {
            scan = &(*scan)->next;
        }
        if (*scan) {
            *scan = (*scan)->next;
        }
        free(pcb->program->page_table);
        free(pcb->program->backing_filename);
        free(pcb->program->name);
        free(pcb->program);
    }
    free(pcb);
}

void program_registry_reset(void) {
    struct Program *it = program_registry;
    while (it) {
        struct Program *next = it->next;
        free(it->page_table);
        free(it->backing_filename);
        free(it->name);
        free(it);
        it = next;
    }
    program_registry = NULL;
}

void preload_program_pages(struct Program *program, int max_pages) {
    int to_load = program->total_pages;
    if (to_load > max_pages) {
        to_load = max_pages;
    }

    for (int page = 0; page < to_load; ++page) {
        if (program->page_table[page] >= 0) {
            continue;
        }

        int frame = frame_find_free();
        if (frame < 0) {
            frame = frame_pick_lru();
            if (frame >= 0) {
                invalidate_evicted_mapping(frame);
            }
        }
        if (frame < 0) {
            return;
        }

        install_page_into_frame(program, page, frame);
    }
}

int ensure_page_loaded_for_pcb(struct PCB *pcb) {
    if (!pcb_has_next_instruction(pcb)) {
        return 0;
    }

    int page = pcb_current_page(pcb);
    int frame = pcb->program->page_table[page];
    if (frame >= 0) {
        frame_touch(frame);
        return 0;
    }

    printf("Page fault!\n");
    int target = frame_find_free();

    if (target < 0) {
        target = frame_pick_lru();
        if (target < 0) {
            return 1;
        }

        printf("Victim page contents:\n");
        for (int i = 0; i < PAGE_SIZE; ++i) {
            const char *victim_line = frame_read_line(target, i);
            if (victim_line) {
                printf("%s", victim_line);
                if (victim_line[strlen(victim_line) - 1] != '\n') {
                    printf("\n");
                }
            }
        }
        printf("End of victim page contents.\n");

        invalidate_evicted_mapping(target);
    }

    install_page_into_frame(pcb->program, page, target);
    return 1;
}
