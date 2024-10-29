#ifndef _HISTORY_H_
#define _HISTORY_H_

extern const char *HISTORY_FILE_PATH;

//Represents the state of the history of the shell
typedef struct history {
    char **lines;
    int max_history;
    int next;
}history_t;

/**
 * alloc_history: allocates and initializes a history structure to store command lines.
 *
 * max_history: The maximum number of command lines the history can store.
 * 
 * Returns: A pointer to the newly allocated history structure; NULL if allocation fails.
 */
history_t *alloc_history(int max_history);

/**
 * add_line_history: adds a new command line to the history.
 *
 * history: A pointer to the history structure where the command line is to be added.
 * 
 * cmd_line: The command line string to add to the history.
 */
void add_line_history(history_t *history, const char *cmd_line);

/**
 * print_history : prints the contents of the history to standard output.
 *
 * history: A pointer to the history structure whose contents are to be printed.
 */
void print_history(history_t *history);

/**
 * find_line_history: retrieves a specific command line from the history based on its index.
 *
 * history: A pointer to the history structure to search within.
 * 
 * index: The 1-based index of the command line to retrieve.
 * 
 * Returns: A pointer to the command line string if found; NULL if the index is out of bounds.
 */
char *find_line_history(history_t *history, int index);

/**
 * free_history: deallocates the history structure and its contents.
 *
 * history: A pointer to the history structure to be deallocated.
 */
void free_history(history_t *history);

#endif
