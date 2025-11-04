// assign3.c
// Author: bajackson1@quinniac.edu

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assign3.h"

#define FIRST_FIT 3
#define NEXT_FIT  4

int out_of_space = 0; // Stop simulation if no available space
struct memNode *last_fit_node = NULL; // Next-Fit pointer

void handle_malloc(struct memNode *head, struct memNode *vars[], int var, int size, int algo);
void handle_free(struct memNode *vars[], int var);
void handle_realloc(struct memNode *head, struct memNode *vars[], int var, int size, int algo);


// Main simulation
int main(int argc, char *argv[]) {
    int memSize = 0;
    int algo = 0;
    int req, var, size;
    FILE *file;

    // Array for allocated variable node pointers
    struct memNode *varList[NUM_VARS];

    // Make all variables in list NULL
    for (int i = 0; i < NUM_VARS; i++) {
        varList[i] = NULL;
    }

    // Check for correct arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Open input file
    file = fopen(argv[1], "r");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read memory size and algorithm
    fscanf(file, "%d", &memSize);
    fscanf(file, "%d", &algo);

    // Create large free block as head of memory list
    struct memNode *head = makeNode(1, 0, memSize, NULL, NULL);

    // Initialize Next-Fit pointer to start of memory
    last_fit_node = head;

    // Simulation loop that goes until no space or -1
    while (fscanf(file, "%d", &req) == 1 && req != -1) {
        // Skip remaining if request already failed
        if (out_of_space) {
            char buffer[100];
            fgets(buffer, sizeof(buffer), file);
            continue;
        }

        // Handle the memory request
        switch (req) {
            case MALLOC: // 1 <var> <size>
                fscanf(file, "%d %d", &var, &size);
                handle_malloc(head, varList, var, size, algo);
                break;

            case FREE: // 2 <var>
                fscanf(file, "%d", &var);
                handle_free(varList, var);
                break;

            case REALLOC: // 3 <var> <size>
                fscanf(file, "%d %d", &var, &size);
                handle_realloc(head, varList, var, size, algo);
                break;

            default:
                printf("Unknown request type: %d\n", req);
        }
    }

    fclose(file);

    // Print error if no space
    if (out_of_space) {
        printf("ERROR: Out of Space\n\n");
    }

    // Print memory and variable states
    printList(head);
    printVars(varList);

    // Free nodes in memory list
    struct memNode *current = head;

    while (current != NULL) {
        struct memNode *temp = current;
        current = current->next;
        free(temp); // Free node
    }

    return 0;
}

// Handle MALLOC request
void handle_malloc(struct memNode *head, struct memNode *vars[], int var, int size, int algo) {
    // Find free block
    struct memNode *block = findFree(head, size, algo);

    // Error flag set if no block found
    if (block == NULL) {
        out_of_space = 1;
        return;
    }

    // Split block if larger than required
    if (block->size > size) {
        split(block, size);
    }

    // Mark block allocated
    block->isFree = 0;

    // Point variable to block
    vars[var] = block;

    // Move Next-Fit pointer after allocation
    if (algo == NEXT_FIT) {
        // Start next search from next node/wrap to head
        last_fit_node = (block->next != NULL) ? block->next : head;
    }
}

// Handle FREE request
void handle_free(struct memNode *vars[], int var) {
    // Do nothing if free/invalid var
    if (var < 0 || var >= NUM_VARS || vars[var] == NULL) {
        return;
    }

    // Get block to free
    struct memNode *block = vars[var];

    // Mark block free
    block->isFree = 1;

    // Remove from variable list
    vars[var] = NULL;

    // Coalesce
    coalesce(block);
}

// Handle REALLOC request
void handle_realloc(struct memNode *head, struct memNode *vars[], int var, int size, int algo) {
    if (size == 0) {
        handle_free(vars, var);
        return;
    }

    // Free original block first
    if (var >= 0 && var < NUM_VARS && vars[var] != NULL) {
         handle_free(vars, var);
    }

    // Allocate new block for variable
    handle_malloc(head, vars, var, size, algo);
}

