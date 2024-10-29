#include "../include/shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>

msh_t* shell=NULL;

/**
 * Allocate and initialize the shell's state.
 *
 * @param max_jobs The max number of jobs that can exist at any time.
 * @param max_line The max number of characters in a command line.
 * @param max_history The max number of commands stored in history.
 * @return A pointer to the newly allocated shell state; Returns NULL if memory allocation fails.
 */
msh_t *alloc_shell(int max_jobs, int max_line, int max_history) {
    if (max_history == 0) max_history = 10;
    if (max_line == 0) max_line = 1024;
    if (max_jobs == 0) max_jobs = 16;

    msh_t *shell = (msh_t *)malloc(sizeof(msh_t));
    if (shell == NULL) {
        fprintf(stderr, "Failed to allocate memory for shell\n");
        return NULL;
    }

    // Initialize the shell state with the provided values
    shell->max_jobs = max_jobs;
    shell->max_line = max_line;
    shell->max_history = max_history;
    shell->jobs = (job_t *)calloc(max_jobs, sizeof(job_t)); // Use calloc to initialize to zero
    if (shell->jobs == NULL) {
        fprintf(stderr, "Failed to allocate memory for jobs\n");
        free(shell); // Make sure to free previously allocated memory
        return NULL;
    }
    for(int i=0;i<max_jobs;i++){
        shell->jobs[i].cmd_line=NULL;
    }
    shell->history=alloc_history(shell->max_history);
    initialize_signal_handlers();
    return shell;
}

/**
 * Parses a command line into jobs, distinguishing between foreground and background jobs.
 *
 * @param line The command line to parse.
 * @param job_type Pointer (1 for foreground store, 0 for background store).
 * @return Command line's following job; NULL if there are no more jobs.
 */
char *parse_tok(char *line, int *job_type) {
    static char *current;
    if (line != NULL) {
        current = line;
    }

    if (current == NULL||*current == '\0') {
        *job_type = -1;
        current = NULL;
        return NULL;
    }

    char *job_start = current;
    while (*current != '\0' && *current != '&' && *current != ';') {
        current++;
    }

    // Determine job type
    if (*current == '&') {
        *current = '\0';
        *job_type = BACKGROUND;
        current++;
    } else if (*current == ';') {
        *current = '\0';
        *job_type = FOREGROUND;
        current++;
    } else if (*current == '\0') {
        *job_type = 0;
    }

    // Skip any separators that directly follow
    while (*current == ';' || *current == '&') {
        //*job_type = 1;
        current++;
    }
    char* current_copy=current;
    while(*current==' '){
        current++;
    }
    if(*current!='\0'){
        current=current_copy;
    }

    return job_start;
}

/**
 * Separate the arguments of a command and store them in an array.
 *
 * @param line The command line that we are going to separate.
 * @param argc Stores the number of arguments produced.
 * @param is_builtin Flag indicating if the command is built-in.
 * @return An array of strings representing the command arguments; NULL if no arguments.
 */
