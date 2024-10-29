#include "../include/job.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<stdio.h>

/**
 * Adds a new job to the job array.
 *
 * @param jobs A pointer to the first element of the job array.
 * @param max_jobs The maximum number of jobs the array can hold.
 * @param pid The process ID of the new job.
 * @param state The state of the new job (e.g., BACKGROUND, FOREGROUND).
 * @param cmd_line The command line string associated with the new job.
 * @return True if the job was successfully added; otherwise, false.
 */
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].cmd_line == NULL) {
            jobs[i].cmd_line = strdup(cmd_line);
            jobs[i].state = state;
            jobs[i].pid = pid;
            jobs[i].jid = i + 1;
            return true;
        }
    }
    return false;
}

/**
 * Removes a job from the job array based on its PID.
 *
 * @param jobs A pointer to the first element of the job array.
 * @param max_jobs The maximum number of jobs the array can hold.
 * @param pid The process ID of the job to be removed.
 * @return True if the job was found and removed; otherwise, false.
 */
bool delete_job(job_t *jobs, int max_jobs, pid_t pid) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid == pid) {
            free(jobs[i].cmd_line);
            jobs[i].cmd_line = NULL;
            jobs[i].pid = 0;
            return true;
        }
    }
    return false;
}

/**
 * Frees all allocated memory for the jobs and their command lines.
 *
 * @param jobs A pointer to the first element of the job array.
 * @param max_jobs The maximum number of jobs the array can hold.
 */
void free_jobs(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].cmd_line != NULL) {
            free(jobs[i].cmd_line);
            jobs[i].cmd_line=NULL;
        }
    }
    free(jobs);
}

/**
 * Checks if the job array is full.
 *
 * @param jobs A pointer to the first element of the job array.
 * @param max_jobs The maximum number of jobs the array can hold.
 * @return True if no available slots are found (i.e., the job array is full); otherwise, false.
 */
bool jobs_full(job_t *jobs, int max_jobs){
    for(int i=0;i<max_jobs;i++){
        if(jobs[i].cmd_line==NULL){
            return false;
        }
    }
    return true;
}

/**
 * Checks for the existence of any background jobs in the job array.
 *
 * @param jobs A pointer to the first element of the job array.
 * @param max_jobs The maximum number of jobs the array can hold.
 * @return True if at least one background job is found; otherwise, false.
 */
bool has_background_job(job_t *jobs, int max_jobs){
    for(int i=0;i<max_jobs;i++){
        if(jobs[i].cmd_line!=NULL&&jobs[i].state==BACKGROUND){
            return true;
        }
    }
    return false;
}

/**
 * Updates the state of a specific job identified by its PID.
 *
 * @param jobs A pointer to the first element of the job array.
 * @param max_jobs The maximum number of jobs the array can hold.
 * @param pid The process ID of the job to update.
 * @param new_state The new state to set for the job.
 */
void update_job_state(job_t* jobs,int max_jobs,pid_t pid,job_state_t new_state){
    for(int i=0;i<max_jobs;i++){
        if(jobs[i].cmd_line!=NULL&&jobs[i].pid==pid){
            jobs[i].state=new_state;
            break;
        }
    }
}

/**
 * Retrieves the job ID of a specific job identified by its PID.
 *
 * @param jobs A pointer to the first element of the job array.
 * @param max_jobs The maximum number of jobs the array can hold.
 * @param pid The process ID of the job whose job ID is to be retrieved.
 * @return The job ID of the specified job; 0 if the job is not found.
 */
int get_job_id_by_pid(job_t* jobs,int max_jobs,pid_t pid){
    for(int i=0;i<max_jobs;i++){
        if(jobs[i].cmd_line!=NULL&&jobs[i].pid==pid){
            return jobs[i].jid;
        }
    }
    return 0;
}

/**
 * Retrieves the process ID of a specific job identified by its job ID.
 *
 * @param jobs A pointer to the first element of the job array.
 * @param max_jobs The maximum number of jobs the array can hold.
 * @param jid The job ID of the job whose process ID is to be retrieved.
 * @return The process ID of the specified job; -1 if the job is not found.
 */
pid_t get_pid_by_job_id(job_t* jobs,int max_jobs,int jid){
    for(int i=0;i<max_jobs;i++){
        if(jobs[i].cmd_line!=NULL&&jobs[i].jid==jid){
            return jobs[i].pid;
        }
    }
    return -1;
}
