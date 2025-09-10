// assign1.c
// Author: bajackson1@quinnipiac.edu

#include <stdio.h>

typedef struct pid_node {
    int pid;
    struct pid_node *next;
} pid_node;

typedef struct pcb {
    int pid;
    int ppid;
    pid_node *children;
} pcb;

int main() {
    // IMPLEMENT
    return 0;
}