char **separate_args(char *line, int *argc,bool* is_builtin) {
    if (*line == '\0' || line == NULL) {
        *argc = 0;
        return NULL;
    }

    // Temporary copy of line to use with strtok
    char *line_copy = strdup(line);
    if (!line_copy) {
        perror("Failed to duplicate line for argument separation");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char *temp = strtok(line_copy, " \t\n");
    while (temp != NULL) {
        temp = strtok(NULL, " \t\n");
        count++;
    }

    // Allocate memory for the argument array
    char **argv = (char **)malloc((count + 1) * sizeof(char *));
    if (!argv) {
        perror("Failed to allocate memory for argument array");
        exit(EXIT_FAILURE);
    }

    // Reset line_copy for actual tokenization
    free(line_copy);
    line_copy = strdup(line);
    if (!line_copy) {
        perror("Failed to duplicate lines for argument separation");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    temp = strtok(line_copy, " \t\n");
    while (temp != NULL) {
        argv[i++] = strdup(temp);
        temp = strtok(NULL, " \t\n");
    }
    argv[i] = NULL;
    *argc = count;

    free(line_copy);
    return argv;
}

/**
 * Determine if a given command is a built-in shell operation and set appropriate command type and parameters.
 *
 * @param shell A pointer to the shell instance, which contains job lists and history for the session.
 * @param argv An array of strings representing the parsed command line arguments.
 * @param argc The number of arguments in argv.
 * @param cmd_type A pointer to an integer to set the command type (1 for 'jobs', 2 for 'history', 3 for 'history' invocation with '!number', 4 for 'bg', 5 for 'fg', 6 for 'kill').
 * @param history_to_evaluate A pointer to a string. 
 * @param pid_to_update A pointer to a PID (Process ID).
 * @param sig_num A pointer to an integer to store the signal number when the 'kill' command is used.
 * @param pid_to_kill A pointer to a PID (Process ID) to be killed when the 'kill' command is used.
 * @return True if the command is a built-in shell operation and was successfully identified; False otherwise.
 */
bool is_builtin(msh_t *shell,char** argv,int argc,int* cmd_type,char** history_to_evaluate,pid_t* pid_to_update,int* sig_num,pid_t* pid_to_kill){
    if(argc==1&&strcmp(argv[0],"jobs")==0){
        *cmd_type=1;
        return true;
    }
    else if(argc==1&&strcmp(argv[0],"history")==0){
        *cmd_type=2;
        return true;
    }
    else if(argc==1&&argv[0][0]=='!'){
        int index=atoi(&argv[0][1]);
        *history_to_evaluate=find_line_history(shell->history,index);
        if(*history_to_evaluate!=NULL){
            *cmd_type=3;
            return true;
        }
    }
    else if(argc==2&&(strcmp(argv[0],"bg")==0||strcmp(argv[0],"fg")==0)){
        char* token=argv[1];

        if(token[0]=='%'){
            int job_id=atoi(token+1);
            if((*pid_to_update=get_pid_by_job_id(shell->jobs,shell->max_jobs,job_id))!=-1){
                *cmd_type=strcmp(argv[0],"bg")==0?4:5;
                return true;
            }
        }
        else{
            *pid_to_update=atoi(token);
            int job_id=get_job_id_by_pid(shell->jobs,shell->max_jobs,*pid_to_update);
            if(job_id!=0){
                *cmd_type=strcmp(argv[0],"bg")==0?4:5;
                return true;
            }
        }
    }
    else if(argc==3&&strcmp(argv[0],"kill")==0){
        *sig_num=atoi(argv[1]);
        *pid_to_kill=atoi(argv[2]);
        if(*sig_num!=2&&*sig_num!=9&&*sig_num!=18&&*sig_num!=19){
            printf("error: invalid signal number\n");
            return false;
        }
        *cmd_type=6;
        return true;
    }
    else{
        return false;
    }
}

/**
 * Waits for a foreground process to complete.
 *
 * @param shell A pointer to the shell instance.
 */
void waitfg(msh_t *shell){
    sigset_t empty_signal_set;
    sigemptyset(&empty_signal_set);
    while(shell->curr_foreground_pid!=0){
        sigsuspend(&empty_signal_set);
    }
}

/**
 * Evaluate the command line string provided.
 *
 * @param shell The current shell state.
 * @param line The command line string that we are going to evaluate.
 * @return Returns a non-zero return value indicating a request to terminate the shell;
 *         0 to indicate that the shell should continue running.
 */
int evaluate(msh_t *shell, char *line) {
    if (!line || strlen(line) > shell->max_line) {
        printf("error: reached the maximum line limit\n");
        return -1;
    }
    //printf("*****\n");
    int job_type;
    char *job = parse_tok(line, &job_type);
    while (job != NULL) {
        if(strstr(job,"exit")!=0){
            return -1;
        }
        int argc;
        char **argv = separate_args(job, &argc,NULL);
        if (argv != NULL) {
            if(jobs_full(shell->jobs,shell->max_jobs)){
                printf("error: reached the maximum jobs limit\n");
                free(argv); // Assuming argv was dynamically allocated
                job = parse_tok(NULL, &job_type);
                continue;
            }
            int cmd_type;
            char* history_to_evaluate;
            pid_t pid_to_update;
            int sig_num;
            pid_t pid_to_kill;
            if(is_builtin(shell,argv,argc,&cmd_type,&history_to_evaluate,&pid_to_update,&sig_num,&pid_to_kill)){
                if(cmd_type!=3){
                    add_line_history(shell->history,job);
                }
                if(cmd_type==1){

                    for(int i=0;i<shell->max_jobs;i++){
                        job_t* j=&(shell->jobs[i]);
                        if(j->cmd_line!=NULL&&j->pid!=0){
                            printf("[%d] %d %s %s\n",j->jid,j->pid,j->state==SUSPENDED?"Stopped":"RUNNING",j->cmd_line);
                        }
                    }
                }
                else if(cmd_type==2){

                    print_history(shell->history);
                }
                else if(cmd_type==3){
                    printf("%s\n",history_to_evaluate);
                    evaluate(shell,history_to_evaluate);
                }
                else if(cmd_type==4){
                    update_job_state(shell->jobs,shell->max_jobs,pid_to_update,BACKGROUND);
                    kill(-pid_to_update,SIGCONT);
                }
                else if(cmd_type==5){
                    update_job_state(shell->jobs,shell->max_jobs,pid_to_update,FOREGROUND);
                    kill(-pid_to_update,SIGCONT);
                }
                else if(cmd_type==6){
                    if(sig_num==2){
                        kill(-pid_to_kill,SIGINT);
                    }
                    else if(sig_num==9){
                        kill(-pid_to_kill,SIGKILL);
                    }
                    else if(sig_num==18){
                        kill(-pid_to_kill,SIGCONT);
                    }
                    else{
                        kill(-pid_to_kill,SIGSTOP);
                    }
                }
            }
            else{
                add_line_history(shell->history,job);
                sigset_t signal_set1;
                sigset_t prev_signal_set1;
                sigset_t signal_set2;
                sigset_t prev_signal_set2;
                sigfillset(&signal_set2);
                sigemptyset(&signal_set1);
                sigaddset(&signal_set1,SIGCHLD);
                sigprocmask(SIG_BLOCK,&signal_set1,&prev_signal_set1);

                pid_t pid = fork();
                if (pid == -1) {
                    // Fork failed
                    perror("fork");
                    return -1;
                }

                else if (pid == 0) {
                    // Child process
                    sigprocmask(SIG_SETMASK,&prev_signal_set1,NULL);
                    setpgid(0,0);
                    if (execve(argv[0], argv, NULL) == -1) {
                        perror("execve");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_SUCCESS);
                } else {
                    // Parent process
                    sigprocmask(SIG_BLOCK,&signal_set2,&prev_signal_set2);
                    add_job(shell->jobs,shell->max_jobs,pid,job_type,job);
                    if (job_type == FOREGROUND) {
                        // For foreground jobs, wait for the job to complete
                        shell->curr_foreground_pid=pid;
                        waitfg(shell);
                    }
                    sigprocmask(SIG_SETMASK,&prev_signal_set1,NULL);
                }
            }
        }
        free(argv); // Assuming argv was dynamically allocated
        job = parse_tok(NULL, &job_type);
    }
    return 0;
}

/**
 * Close down the shell.
 *
 * @param shell The current shell state value; If NULL, the function performs no operation.
 */
void exit_shell(msh_t *shell) {
int status;
    while(has_background_job(shell->jobs,shell->max_jobs)){
        pid_t term_pid = wait(&status);
        if (term_pid > 0) {
            // Remove the completed background job from the jobs array here
            delete_job(shell->jobs,shell->max_jobs,term_pid);
        }
    }
    if (shell != NULL) {
        free_jobs(shell->jobs, shell->max_jobs); // Ensure jobs are freed
        free(shell);
    }
}
