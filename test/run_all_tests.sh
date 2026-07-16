#!/bin/bash

# Script to run all test_dynamic_string tests
# Records pass/fail for each test and reports the final result

# Do not use set -e (continue even when a test fails)

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# Change to build directory
cd "${BUILD_DIR}"

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Arrays to record test results
declare -a TEST_NAMES
declare -a TEST_RESULTS
declare -a TEST_EXIT_CODES
declare -a TEST_DURATIONS

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Format duration (convert seconds to a readable form)
format_duration() {
    local seconds=$1
    local hours=$((seconds / 3600))
    local minutes=$(((seconds % 3600) / 60))
    local secs=$((seconds % 60))
    
    if [ $hours -gt 0 ]; then
        printf "%dh %dm %ds" $hours $minutes $secs
    elif [ $minutes -gt 0 ]; then
        printf "%dm %ds" $minutes $secs
    else
        printf "%ds" $secs
    fi
}

# Run a test
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    TEST_NAMES+=("${test_name}")
    
    echo ""
    echo "========================================"
    echo "Running: ${test_name}"
    echo "Command: ${test_command}"
    echo "========================================"
    
    # Record start time
    local start_time=$(date +%s)
    
    # Run test
    if eval "${test_command}"; then
        local exit_code=0
        TEST_RESULTS+=("PASS")
        TEST_EXIT_CODES+=(0)
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        local exit_code=$?
        TEST_RESULTS+=("FAIL")
        TEST_EXIT_CODES+=(${exit_code})
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    # Record end time and compute duration
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    TEST_DURATIONS+=(${duration})
    
    # Show result (including duration)
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✓ ${test_name} passed${NC} (duration: $(format_duration ${duration}))"
    else
        echo -e "${RED}✗ ${test_name} failed (exit code: ${exit_code}, duration: $(format_duration ${duration}))${NC}"
    fi
}

# Main
echo "========================================"
echo "test_dynamic_string: run all tests"
echo "========================================"
echo "Start time: $(date)"
echo ""

# Run each test
# Execute in the user-specified command format

run_test "test_dynamic_string_constructor (RR-STANDARD)" "./test_dynamic_string_constructor -r true -s -1 -m standard"
run_test "test_dynamic_string_constructor (RR-FAST)    " "./test_dynamic_string_constructor -r true -s -1 -m fast"
run_test "test_dynamic_string_constructor (SE-STANDARD)" "./test_dynamic_string_constructor -r false -s -1 -m standard"
run_test "test_dynamic_string_constructor (SE-FAST)    " "./test_dynamic_string_constructor -r false -s -1 -m fast"

run_test "test_dynamic_string_constructor_int64 (RR-STANDARD)" "./test_dynamic_string_constructor_int64 -r true -s -1 -m standard"
run_test "test_dynamic_string_constructor_int64 (RR-FAST)    " "./test_dynamic_string_constructor_int64 -r true -s -1 -m fast"
run_test "test_dynamic_string_constructor_int64 (SE-STANDARD)" "./test_dynamic_string_constructor_int64 -r false -s -1 -m standard"
run_test "test_dynamic_string_constructor_int64 (SE-FAST)    " "./test_dynamic_string_constructor_int64 -r false -s -1 -m fast"

run_test "test_dynamic_string_access (RR-STANDARD)" "./test_dynamic_string_access -r true -s -1 -m standard"
run_test "test_dynamic_string_access (RR-FAST)    " "./test_dynamic_string_access -r true -s -1 -m fast"
run_test "test_dynamic_string_access (SE-STANDARD)" "./test_dynamic_string_access -r false -s -1 -m standard"
run_test "test_dynamic_string_access (SE-FAST)    " "./test_dynamic_string_access -r false -s -1 -m fast"

run_test "test_dynamic_string_access_int64 (RR-STANDARD)" "./test_dynamic_string_access_int64 -r true -s -1 -m standard"
run_test "test_dynamic_string_access_int64 (RR-FAST)    " "./test_dynamic_string_access_int64 -r true -s -1 -m fast"
run_test "test_dynamic_string_access_int64 (SE-STANDARD)" "./test_dynamic_string_access_int64 -r false -s -1 -m standard"
run_test "test_dynamic_string_access_int64 (SE-FAST)    " "./test_dynamic_string_access_int64 -r false -s -1 -m fast"

