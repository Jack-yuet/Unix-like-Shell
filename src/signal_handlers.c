#include "signal_handlers.h"
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include"job.h"
#include"shell.h"

/*
* sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
*     a child job terminates (becomes a zombie), or stops because it
*     received a SIGSTOP or SIGTSTP signal. The handler reaps all
*     available zombie children, but doesn't wait for any other
*     currently running children to terminate.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigchld_handler(int sig)
{
    int last_errno=errno;
    sigset_t signal_set;
    sigset_t old_signal_set;
    int status;
    pid_t pid=waitpid(-1,&status,WNOHANG|WUNTRACED|WCONTINUED);
    if(pid>0){
        sigprocmask(SIG_BLOCK,&signal_set,&old_signal_set);
        if(WIFSTOPPED(status)){
            update_job_state(shell->jobs,shell->max_jobs,pid,SUSPENDED);
            sigprocmask(SIG_SETMASK,&old_signal_set,NULL);
            if(pid==shell->curr_foreground_pid){
                shell->curr_foreground_pid=0;
            }
        }
        else if(WIFSIGNALED(status)){
            delete_job(shell->jobs,shell->max_jobs,pid);
            sigprocmask(SIG_SETMASK,&old_signal_set,NULL);
            if(pid==shell->curr_foreground_pid){
                shell->curr_foreground_pid=0;
            }
        }
        else if(WIFCONTINUED(status)){
            shell->curr_foreground_pid=pid;
            update_job_state(shell->jobs,shell->max_jobs,pid,FOREGROUND);
            sigprocmask(SIG_SETMASK,&old_signal_set,NULL);
        }
        else if(WIFEXITED(status)){
            delete_job(shell->jobs,shell->max_jobs,pid);
            sigprocmask(SIG_SETMASK,&old_signal_set,NULL);
            if(pid==shell->curr_foreground_pid){
                shell->curr_foreground_pid=0;
            }
        }
    }

}

/*
* sigint_handler - The kernel sends a SIGINT to the shell whenver the
*    user types ctrl-c at the keyboard.  Catch it and send it along
*    to the foreground job.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigint_handler(int sig)
{
    if(shell->curr_foreground_pid==0){
        pid_t pid=getpid();
        signal(SIGINT,SIG_DFL);
        kill(-pid,SIGINT);
    }
    else{
        kill(-shell->curr_foreground_pid,SIGINT);
    }
}

/*
* sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
*     the user types ctrl-z at the keyboard. Catch it and suspend the
*     foreground job by sending it a SIGTSTP.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigtstp_handler(int sig)
{
    if(shell->curr_foreground_pid==0){
        pid_t pid=getpid();
        signal(SIGINT,SIG_DFL);
        kill(-pid,SIGTSTP);
    }
    else{
        kill(-shell->curr_foreground_pid,SIGTSTP);
    }

}

/*
* setup_handler - wrapper for the sigaction function
*
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
typedef void handler_t(int);
handler_t *setup_handler(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0) {
        perror("Signal error");
        exit(1);
    }
    return (old_action.sa_handler);
}


void initialize_signal_handlers() {

    // sigint handler: Catches SIGINT (ctrl-c) signals.
    setup_handler(SIGINT,  sigint_handler);   /* ctrl-c */
    // sigtstp handler: Catches SIGTSTP (ctrl-z) signals.
    setup_handler(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    // sigchld handler: Catches SIGCHILD signals.
    setup_handler(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
}
