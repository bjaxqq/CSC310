// check_hamming.c
// Author: bajackson1@quinniac.edu
//
// Read file with Hamming bits
// Check for single-bit errors with odd parity
// Correct errors and write correct data to new file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Sample "binary" file descriptor
typedef struct fileInfoText {
    char *filePath;
    long fileSize;
    char *fileContents;
} fileInfo_t;

// Function prototypes
void readFile(const char *filePath, fileInfo_t *fileInfo);
void writeFile(const char *filePath, fileInfo_t *fileInfo);
void freeFileInfo(fileInfo_t *fileInfo);
int getBit(char c);

// Convert 0/1 char to int
int getBit(char c) {
    return (c == '1');
}

// Main function
int main(int argc, char *argv[]) {
    fileInfo_t inputFile;
    fileInfo_t outputFile;
    long codedSize;
    char *correctedContents;
    int errorFound = 0;
    long firstErrorPos = -1;

    // Check for correct arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    // Read input file into struct
    readFile(argv[1], &inputFile);
    codedSize = inputFile.fileSize;
    // Make mutable copy of file contents for correcting
    correctedContents = (char *)malloc(codedSize + 1);

    if (correctedContents == NULL) {
        perror("malloc failed for correctedContents");
        exit(1);
    }

    strcpy(correctedContents, inputFile.fileContents);

    // Loop through coded data 12 bits (chars) at a time
    for (long i = 0; i < codedSize; i += 12) {
        // 12 bit array
        int b[12];
        // Syndrome bits
        int s1, s2, s4, s8;
        int check1, check2, check4, check8;
        int errorBitPos;

        // Get 12 bits from input
        for (int k = 0; k < 12; k++) {
            b[k] = getBit(inputFile.fileContents[i + k]);
        }

        // Check parity
        // p1 -> 1, 3, 5, 7, 9, 11
        check1 = b[0] ^ b[2] ^ b[4] ^ b[6] ^ b[8] ^ b[10];
        // p2 -> 2, 3, 6, 7, 10, 11
        check2 = b[1] ^ b[2] ^ b[5] ^ b[6] ^ b[9] ^ b[10];
        // p4 -> 4, 5, 6, 7, 12
        check4 = b[3] ^ b[4] ^ b[5] ^ b[6] ^ b[11];
        // p8 -> 8, 9, 10, 11, 12
        check8 = b[7] ^ b[8] ^ b[9] ^ b[10] ^ b[11];

        // Check should be 1
        // If 0 then error
        s1 = (check1 == 0) ? 1 : 0;
        s2 = (check2 == 0) ? 1 : 0;
        s4 = (check4 == 0) ? 1 : 0;
        s8 = (check8 == 0) ? 1 : 0;

        // Calculate error position
        errorBitPos = s8 * 8 + s4 * 4 + s2 * 2 + s1 * 1;

        // If error detected
        if (errorBitPos > 0) {
            errorFound = 1;
            // Get 0-indexed position in file
            long globalErrorPos = i + errorBitPos - 1;

            if (firstErrorPos == -1) {
                // Store 1-indexed position for print
                firstErrorPos = globalErrorPos + 1;
            }

            // Correct bit in copied buffer
            correctedContents[globalErrorPos] = (correctedContents[globalErrorPos] == '0') ? '1' : '0';
        }
    }

    // Output file struct
    outputFile.fileSize = codedSize;
    // Use corrected buffer
    outputFile.fileContents = correctedContents;
    outputFile.filePath = (char *)malloc(strlen(argv[2]) + 1);

    if (outputFile.filePath == NULL) {
        perror("malloc failed for output filePath");
        exit(1);
    }

    strcpy(outputFile.filePath, argv[2]);

    // Print status to console
    if (errorFound) {
        printf("Error detected at position %ld\n", firstErrorPos);
        // Write corrected data to output file
        writeFile(argv[2], &outputFile);
        printf("Corrected file written to %s\n", argv[2]);
    } else {
        printf("No errors detected\n");
        // Write original data to output file
        writeFile(argv[2], &outputFile);
    }

    // Free allocated memory
    freeFileInfo(&inputFile);
    // Points to correctedContents
    free(outputFile.filePath);
    // Free buffer
    free(correctedContents);

    return 0;
}

// Free memory for fileInfo struct
void freeFileInfo(fileInfo_t *fileInfo) {
    free(fileInfo->filePath);
    free(fileInfo->fileContents);
}

// Read file contents into fileInfo struct
void readFile(const char *filePath, fileInfo_t *fileInfo) {
    FILE *file;
    long fileSize;

    // Open input file
    file = fopen(filePath, "r");

    if (file == NULL) {
        perror("Error opening input file");
        exit(1);
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for file path
    fileInfo->filePath = (char *)malloc(strlen(filePath) + 1);

    if (fileInfo->filePath == NULL) {
        perror("malloc failed for filePath");
        exit(1);
    }

    strcpy(fileInfo->filePath, filePath);

    // Allocate memory for file contents
    fileInfo->fileContents = (char *)malloc(fileSize + 1);

    if (fileInfo->fileContents == NULL) {
        perror("malloc failed for fileContents");
        exit(1);
    }

    // Read file into contents buffer
    if (fread(fileInfo->fileContents, 1, fileSize, file) != fileSize) {
        fprintf(stderr, "Error reading file\n");
        exit(1);
    }

    // Add null terminator
    fileInfo->fileContents[fileSize] = '\0';
    fileInfo->fileSize = fileSize;

    fclose(file);
}

// Write file contents from struct to new file
void writeFile(const char *filePath, fileInfo_t *fileInfo) {
    FILE *file;

    // Open output file
    file = fopen(filePath, "w");

    if (file == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    // Write contents to file
    if (fwrite(fileInfo->fileContents, 1, fileInfo->fileSize, file) != fileInfo->fileSize) {
        fprintf(stderr, "Error writing to file\n");
        exit(1);
    }

    fclose(file);
}
