#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "--- Compiling program ---"
make clean && make

if [ $? -ne 0 ]; then
    echo -e "${RED}FAIL: Program did not compile.${NC}"
    exit 1
fi

echo -e "${GREEN}SUCCESS: Program compiled.${NC}\n"

TESTS=(
    "best_fm_b"
    "best_m_g"
    "best_rfm_g"
    "worst_fm_g"
    "worst_m_g"
    "worst_rfm_b"
)

echo "--- Running Correctness Tests (diff) ---"

PASSED_COUNT=0
FAILED_COUNT=0

for test in "${TESTS[@]}"; do
    INPUT_FILE="tests/${test}.txt"
    EXPECTED_FILE="tests/${test}_out.txt"
    MY_OUTPUT_FILE="tests/${test}_my_out.txt"

    ./assign3 "$INPUT_FILE" > "$MY_OUTPUT_FILE"

    diff -w "$MY_OUTPUT_FILE" "$EXPECTED_FILE" > /dev/null

    if [ $? -eq 0 ]; then
        echo -e "[ ${GREEN}PASS${NC} ] $test"
        ((PASSED_COUNT++))
    else
        echo -e "[ ${RED}FAIL${NC} ] $test (Outputs do not match)"
        ((FAILED_COUNT++))
    fi

    rm "$MY_OUTPUT_FILE"
done

echo "----------------------------------------"
echo -e "Summary: ${GREEN}$PASSED_COUNT Passed${NC}, ${RED}$FAILED_COUNT Failed${NC}\n"


echo "--- Running Memory Leak Test (valgrind) ---"
echo "Running valgrind on 'best_m_g.txt'..."

valgrind --leak-check=full ./assign3 "tests/best_m_g.txt" > /dev/null

echo "----------------------------------------"
echo "Check the valgrind output above for 'All heap blocks were freed'."
echo "Look for 'in use at exit: 0 bytes in 0 blocks'."
echo ""
