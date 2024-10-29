#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "../include/shell.h"

int main(int argc, char *argv[]) {
    int max_jobs = 16;
    int max_line = 1024;
    int max_history = 10;

    int opt;
    char *endptr;
    long val;
    int errors = 0;

    while ((opt = getopt(argc, argv, "s:j:l:")) != -1) {
        switch (opt) {
            case 's':
                val = strtol(optarg, &endptr, 10);
                if (*endptr == '\0' && val > 0) {
                    max_history = (int)val;
                } else {
                    errors++;
                }
                break;
            case 'j':
                val = strtol(optarg, &endptr, 10);
                if (*endptr == '\0' && val > 0) {
                    max_jobs = (int)val;
                } else {
                    errors++;
                }
                break;
            case 'l':
                val = strtol(optarg, &endptr, 10);
                if (*endptr == '\0' && val > 0) {
                    max_line = (int)val;
                } else {
                    errors++;
                }
                break;
            case '?':
                errors++;
                break;
        }
    }

    // If there were any errors in parsing options, show usage and exit
    if (optind<argc||errors > 0) {
        fprintf(stdout, "usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
        return 1;
    }

    // Allocate and initialize the shell state
    shell = alloc_shell(max_jobs, max_line, max_history);
    if (shell == NULL) {
        fprintf(stdout, "Failed to initialize shell\n");
        return 1;
    }

    // REPL loop using getline
    char *line = NULL;
    size_t len = 0;
    ssize_t nRead;

    printf("msh> ");
    while ((nRead = getline(&line, &len, stdin)) != -1) {
        if (nRead > 0 && line[nRead - 1] == '\n') {
            line[nRead - 1] = '\0';
        }
        if(strlen(line)!=0){
            if(evaluate(shell, line)!=0){
                break;
            }
        }
        free(line);
        line = NULL;
        len = 0;
        printf("msh> ");
    }

    // Cleanup
    free(line);
    exit_shell(shell);

    return 0;
}