// Find free block of memory with specified algorithm
struct memNode *findFree(struct memNode *h, int size, int algo) {
    struct memNode *current = NULL;
    struct memNode *best = NULL;

    switch (algo) {
        // Best-Fit
        case BEST_FIT:
            current = h;

            while (current != NULL) {
                if (current->isFree && current->size >= size) {
                    if (best == NULL || current->size < best->size) {
                        best = current;
                    }
                }
                current = current->next;
            }

            return best; // Return best block

        // Worst-Fit
        case WORST_FIT:
            current = h;

            while (current != NULL) {
                if (current->isFree && current->size >= size) {
                    if (best == NULL || current->size > best->size) {
                        best = current;
                    }
                }
                current = current->next;
            }

            return best; // Return worst block

        // First-Fit
        case FIRST_FIT:
            current = h;

            while (current != NULL) {
                if (current->isFree && current->size >= size) {
                    return current; // Return first one
                }
                current = current->next;
            }

            return NULL; // No block found

        // Next-Fit
        case NEXT_FIT:
            // Start search from last spot
            current = last_fit_node;

            while (current != NULL) {
                if (current->isFree && current->size >= size) {
                    return current; // Return first one found
                }
                current = current->next;
            }

            // Wrap around if not found
            current = h;

            while (current != last_fit_node) {
                 if (current->isFree && current->size >= size) {
                    return current; // Return first one found
                }

                current = current->next;
            }

            return NULL; // No block found
    }

    return NULL; // Default case
}

// Split free node into allocated and free parts
void split(struct memNode *p, int size) {
    // Split if leftover space
    if (p->size > size) {
        int newSize = p->size - size;
        int newStart = p->start + size;

        // Create new free node for remainder
        struct memNode *newNode = makeNode(1, newStart, newSize, p, p->next);

        // Link new node into list
        if (p->next != NULL) {
            p->next->prev = newNode;
        }

        p->next = newNode;

        // Update original node size
        p->size = size;
    }
}

// Merge newly freed node with free neighbors
void coalesce(struct memNode *p) {
    // Check next/right neighbor
    if (p->next != NULL && p->next->isFree) {
        struct memNode *toRemove = p->next;
        p->size += toRemove->size; // Absorb size
        p->next = toRemove->next; // Bypass

        if (toRemove->next != NULL) {
            toRemove->next->prev = p;
        }

        // Move if Next-Fit points to merging node
        if (last_fit_node == toRemove) {
            last_fit_node = p;
        }

        free(toRemove); // Free merged node
    }

    // Check previous/left neighbor
    if (p->prev != NULL && p->prev->isFree) {
        struct memNode *prevNode = p->prev;
        prevNode->size += p->size; // Absorb size
        prevNode->next = p->next; // Bypass

        if (p->next != NULL) {
            p->next->prev = prevNode;
        }

        // Move if Next-Fit points to merging node
        if (last_fit_node == p) {
            last_fit_node = prevNode;
        }

        free(p); // Free original node
    }
}

// Create new memNode
struct memNode *makeNode(int isfree, int start, int size, struct memNode *prev, struct memNode *next) {
    struct memNode *newNode = (struct memNode *)malloc(sizeof(struct memNode));

    if (newNode == NULL) {
        perror("malloc failed in makeNode");
        exit(1);
    }

    newNode->isFree = isfree;
    newNode->start = start;
    newNode->size = size;
    newNode->prev = prev;
    newNode->next = next;
    return newNode;
}

// Print memory list
void printList(struct memNode *p) {
    printf("List:\n");
    struct memNode *current = p;

    // Print each node
    while (current != NULL) {
        printf("(%d,%d,%d)\n", current->isFree, current->start, current->size);
        current = current->next;
    }

    printf("\n");
}

// Print variable list
void printVars(struct memNode *p[]) {
    printf("Vars:\n");

    // Loop through all possible variables
    for (int i = 0; i < NUM_VARS; i++) {
        // Print info if allocated variable
        if (p[i] != NULL) {
            printf("(%d,%d,%d)\n", i, p[i]->start, p[i]->size);
        }
    }
}
