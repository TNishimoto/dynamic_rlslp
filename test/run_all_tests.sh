#!/bin/bash

# test_dynamic_stringのすべてのテストを実行するスクリプト
# 各テストの成功/失敗を記録し、最終的な結果を報告します

# set -e は使用しない（テストが失敗しても続行するため）

# スクリプトのディレクトリを取得
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# ビルドディレクトリに移動
cd "${BUILD_DIR}"

# カラーコードの定義
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# テスト結果を記録する配列
declare -a TEST_NAMES
declare -a TEST_RESULTS
declare -a TEST_EXIT_CODES
declare -a TEST_DURATIONS

# テストカウンター
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# 時間をフォーマットする関数（秒を読みやすい形式に変換）
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

# テストを実行する関数
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    TEST_NAMES+=("${test_name}")
    
    echo ""
    echo "========================================"
    echo "実行中: ${test_name}"
    echo "コマンド: ${test_command}"
    echo "========================================"
    
    # 開始時刻を記録
    local start_time=$(date +%s)
    
    # テストを実行
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
    
    # 終了時刻を記録して実行時間を計算
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    TEST_DURATIONS+=(${duration})
    
    # 結果を表示（実行時間を含む）
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✓ ${test_name} 成功${NC} (実行時間: $(format_duration ${duration}))"
    else
        echo -e "${RED}✗ ${test_name} 失敗 (終了コード: ${exit_code}, 実行時間: $(format_duration ${duration}))${NC}"
    fi
}

# メイン処理
echo "========================================"
echo "test_dynamic_string 全テスト実行"
echo "========================================"
echo "開始時刻: $(date)"
echo ""

# 各テストを実行
# ユーザー指定のコマンド形式で実行

run_test "test_dynamic_string_constructor (RR-STANDARD)" "./test_dynamic_string_constructor -r true -s -1 -m standard"
run_test "test_dynamic_string_constructor (RR-FAST)    " "./test_dynamic_string_constructor -r true -s -1 -m fast"
run_test "test_dynamic_string_constructor (SE-STANDARD)" "./test_dynamic_string_constructor -r false -s -1 -m standard"
run_test "test_dynamic_string_constructor (SE-FAST)    " "./test_dynamic_string_constructor -r false -s -1 -m fast"

run_test "test_dynamic_string_access (RR-STANDARD)" "./test_dynamic_string_access -r true -s -1 -m standard"
run_test "test_dynamic_string_access (RR-FAST)    " "./test_dynamic_string_access -r true -s -1 -m fast"
run_test "test_dynamic_string_access (SE-STANDARD)" "./test_dynamic_string_access -r false -s -1 -m standard"
run_test "test_dynamic_string_access (SE-FAST)    " "./test_dynamic_string_access -r false -s -1 -m fast"

run_test "test_dynamic_string_lce (RR-STANDARD)" "./test_dynamic_string_lce -r true -s -1 -m standard"
run_test "test_dynamic_string_lce (RR-FAST)    " "./test_dynamic_string_lce -r true -s -1 -m fast"
run_test "test_dynamic_string_lce (SE-STANDARD)" "./test_dynamic_string_lce -r false -s -1 -m standard"
run_test "test_dynamic_string_lce (SE-FAST)    " "./test_dynamic_string_lce -r false -s -1 -m fast"

run_test "test_dynamic_string_insert (RR-STANDARD)" "./test_dynamic_string_insert -r true -s -1 -m standard"
run_test "test_dynamic_string_insert (RR-FAST)    " "./test_dynamic_string_insert -r true -s -1 -m fast"
run_test "test_dynamic_string_insert (SE-STANDARD)" "./test_dynamic_string_insert -r false -s -1 -m standard"
run_test "test_dynamic_string_insert (SE-FAST)    " "./test_dynamic_string_insert -r false -s -1 -m fast"

run_test "test_dynamic_string_delete (RR-STANDARD)" "./test_dynamic_string_delete -r true -s -1 -m standard"
run_test "test_dynamic_string_delete (RR-FAST)    " "./test_dynamic_string_delete -r true -s -1 -m fast"
run_test "test_dynamic_string_delete (SE-STANDARD)" "./test_dynamic_string_delete -r false -s -1 -m standard"
run_test "test_dynamic_string_delete (SE-FAST)    " "./test_dynamic_string_delete -r false -s -1 -m fast"

run_test "test_dynamic_string_file_io (RR-STANDARD)" "./test_dynamic_string_file_io -r true -s -1 -m standard"
run_test "test_dynamic_string_file_io (RR-FAST)    " "./test_dynamic_string_file_io -r true -s -1 -m fast"
run_test "test_dynamic_string_file_io (SE-STANDARD)" "./test_dynamic_string_file_io -r false -s -1 -m standard"
run_test "test_dynamic_string_file_io (SE-FAST)    " "./test_dynamic_string_file_io -r false -s -1 -m fast"

# 結果サマリーを表示
echo ""
echo "========================================"
echo "テスト結果サマリー"
echo "========================================"
echo "終了時刻: $(date)"
echo ""

for i in "${!TEST_NAMES[@]}"; do
    if [ "${TEST_RESULTS[$i]}" = "PASS" ]; then
        echo -e "${GREEN}✓${NC} ${TEST_NAMES[$i]}: ${TEST_RESULTS[$i]} (実行時間: $(format_duration ${TEST_DURATIONS[$i]}))"
    else
        echo -e "${RED}✗${NC} ${TEST_NAMES[$i]}: ${TEST_RESULTS[$i]} (終了コード: ${TEST_EXIT_CODES[$i]}, 実行時間: $(format_duration ${TEST_DURATIONS[$i]}))"
    fi
done

echo ""
echo "========================================"
echo "合計: ${TOTAL_TESTS} テスト"
echo -e "成功: ${GREEN}${PASSED_TESTS}${NC}"
echo -e "失敗: ${RED}${FAILED_TESTS}${NC}"
echo "========================================"

# 失敗したテストがある場合は終了コード1を返す
if [ ${FAILED_TESTS} -gt 0 ]; then
    echo ""
    echo -e "${RED}エラー: ${FAILED_TESTS} 個のテストが失敗しました。${NC}"
    exit 1
else
    echo ""
    echo -e "${GREEN}すべてのテストが成功しました！${NC}"
    exit 0
fi

