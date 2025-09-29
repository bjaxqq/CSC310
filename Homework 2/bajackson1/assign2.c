// assign2.c
// Author: bajackson1@quinnipiac.edu

#include <stdio.h>
#include <semaphore.h>

struct transaction {
    short startTime ; // Time in simulated clock ticks when transaction request received
    short duration ; // Number of simulated clock ticks the transaction will take
    short amount ; // Amount of transaction (negative for withdrawal)
};

struct procStruct {
    short count ; // Number of transactions in the for this process
    struct transaction arr [1000]; // Array of transactions
};

int main() {
    // Create a global array of type procStruct [] in shared memory using mmap.
}