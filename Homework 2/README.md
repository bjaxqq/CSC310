# Assignment 2

## Introduction

For this second assignment you will be experimenting with process generation using fork/wait, and basic semaphore creation/use using `semaphore.h` in C. Specifically, you will write a program that simulates access to a bank account from a collection of independent processes, each requesting a series of deposit and/or withdrawal requests. Your program will read all input from the keyboard (as you did in the first assignment). Before you attempt this assignment, you should fully understand the following example programs because they contain almost all you need to set up this assignment:

- `time_test.c` - This program, when run, will give you a quick measurement of how long each iteration of a loop (in microseconds) takes on your machine.
- `sem.c` - This program demonstrates a simple semaphore implementation in C using `semaphore.h`. It implements the counter problem discussed in a previous example, showing how a semaphore will address the race condition even with an access delay
- `clockDemo.c` - One of the most important parts of any simulation is the management of4 time. Specifically, with a schedule or script of when things should occur in the simulation, different computers could struggle keeping up with real-time dependencies. This program demonstrates a "clock" that ticks based on simple units of real time. Simulations can be run based on these clock ticks, and the time unit can be lengthened to account for slower machines.

Your program should report all failed transactions (one per line), and the final balance of the account. Details of the reporting are described below.

The following listing provides two struct definitions for your program. The first describes a single transaction request made to the account. This request includes when the transaction request occurs, how long the request takes to process once the request has been received, and what the monetary value of the request is (positive integers for deposits and negative integers for withdrawals). The second struct in the listing provides the script for one of the interactig processes. This struct includes an array of the transactions, and a count of the number of items in the array.

```c
struct transaction {
    short startTime ; // Time in simulated clock ticks when transaction request received
    short duration ; // Number of simulated clock ticks the transaction will take
    short amount ; // Amount of transaction (negative for withdrawal)
};

struct procStruct {
    short count ; // Number of transactions in the for this process
    struct transaction arr [1000]; // Array of transactions
};

// Create a global array of type procStruct [] in shared memory using mmap.
```

Per the explanation of Assignment 1, please follow the following requirements when packaging your assignment:

1. Sample input/output files will be provided, but programs will be tested on others as well.
2. You should put your code in a folder named with your QU shortname (email id), zip the folder, and attach to the Blackboard assignment.
3. You should include a Makefile in this zipped folder that compiles your code with all warnings turned on (`-Wall`) and generates an executable named `assign2`.
4. All code should be at the top level of this folder (no subfolders!)
5. Do not include any spaces in any file or folder names!

Because we are not allocating any memory off the heap, I will not be checking for memory leaks using valgrind.

## Assignment

Input to your program will only be integers. As such you will not need to adjust your input method like you did for assignment 1 where input was a mixture of chars and ints. The first line of the input will con sist of an int indicating the number of processes ($N_p$), and int indicating the total number of transactions ($N_t$). The remaining ($N_t$) lines will each have $4$ integers. These integers represent the transaction arrival time, the process number ($1$ to $N_p$), the transaction processing time, and the amount of the transaction.

The input line:

```bash
15 2 3 -100
```

indicates that process $2$ requests a withdrawal of $\$100$ at time cycle $15$ which, once started, will take $3$ time cycles to complete.

Note the following about running/testing:

1. Your program will never need to handle bad input.
2. **Never prompt the user for anything!**
3. Sample input/output files will be provided, but programs will be tested on others as well.
4. There will never be more than $10$ child processes.
5. There will never be more than $100$ clock ticks $[0,100]$.
6. All input files will list transactions sorted by arrival time (although transactions with identical ar-
rival times will not necessarily be sorted based on any other fields).
7. No two transactions will have the exact same arrival time.
8. No process will generate transactions such that new ones are generated before old ones finish.

For each transaction in the input, your program should report the process number, when it arrived, when it started executing, whether it was successful ($1$) or unsuccessful ($0$), the transaction amount, and what the balance was after the transaction completed. Each of these should be reported on a single line per transaction using a space as a separator.

## Example

Here is an example of a simple input to the program:

```bash
3 5
2 1 5 200
15 2 3 -150
16 1 2 -100
21 3 5 200
25 2 3 -300
```

And here is the output produced by the program:

```bash
1 2 7 1 200 200
2 15 18 1 -150 50
1 16 20 0 -100 50
3 21 26 1 200 250
2 25 29 0 -300 250
```

More example inputs for you to test with will be provided.