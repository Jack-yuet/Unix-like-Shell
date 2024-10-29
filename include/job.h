#ifndef _JOB_H_
#define _JOB_H_

#include <sys/types.h>
#include <stdbool.h>

typedef enum job_state {FOREGROUND, BACKGROUND, SUSPENDED, UNDEFINED} job_state_t;

typedef struct job {
    char *cmd_line;
    job_state_t state;
    pid_t pid;
    int jid;
} job_t;

/**
 * add_job : adds a new job to the job array
 *
 * jobs: A pointer to the first element of the job array.
 *
 * max_jobs: The maximum number of jobs the array can hold.
 *
 * pid: The process ID of the new job.
 *
 * state: The state of the new job (e.g., BACKGROUND, FOREGROUND).
 *
 * cmd_line: The command line string associated with the new job.
 *
 * Returns: True if the job was successfully added; otherwise, false.
 */
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);

/**
 * delete_job: deletes a job from the job array based on its process ID.
 *
 * jobs: A pointer to the first element of the job array.
 *
 * max_jobs: The maximum number of jobs the array can hold.
 *
 * pid: The process ID of the job to be deleted.
 *
 * Returns: True if the job was successfully deleted; otherwise, false.
 */
bool delete_job(job_t *jobs, int max_jobs, pid_t pid);

/**
 * free_jobs: Frees the resources allocated for the jobs array.
 *
 * jobs: A pointer to the first element of the job array to be freed.
 *
 * max_jobs: The maximum number of jobs the array can hold.
 */
void free_jobs(job_t *jobs, int max_jobs);

/**
 * jobs_full : checks if the job array is full.
 *
 * jobs: A pointer to the first element of the job array.
 * 
 * max_jobs: The maximum number of jobs the array can hold.
 * 
 * Returns: True if the job array is full; otherwise, false.
 */
bool jobs_full(job_t *jobs, int max_jobs);

/**
 * has_background_job: checks for the existence of any background job in the job array.
 *
 * jobs: A pointer to the first element of the job array.
 * 
 * max_jobs: The maximum number of jobs the array can hold.
 * 
 * Returns: True if there is at least one job in the array with its state set to BACKGROUND; otherwise, false.
 */
bool has_background_job(job_t *jobs, int max_jobs);

/**
 * update_job_state: updates the state of a specific job identified by its PID.
 *
 * jobs: A pointer to the first element of the job array.
 * 
 * max_jobs: The maximum number of jobs the array can hold.
 * 
 * pid: The process ID of the job to update.
 * 
 * new_state: The new state to set for the job.
 */
void update_job_state(job_t* jobs,int max_jobs,pid_t pid,job_state_t new_state);

/**
 * get_job_id_by_pid: retrieves the job ID of a specific job identified by its PID.
 *
 * jobs: A pointer to the first element of the job array.
 * 
 * max_jobs: The maximum number of jobs the array can hold.
 * 
 * pid: The process ID of the job whose job ID is to be retrieved.
 * 
 * Returns: The job ID of the specified job; -1 if the job is not found.
 */
int get_job_id_by_pid(job_t* jobs,int max_jobs,pid_t pid);

/**
 * get_pid_by_job_id: retrieves the process ID of a specific job identified by its job ID.
 *
 * jobs: A pointer to the first element of the job array.
 * 
 * max_jobs: The maximum number of jobs the array can hold.
 * 
 * jid: The job ID of the job whose process ID is to be retrieved.
 * 
 * Returns: The process ID of the specified job; -1 if the job is not found.
 */
pid_t get_pid_by_job_id(job_t* jobs,int max_jobs,int jid);
#endif
