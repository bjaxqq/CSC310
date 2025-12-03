#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "qfs.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <disk image file> <file to remove>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb+");
    if (!fp) {
        perror("fopen");
        return 2;
    }

#ifdef DEBUG
    printf("Opened disk image: %s\n", argv[1]);
#endif

    // Read superblock
    superblock_t sb;

    if (fread(&sb, sizeof(superblock_t), 1, fp) != 1) {
        fprintf(stderr, "Error: Failed to read superblock.\n");
        fclose(fp);
        return 3;
    }

    // Find file in directory entries
    direntry_t entry;
    int entry_index = -1;
    long entry_offset = 0;

    // Seek to start of directory entries
    fseek(fp, sizeof(superblock_t), SEEK_SET);

    for (int i = 0; i < sb.total_direntries; i++) {
        // Store current offset to jump back to write later updates
        long current_pos = ftell(fp);
        fread(&entry, sizeof(direntry_t), 1, fp);
        
        // Check if matching filename and not deleted
        if (entry.filename[0] != '\0' && strcmp(entry.filename, argv[2]) == 0) {
            entry_index = i;
            entry_offset = current_pos;
            break; 
        }
    }

    if (entry_index == -1) {
        fprintf(stderr, "Error: File '%s' not found.\n", argv[2]);
        fclose(fp);
        return 4;
    }

    // Prepare to delete when file found
    uint16_t current_block = entry.starting_block;
    uint32_t file_size = entry.file_size;
    
    printf("Deleting file '%s' (Size: %u, Start Block: %u)...\n", 
           entry.filename, file_size, current_block);

    // Mark free directory entry
    // Set first char of filename to '\0'
    fseek(fp, entry_offset, SEEK_SET); 
    char empty_byte = '\0';
    fwrite(&empty_byte, 1, 1, fp);

    // Update superblock for one more available directory entry
    sb.available_direntries++;

    // Traverse/free Blocks
    long data_start_offset = sizeof(superblock_t) + (sizeof(direntry_t) * 255);

    // Calculate data in one block
    // Need to know if at last block
    uint32_t bytes_remaining = file_size;
    uint32_t data_per_block = sb.bytes_per_block - 3;

    while (bytes_remaining > 0) {
        // Calculate offset of current block
        long block_offset = data_start_offset + (current_block * sb.bytes_per_block);
        
        // Mark block as free
        fseek(fp, block_offset, SEEK_SET);
        uint8_t free_flag = 0;
        fwrite(&free_flag, 1, 1, fp);
        
        // Update superblock count
        sb.available_blocks++;

        // Determine next block
        // Look up next pointer if too much data for block
        if (bytes_remaining > data_per_block) {
            // Pointer at end of the block
            fseek(fp, block_offset + sb.bytes_per_block - 2, SEEK_SET);
            uint16_t next_block_val;
            fread(&next_block_val, sizeof(uint16_t), 1, fp);
            
            current_block = next_block_val;
            bytes_remaining -= data_per_block;
        } else {
            // Last block
            bytes_remaining = 0;
        }
    }

    // Write updated superblock to disk
    fseek(fp, 0, SEEK_SET);
    fwrite(&sb, sizeof(superblock_t), 1, fp);

    printf("File deleted successfully.\n");

    fclose(fp);
    return 0;
}
