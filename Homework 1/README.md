# Assignment 1

## Introduction

For this first assignment you will be developing a program to simulate the creation and deletion of pro-
cesses in a model system. The processes are represented by a very simple Process Control Block (PCB).
that consists of a process id (PID) incrementally assigned starting with 1, the parent process id (PPID) [0
if this is the first process created], and a linked list of nodes containing process ids of all child processes.
These PCBs will be stored in a simple array. You should use the following struct definitions for your pro-
gram:

```c
// Struct to represent a node in a pid list
typedef struct pid_node {
    int pid;
    pid_node *next;
} pid_node;

// Struct to represent a process
typedef struct pcb {
    int pid;
    int ppid;
    pid_node *children;
} pcb;
```

Write a program in C that reads in data from the user describing a series of *create* or *delete* operations on
processes in a model system. The first line of input will contain an integer indicating the maximum num-
ber of processes that can be created ($<= 1000$). Lines $2 − n$ will have $2$ entries, separated by a space. The
first is either a $c$ or $d$ to create or delete a process. If $c$, the second entry on that line will be the PPID of
the process (note that your simulation will always start with the first entry in the array of PCBs popu-
lated with a process with the PID of $0$). If $d$, the second entry with be the PID to be deleted. The final
input line will consist of the letter $q$. Here is an example of a simple input to the simulation:

| | |
| :--: | :--: |
| $5$ |
| $c$ | $0$ |
| $c$ | $0$ |
| $c$ | $1$ |
| $c$ | $2$ |
| $d$ | $3$ |
| $q$ |

For this example, the simulation begins with an initial process with a PID of $0$ in the first position. The
simulation will create $4$ more processes (for a total of $5$, the max allowed) with PIDs $1$ through $4$ (PIDs
are incrementally assigned as processes are created). Finally, process $3$ is deleted, leaving the following:

1. PID $0$ with PPID $0$
2. PID $1$ with PPID $0$
3. PID $2$ with PPID $0$
4. PID $4$ with PPID $2$

Your program should print out the active processes, 1 per line separated by a space. Each line should con-
tain the PID and the PPID. For the above example, your program should only print the following:

| | |
| :--: | :--: |
| 0 | 0 |
| 1 | 0 |
| 2 | 0 |
| 4 | 2 |

## Notes

1. If a process is deleted, all descendent processes are also deleted
2. Your program will never need to handle bad input
3. Sample input/output files will be provided, but programs will be tested on others as well
4. You should put your code in a folder named with your QU shortname (email id), zip the folder, and
attach to the Blackboard assignment
5. You should include a Makefile in this zipped folder that compiles your code with all warnings turned
on (`-Wall`) and generates an executable named `assign1
6. All code should be at the top level of this folder (no subfolders!)
7. If your program has no memory leaks (as indicated by `valgrind`) you will get a small amount of ex-
tra credit. For the rest of the semester, all assignments will be required to have no memory leaks

## Dependent Delete Example

`INPUT`:

| | |
| :--: | :--: |
| $5$ |
| $c$ | $0$ |
| $c$ | $1$ |
| $c$ | $2$ |
| $c$ | $2$ |
| $d$ | $1$ |
| $q$ |

`OUTPUT`:

| | |
| :--: | :--: |
| $0$ | $0$ |