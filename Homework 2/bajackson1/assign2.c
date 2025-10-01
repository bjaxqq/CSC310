// assign2.c
// Author: bajackson1@quinniac.edu

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

struct transaction {
    short startTime; // Time in simulated clock ticks when transaction request received
    short duration; // Number of simulated clock ticks the transaction will take
    short amount; // Amount of transaction (negative for withdrawal)
};

struct procStruct {
    short count; // Number of transactions in the for this process
    struct transaction arr [1000]; // Array of transactions
};

int main() {
    int num_processes;
    int num_transactions;
    char line_buffer[100];

    // Getting input for processes and transactions
    if (fgets(line_buffer, sizeof(line_buffer), stdin) != NULL) {
        if (sscanf(line_buffer, "%d %d", &num_processes, &num_transactions) != 2) {
            return 1; // Bad input
        }
    } else {
        return 1; // Input failed
    }
    
    // Create shared memory for process data
    struct procStruct *shared_data = mmap(NULL, 
                                          num_processes * sizeof(struct procStruct), 
                                          PROT_READ | PROT_WRITE, 
                                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Create shared memory for bank balance
    int *balance = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *balance = 0; // Initialize balance

    // Create shared memory for clock
    int *clock = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *clock = 0; // Initialize clock

    // Create semaphore in shared memory
    sem_t *sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (sem_init(sem, 1, 1) == -1) {
        perror("sem_init failed");
        exit(1);
    };
    
    // Initialize transaction counts
    for (int i = 0; i < num_processes; i++) {
        shared_data[i].count = 0;
    }

    // Read all transaction data from input
    int arrival, proc_num, duration, amount;

    for (int i = 0; i < num_transactions; i++) {
        if (fgets(line_buffer, sizeof(line_buffer), stdin) == NULL) {
            break; 
        }

        sscanf(line_buffer, "%d %d %d %d", &arrival, &proc_num, &duration, &amount);

        int process_index = proc_num - 1; 
        int transaction_index = shared_data[process_index].count;

        // Store transaction
        shared_data[process_index].arr[transaction_index].startTime = arrival;
        shared_data[process_index].arr[transaction_index].duration = duration;
        shared_data[process_index].arr[transaction_index].amount = amount;

        // Increment transaction count for process
        shared_data[process_index].count++;

        // Debug statement
        printf("Process: %d, New Count: %d\n", proc_num, shared_data[process_index].count);
    }

    return 0;
}