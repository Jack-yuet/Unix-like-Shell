# Unix Shell (`msh`)

## Overview
The `msh` project is a custom Unix-like shell, developed as a course project in stages (milestones). The shell provides a basic command-line interface that interprets and executes user commands, managing job control, signal handling, and maintaining a command history. This shell does not replicate all features of Bash but incorporates core Unix shell functionalities.

## Project Structure

### Directory Layout
```plaintext
├── bin                # Contains the compiled msh executable
├── data               # Stores auxiliary data, including shell history
├── include            # Header files for the shell modules
├── src                # Source files for shell components
├── scripts            # Scripts for building and managing the shell
├── tests              # Test files for validating shell functionality
├── README.md          # Project documentation
└── Makefile           # (Optional) Makefile for compiling the shell

## Features

### Command Parsing and Execution
The `msh` shell allows users to enter commands and interpret multiple jobs on a single command line using special characters:

- `;` separates commands to run sequentially in the foreground.
- `&` denotes background jobs, enabling asynchronous processing.

The `evaluate` function manages parsing and executing commands, creating child processes to execute each job and managing their lifecycle.

### Job Control and Process Management
`msh` supports job control, allowing users to manage foreground and background tasks:

- **Foreground Jobs**: Run synchronously, with `msh` waiting for their completion.
- **Background Jobs**: Run asynchronously, with `msh` immediately returning control to the user.

Job attributes are tracked using a `job_t` struct, with functions like `add_job` and `delete_job` managing job records. This allows `msh` to keep track of active processes and manage resources efficiently.

### Signal Handling
The shell includes robust signal handling for effective job control:

- **SIGINT** (`ctrl-c`): Interrupts the current foreground job.
- **SIGTSTP** (`ctrl-z`): Suspends the current foreground job.
- **SIGCHLD**: Monitors and reaps terminated background jobs, maintaining stability in long-running sessions.

These signals ensure that user inputs for process control are responsive and that system resources are managed properly across all running jobs.

### Command History
`msh` maintains a history of executed commands, allowing users to:

- Recall previous commands with the `history` command.
- Re-execute a specific command with the `!N` syntax, where `N` is the history line number.

The `history` module supports command storage and retrieval, providing continuity and user convenience across sessions. Persistent storage in `.msh_history` enables command history to be saved between shell invocations.

### Built-in Commands
The shell provides several built-in commands to manage jobs and retrieve history:

- **jobs**: Lists active jobs and their states (e.g., RUNNING or SUSPENDED).
- **history**: Displays the command history.
- **!N**: Re-runs the `N`th command from history.
- **bg <job>**: Resumes a stopped job in the background.
- **fg <job>**: Brings a background job to the foreground.
- **kill SIG_NUM PID**: Sends a signal to a specified process.

## Building and Running

### Quick Start Guide
1. **Build**: Run the `build.sh` script located in the `scripts/` directory:
   cd scripts
   source build.sh

2.Run: Launch the shell from bin/:
    ./bin/msh

### Sample Usage
1. Command Parsing: msh> ls -la /; echo "Hello, world!"
2. Foreground and Background Jobs: msh> /usr/bin/ls -la & echo "Running in background"
