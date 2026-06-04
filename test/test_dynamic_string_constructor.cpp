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
        // Type 1: Σ={a, b}
        return std::vector<uint8_t>{'a', 'b'};
    case 2:
        // Type 2: Σ={A, C, G, T}
        return std::vector<uint8_t>{'A', 'C', 'G', 'T'};
    case 3:
        // Type 3: Σ={0,1,2,3,4,5,6,7,8,9}
        return std::vector<uint8_t>{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    case 4:
        // Type 4: Σ={a-z}
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
 * @brief 固定長の文字列Xを使ってDynamicString yインスタンスを構築し、
 *        y.to_string()がXと等しいか検証するテスト関数
 * @param use_restricted_block_compression 制限付きブロック圧縮を使用するかどうか
 * @param seed 乱数のシード値
 * @param alphabet_type アルファベットタイプ (1-4)
 * @param mode DictionaryMode
 * @param num_trials_per_length 各長さでのテスト回数（デフォルト: 100）
 * @return すべてのテストが成功した場合true、それ以外false
 */
bool build_test(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials_per_length = 100)
{
    // アルファベットを取得
    std::vector<uint8_t> alphabet = get_alphabet_by_type(alphabet_type);
    
    std::cout << "========================================" << std::endl;
    std::cout << "DynamicString コンストラクタテスト実行中" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::string mode_str = (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" : 
                           (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight" : "fast";
    std::cout << "mode: " << mode_str << std::endl;
    std::cout << "seed: " << seed << std::endl;
    std::cout << "alphabet_type: " << alphabet_type << std::endl;
    std::cout << "alphabet: [";
    for (size_t i = 0; i < alphabet.size(); i++)
    {
        std::cout << "'" << (char)alphabet[i] << "'";
        if (i < alphabet.size() - 1)
            std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    std::cout << "num_trials_per_length: " << num_trials_per_length << std::endl;
    std::cout << "文字列の長さ: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "各試行で以下の検証を行います:" << std::endl;
    std::cout << "  1. ランダムな文字列Xを生成（固定長、指定されたアルファベットから選択）" << std::endl;
    std::cout << "  2. dynRLSLP::DynamicRLSLPString::build_from_text(X) で構築" << std::endl;
    std::cout << "  3. dynRLSLP::DynamicRLSLPString::to_string() と X を比較" << std::endl;
    std::cout << std::endl;

    // 固定の文字列長のリスト: 1, 2, 4, ..., 8192
    std::vector<uint64_t> lengths;
    for (uint64_t d = 1; d <= 8192; d *= 2)
    {
        lengths.push_back(d);
    }

    std::mt19937_64 mt(seed);
    std::uniform_int_distribution<size_t> char_index_dist(0, alphabet.size() - 1);

    int passed_tests = 0;
    int failed_tests = 0;
    uint64_t total_tests = 0;

    // 各長さについてテストを実行
    for (uint64_t length : lengths)
    {
        std::cout << "長さ " << length << " のテスト: " << std::flush;
        uint64_t passed_for_length = 0;
        uint64_t failed_for_length = 0;

        for (uint64_t trial = 0; trial < num_trials_per_length; trial++)
        {
            // ランダムな文字列Xを生成（固定長、指定されたアルファベットから選択）
            std::vector<uint8_t> X;
            for (uint64_t i = 0; i < length; i++)
            {
                size_t char_index = char_index_dist(mt);
                X.push_back(alphabet[char_index]);
            }

            try
            {
                auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;
                // DynamicString yインスタンスを構築
                dynRLSLP::DynamicRLSLPString y = dynRLSLP::DynamicRLSLPString::offline_build_from_text(X, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);
                // y.to_string()を取得
                std::string y_string = y.to_string();
                std::string X_string = std::string(X.begin(), X.end());
                // 検証: y.to_string()がXと等しいか
                if (y_string == X_string)
                {
                    passed_tests++;
                    passed_for_length++;
                }
                else
                {
                    failed_tests++;
                    failed_for_length++;
                    std::cout << std::endl;
                    std::cout << "✗ テスト失敗 (長さ: " << length << ", 試行: " << (trial + 1) << ")" << std::endl;
                    std::cout << "  期待値の長さ: " << X_string.size() << std::endl;
                    std::cout << "  実際の値の長さ: " << y_string.size() << std::endl;
                    if (X_string.size() <= 100 && y_string.size() <= 100)
                    {
                        std::cout << "  期待値: \"" << X_string << "\"" << std::endl;
                        std::cout << "  実際の値: \"" << y_string << "\"" << std::endl;
                    }
                }
                
            }
            catch (const std::logic_error &e)
            {
                failed_tests++;
                failed_for_length++;
                std::cout << std::endl;
                std::cout << "✗ テストロジックエラー例外発生 (長さ: " << length << ", 試行: " << (trial + 1) << "): " << e.what() << std::endl;
                //throw e;

            }
            catch (const std::exception &e)
            {
                failed_tests++;
                failed_for_length++;
                std::cout << std::endl;
                std::cout << "✗ テスト例外発生 (長さ: " << length << ", 試行: " << (trial + 1) << "): " << e.what() << std::endl;
                //throw e;
            }
            catch (...)
            {
                failed_tests++;
                failed_for_length++;
                std::cout << std::endl;
                std::cout << "✗ テスト未知の例外発生 (長さ: " << length << ", 試行: " << (trial + 1) << ")" << std::endl;
                throw std::runtime_error("Unknown exception");
            }
            
            
            total_tests++;
        }

        // 各長さの結果を表示
        if (failed_for_length == 0)
        {
            std::cout << "✓ " << passed_for_length << "/" << num_trials_per_length << " 成功" << std::endl;
        }
        else
        {
            std::cout << "✗ " << passed_for_length << "/" << num_trials_per_length << " 成功, " << failed_for_length << " 失敗" << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "テスト結果" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "成功: " << passed_tests << std::endl;
    std::cout << "失敗: " << failed_tests << std::endl;
    std::cout << "合計: " << total_tests << std::endl;
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
    // テストの説明を表示
    std::cout << "========================================" << std::endl;
    std::cout << "DynamicString テストスイート" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "【テストの目的】" << std::endl;
    std::cout << "固定長の文字列Xを使ってDynamicString yインスタンスを構築し、" << std::endl;
    std::cout << "y.to_string()がXと等しいか検証します。" << std::endl;
    std::cout << std::endl;
    std::cout << "【テストの実行内容】" << std::endl;
    std::cout << "1. 複数の文字列長（1, 2, 4, 8, ..., 8192）でテスト" << std::endl;
    std::cout << "2. 各長さについて、ランダムな文字列Xを生成" << std::endl;
    std::cout << "3. dynRLSLP::DynamicRLSLPString::build_from_text(X) で構築" << std::endl;
    std::cout << "4. dynRLSLP::DynamicRLSLPString::to_string() と X を比較" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // デフォルト値
    bool use_restricted_block_compression = false;
    int64_t seed = 42;
    uint64_t alphabet_type = 0; // 0は未指定を意味する（全てのタイプをテスト）
    dynRLSLP::DictionaryMode mode = dynRLSLP::DictionaryMode::Standard;
    uint64_t num_trials_per_length = 100;

    // コマンドライン引数の解析
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        
        if (arg == "--use-restricted" || arg == "-r" || arg == "--x")
        {
            // -r の後に値が指定されているかチェック
            if (i + 1 < argc)
            {
                std::string next_arg = argv[i + 1];
                if (next_arg == "true" || next_arg == "false")
                {
                    use_restricted_block_compression = (next_arg == "true");
                    i++; // 次の引数をスキップ
                }
                else
                {
                    // 値が指定されていない場合はデフォルトでtrue
                    use_restricted_block_compression = true;
                }
            }
            else
            {
                // 値が指定されていない場合はデフォルトでtrue
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
                    // ランダムなseedを生成
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
                num_trials_per_length = std::stoull(argv[++i]);
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
            std::cout << "  --trials, -t <値>             各長さでのテスト回数を指定 (デフォルト: 100)" << std::endl;
            std::cout << "  --mode, -m <値>                DictionaryModeを指定 (デフォルト: standard)" << std::endl;
            std::cout << "                                standard: Standardモード" << std::endl;
            std::cout << "                                lightweight: Lightweightモード" << std::endl;
            std::cout << "                                fast: Fastモード" << std::endl;
            std::cout << "  --help, -h                    このヘルプを表示" << std::endl;
            std::cout << std::endl;
            std::cout << "例:" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --alphabet-type 2" << std::endl;
            std::cout << "  " << argv[0] << " --use-restricted --seed 456 --alphabet-type 4" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --trials 50 --alphabet-type 3" << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "警告: 不明なオプション '" << arg << "' を無視します" << std::endl;
        }
    }

    // 実際に使用される設定値を表示
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
    std::cout << "num_trials_per_length: " << num_trials_per_length << std::endl;
    std::cout << "文字列の長さ: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // テストを実行
    bool all_success = true;
    
    // build_testを実行
    if (alphabet_type == 0)
        {
            // アルファベットタイプが指定されていない場合は、全てのタイプをテスト
            std::cout << "アルファベットタイプが指定されていないため、全タイプ（1-4）をテストします。" << std::endl;
            std::cout << std::endl;
            
            for (uint64_t type = 1; type <= 4; type++)
            {
                std::cout << "========================================" << std::endl;
                std::cout << "アルファベットタイプ " << type << " のテスト開始" << std::endl;
                std::cout << "========================================" << std::endl;
                std::cout << std::endl;
                
                bool success = build_test(use_restricted_block_compression, seed, type, mode, num_trials_per_length);
                
                if (!success)
                {
                    all_success = false;
                    if (!use_restricted_block_compression)
                    {
                        // restricted_block_compressionモード以外で失敗した場合はエラー終了
                        return 1;
                    }
                }
                
                std::cout << std::endl;
            }
        }
        else
        {
            // 指定されたアルファベットタイプのみをテスト
            all_success = build_test(use_restricted_block_compression, seed, alphabet_type, mode, num_trials_per_length);
            
            if (!all_success && !use_restricted_block_compression)
            {
                // restricted_block_compressionモード以外で失敗した場合はエラー終了
                return 1;
            }
        }

    return all_success ? 0 : 1;
}

