#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "qfs.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <disk image file>\n", argv[0]);
        return 1;
    }
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 2;
    }

#ifdef DEBUG
    printf("Opened disk image: %s\n", argv[1]);
#endif

    //TODO

    // Read superblock
    superblock_t sb;

    if (fread(&sb, sizeof(superblock_t), 1, fp) != 1) {
        fprintf(stderr, "Error: Failed to read superblock.\n");
        fclose(fp);
        return 3;
    }

    // Print information from superblock
    printf("--- Superblock Information ---\n");
    printf("Block size: %u\n", sb.bytes_per_block);
    printf("Total number of blocks: %u\n", sb.total_blocks);
    printf("Number of free blocks: %u\n", sb.available_blocks);
    printf("Total number of directory entries: %u\n", sb.total_direntries);
    printf("Number of free directory entries: %u\n", sb.available_direntries);

    // Print volume label if not empty
    if (sb.label[0] != '\0') {
        printf("Label: %s\n", sb.label);
    }

    printf("------------------------------\n");

    // List directory contents
    printf("\n--- Directory Listing ---\n");
    printf("%-24s %-10s %-10s %-15s\n", "Filename", "Size", "Type", "Start Block");
    printf("----------------------------------------------------------------\n");

    // Make sure at start of directory entries
    fseek(fp, sizeof(superblock_t), SEEK_SET);

    direntry_t entry;

    // Iterate through directory entries
    for (int i = 0; i < sb.total_direntries; i++) {
        // Read one directory entry at a time
        if (fread(&entry, sizeof(direntry_t), 1, fp) != 1) {
            // Stop if end of file or error
            break;
        }

        // Directory entry valid if filename not empty
        if (entry.filename[0] != '\0') {
            printf("%-24s %-10u %-10u %-15u\n",
                   entry.filename,
                   entry.file_size,
                   entry.permissions,
                   entry.starting_block);
        }
    }

    fclose(fp);
    return 0;
}