run_test "test_dynamic_string_lce (RR-STANDARD)" "./test_dynamic_string_lce -r true -s -1 -m standard"
run_test "test_dynamic_string_lce (RR-FAST)    " "./test_dynamic_string_lce -r true -s -1 -m fast"
run_test "test_dynamic_string_lce (SE-STANDARD)" "./test_dynamic_string_lce -r false -s -1 -m standard"
run_test "test_dynamic_string_lce (SE-FAST)    " "./test_dynamic_string_lce -r false -s -1 -m fast"

run_test "test_dynamic_string_reverse_lce (RR-STANDARD)" "./test_dynamic_string_reverse_lce -r true -s -1 -m standard"
run_test "test_dynamic_string_reverse_lce (RR-FAST)    " "./test_dynamic_string_reverse_lce -r true -s -1 -m fast"
run_test "test_dynamic_string_reverse_lce (SE-STANDARD)" "./test_dynamic_string_reverse_lce -r false -s -1 -m standard"
run_test "test_dynamic_string_reverse_lce (SE-FAST)    " "./test_dynamic_string_reverse_lce -r false -s -1 -m fast"

run_test "test_dynamic_string_get_all_occurrences (RR-STANDARD)" "./test_dynamic_string_get_all_occurrences -r true -s -1 -m standard"
run_test "test_dynamic_string_get_all_occurrences (RR-FAST)    " "./test_dynamic_string_get_all_occurrences -r true -s -1 -m fast"
run_test "test_dynamic_string_get_all_occurrences (SE-STANDARD)" "./test_dynamic_string_get_all_occurrences -r false -s -1 -m standard"
run_test "test_dynamic_string_get_all_occurrences (SE-FAST)    " "./test_dynamic_string_get_all_occurrences -r false -s -1 -m fast"

run_test "test_dynamic_string_lcp (RR-STANDARD)" "./test_dynamic_string_lcp -r true -s -1 -m standard"
run_test "test_dynamic_string_lcp (RR-FAST)    " "./test_dynamic_string_lcp -r true -s -1 -m fast"
run_test "test_dynamic_string_lcp (SE-STANDARD)" "./test_dynamic_string_lcp -r false -s -1 -m standard"
run_test "test_dynamic_string_lcp (SE-FAST)    " "./test_dynamic_string_lcp -r false -s -1 -m fast"

run_test "test_dynamic_string_lcs (RR-STANDARD)" "./test_dynamic_string_lcs -r true -s -1 -m standard"
run_test "test_dynamic_string_lcs (RR-FAST)    " "./test_dynamic_string_lcs -r true -s -1 -m fast"
run_test "test_dynamic_string_lcs (SE-STANDARD)" "./test_dynamic_string_lcs -r false -s -1 -m standard"
run_test "test_dynamic_string_lcs (SE-FAST)    " "./test_dynamic_string_lcs -r false -s -1 -m fast"


run_test "test_dynamic_string_insert (RR-STANDARD)" "./test_dynamic_string_insert -r true -s -1 -m standard"
run_test "test_dynamic_string_insert (RR-FAST)    " "./test_dynamic_string_insert -r true -s -1 -m fast"
run_test "test_dynamic_string_insert (SE-STANDARD)" "./test_dynamic_string_insert -r false -s -1 -m standard"
run_test "test_dynamic_string_insert (SE-FAST)    " "./test_dynamic_string_insert -r false -s -1 -m fast"

run_test "test_dynamic_string_insert_int64 (RR-STANDARD)" "./test_dynamic_string_insert_int64 -r true -s -1 -m standard"
run_test "test_dynamic_string_insert_int64 (RR-FAST)    " "./test_dynamic_string_insert_int64 -r true -s -1 -m fast"
run_test "test_dynamic_string_insert_int64 (SE-STANDARD)" "./test_dynamic_string_insert_int64 -r false -s -1 -m standard"
run_test "test_dynamic_string_insert_int64 (SE-FAST)    " "./test_dynamic_string_insert_int64 -r false -s -1 -m fast"

