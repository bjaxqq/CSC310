// assign1.c
// Author: bajackson1@quinnipiac.edu

#include <stdio.h>
#include <stdlib.h>

// Struct to represent a node in a pid list
typedef struct pid_node {
    int pid;
    struct pid_node *next;
} pid_node;

// Struct to represent a process
typedef struct pcb {
    int pid;
    int ppid;
    struct pid_node *children;
} pcb;

// Helper to free list of children
void free_child_list(pid_node *head) {
    // Loop through and free each node
    while (head != NULL) {
        pid_node *temp = head; // Save current
        head = head->next; // Move to next
        free(temp); // Free saved one
    }
}

// Deletes process and kids
void delete_process(int pid, pcb *processes) {
    // If its gone do nothing
    if (processes[pid].pid == -1) {
        return;
    }

    // Recursively delete all children
    pid_node *child = processes[pid].children;
    while (child != NULL) {
        delete_process(child->pid, processes);
        child = child->next;
    }

    // Clean up process
    free_child_list(processes[pid].children); // Free children list
    processes[pid].pid = -1; // Mark as deleted
}

int main() {
    int max_processes;

    // Get max processes from user
    if (scanf("%d", &max_processes) != 1 || max_processes <= 0) {
        return 1; // Bad input
    }

    // Eat newline character left by scanf
    int temp_char;
    while ((temp_char = getchar()) != '\n' && temp_char != EOF);

    // Make space for all processes
    pcb *processes = (pcb *)malloc(max_processes * sizeof(pcb));
    if (processes == NULL) return 1; // Out of memory

    // Mark all process slots as empty
    for (int i = 0; i < max_processes; i++) {
        processes[i].pid = -1;
        processes[i].children = NULL;
    }

    // Create process 0
    processes[0].pid = 0;
    processes[0].ppid = 0;
    int next_pid = 1; // Next new process will be ID 1

    char command;
    int target_id;
    char line_buffer[100];
    
    // Main command loop
    // Runs until user types 'q'
    while (fgets(line_buffer, sizeof(line_buffer), stdin) != NULL) {
        // Parse input line for command and ID
        int items_scanned = sscanf(line_buffer, " %c %d", &command, &target_id);

        // 'q' to quit
        if (items_scanned == 1 && command == 'q') {
            break;
        }

        // If input is bad skip to next line
        if (items_scanned != 2) {
            continue;
        }

        // 'c' for create
        if (command == 'c') {
            // Check for space and does parent exist
            if (next_pid < max_processes && target_id >= 0 && target_id < max_processes && processes[target_id].pid != -1) {
                // Set up the new process's info
                processes[next_pid].pid = next_pid;
                processes[next_pid].ppid = target_id;
                processes[next_pid].children = NULL;

                // Add new process to parent's children list
                pid_node *child_node = (pid_node *)malloc(sizeof(pid_node));
                if (child_node == NULL) {
                    free(processes);
                    return 1; // Out of memory
                }
                child_node->pid = next_pid;
                // Stick at the front of list
                child_node->next = processes[target_id].children;
                processes[target_id].children = child_node;

                next_pid++; // Get ready for next one
            }
        // 'd' for delete
        } else if (command == 'd') {
            // If target in valid range
            if (target_id >= 0 && target_id < max_processes) {
                // Kill process and family tree
                delete_process(target_id, processes);
            }
        }
    }

    // Print what's left
    for (int i = 0; i < max_processes; i++) {
        if (processes[i].pid != -1) {
            printf("%d %d\n", processes[i].pid, processes[i].ppid);
        }
    }

    // Free all memory used
    for (int i = 0; i < max_processes; i++) {
        if (processes[i].pid != -1) {
            // Free leftover children lists
            free_child_list(processes[i].children);
        }
    }
    
    // Free main process array
    free(processes);
    return 0;
}