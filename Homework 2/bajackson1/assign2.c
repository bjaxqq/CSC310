// assign2.c
// Author: bajackson1@quinnipiac.edu

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <stdatomic.h>

#define DURATION 150
#define SLEEP_TIME 10000
#define CHILD_POLL_USEC 100

// Struct for a single transaction
struct transaction {
    short startTime;
    short duration;
    short amount;
};

// Struct for a process's script of transactions
struct procStruct {
    short count;
    struct transaction arr[1000];
};

// Shared clock
atomic_int *cT;
// Shared bank balance
int *balance;
// Shared semaphore
sem_t *sem;
// Shared process data array
struct procStruct *processes;

// Function for clock child process
void timerChild() {
	// Loop until max simulation time
	while (atomic_load(cT) < DURATION) {
		// Wait for a bit
		usleep(SLEEP_TIME);
		// Tick the clock
		atomic_fetch_add(cT, 1);
	}

	exit(0);
}

// Function for worker child processes
void procChild(int proc_index) {
    // Get this process's data and ID
    struct procStruct *my_proc_data = &processes[proc_index];
    int proc_id = proc_index + 1;

    // Loop through transactions for this process
    for (int i = 0; i < my_proc_data->count; i++) {
        struct transaction current_tx = my_proc_data->arr[i];
        int curTime = 0;

        // Poll clock until time for this transaction
        do {
            curTime = atomic_load(cT);
            usleep(CHILD_POLL_USEC);
        } while (curTime < current_tx.startTime);

        // Wait for lock
        sem_wait(sem);

        // Record start time after getting lock
        int start_time = atomic_load(cT);
        int success_flag = 0;
        int final_balance;

        // Check if withdrawal is possible
        if (current_tx.amount < 0 && (*balance + current_tx.amount < 0)) {
            // Fail transaction
            success_flag = 0;
            final_balance = *balance;
        } else {
            // Succeed transaction and update balance
            success_flag = 1;
            *balance += current_tx.amount;
            final_balance = *balance;
        }

        // Calculate when transaction will finish
        int completion_time = start_time + current_tx.duration;

        // Print results on single line
        printf("%d %d %d %d %d %d\n",
               proc_id, current_tx.startTime, completion_time,
               success_flag, current_tx.amount, final_balance);
        
        // Flush output buffer
        fflush(stdout);

        // Hold lock while processing transaction
        while (atomic_load(cT) < completion_time) {
            usleep(CHILD_POLL_USEC);
        }
        
        // Return lock
        sem_post(sem);
    }

    exit(0);
}

int main() {
    int num_procs, num_trans;

    // Read initial process and transaction counts
    scanf("%d %d", &num_procs, &num_trans);

    // Create shared clock using mmap
	cT = (atomic_int *)mmap(NULL, sizeof(atomic_int), PROT_READ | PROT_WRITE,
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	atomic_store(cT,0); // Initialize clock

    // Create shared balance using mmap
    balance = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
								MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *balance = 0;

    // Create shared semaphore using mmap
	sem = (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
								MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Create shared memory for all process data
    processes = (struct procStruct *)mmap(NULL, num_procs * sizeof(struct procStruct),
                                          PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize semaphore
    if (sem_init(sem, 1, 1) == -1) {
        perror("sem_init failed");
        exit(1);
    };

    // Set initial transaction counts to 0 for all processes
    for (int i = 0; i < num_procs; i++) {
        processes[i].count = 0;
    }

    // Read all transaction data from input
    for (int i = 0; i < num_trans; i++) {
        int arrival, proc_num, duration, amount;
        scanf("%d %d %d %d", &arrival, &proc_num, &duration, &amount);
        
        // Convert to 0-based index for array
        int proc_index = proc_num - 1;
        int trans_index = processes[proc_index].count;
        
        // Store the transaction data
        processes[proc_index].arr[trans_index].startTime = arrival;
        processes[proc_index].arr[trans_index].duration = duration;
        processes[proc_index].arr[trans_index].amount = amount;
        
        // Increment the count of transactions for that process
        processes[proc_index].count++;
    }

    // Fork timer process
	int pid = fork();
	if (pid == 0)
		timerChild();

    // Fork child processes
    for (int i = 0; i < num_procs; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process runs
            procChild(i);
        } else if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
    }
    
    // Parent waits for all children
    for (int i = 0; i < num_procs + 1; i++) {
        wait(NULL);
    }

	// Clean up the semaphore and shared memory
	sem_destroy(sem);
	munmap(cT, sizeof(atomic_int));
	munmap(balance, sizeof(int));
	munmap(sem, sizeof(sem_t));
	munmap(processes, num_procs * sizeof(struct procStruct));

    return 0;
}