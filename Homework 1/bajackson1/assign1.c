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

// Free the child list
void free_child_list(pid_node *head) {
    while (head != NULL) {
        pid_node *temp = head;
        head = head->next;
        free(temp);
    }
}

// Recursively delete a process and its kids
void delete_process(int pid, pcb *processes) {
    // If process gone do nothing
    if (processes[pid].pid == -1) {
        return;
    }

    // Recursively delete all the kids
    pid_node *child = processes[pid].children;
    while (child != NULL) {
        delete_process(child->pid, processes);
        child = child -> next;
    }

    // Free the child list when kids are gone
    free_child_list(processes[pid].children);
    // Mark slot in main array empty
    processes[pid].pid = -1;
}

int main() {
    int max_processes;

    if (scanf("%d", &max_processes) != 1 || max_processes <= 0) {
        // Exit if input is invalid
        return 1;
    }

    // Eat the rest of the line so fgets doesnt break
    int temp_char;
    while ((temp_char = getchar()) != '\n' && temp_char != EOF);

    // Allocate main array for PCBs
    pcb *processes = (pcb *)malloc(max_processes * sizeof(pcb));
     // Exit if memory allocation fails
    if (processes == NULL) return 1;

    // Set all pcb slots to empty
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
    // Buffer for one line of input
    char line_buffer[100];
    
    // Read commands line by line with fgets
    while (fgets(line_buffer, sizeof(line_buffer), stdin) != NULL) {
        // Get the command from the buffer
        int items_scanned = sscanf(line_buffer, " %c %d", &command, &target_id);

        // Handle the 'q' quit command
        if (items_scanned == 1 && command == 'q') {
            break;
        }

        // If input is bad just skip it
        if (items_scanned != 2) {
            continue;
        }

        if (command == 'c') {
            // Create if space and the parent exist
            if (next_pid < max_processes && target_id >= 0 && target_id < max_processes && processes[target_id].pid != -1) {
                // Setup the new pcb
                processes[next_pid].pid = next_pid;
                processes[next_pid].ppid = target_id;
                processes[next_pid].children = NULL;

                // Make a new node for the parent's list
                pid_node *child_node = (pid_node *)malloc(sizeof(pid_node));
                
                // Check if malloc succeeded
                if (child_node == NULL) {
                    free(processes);
                    return 1;
                }

                child_node -> pid = next_pid;
                
                // Add child to front of the list
                child_node -> next = processes[target_id].children;
                processes[target_id].children = child_node;
                // Increment for next creation
                next_pid++;
            }
        } else if (command == 'd') {
            if (target_id >= 0 && target_id < max_processes) {
                 delete_process(target_id, processes);
            }
        }
    }

    // Print all active processes
    for (int i = 0; i < max_processes; i++) {
        if (processes[i].pid != -1) {
            printf("%d %d\n", processes[i].pid, processes[i].ppid);
        }
    }

    // Free any child lists left
    for (int i = 0; i < max_processes; i++) {
        if (processes[i].pid != -1) {
            free_child_list(processes[i].children);
        }
    }
    
    // Free the main array
    free(processes);

    return 0;
}