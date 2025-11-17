// remove_hamming.c
// Author: bajackson1@quinniac.edu
//
// Read file with Hamming bits
// Remove parity bits
// Write original 8 bits of data to new file.

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
    long codedSize;
    long dataSize;

    // Check for correct arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    // Read input file into struct
    readFile(argv[1], &inputFile);
    codedSize = inputFile.fileSize;
    // Calculate output size
    dataSize = (codedSize / 12) * 8;

    // Allocate space for output file struct
    outputFile.fileSize = dataSize;
    outputFile.filePath = (char *)malloc(strlen(argv[2]) + 1);

    if (outputFile.filePath == NULL) {
        perror("malloc failed for output filePath");
        exit(1);
    }

    strcpy(outputFile.filePath, argv[2]);

    // Allocate space for output file contents
    outputFile.fileContents = (char *)malloc(dataSize + 1);
    if (outputFile.fileContents == NULL) {
        perror("malloc failed for output fileContents");
        exit(1);
    }

    // Loop through input data 12 bits at a time
    for (long i = 0, j = 0; i < codedSize; i += 12, j += 8) {
        // Get 8 data bits
        outputFile.fileContents[j + 0] = inputFile.fileContents[i + 2];  // Pos 3
        outputFile.fileContents[j + 1] = inputFile.fileContents[i + 4];  // Pos 5
        outputFile.fileContents[j + 2] = inputFile.fileContents[i + 5];  // Pos 6
        outputFile.fileContents[j + 3] = inputFile.fileContents[i + 6];  // Pos 7
        outputFile.fileContents[j + 4] = inputFile.fileContents[i + 8];  // Pos 9
        outputFile.fileContents[j + 5] = inputFile.fileContents[i + 9];  // Pos 10
        outputFile.fileContents[j + 6] = inputFile.fileContents[i + 10]; // Pos 11
        outputFile.fileContents[j + 7] = inputFile.fileContents[i + 11]; // Pos 12
    }

    // Add null terminator
    outputFile.fileContents[dataSize] = '\0';

    // Write new coded data to output file
    writeFile(argv[2], &outputFile);

    // Free allocated memory
    freeFileInfo(&inputFile);
    freeFileInfo(&outputFile);

    return 0;
}
