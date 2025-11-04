// assign2.c
// Author: bajackson1@quinniac.edu

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>

#define DURATION 100
#define SLEEP_TIME 10000

// Single bank transaction
struct transaction {
    short startTime; // Time in simulated clock ticks when transaction request received
    short duration; // Number of simulated clock ticks the transaction will take
    short amount; // Amount of transaction (negative for withdrawal)
};

// Transactions for single process
struct procStruct {
    short count; // Number of transactions in the for this process
    struct transaction arr[1000]; // Array of transactions
};

// Global pointers for shared memory
int *simulated_clock;
int *shared_account_balance;
sem_t *balance_mutex;
struct procStruct *all_process_data;
int *next_available_tick; // Tracks next available tick for resource

// Simulates the system clock
void timerChild() {
    // Increments shared clock until DURATION is reached
    while (*simulated_clock <= DURATION) {
        usleep(SLEEP_TIME);
        (*simulated_clock)++;
    }

    exit(0);
}

// Simulates a single user process
void procChild(int process_id) {
    struct procStruct *my_process = &all_process_data[process_id];
    int total_transactions = my_process->count;
    int transactions_processed = 0;
    int next_transaction_idx = 0; // Index for the next transaction to check

    // Loop until all of this process's transactions are handled
    while (transactions_processed < total_transactions) {
        // Check if it is time to process the next scheduled transaction
        if (*simulated_clock >= my_process->arr[next_transaction_idx].startTime) {
            struct transaction current_transaction = my_process->arr[next_transaction_idx];
            int is_successful = 0;
            int balance_after_transaction = 0;
            int finish_tick = 0;

            // Request access to critical section
            sem_wait(balance_mutex);

            // Find effective start time based on resource availability
            int effective_start_time = (current_transaction.startTime > *next_available_tick) ? current_transaction.startTime : *next_available_tick;
            // Find when resource will be free next
            int resource_busy_until = effective_start_time + current_transaction.duration;
            // Set finish time as inclusive boundary
            finish_tick = resource_busy_until - 1;

            // Update shared next available tick
            *next_available_tick = resource_busy_until;

            if (current_transaction.amount > 0) { // Deposit
                *shared_account_balance += current_transaction.amount;
                is_successful = 1;
            } else { // Withdrawl
                if (*shared_account_balance >= -current_transaction.amount) { // Check for sufficient funds
                    *shared_account_balance += current_transaction.amount; // Negative for withdrawals
                    is_successful = 1;
                }
            }

            balance_after_transaction = *shared_account_balance;

            // Release access to critical section
            sem_post(balance_mutex);

            // Print transaction results
            printf("%d %d %d %d %d %d\n",
                   current_transaction.startTime,
                   process_id + 1, // process_id is 0-indexed, output is 1-indexed
                   finish_tick,
                   is_successful,
                   current_transaction.amount,
                   balance_after_transaction);
            fflush(stdout); // Flush stdout for immediate output

            transactions_processed++;
            next_transaction_idx++;
        } else {
             usleep(100); // Sleep to prevent busy-waiting
        }
    }

    exit(0);
}

int main() {
    int process_count, transaction_count;
    // Read process and transaction counts
    scanf("%d %d", &process_count, &transaction_count);

    // Allocate shared memory for process data
    all_process_data = (struct procStruct *)mmap(NULL, process_count * sizeof(struct procStruct),
                                                 PROT_READ | PROT_WRITE,
                                                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (all_process_data == MAP_FAILED) { perror("mmap for process data failed"); exit(1); }

    // Allocate remaining shared memory regions
    simulated_clock = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (simulated_clock == MAP_FAILED) { perror("mmap for clock failed"); exit(1); }

    shared_account_balance = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_account_balance == MAP_FAILED) { perror("mmap for balance failed"); exit(1); }

    balance_mutex = (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (balance_mutex == MAP_FAILED) { perror("mmap for semaphore failed"); exit(1); }

    next_available_tick = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (next_available_tick == MAP_FAILED) { perror("mmap for next tick failed"); exit(1); }


    // Initialize transaction counts for each process
    for (int i = 0; i < process_count; i++) {
        all_process_data[i].count = 0;
    }

    // Initialize shared variables
    *simulated_clock = 0;
    *shared_account_balance = 0;
    *next_available_tick = 0; // Resource available at time 0
    sem_init(balance_mutex, 1, 1); // Initialize semaphore as unlocked mutex

    // Read and store transaction data
    for (int i = 0; i < transaction_count; i++) {
        int arrival, proc_num, duration, amount;
        scanf("%d %d %d %d", &arrival, &proc_num, &duration, &amount);

        int process_index = proc_num - 1; // Convert 1-indexed process number to 0-indexed
        int transaction_index = all_process_data[process_index].count;

        all_process_data[process_index].arr[transaction_index].startTime = arrival;
        all_process_data[process_index].arr[transaction_index].duration = duration;
        all_process_data[process_index].arr[transaction_index].amount = amount;
        all_process_data[process_index].count++;
    }

    // Fork child process for timer
    pid_t pid = fork();

    if (pid < 0) { perror("fork for timer failed"); exit(1); }

    if (pid == 0) {
        timerChild();
    }

    // Fork child process for each user
    for (int i = 0; i < process_count; i++) {
        pid = fork();

        if (pid < 0) { perror("fork for process child failed"); exit(1); }

        if (pid == 0) {
            procChild(i);
        }
    }

    // Parent waits for all children to terminate
    for (int i = 0; i < process_count + 1; i++) {
        wait(NULL);
    }

    // Clean up shared memory and semaphore
    sem_destroy(balance_mutex);
    munmap(all_process_data, process_count * sizeof(struct procStruct));
    munmap(simulated_clock, sizeof(int));
    munmap(shared_account_balance, sizeof(int));
    munmap(balance_mutex, sizeof(sem_t));
    munmap(next_available_tick, sizeof(int));

    return 0;
}
