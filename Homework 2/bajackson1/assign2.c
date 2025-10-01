// assign2.c
// Author: bajackson1@quinnipiac.edu

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <stdatomic.h>

// Constants for the simulation
#define MAX_CLOCK 150          // Max time for simulation to run, safely above 100
#define CLOCK_SLEEP_USEC 10000 // 10ms per clock tick to slow down simulation
#define CHILD_POLL_USEC 100    // Small sleep for children while waiting

// Struct to hold data for a single transaction
struct transaction {
    short startTime; // Time in simulated clock ticks when transaction request received
    short duration;  // Number of simulated clock ticks the transaction will take
    short amount;    // Amount of transaction (negative for withdrawal)
};

// Struct to hold all transactions for a single process
struct procStruct {
    short count;                  // Number of transactions in the for this process
    struct transaction arr[1000]; // Array of transactions
};

// Create global pointers for shared memory variables
atomic_int *clockTime;      // Shared simulation clock
int *balance;               // Shared bank account balance
sem_t *sem;                 // Shared semaphore for controlling access to balance
struct procStruct *processes; // Shared array of process transaction data

// Function for the clock child process
void timerChild() {
    // Loop until the simulation duration is met
    while (atomic_load(clockTime) < MAX_CLOCK) {
        // Wait for a bit to define the speed of a clock tick
        usleep(CLOCK_SLEEP_USEC);
        // Increment the clock
        atomic_fetch_add(clockTime, 1);
    }
    // Exit when the clock's job is done
    exit(0);
}

// Function for each worker child process
void procChild(int proc_index) {
    // Get a pointer to this process's data and set its 1-based ID for printing
    struct procStruct *my_proc_data = &processes[proc_index];
    int proc_id = proc_index + 1;

    // Loop through all assigned transactions for this process
    for (int i = 0; i < my_proc_data->count; i++) {
        struct transaction current_tx = my_proc_data->arr[i];

        // Wait until the simulation clock reaches the transaction's arrival time
        while (atomic_load(clockTime) < current_tx.startTime) {
            usleep(CHILD_POLL_USEC);
        }

        // Request the lock for the bank account
        sem_wait(sem);

        // --- Critical Section Start ---

        // Record the actual time the transaction begins after acquiring the lock
        int actual_start_time = atomic_load(clockTime);
        int success_flag = 0;
        int final_balance;

        // Check if a withdrawal would result in a negative balance
        if (current_tx.amount < 0 && (*balance + current_tx.amount < 0)) {
            // Mark transaction as failed
            success_flag = 0;
            // Balance does not change
            final_balance = *balance;
        } else {
            // Mark transaction as successful
            success_flag = 1;
            // Update the balance
            *balance += current_tx.amount;
            // Record the new balance
            final_balance = *balance;
        }

        // Calculate when the transaction will be complete
        int completion_time = actual_start_time + current_tx.duration;

        // Print the results for this transaction as specified in the README
        printf("%d %d %d %d %d %d\n",
               proc_id,
               current_tx.startTime,
               completion_time,
               success_flag,
               current_tx.amount,
               final_balance);
        
        // Ensure output is printed immediately
        fflush(stdout);

        // Hold the lock to simulate the transaction duration
        while (atomic_load(clockTime) < completion_time) {
            usleep(CHILD_POLL_USEC);
        }

        // Release the lock for the bank account
        sem_post(sem);
        
        // --- Critical Section End ---
    }

    // Exit when all transactions for this process are done
    exit(0);
}

int main() {
    int num_procs, num_trans;

    // Read the total number of processes and transactions
    scanf("%d %d", &num_procs, &num_trans);

    // Set up shared memory for the clock using mmap
    clockTime = (atomic_int *)mmap(NULL, sizeof(atomic_int), PROT_READ | PROT_WRITE,
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // Set up shared memory for the bank balance
    balance = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // Set up shared memory for the semaphore
    sem = (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // Set up shared memory for the array of process data
    processes = (struct procStruct *)mmap(NULL, num_procs * sizeof(struct procStruct),
                                          PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize shared variables to their starting values
    atomic_store(clockTime, 0);
    *balance = 0;

    // Initialize the transaction counts for each process to zero
    for (int i = 0; i < num_procs; i++) {
        processes[i].count = 0;
    }

    // Initialize the semaphore to be shared between processes, with an initial value of 1 (unlocked)
    if (sem_init(sem, 1, 1) == -1) {
        perror("sem_init failed");
        exit(1);
    }

    // Loop to read all transaction data from input
    for (int i = 0; i < num_trans; i++) {
        int arrival, proc_num, duration, amount;
        scanf("%d %d %d %d", &arrival, &proc_num, &duration, &amount);

        // Adjust 1-based process number to 0-based index
        int proc_index = proc_num - 1;
        // Find the next empty transaction slot for this process
        int trans_index = processes[proc_index].count;

        // Store the transaction details in the correct process's array
        processes[proc_index].arr[trans_index].startTime = arrival;
        processes[proc_index].arr[trans_index].duration = duration;
        processes[proc_index].arr[trans_index].amount = amount;

        // Increment the transaction count for that process
        processes[proc_index].count++;
    }

    // Fork the clock child process
    if (fork() == 0) {
        timerChild();
    }

    // Fork a child process for each banking client
    for (int i = 0; i < num_procs; i++) {
        if (fork() == 0) {
            procChild(i);
        }
    }

    // Wait for all children (clock + workers) to finish
    for (int i = 0; i < num_procs + 1; i++) {
        wait(NULL);
    }

    // Clean up the semaphore
    sem_destroy(sem);

    // Unmap and free all shared memory segments
    munmap(clockTime, sizeof(atomic_int));
    munmap(balance, sizeof(int));
    munmap(sem, sizeof(sem_t));
    munmap(processes, num_procs * sizeof(struct procStruct));

    return 0;
}