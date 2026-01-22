#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int badcommandFileDoesNotExist();
int echo(char *words[], int wordCount);
int my_ls();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);
    }
	else if (strcmp(command_args[0], "echo") == 0) {
		return echo(command_args, args_size);	
    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1)
            return badcommand();
        return my_ls();
    } else {
        return badcommand();
    }
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n \
echo STRING/VAR	Prints string or string corresponding to variable\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}
int echo(char *words[], int wordCount) {
        char variable = '$';
        int found = 0;
	if (wordCount < 2) {
		printf("\n");
		return 0;
	}
	
	// Check if we want a variable
	if (words[1][0] == variable) {
		char *varName = words[1] + 1;
		char *output = mem_get_value(varName);
		
		// Check if variable exists
		if (strcmp(output, "Variable does not exist") == 0) {
			printf("\n");
		}
		else {
			printf("%s\n", output);
			free(output);
		}
		return 0; 
	}
	// Skip command name so start at 1
	// Print all the words
	for (int i = 1; i < wordCount; i++) {
		printf("%s", words[i]);
		if (i < wordCount - 1) {
			printf(" ");
		}
	}
	printf("\n");
	return 0;
}

int my_ls(){
    DIR *dir;
    struct dirent *entry;
    char **entries = NULL;
    int count = 0;
    int capacity = 10;
    
    // Allocate initial memory for entries
    entries = (char **)malloc(capacity * sizeof(char *));
    if (entries == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Open the current directory
    dir = opendir(".");
    if (dir == NULL) {
        printf("Error opening directory\n");
        free(entries);
        return 1;
    }
    
    // Read all entries from the directory
    while ((entry = readdir(dir)) != NULL) {
        // Reallocate if necessary
        if (count >= capacity) {
            capacity *= 2;
            entries = (char **)realloc(entries, capacity * sizeof(char *));
            if (entries == NULL) {
                printf("Memory allocation failed\n");
                closedir(dir);
                return 1;
            }
        }
        
        // Allocate and copy the entry name
        entries[count] = (char *)malloc((strlen(entry->d_name) + 1) * sizeof(char));
        if (entries[count] == NULL) {
            printf("Memory allocation failed\n");
            closedir(dir);
            return 1;
        }
        strcpy(entries[count], entry->d_name);
        count++;
    }
    
    closedir(dir);
    
    // Sort entries alphabetically
    // Numbers come before letters, uppercase before lowercase
    qsort(entries, count, sizeof(char *), (int (*)(const void *, const void *))strcmp);
    
    // Print all entries
    for (int i = 0; i < count; i++) {
        printf("%s\n", entries[i]);
        free(entries[i]);
    }
    
    free(entries);
    return 0;
}
