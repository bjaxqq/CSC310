#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "qfs.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <disk image file> <file to add>\n", argv[0]);
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

    // Open source file to read data
    FILE *src_fp = fopen(argv[2], "rb");
    
    if (!src_fp) {
        perror("fopen source file");
        fclose(fp);
        return 3;
    }

    // Read superblock to get filesystem details
    superblock_t sb;

    if (fread(&sb, sizeof(superblock_t), 1, fp) != 1) {
        fprintf(stderr, "Error: Failed to read superblock.\n");
        fclose(src_fp);
        fclose(fp);
        return 4;
    }

    // Determine size of source file
    fseek(src_fp, 0, SEEK_END);
    long src_file_size = ftell(src_fp);
    fseek(src_fp, 0, SEEK_SET);

    if (src_file_size == 0) {
        fprintf(stderr, "Error: Source file is empty.\n");
        fclose(src_fp);
        fclose(fp);
        return 5;
    }

    // Check for enough space
    uint32_t data_per_block = sb.bytes_per_block - 3;
    uint32_t blocks_needed = (src_file_size + data_per_block - 1) / data_per_block;

    if (sb.available_direntries == 0) {
        fprintf(stderr, "Error: No directory entries available.\n");
        fclose(src_fp);
        fclose(fp);
        return 6;
    }

    if (sb.available_blocks < blocks_needed) {
        fprintf(stderr, "Error: Not enough free blocks. Needed: %u, Available: %u\n", 
                blocks_needed, sb.available_blocks);
        fclose(src_fp);
        fclose(fp);
        return 7;
    }

    // Find free directory entry
    int dir_index = -1;
    long dir_entry_offset = 0;
    direntry_t entry;
    
    // Seek to start of directory entries
    fseek(fp, sizeof(superblock_t), SEEK_SET);
    
    for (int i = 0; i < sb.total_direntries; i++) {
        long pos = ftell(fp);
        
        if (fread(&entry, sizeof(direntry_t), 1, fp) != 1) break;
        
        // Free entry identified by empty filename
        if (entry.filename[0] == '\0') {
            dir_index = i;
            dir_entry_offset = pos;
            break;
        }
    }

    // Error handling for no free directory slot
    if (dir_index == -1) {
        fprintf(stderr, "Error: Could not locate free directory entry slot.\n");
        fclose(src_fp);
        fclose(fp);
        return 8;
    }

    // Writing data blocks
    // Track previous block to update next_block pointer
    int prev_block_idx = -1;
    int first_block_idx = -1;
    long data_start_offset = sizeof(superblock_t) + (sizeof(direntry_t) * 255);
    
    // Max block size buffer
    uint8_t buffer[2048];
    long bytes_remaining = src_file_size;
    // Searching where we left off
    int search_start_idx = 0;

    printf("Writing '%s' (%ld bytes) requiring %u blocks...\n", argv[2], src_file_size, blocks_needed);

    while (bytes_remaining > 0) {
        int curr_block_idx = -1;
        long curr_block_offset = 0;

        // Find free block
        // Seek to start of block area to search
        fseek(fp, data_start_offset + (search_start_idx * sb.bytes_per_block), SEEK_SET);

        for (int i = search_start_idx; i < sb.total_blocks; i++) {
            uint8_t busy;

            if (fread(&busy, 1, 1, fp) != 1) break;

            // Free block
            if (busy == 0) {
                curr_block_idx = i;
                curr_block_offset = data_start_offset + (i * sb.bytes_per_block);
                // Start next search after this one
                search_start_idx = i + 1;
                break;
            }

            // Skip to next block busy byte
            fseek(fp, sb.bytes_per_block - 1, SEEK_CUR);
        }

        if (curr_block_idx == -1) {
            fprintf(stderr, "Error: Unexpectedly ran out of blocks during write.\n");
            break;
        }

        // Mark block as busy
        fseek(fp, curr_block_offset, SEEK_SET);
        uint8_t busy_val = 1;
        fwrite(&busy_val, 1, 1, fp);

        // Link previous block to this one
        if (prev_block_idx != -1) {
            // Pointer at end of previous block
            long prev_ptr_offset = data_start_offset + (prev_block_idx * sb.bytes_per_block) + sb.bytes_per_block - 2;
            fseek(fp, prev_ptr_offset, SEEK_SET);
            uint16_t next_val = (uint16_t)curr_block_idx;
            fwrite(&next_val, sizeof(uint16_t), 1, fp);
        } else {
            // First block
            first_block_idx = curr_block_idx;
        }

        // Write data
        uint32_t chunk_size = (bytes_remaining > data_per_block) ? data_per_block : bytes_remaining;
        
        // Clear buffer
        memset(buffer, 0, sb.bytes_per_block);
        // Read from source file
        fread(buffer, 1, chunk_size, src_fp);
        
        // Write data to disk
        fseek(fp, curr_block_offset + 1, SEEK_SET);
        fwrite(buffer, 1, chunk_size, fp);

        // Update state
        bytes_remaining -= chunk_size;
        prev_block_idx = curr_block_idx;
        sb.available_blocks--;
    }

    // Write directory entry
    memset(&entry, 0, sizeof(direntry_t));
    // Copy filename
    strncpy(entry.filename, argv[2], 22);
    // Ensure null termination
    entry.filename[22] = '\0';
    entry.file_size = (uint32_t)src_file_size;
    entry.starting_block = (uint16_t)first_block_idx;
    // Default permissions
    entry.permissions = 0;
    
    fseek(fp, dir_entry_offset, SEEK_SET);
    fwrite(&entry, sizeof(direntry_t), 1, fp);

    // Update superblock stats
    sb.available_direntries--;
    fseek(fp, 0, SEEK_SET);
    fwrite(&sb, sizeof(superblock_t), 1, fp);

    printf("File written successfully.\n");

    fclose(src_fp);

    fclose(fp);
    return 0;
}