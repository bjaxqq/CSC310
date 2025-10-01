// assign2.c
// Author: bajackson1@quinniac.edu

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>

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

    // Get input for processes and transactions
    if (fgets(line_buffer, sizeof(line_buffer), stdin) != NULL) {
        if (sscanf(line_buffer, "%d %d", &num_processes, &num_transactions) != 2) {
            return 1; // Bad input
        }
    } else {
        return 1; // Input failed
    }
    
    // Create shared memory for process data
    struct procStruct *shared_data = mmap(NULL, num_processes * sizeof(struct procStruct), 
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
    }

    // Fork child processes
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // Child process
            int process_id = i; 
            // Array to track completed transactions for this child
            int processed[1000] = {0};

            // Child process main loop
            while (*clock < 100) {
                // Loop through process's transactions
                for (int j = 0; j < shared_data[process_id].count; j++) {
                    // Check time and if transaction is not yet processed
                    if (*clock >= shared_data[process_id].arr[j].startTime && processed[j] == 0) {
                        
                        // Request access to shared balance
                        sem_wait(sem);
                    
                        // Record start time
                        int actual_start_time = *clock;
                        
                        // Local variables to store the result of this transaction
                        int success = 0;
                        int final_balance = 0;
                        short current_amount = shared_data[process_id].arr[j].amount;
                        short current_duration = shared_data[process_id].arr[j].duration;

                        // Check if transaction is possible
                        if (current_amount > 0 || (*balance + current_amount >= 0)) {
                            *balance += current_amount;
                            success = 1;
                        }
                        // Store balance after the transaction for printing
                        final_balance = *balance;

                        // Calculate when the transaction will finish
                        int finish_time = actual_start_time + current_duration;

                        // Release access to shared balance
                        sem_post(sem);

                        // Mark transaction as processed
                        processed[j] = 1;
                    }
                }
                // Sleep for waiting
                usleep(100);
            }

            // Exit after main loop is done
            exit(0);
        }
    }

    // Parent process as timekeeper
    while (*clock < 100) {
        // Simulate one tick of clock
        usleep(10000);
        (*clock)++;
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    // Clean up shared memory and semaphore
    sem_destroy(sem);
    munmap(shared_data, num_processes * sizeof(struct procStruct));
    munmap(balance, sizeof(int));
    munmap(clock, sizeof(int));
    munmap(sem, sizeof(sem_t));

    return 0;
}