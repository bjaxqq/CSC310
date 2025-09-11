// assign1.c
// Author: bajackson1@quinnipiac.edu

// Packages
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct to represent a node in a pid list
typedef struct pid_node {
    int pid ;
    struct pid_node * next ;
} pid_node ;

// Struct to represent a process
typedef struct pcb {
    int pid ;
    int ppid ;
    struct pid_node * children ;
} pcb ;

// Main function
int main() {
    printf("Test message");
    return 0;
}