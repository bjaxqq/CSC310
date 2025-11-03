# Assignment 3

## Objectives

At the end of this assignment, you should be able

- To design and build a C program using dynamic data structures
- To test a program for memory leaks
- To develop a model to test free space allocation using Best- and Worst-Fit

## Background

Chapter 6 describes several different strategies for free space management in memory allocation systems. Two of them include:

1. **Best-Fit** - All available spaces are searched, and the smallest one that will accommodate the request is used.
2. **Worst-Fit** - All available spaces are searched, and the largest one that will accommodate the request is used.

While these algorithms are discussed in context of memory allocation for processes in system memory, the same algorithms can be applied to the allocation of heap memory as a process runs. Your program will simulate a series of memory requests placed by a running process. This will be accomplished by reading in data describing the memory, then a list of memory requests. The memory requests include:

1. `malloc`
2. `free`
3. `realloc`

Since we are not actually implementing a memory manager directly, your program will be a simulation, and should manage a free list as discussed in the chapter using a doubly-linked list:

```c
// Doubly linked list node
struct memNode {
	int isFree;
	int start;
	int size;
	struct memNode *prev;
	struct memNode *next;
};
```

You will be provided with a header file (**`assign3.h`**) with this code, as well as prototypes for functions you should implement as part of your solution.

## Assignment

For this assignment, you should write a C program that takes the name of an input file on the command line, and runs a simulation based on the data in that file. Note the change in process! Your program will now read the name of the input file and open it directly. We will not be redirecting files!

The input file has the following format:

```
<memory size>
<memory allocation algorithm>
<request 1>
<request 2>
.
.
.
<request n>
-1
```

[Note that **all** entries in the file are ints.]

The memory allocation algorithm in this file is either 1 (Best-Fit) or 2 (Worst-Fit) as shown above.

The requests are also numbered 1 through 3 and represent the three functions listed above in the memory API. These lines will therefore be one of the following:

```
1 <var> <size> // This is a malloc request of <size> bytes stored in variable <var>
2 <var>        // This is a request to free variable <var>
3 <var> <size> // This is a realloc request of <size> bytes stored in variable <var>
```

In these cases, `<var>` refers to a particular variable (from 0 to 24).

Your program will set up the simulation, and read in the requests from the file until one of two things happens. Either the requests finish (a -1 is read in as the request) or a request cannot be filled because there is not enough free space available. If the request cannot be filled, your program should print out the following:

**ERROR: Out Of Space**

Regardless of how the simulation finishes, your program should print out the memory list and the variable list before stopping.

## Submission

Please pass in a zipped folder containing at a minumum the three files for this assignment:P

- `assign3.h`
- `assign3.c`
- `Makefile`

The folder should be named with your QU login shortname (ie jqpublic)

## Example Runs

Example input and output files will be provided separately

## Notes

- In addition to the header file **`assign3.h`**, you will also be provided with a Makefile to be used for this program, a script to test on a number of test files, and several input files with their corresponding output. I will be grading this assignment using **`diff`** on the command line (see the test script).
- For the **`realloc`** request, I want to make things a bit simpler for us. Your program should free the space allocated to the variable (coalescing as necessary), and *then* should search for space to allocate the new request. The new request could be larger or smaller than the original allocation, but if the new request is for 0 bytes, the request should act like a **free**.
- I will be running your program through **`valgrind`** to check for memory leaks. 5% of your grade will be based on this.
- When you split a node to allocate space, the allocated space should be placed at the beginning of the free chunk in memory.

## Grading Rubric

As this program has several moving parts, I am including a rubric to help guide your work.
**Compile and Run** 25%
**Style and Comments** 5%
**No Memory Leaks**
**Find Free Space** 20% (10% each for the 2 algorithms)
**Malloc (Split)** 15%
**Free (Coalesce)** 15%
**Realloc** 15%