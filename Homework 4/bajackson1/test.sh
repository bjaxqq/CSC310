#!/bin/bash

set -e

UNCODED_DIR="tests/uncoded"
CODED_DIR="tests/coded"
ERROR_DIR="tests/error"

TEMP_DIR="temp_output"

GREEN="\033[0;32m"
RED="\033[0;31m"
CYAN="\033[0;36m"
NC="\033[0m"

pass() {
    echo -e "${GREEN}PASS:${NC} $1"
}

fail() {
    echo -e "${RED}FAIL:${NC} $1"
    if [ -n "$2" ] && [ -n "$3" ]; then
        echo "--- DIFF START ---"
        diff "$2" "$3"
        echo "--- DIFF END ---"
    fi
    exit 1
}

check_diff() {
    if ! diff -q "$1" "$2" >/dev/null 2>&1; then
        fail "$3" "$1" "$2"
    else
        pass "$3"
    fi
}

cleanup() {
    echo -e "\n${CYAN}Cleaning up...${NC}"
    rm -rf "$TEMP_DIR"
    echo "Done."
}

trap cleanup EXIT

echo -e "${CYAN}Starting Hamming Code Test Script...${NC}"

echo -e "\n${CYAN}### 1. Compiling Programs ###${NC}"
make clean > /dev/null
make all > /dev/null

if [ ! -f add_hamming ] || [ ! -f remove_hamming ] || [ ! -f check_hamming ]; then
    fail "Compilation failed. Executables not found."
    make all
    exit 1
else
    pass "Compilation successful"
fi

mkdir -p "$TEMP_DIR"

echo -e "\n${CYAN}### 2. Testing add_hamming ###${NC}"

for i in {1..4}; do
    UNC_FILE="$UNCODED_DIR/uncoded${i}.txt"
    COD_FILE="$CODED_DIR/coded${i}.txt"
    OUT_FILE="$TEMP_DIR/temp_coded${i}.txt"
    
    ./add_hamming "$UNC_FILE" "$OUT_FILE"
    check_diff "$COD_FILE" "$OUT_FILE" "add_hamming (uncoded${i}.txt -> coded${i}.txt)"
done

echo -e "\n${CYAN}### 3. Testing remove_hamming ###${NC}"

for i in {1..4}; do
    UNC_FILE="$UNCODED_DIR/uncoded${i}.txt"
    COD_FILE="$CODED_DIR/coded${i}.txt"
    OUT_FILE="$TEMP_DIR/temp_uncoded${i}.txt"
    
    ./remove_hamming "$COD_FILE" "$OUT_FILE"
    check_diff "$UNC_FILE" "$OUT_FILE" "remove_hamming (coded${i}.txt -> uncoded${i}.txt)"
done

echo -e "\n${CYAN}### 4. Testing check_hamming (No Errors) ###${NC}"

for i in {1..4}; do
    COD_FILE="$CODED_DIR/coded${i}.txt"
    OUT_FILE="$TEMP_DIR/temp_checked${i}.txt"
    
    OUTPUT=$(./check_hamming "$COD_FILE" "$OUT_FILE")
    EXPECTED_OUTPUT="No errors detected"
    
    if [ "$OUTPUT" != "$EXPECTED_OUTPUT" ]; then
        fail "check_hamming (no error) output for coded${i}.txt was incorrect. Expected: '$EXPECTED_OUTPUT', Got: '$OUTPUT'"
    else
        pass "check_hamming (no error) output for coded${i}.txt"
    fi
    
    check_diff "$COD_FILE" "$OUT_FILE" "check_hamming (no error) file content for coded${i}.txt"
done

echo -e "\n${CYAN}### 5. Testing check_hamming (Error Correction) ###${NC}"

for i in {1..4}; do
    GOOD_FILE="$CODED_DIR/coded${i}.txt"
    BAD_FILE="$ERROR_DIR/coded${i}_bad.txt"
    OUT_FILE="$TEMP_DIR/temp_corrected${i}.txt"
    
    OUTPUT=$(./check_hamming "$BAD_FILE" "$OUT_FILE")
    
    if ! echo "$OUTPUT" | grep -q "Error detected at position"; then
        fail "check_hamming (error) output for coded${i}_bad.txt did not report an error."
    fi
    
    if ! echo "$OUTPUT" | grep -q "Corrected file written to $OUT_FILE"; then
        fail "check_hamming (error) output for coded${i}_bad.txt did not report writing to file."
    fi
    
    pass "check_hamming (error) output format for coded${i}_bad.txt"
    
    check_diff "$GOOD_FILE" "$OUT_FILE" "check_hamming (error) file correction for coded${i}_bad.txt"
done

echo -e "\n${CYAN}### 6. Testing for Memory Leaks (Valgrind) ###${NC}"

if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}SKIPPING:${NC} valgrind not found. Please install valgrind to check for memory leaks."
else
    valgrind --leak-check=full --error-exitcode=1 ./add_hamming "$UNCODED_DIR/uncoded1.txt" "$TEMP_DIR/valgrind_add.txt" > /dev/null 2>&1
    pass "add_hamming (valgrind)"
    
    valgrind --leak-check=full --error-exitcode=1 ./remove_hamming "$CODED_DIR/coded1.txt" "$TEMP_DIR/valgrind_remove.txt" > /dev/null 2>&1
    pass "remove_hamming (valgrind)"
    
    valgrind --leak-check=full --error-exitcode=1 ./check_hamming "$CODED_DIR/coded1.txt" "$TEMP_DIR/valgrind_check_good.txt" > /dev/null 2>&1
    pass "check_hamming (no error valgrind)"
    
    valgrind --leak-check=full --error-exitcode=1 ./check_hamming "$ERROR_DIR/coded1_bad.txt" "$TEMP_DIR/valgrind_check_bad.txt" > /dev/null 2>&1
    pass "check_hamming (error correction valgrind)"
fi


echo -e "\n${GREEN}========== ALL TESTS PASSED ==========${NC}"
