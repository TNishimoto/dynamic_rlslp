#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <random>
#include "stool/include/all.hpp"
#include "../include/all.hpp"

/**
 * @brief 文字列からDictionaryModeを取得
 * @param mode_str モード文字列 ("standard", "lightweight", "fast")
 * @return DictionaryMode
 */
dynRLSLP::DictionaryMode get_mode_from_string(const std::string &mode_str)
{
    if (mode_str == "standard" || mode_str == "Standard" || mode_str == "STANDARD")
    {
        return dynRLSLP::DictionaryMode::Standard;
    }
    else if (mode_str == "lightweight" || mode_str == "Lightweight" || mode_str == "LIGHTWEIGHT")
    {
        return dynRLSLP::DictionaryMode::Lightweight;
    }
    else if (mode_str == "fast" || mode_str == "Fast" || mode_str == "FAST")
    {
        return dynRLSLP::DictionaryMode::Fast;
    }
    else
    {
        throw std::runtime_error("無効なモード: " + mode_str + " (standard, lightweight, fast のいずれかを指定してください)");
    }
}

/**
 * @brief アルファベットタイプに応じたアルファベットを取得
 * @param alphabet_type アルファベットタイプ (1-4)
 * @return アルファベットのベクトル
 */
std::vector<uint8_t> get_alphabet_by_type(uint64_t alphabet_type)
{
    switch (alphabet_type)
    {
    case 1:
        return std::vector<uint8_t>{'a', 'b'};
    case 2:
        return std::vector<uint8_t>{'A', 'C', 'G', 'T'};
    case 3:
        return std::vector<uint8_t>{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    case 4:
    {
        std::vector<uint8_t> alphabet;
        for (uint8_t c = 'a'; c <= 'z'; c++)
        {
            alphabet.push_back(c);
        }
        return alphabet;
    }
    default:
        throw std::runtime_error("無効なアルファベットタイプ: " + std::to_string(alphabet_type) + " (1-4の範囲で指定してください)");
    }
}

/**
 * @brief 2つの接頭辞 T[0..i] と T[0..j] の最長共通接尾辞の長さを返す（ナイーブ実装）
 * @param T 文字列
 * @param i 1つ目の接頭辞の終端位置（0-indexed, 含む）
 * @param j 2つ目の接頭辞の終端位置（0-indexed, 含む）
 * @return 最長共通接尾辞の長さ
 */
uint64_t naive_reverse_lce(const std::vector<uint8_t> &T, uint64_t i, uint64_t j)
{
    uint64_t x = 0;
    while (x <= i && x <= j && T[i - x] == T[j - x])
    {
        x++;
    }
    return x;
}

/**
 * @brief 文字列Tから構築されたDynamicStringについて、ランダムなiとjに対して
 *        dynRLSLP::DynamicRLSLPString::reverse_lce(i, j) == naive_reverse_lce(T, i, j)が成り立つことを検証するテスト関数
 */
bool reverse_lce_test(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials = 1000, uint64_t num_queries_per_string = 10, uint64_t max_length = 10000)
{
    std::vector<uint8_t> alphabet = get_alphabet_by_type(alphabet_type);

    std::cout << "========================================" << std::endl;
    std::cout << "DynamicString reverse_lce テスト実行中" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::string mode_str = (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" :
                           (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight" : "fast";
    std::cout << "mode: " << mode_str << std::endl;
    std::cout << "seed: " << seed << std::endl;
    std::cout << "alphabet_type: " << alphabet_type << std::endl;
    std::cout << "alphabet: [";
    for (size_t k = 0; k < alphabet.size(); k++)
    {
        std::cout << "'" << (char)alphabet[k] << "'";
        if (k < alphabet.size() - 1)
            std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    std::cout << "num_trials: " << num_trials << std::endl;
    std::cout << "num_queries_per_string: " << num_queries_per_string << std::endl;
    std::cout << "max_length: " << max_length << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "各試行で以下の検証を行います:" << std::endl;
    std::cout << "  1. ランダムな文字列Tを生成（長さは1からmax_lengthまでランダム）" << std::endl;
    std::cout << "  2. dynRLSLP::DynamicRLSLPString::build_from_text(T) で構築" << std::endl;
    std::cout << "  3. ランダムな位置iとjを選択 (0 <= i, j < T.size()) を " << num_queries_per_string << " 回実行" << std::endl;
    std::cout << "  4. 各回で dynRLSLP::DynamicRLSLPString::reverse_lce(i, j) == naive_reverse_lce(T, i, j) を検証" << std::endl;
    std::cout << std::endl;

    std::mt19937_64 mt(seed);
    std::uniform_int_distribution<size_t> char_index_dist(0, alphabet.size() - 1);
    std::uniform_int_distribution<uint64_t> length_dist(1, max_length);

    int passed_tests = 0;
    int failed_tests = 0;
    uint64_t last_length = 0;

    for (uint64_t trial = 0; trial < num_trials; trial++)
    {
        uint64_t length = length_dist(mt);
        last_length = length;
        std::vector<uint8_t> T;
        for (uint64_t k = 0; k < length; k++)
        {
            size_t char_index = char_index_dist(mt);
            T.push_back(alphabet[char_index]);
        }

        try
        {
            auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;
            dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::offline_build_from_text(T, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);

            if (length == 0)
            {
                continue;
            }

            std::uniform_int_distribution<int64_t> pos_dist(0, static_cast<int64_t>(length - 1));

            for (uint64_t query = 0; query < num_queries_per_string; query++)
            {
                int64_t i = pos_dist(mt);
                int64_t j = pos_dist(mt);

                uint64_t ds_reverse_lce = ds.reverse_lce(static_cast<uint64_t>(i), static_cast<uint64_t>(j));
                uint64_t correct_reverse_lce = naive_reverse_lce(T, static_cast<uint64_t>(i), static_cast<uint64_t>(j));

                if (ds_reverse_lce == correct_reverse_lce)
                {
                    passed_tests++;
                }
                else
                {
                    failed_tests++;
                    std::cout << "✗ テスト " << (trial + 1) << " クエリ " << (query + 1) << " 失敗 (長さ: " << length << ", 位置i: " << i << ", 位置j: " << j << ")" << std::endl;
                    std::cout << "  期待値: " << correct_reverse_lce << std::endl;
                    std::cout << "  実際の値: " << ds_reverse_lce << std::endl;

                    if (failed_tests <= 10)
                    {
                        std::string T_str = std::string(T.begin(), T.end());
                        if (T_str.size() <= 100)
                        {
                            std::cout << "  文字列T: \"" << T_str << "\"" << std::endl;
                        }
                        else
                        {
                            std::cout << "  文字列T: (長さ " << T_str.size() << ")" << std::endl;
                        }
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            failed_tests++;
            std::cout << "✗ テスト " << (trial + 1) << " 例外発生 (長さ: " << length << "): " << e.what() << std::endl;
        }
        catch (...)
        {
            failed_tests++;
            std::cout << "✗ テスト " << (trial + 1) << " 未知の例外発生 (長さ: " << length << ")" << std::endl;
        }

        if ((trial + 1) % 100 == 0)
        {
            uint64_t total_queries = (trial + 1) * num_queries_per_string;
            std::cout << "進捗: " << (trial + 1) << "/" << num_trials << " 文字列 (長さ: " << last_length << ", 合計クエリ: " << total_queries << ", 成功: " << passed_tests << ", 失敗: " << failed_tests << ")" << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "テスト結果" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "成功: " << passed_tests << std::endl;
    std::cout << "失敗: " << failed_tests << std::endl;
    std::cout << "合計: " << (passed_tests + failed_tests) << std::endl;
    std::cout << "========================================" << std::endl;

    if (failed_tests > 0)
    {
        if (use_restricted_block_compression)
        {
            std::cerr << "警告: restricted_block_compressionモードで " << failed_tests << " 個のテストが失敗しました。" << std::endl;
            std::cerr << "このモードは特定の条件下で動作しない可能性があります。" << std::endl;
        }
        else
        {
            std::cerr << "エラー: " << failed_tests << " 個のテストが失敗しました。" << std::endl;
        }
        return false;
    }
    else
    {
        std::cout << "すべてのテストが成功しました！" << std::endl;
        return true;
    }
}

int main(int argc, char *argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "dynRLSLP::DynamicRLSLPString::reverse_lce() メソッドのテスト" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "【テストの目的】" << std::endl;
    std::cout << "文字列Tから構築されたDynamicStringについて、" << std::endl;
    std::cout << "ランダムな位置iとjに対して dynRLSLP::DynamicRLSLPString::reverse_lce(i, j) が" << std::endl;
    std::cout << "接頭辞 T[0..i] と T[0..j] の最長共通接尾辞の長さと一致することを検証します。" << std::endl;
    std::cout << std::endl;
    std::cout << "【テストの実行内容】" << std::endl;
    std::cout << "1. ランダムな文字列Tを生成します（長さは1からmax_lengthまでランダム）" << std::endl;
    std::cout << "2. 文字列TからDynamicStringインスタンスを構築します" << std::endl;
    std::cout << "3. ランダムな位置iとjを選択します（0 <= i, j < T.size()）を指定回数実行" << std::endl;
    std::cout << "4. 各回で dynRLSLP::DynamicRLSLPString::reverse_lce(i, j) と naive_reverse_lce(T, i, j) を比較します" << std::endl;
    std::cout << "5. 上記の手順をnum_trials回繰り返します" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    bool use_restricted_block_compression = false;
    int64_t seed = 42;
    uint64_t alphabet_type = 0;
    dynRLSLP::DictionaryMode mode = dynRLSLP::DictionaryMode::Standard;
    uint64_t num_trials = 1000;
    uint64_t num_queries_per_string = 10;
    uint64_t max_length = 10000;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--use-restricted" || arg == "-r" || arg == "--x")
        {
            if (i + 1 < argc)
            {
                std::string next_arg = argv[i + 1];
                if (next_arg == "true" || next_arg == "false")
                {
                    use_restricted_block_compression = (next_arg == "true");
                    i++;
                }
                else
                {
                    use_restricted_block_compression = true;
                }
            }
            else
            {
                use_restricted_block_compression = true;
            }
        }
        else if (arg == "--no-restricted" || arg == "-n")
        {
            use_restricted_block_compression = false;
        }
        else if (arg == "--seed" || arg == "-s")
        {
            if (i + 1 < argc)
            {
                seed = std::stoll(argv[++i]);
                if (seed == -1)
                {
                    std::random_device rd;
                    seed = static_cast<int64_t>(rd());
                    std::cout << "ランダムなseedが生成されました: " << seed << std::endl;
                }
            }
            else
            {
                std::cerr << "エラー: --seed オプションには値が必要です" << std::endl;
                return 1;
            }
        }
        else if (arg == "--trials" || arg == "-t")
        {
            if (i + 1 < argc)
            {
                num_trials = std::stoull(argv[++i]);
            }
            else
            {
                std::cerr << "エラー: --trials オプションには値が必要です" << std::endl;
                return 1;
            }
        }
        else if (arg == "--alphabet-type" || arg == "-a" || arg == "--type")
        {
            if (i + 1 < argc)
            {
                alphabet_type = std::stoull(argv[++i]);
                if (alphabet_type < 1 || alphabet_type > 4)
                {
                    std::cerr << "エラー: アルファベットタイプは1-4の範囲で指定してください" << std::endl;
                    return 1;
                }
            }
            else
            {
                std::cerr << "エラー: --alphabet-type オプションには値が必要です" << std::endl;
                return 1;
            }
        }
        else if (arg == "--queries-per-string" || arg == "-q")
        {
            if (i + 1 < argc)
            {
                num_queries_per_string = std::stoull(argv[++i]);
            }
            else
            {
                std::cerr << "エラー: --queries-per-string オプションには値が必要です" << std::endl;
                return 1;
            }
        }
        else if (arg == "--max-length")
        {
            if (i + 1 < argc)
            {
                max_length = std::stoull(argv[++i]);
            }
            else
            {
                std::cerr << "エラー: --max-length オプションには値が必要です" << std::endl;
                return 1;
            }
        }
        else if (arg == "--mode" || arg == "-m")
        {
            if (i + 1 < argc)
            {
                try
                {
                    mode = get_mode_from_string(argv[++i]);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "エラー: " << e.what() << std::endl;
                    return 1;
                }
            }
            else
            {
                std::cerr << "エラー: --mode オプションには値が必要です" << std::endl;
                return 1;
            }
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "使用方法: " << argv[0] << " [オプション]" << std::endl;
            std::cout << std::endl;
            std::cout << "オプション:" << std::endl;
            std::cout << "  --use-restricted, -r, --x    制限付きブロック圧縮を使用 (デフォルト: false)" << std::endl;
            std::cout << "  --no-restricted, -n           制限付きブロック圧縮を使用しない (デフォルト)" << std::endl;
            std::cout << "  --seed, -s <値>               乱数のシード値を指定 (デフォルト: 42, -1でランダム生成)" << std::endl;
            std::cout << "  --alphabet-type, -a, --type <値> アルファベットタイプを指定 (未指定の場合は全タイプをテスト)" << std::endl;
            std::cout << "                                1: Σ={a, b}" << std::endl;
            std::cout << "                                2: Σ={A, C, G, T}" << std::endl;
            std::cout << "                                3: Σ={0,1,2,3,4,5,6,7,8,9}" << std::endl;
            std::cout << "                                4: Σ={a-z}" << std::endl;
            std::cout << "  --trials, -t <値>             文字列Tを生成する回数を指定 (デフォルト: 1000)" << std::endl;
            std::cout << "  --queries-per-string, -q <値> 各文字列Tに対してreverse_lceクエリを実行する回数を指定 (デフォルト: 10)" << std::endl;
            std::cout << "  --max-length <値>              生成する文字列の最大長を指定 (デフォルト: 10000)" << std::endl;
            std::cout << "  --mode, -m <値>                DictionaryModeを指定 (デフォルト: standard)" << std::endl;
            std::cout << "                                standard: Standardモード" << std::endl;
            std::cout << "                                lightweight: Lightweightモード" << std::endl;
            std::cout << "                                fast: Fastモード" << std::endl;
            std::cout << "  --help, -h                    このヘルプを表示" << std::endl;
            std::cout << std::endl;
            std::cout << "例:" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --alphabet-type 2 --trials 500" << std::endl;
            std::cout << "  " << argv[0] << " --use-restricted --seed 456 --alphabet-type 4" << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "警告: 不明なオプション '" << arg << "' を無視します" << std::endl;
        }
    }

    std::cout << "========================================" << std::endl;
    std::cout << "テスト設定" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::string mode_str = (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" :
                           (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight" : "fast";
    std::cout << "mode: " << mode_str << std::endl;
    std::cout << "seed: " << seed << std::endl;
    if (alphabet_type == 0)
    {
        std::cout << "alphabet_type: 未指定（全タイプ1-4をテスト）" << std::endl;
    }
    else
    {
        std::cout << "alphabet_type: " << alphabet_type << std::endl;
    }
    std::cout << "num_trials: " << num_trials << std::endl;
    std::cout << "num_queries_per_string: " << num_queries_per_string << std::endl;
    std::cout << "max_length: " << max_length << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    bool all_success = true;

    if (alphabet_type == 0)
    {
        std::cout << "アルファベットタイプが指定されていないため、全タイプ（1-4）をテストします。" << std::endl;
        std::cout << std::endl;

        for (uint64_t type = 1; type <= 4; type++)
        {
            std::cout << "========================================" << std::endl;
            std::cout << "アルファベットタイプ " << type << " のテスト開始" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << std::endl;

            bool success = reverse_lce_test(use_restricted_block_compression, seed, type, mode, num_trials, num_queries_per_string, max_length);

            if (!success)
            {
                all_success = false;
                if (!use_restricted_block_compression)
                {
                    return 1;
                }
            }

            std::cout << std::endl;
        }
    }
    else
    {
        all_success = reverse_lce_test(use_restricted_block_compression, seed, alphabet_type, mode, num_trials, num_queries_per_string, max_length);

        if (!all_success && !use_restricted_block_compression)
        {
            return 1;
        }
    }

    return all_success ? 0 : 1;
}
