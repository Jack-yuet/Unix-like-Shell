#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *HISTORY_FILE_PATH = "../data/.msh_history";

/**
 * Allocates and initializes a history structure for storing command lines.
 *
 * @param max_history The maximum number of command lines the history can store.
 * @return A pointer to the newly allocated history structure; NULL if allocation fails.
 */
history_t *alloc_history(int max_history){
    history_t* history=(history_t*)malloc(sizeof(history_t));
    history->lines=malloc(sizeof(char*)*max_history);
    history->max_history=max_history;
    for(int i=0;i<max_history;i++){
        history->lines[i]=NULL;
    }
    history->next=0;
    FILE* fin=fopen(HISTORY_FILE_PATH,"r");
    if(fin!=NULL){
        char *line = NULL;
        size_t len = 0;
        ssize_t nRead;
        while((nRead=getline(&line,&len,fin))!=-1&&history->next<history->max_history){
            if(nRead>=1&&line[nRead-1]=='\n'){
                line[nRead-1]='\0';
            }
            add_line_history(history,line);
            free(line);
            line=NULL;
        }
        fclose(fin);
    }
    return history;
}

/**
 * Adds a new command line to the command history.
 *
 * @param history A pointer to the history structure where the command line will be added.
 * @param cmd_line The command line string to add. It must not be NULL.
 */
void add_line_history(history_t *history, const char *cmd_line){
    if(cmd_line==NULL||history==NULL){
        return;
    }
    if(history->next==history->max_history){
        free(history->lines[0]);
        for(int i=0;i<history->next-1;i++){
            history->lines[i]=history->lines[i+1];
        }
        history->next=history->max_history-1;
    }
    history->lines[history->next]=strdup(cmd_line);
    history->next++;
}

/**
 * Prints the entire command history to standard output.
 *
 * @param history A pointer to the history structure whose contents are to be printed.
 */
void print_history(history_t *history){
    for(int i=1;i<=history->next;i++){
        printf("%5d\t%s\n",i,history->lines[i-1]);
    }
}

/**
 * Retrieves a command line from history by its index.
 *
 * @param history A pointer to the history structure to search within.
 * @param index The 1-based index of the command line to retrieve.
 * @return A pointer to the command line string if found; NULL if the index is out of bounds.
 */
char *find_line_history(history_t *history, int index){
    if(index<1||index>history->max_history){
        return NULL;
    }
    return history->lines[index-1];
}

/**
 * Deallocates the history structure and its stored command lines.
 *
 * @param history A pointer to the history structure to be deallocated.
 */
void free_history(history_t *history){
    FILE* fout=fopen(HISTORY_FILE_PATH,"w");
    for(int i=0;i<history->next-1;i++){
        fprintf(fout,"%s\n",history->lines[i]);
        free(history->lines[i]);
    }
    if(history->next-1>=0){
        fprintf(fout,"%s",history->lines[history->next-1]);
        free(history->lines[history->next-1]);
    }
    fclose(fout);
    free(history->lines);
    free(history);
}
