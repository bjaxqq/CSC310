#include <stdio.h>
#include <stdlib.h>

// A node to list a process's kids
typedef struct pid_node {
    int pid;               // a kid's ID
    struct pid_node *next; // next kid in the list
} pid_node;

// The "Process Control Block" (PCB)
// Holds all the info for one process
typedef struct pcb {
    int pid;
    int ppid;
    struct pid_node *children; // list of my kids
} pcb;

// helper to free list of children
void free_child_list(pid_node *head) {
    // loop through and free each node
    while (head != NULL) {
        pid_node *temp = head; // save current
        head = head->next; // move to next
        free(temp); // free saved one
    }
}

// deletes process and kids
void delete_process(int pid, pcb *processes) {
    // if its gone do nothing
    if (processes[pid].pid == -1) {
        return;
    }

    // recursively delete all children
    pid_node *child = processes[pid].children;
    while (child != NULL) {
        delete_process(child->pid, processes);
        child = child->next;
    }

    // clean up process
    free_child_list(processes[pid].children); // free children list
    processes[pid].pid = -1; // mark as deleted
}

int main() {
    int max_processes;

    // get max processes from user
    if (scanf("%d", &max_processes) != 1 || max_processes <= 0) {
        return 1; // bad input
    }

    // eat newline character left by scanf
    int temp_char;
    while ((temp_char = getchar()) != '\n' && temp_char != EOF);

    // make space for all processes
    pcb *processes = (pcb *)malloc(max_processes * sizeof(pcb));
    if (processes == NULL) return 1; // out of memory

    // mark all process slots as empty
    for (int i = 0; i < max_processes; i++) {
        processes[i].pid = -1;
        processes[i].children = NULL;
    }

    // create process 0
    processes[0].pid = 0;
    processes[0].ppid = 0;
    int next_pid = 1; // next new process will be ID 1

    char command;
    int target_id;
    char line_buffer[100];
    
    // main command loop
    // runs until user types 'q'
    while (fgets(line_buffer, sizeof(line_buffer), stdin) != NULL) {
        // parse input line for command and ID
        int items_scanned = sscanf(line_buffer, " %c %d", &command, &target_id);

        // 'q' to quit
        if (items_scanned == 1 && command == 'q') {
            break;
        }

        // if input is bad skip to next line
        if (items_scanned != 2) {
            continue;
        }

        // 'c' for create
        if (command == 'c') {
            // check for space and does parent exist
            if (next_pid < max_processes && target_id >= 0 && target_id < max_processes && processes[target_id].pid != -1) {
                // set up the new process's info
                processes[next_pid].pid = next_pid;
                processes[next_pid].ppid = target_id;
                processes[next_pid].children = NULL;

                // add new process to parent's children list
                pid_node *child_node = (pid_node *)malloc(sizeof(pid_node));
                if (child_node == NULL) {
                    free(processes);
                    return 1; // out of memory
                }
                child_node->pid = next_pid;
                // stick at the front of list
                child_node->next = processes[target_id].children;
                processes[target_id].children = child_node;

                next_pid++; // get ready for next one
            }
        // 'd' for delete
        } else if (command == 'd') {
            // if target in valid range
            if (target_id >= 0 && target_id < max_processes) {
                // kill process and family tree
                delete_process(target_id, processes);
            }
        }
    }

    // print what's left
    for (int i = 0; i < max_processes; i++) {
        if (processes[i].pid != -1) {
            printf("%d %d\n", processes[i].pid, processes[i].ppid);
        }
    }

    // free all memory used
    for (int i = 0; i < max_processes; i++) {
        if (processes[i].pid != -1) {
            // free leftover children lists
            free_child_list(processes[i].children);
        }
    }
    
    // free main process array
    free(processes);
    return 0;
}