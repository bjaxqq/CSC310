# Assignment 4

## Objectives

At the end of this assignment, you should be able

- To write C programs that use command line arguments to enter data
- To read and write text files in C
- To test a program for memory leaks
- To add Hamming code bits to an uncoded "binary" file stored as a sequence of $[0|1]$ text characters
- To remove Hamming code bits from a coded "binary" file
- To check a "binary" file coded with Hamming code bits for errors and correct them

## Background

Chapter 9 of the textbook describes the Hamming code for error detection and correction. This code adds extra bits to a data stream to allow for the detection and correction of single-bit errors. The Hamming code works by adding redundant bits at specific positions in this stream. These bits are placed at positions that are powers of 2 (1, 2, 4, 8, etc.). The value of each redundant bit is determined from a specific set of bits in the data stream. The value of each redundant bit is calculated based on the parity (even or odd) of the bits it checks. Errors are determined when the Hamming bits in an encoded file or stream do not match those calculated when processing the file. Note: We will be assuming odd parity for this assignment.

## Assignment

In this assignment you will be considering "binary" files that contain only the characters '0' and '1'. These files will be treated as a sequence of bits. Your assignment is to write three programs:

**add_hamming.c** Your first program will read in a file, add Hamming bits to it, and write the result to a new file.  
**remove_hamming.c** Your second program will read in a file with Hamming bits, remove the Hamming bits, and write the original data to a new file.  
**check_hamming.c** Your third program will read in a file with Hamming bits, check for errors, correct any single-bit errors found, and write the correct data to a new file.  

All three programs should read input from the user as command line arguments (no file redirection). For example to add Hamming bits, your program would be run as follows:

```bash
./add_hamming input.txt output.txt
```

To remove Hamming bits, your program would be run as follows:

```bash
./remove_hamming input.txt output.txt
```

To check and correct Hamming bits, your program would be run as follows:

```bash
./check_hamming input.txt
```

In the case of errors in `check_hamming`, the user should be prompted for the name of the output file to write the corrected data to.  

For consistency, you should consider using a struct to represent your files in your programs. Here is a sample struct definition you could use for text files:

```c
// Sample "binary" file descriptor
typedef struct fileInfoText {
    char *filePath;
    long fileSize;
    char *fileContents;
} fileInfo_t;
```

## Submission

Please pass in a zipped folder containing at a minimum the following four files for this assignment:

- `add_hamming.c`
- `remove_hamming.c`
- `check_hamming.c`
- `Makefile`

The folder should be named with your QU login shortname (ie jqpublic).  

A makefile will be provided that supports the following targets:

- **`all`** - compiles all three programs
- **`add_hamming`** - compiles only `add_hamming.c`
- **`remove_hamming`** - compiles only `remove_hamming.c`
- **`check_hamming`** - compiles only `check_hamming.c`
- **`clean`** - removes all compiled files

You can modify the makefile as needed but it must support these targets and leave the executable names the same.

## Testing

Test files will be provided in the starter code for you to test your programs. You should also create your own test files to ensure that your programs will work correctly in all cases.

## Example Runs

Neither `add_hamming` nor `remove_hamming` will produce any output to the terminal. Here are some example runs for `check_hamming` (use these examples to determine the required output format):

```bash
~ ❯ ./check_hamming fileWithError.txt correctedFile.txt
Error detected at position 442
Corrected file written to correctedFile.txt

~ ❯ ./check_hamming coded_no_error.txt correctedFile.txt
No errors detected
```

## Notes

- None of the input files (or files you generate) will have trailing whitespaces such as newlines or EOF characters after the last entry.
- Sample files for testing are included with the starter code.
- You may assume that all input files are well-formed (i.e., they only contain ''0' and '1' characters).
- You may assume that the input files are not empty
- You may assume that the input files for `add_hamming` will have a length that is a multiple of 8.
- Any files with an error will have at most 1 data bit wrong.
- You should test your program with valgrind to ensure that there are no memory leaks.
- You should use a consisdent coding style, and include comments in your code to explain your logic.
- Your should comment your code appropriately to explain your logic. Include a header comment at the top of each file with your name, the date, and a brief description of the file's purpose.

## Grading Rubric

Here is a simple grading rubric to help guide your work.  

**Compile and Run** 30%  
**Style and Comments** 5%  
**No Memory Leaks** 5%  
**Add Hamming Code** 30%  
**Check Hamming Code** 15%  
**Fix Hamming Code** 15%  