// add_hamming.c
// Author: bajackson1@quinniac.edu
//
// Reads file of 0/1 chars that are multiple of 8
// Adds odd-paritied Hamming bits
// Writes result to new file

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
char toChar(int bit);

// Convert 0/1 char to int
int getBit(char c) {
    return (c == '1');
}

// Convert int to 0/1 char
char toChar(int bit) {
    return (bit ? '1' : '0');
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

// Main function
int main(int argc, char *argv[]) {
    fileInfo_t inputFile;
    fileInfo_t outputFile;
    long dataSize;
    long codedSize;

    // Check for correct arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    // Read input file into struct
    readFile(argv[1], &inputFile);
    dataSize = inputFile.fileSize;
    // Calculate output size
    codedSize = (dataSize / 8) * 12;

    // Allocate space for output file struct
    outputFile.fileSize = codedSize;
    outputFile.filePath = (char *)malloc(strlen(argv[2]) + 1);

    if (outputFile.filePath == NULL) {
        perror("malloc failed for output filePath");
        exit(1);
    }

    strcpy(outputFile.filePath, argv[2]);

    // Allocate space for output file contents
    outputFile.fileContents = (char *)malloc(codedSize + 1);

    if (outputFile.fileContents == NULL) {
        perror("malloc failed for output fileContents");
        exit(1);
    }

    // Loop through input data 8 bits at a time
    for (long i = 0, j = 0; i < dataSize; i += 8, j += 12) {
        // 8 data bit array
        int d[8];
         // Parity bits
        int p1, p2, p4, p8;

        // Get 8 data bits from input
        for (int k = 0; k < 8; k++) {
            d[k] = getBit(inputFile.fileContents[i + k]);
        }

        // Calculate odd parity bits
        // 1 if data has even 1s, 0 if data has odd 1s
        // p1 -> 3, 5, 7, 9, 11 (1, 2, 4, 5, 7)
        p1 = !(d[0] ^ d[1] ^ d[3] ^ d[4] ^ d[6]);
        // p2 -> 3, 6, 7, 10, 11 (1, 3, 4, 6, 7)
        p2 = !(d[0] ^ d[2] ^ d[3] ^ d[5] ^ d[6]);
        // p4 -> 5, 6, 7, 12 (2, 3, 4, 8)
        p4 = !(d[1] ^ d[2] ^ d[3] ^ d[7]);
        // p8 -> 9, 10, 11, 12 (5, 6, 7, 8)
        p8 = !(d[4] ^ d[5] ^ d[6] ^ d[7]);

        // Place bits into output buffer in correct order
        outputFile.fileContents[j + 0] = toChar(p1); // Pos 1
        outputFile.fileContents[j + 1] = toChar(p2); // Pos 2
        outputFile.fileContents[j + 2] = toChar(d[0]); // Pos 3
        outputFile.fileContents[j + 3] = toChar(p4); // Pos 4
        outputFile.fileContents[j + 4] = toChar(d[1]); // Pos 5
        outputFile.fileContents[j + 5] = toChar(d[2]); // Pos 6
        outputFile.fileContents[j + 6] = toChar(d[3]); // Pos 7
        outputFile.fileContents[j + 7] = toChar(p8); // Pos 8
        outputFile.fileContents[j + 8] = toChar(d[4]); // Pos 9
        outputFile.fileContents[j + 9] = toChar(d[5]); // Pos 10
        outputFile.fileContents[j + 10] = toChar(d[6]); // Pos 11
        outputFile.fileContents[j + 11] = toChar(d[7]); // Pos 12
    }

    // Add null terminator
    outputFile.fileContents[codedSize] = '\0';

    // Write new coded data to output file
    writeFile(argv[2], &outputFile);

    // Free allocated memory
    freeFileInfo(&inputFile);
    freeFileInfo(&outputFile);

    return 0;
}
