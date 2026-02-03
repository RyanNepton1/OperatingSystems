#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/wait.h>
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
int comp(const void *a, const void *b);
int run(char *words[], int wordCount);
int my_mkdir(char *dirName);
int my_touch(char *fileName);
int my_cd(char *dirName);

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
    } else if (strcmp(command_args[0], "run") == 0) {
        return run(command_args, args_size);
    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2) {
            return badcommand();
        }
        return my_mkdir(command_args[1]);
    } else if (strcmp(command_args[0], "my_touch") == 0) {
        if (args_size != 2) {
            return badcommand();
        }
        return my_touch(command_args[1]);
    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2) {
            return badcommand();
        }
        return my_cd(command_args[1]);
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
echo STRING/VAR	Prints string or string corresponding to variable\n \
my_ls			Lists all files and directories in the current directory\n \
run CMD [ARG...]	Executes CMD with optional ARGuments in a new process\n \
my_mkdir DIR_NAME	Creates a new directory with name DIR_NAME\n \
my_touch FILE_NAME	Creates a new empty file with name FILE_NAME\n \
my_cd DIR_NAME         Changes the current directory to DIR_NAME\n";
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
            // Free the output that is given space by mem_get_value
			printf("%s\n", output);
			free(output);
		}
		return 0; 
	}
	// Skip command name so start at 1
	// Print all the words to output
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
    // removed strcmp to create custom comparison function ( (int (*)(const void *, const void *))strcmp) )
    qsort(entries, count, sizeof(char *), comp);
    
    // Print all entries
    for (int i = 0; i < count; i++) {
        printf("%s\n", entries[i]);
        free(entries[i]);
    }
    
    free(entries);
    return 0;
}

// helpter function for qsort to compare two strings
int comp(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;

    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        char ch1 = str1[i];
        char ch2 = str2[i];

        // Check if characters are different
        int c1_is_digit = (ch1 >= '0' && ch1 <= '9');
        int c2_is_digit = (ch2 >= '0' && ch2 <= '9');

        // Numbers come before letters
        if (c1_is_digit && !c2_is_digit) {
            return -1; // c1 is a digit, c2 is not
        }
        if (!c1_is_digit && c2_is_digit) {
            return 1; // c2 is a digit, c1 is not
        }
        
        // Both are digits or both letters
        if (ch1 != ch2) {
            return (unsigned char)ch1 - (unsigned char)ch2; // direct comaprison
        }
        i++;
    }
    //If all characters matched, shorter string comes first
    return (int)strlen(str1) - (int)strlen(str2);
}


int run(char *words[], int wordCount) {
    ffflush(stdout);
    // Create a new process
    pid_t pid = fork();
    // Error handling for fork
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }
    // If work, iterate through args and execute command
    if (pid == 0) {
        char *args[wordCount];
        for (int i = 1; i < wordCount; i++) {
            args[i - 1] = words[i];
        }
        args[wordCount - 1] = NULL;
        execvp(args[0], args);
        exit(1);
    }
    else {
        // Parent process waits for child to finish
        wait(NULL);

    }
    return 0;
}

int my_mkdir(char *dirName) {
    char var = '$';
    // Check if dirName is a variable
    if (dirName[0] == var) {
        char *varName = dirName + 1;
        char *resolvedName = mem_get_value(varName);
        if (strcmp(resolvedName, "Variable does not exist") == 0) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }
        dirName = resolvedName;
    }

    // Check if dirName is alphanumeric
    for (int i = 0; dirName[i] != '\0'; i++) {
        if (!isalnum((unsigned char)dirName[i])) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }
    }

    // Create the directory
    if (mkdir(dirName, 0755) == 0) { // use mkdir, give owner complete control, and group/others read & execute
        return 0;
    } else {
        printf("Bad command: my_mkdir\n");
        return 1;
    }
}

int my_touch(char *fileName) {
    // Assumption: filename is not a variable
    // Assumption: filename is alphanumeric
    // Create the file
    FILE *file = fopen(fileName, "a"); // open in append mode, creates file if it doesn't exist
    if (file == NULL) {
        printf("Bad command: my_touch\n");
        return 1;
    }
    fclose(file);
    return 0;
}

int my_cd(char *dirName) {
    // Assumption: dirName is not a variable
    // Allow ".." for parent directory and "." for current directory
    if (strcmp(dirName, "..") != 0 && strcmp(dirName, ".") != 0) {
        // Check if dirName is alphanumeric
        for (int i = 0; dirName[i] != '\0'; i++) {
            if (!isalnum((unsigned char)dirName[i])) {
                printf("Bad command: my_mkdir\n");
                return 1;
            }
        }
    }
    // Change the current directory
    if (chdir(dirName) == 0) {
        return 0;
    } else {
        printf("Bad command: my_cd\n");
        return 1;
    }
}