run_test "test_dynamic_string_delete (RR-STANDARD)" "./test_dynamic_string_delete -r true -s -1 -m standard"
run_test "test_dynamic_string_delete (RR-FAST)    " "./test_dynamic_string_delete -r true -s -1 -m fast"
run_test "test_dynamic_string_delete (SE-STANDARD)" "./test_dynamic_string_delete -r false -s -1 -m standard"
run_test "test_dynamic_string_delete (SE-FAST)    " "./test_dynamic_string_delete -r false -s -1 -m fast"

run_test "test_dynamic_string_delete_int64 (RR-STANDARD)" "./test_dynamic_string_delete_int64 -r true -s -1 -m standard"
run_test "test_dynamic_string_delete_int64 (RR-FAST)    " "./test_dynamic_string_delete_int64 -r true -s -1 -m fast"
run_test "test_dynamic_string_delete_int64 (SE-STANDARD)" "./test_dynamic_string_delete_int64 -r false -s -1 -m standard"
run_test "test_dynamic_string_delete_int64 (SE-FAST)    " "./test_dynamic_string_delete_int64 -r false -s -1 -m fast"

run_test "test_dynamic_string_random_ops (RR-STANDARD)" "./test_dynamic_string_random_ops -r true -s -1 -m standard -t 100 -o 50 -l 5000 -L 10000 -p 100"
run_test "test_dynamic_string_random_ops (RR-FAST)    " "./test_dynamic_string_random_ops -r true -s -1 -m fast -t 100 -o 50 -l 5000 -L 10000 -p 100"
run_test "test_dynamic_string_random_ops (SE-STANDARD)" "./test_dynamic_string_random_ops -r false -s -1 -m standard -t 100 -o 50 -l 5000 -L 10000 -p 100"
run_test "test_dynamic_string_random_ops (SE-FAST)    " "./test_dynamic_string_random_ops -r false -s -1 -m fast -t 100 -o 50 -l 5000 -L 10000 -p 100"

run_test "test_dynamic_string_file_io (RR-STANDARD)" "./test_dynamic_string_file_io -r true -s -1 -m standard"
run_test "test_dynamic_string_file_io (RR-FAST)    " "./test_dynamic_string_file_io -r true -s -1 -m fast"
run_test "test_dynamic_string_file_io (SE-STANDARD)" "./test_dynamic_string_file_io -r false -s -1 -m standard"
run_test "test_dynamic_string_file_io (SE-FAST)    " "./test_dynamic_string_file_io -r false -s -1 -m fast"

# Show result summary
echo ""
echo "========================================"
echo "Test result summary"
echo "========================================"
echo "End time: $(date)"
echo ""

for i in "${!TEST_NAMES[@]}"; do
    if [ "${TEST_RESULTS[$i]}" = "PASS" ]; then
        echo -e "${GREEN}✓${NC} ${TEST_NAMES[$i]}: ${TEST_RESULTS[$i]} (duration: $(format_duration ${TEST_DURATIONS[$i]}))"
    else
        echo -e "${RED}✗${NC} ${TEST_NAMES[$i]}: ${TEST_RESULTS[$i]} (exit code: ${TEST_EXIT_CODES[$i]}, duration: $(format_duration ${TEST_DURATIONS[$i]}))"
    fi
done

echo ""
echo "========================================"
echo "Total: ${TOTAL_TESTS} tests"
echo -e "Passed: ${GREEN}${PASSED_TESTS}${NC}"
echo -e "Failed: ${RED}${FAILED_TESTS}${NC}"
echo "========================================"

# Return exit code 1 if any test failed
if [ ${FAILED_TESTS} -gt 0 ]; then
    echo ""
    echo -e "${RED}Error: ${FAILED_TESTS} test(s) failed.${NC}"
    exit 1
else
    echo ""
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi
