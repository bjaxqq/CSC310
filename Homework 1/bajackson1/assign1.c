// assign.c
// Author: bajackson1@quinnipiac.edu

#include <stdio.h>
#include <stdlib.h>

typedef struct pid_node {
    int pid;
    struct pid_node *next;
} pid_node;

typedef struct pcb {
    int pid;
    int ppid;
    struct pid_node *children;
} pcb;

/**
 * Freeing linked list for memory leaks
 */
void free_child_list(pid_node *head) {
    while (head != NULL) {
        pid_node *temp = head;
        head = head->next;
        free(temp);
    }
}

/**
 * Recursively deletes process and descendants
 */
void delete_process(int pid, pcb *processes) {
    // If process gone do nothing
    if (processes[pid].pid == -1) {
        return;
    }

    // Recursively delete all children of process
    pid_node *child = processes[pid].children;
    while (child != NULL) {
        delete_process(child->pid, processes);
        child = child->next;
    }

    // After descendants are gone free process children list
    free_child_list(processes[pid].children);
    // Mark slot in main array empty
    processes[pid].pid = -1;
}

int main() {
    int max_processes;
    if (scanf("%d", &max_processes) != 1 || max_processes <= 0) {
        return 1; // Exit if input is invalid
    }

    // Allocate main array for PCBs
    pcb *processes = (pcb *)malloc(max_processes * sizeof(pcb));
    if (processes == NULL) return 1; // Exit if memory allocation fails

    // Initialize PCB slots to empty state
    for (int i = 0; i < max_processes; i++) {
        processes[i].pid = -1;
        processes[i].children = NULL;
    }

    // Create root process
    processes[0].pid = 0;
    processes[0].ppid = 0;
    int next_pid = 1; // Next available PID starts at 1

    char command;
    int target_id;
    
    // Read commands
    while (scanf(" %c", &command) == 1 && command != 'q') {
        scanf("%d", &target_id);

        if (command == 'c') {
            // Create if space and the parent exist
            if (next_pid < max_processes && target_id >= 0 && target_id < max_processes && processes[target_id].pid != -1) {
                // Set up new process PCB at designated index
                processes[next_pid].pid = next_pid;
                processes[next_pid].ppid = target_id;
                processes[next_pid].children = NULL;

                // Create node to add to parent children list
                pid_node *child_node = (pid_node *)malloc(sizeof(pid_node));
                if (child_node == NULL) { // Check if malloc succeeded
                    free(processes);
                    return 1;
                }
                child_node->pid = next_pid;
                
                // Add new child to front of parent list
                child_node->next = processes[target_id].children;
                processes[target_id].children = child_node;

                next_pid++; // Increment for next creation
            }
        } else if (command == 'd') {
            if (target_id >= 0 && target_id < max_processes) {
                 delete_process(target_id, processes);
            }
        }
    }

    // Loop through array and print active processes
    for (int i = 0; i < max_processes; i++) {
        if (processes[i].pid != -1) {
            printf("%d %d\n", processes[i].pid, processes[i].ppid);
        }
    }

    // Free the children list of any remaining active processes
    for (int i = 0; i < max_processes; i++) {
        if (processes[i].pid != -1) {
            free_child_list(processes[i].children);
        }
    }
    
    // Free the main processes array itself
    free(processes);

    return 0;
}