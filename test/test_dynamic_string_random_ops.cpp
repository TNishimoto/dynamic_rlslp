#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include "stool/include/all.hpp"
#include "../include/all.hpp"

enum class RandomOpType : uint8_t
{
    INSERT = 0,
    DELETE = 1,
    ACCESS = 2,
    GET_ALL_OCCURRENCES = 3,
    LCE = 4,
    LCP = 5,
    LCS = 6,
    REVERSE_LCE = 7,
};

constexpr size_t kRandomOpCount = 8;

/**
 * @brief ランダム操作テストの実行統計
 */
struct RandomOpStats
{
    uint64_t random_draws = 0;
    uint64_t attempted[kRandomOpCount] = {};
    uint64_t executed[kRandomOpCount] = {};
    uint64_t skipped[kRandomOpCount] = {};
    uint64_t passed[kRandomOpCount] = {};
    uint64_t failed[kRandomOpCount] = {};
    uint64_t trial_build_failures = 0;
    uint64_t trial_exceptions = 0;

    void record_attempt(RandomOpType op, bool was_executed, bool was_ok)
    {
        size_t i = static_cast<size_t>(op);
        attempted[i]++;
        if (was_executed)
        {
            executed[i]++;
            if (was_ok)
            {
                passed[i]++;
            }
            else
            {
                failed[i]++;
            }
        }
        else
        {
            skipped[i]++;
        }
    }

    uint64_t total_executed() const
    {
        uint64_t sum = 0;
        for (size_t i = 0; i < kRandomOpCount; i++)
        {
            sum += executed[i];
        }
        return sum;
    }

    uint64_t total_skipped() const
    {
        uint64_t sum = 0;
        for (size_t i = 0; i < kRandomOpCount; i++)
        {
            sum += skipped[i];
        }
        return sum;
    }

    uint64_t total_passed() const
    {
        uint64_t sum = 0;
        for (size_t i = 0; i < kRandomOpCount; i++)
        {
            sum += passed[i];
        }
        return sum;
    }

    uint64_t total_failed() const
    {
        uint64_t sum = 0;
        for (size_t i = 0; i < kRandomOpCount; i++)
        {
            sum += failed[i];
        }
        return sum;
    }
};

const char *random_op_name(RandomOpType op)
{
    switch (op)
    {
    case RandomOpType::INSERT:
        return "insert";
    case RandomOpType::DELETE:
        return "delete";
    case RandomOpType::ACCESS:
        return "access";
    case RandomOpType::GET_ALL_OCCURRENCES:
        return "get_all_occurrences";
    case RandomOpType::LCE:
        return "lce";
    case RandomOpType::LCP:
        return "lcp";
    case RandomOpType::LCS:
        return "lcs";
    case RandomOpType::REVERSE_LCE:
        return "reverse_lce";
    default:
        return "unknown";
    }
}

void print_random_op_statistics(const RandomOpStats &stats)
{
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "操作別統計" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "ランダム抽選回数:           " << stats.random_draws << std::endl;
    std::cout << "実行された操作 (検証あり): " << stats.total_executed() << std::endl;
    std::cout << "スキップ (前提不成立):      " << stats.total_skipped() << std::endl;
    std::cout << "成功:                       " << stats.total_passed() << std::endl;
    std::cout << "失敗:                       " << stats.total_failed() << std::endl;
    if (stats.trial_build_failures > 0)
    {
        std::cout << "試行失敗 (初期構築不一致):  " << stats.trial_build_failures << std::endl;
    }
    if (stats.trial_exceptions > 0)
    {
        std::cout << "試行失敗 (例外):            " << stats.trial_exceptions << std::endl;
    }
    std::cout << std::endl;

    std::cout << std::left << std::setw(22) << "操作"
              << std::right << std::setw(10) << "抽選"
              << "\t" << "実行"
              << "\t" << "スキップ"
              << "\t" << "成功"
              << "\t" << "失敗"
              << "\t" << "実行率"
              << std::endl;
    std::cout << std::string(82, '-') << std::endl;

    for (size_t i = 0; i < kRandomOpCount; i++)
    {
        RandomOpType op = static_cast<RandomOpType>(i);
        double exec_rate = stats.attempted[i] > 0
                               ? 100.0 * static_cast<double>(stats.executed[i]) / static_cast<double>(stats.attempted[i])
                               : 0.0;
        std::cout << std::left << std::setw(22) << random_op_name(op)
                  << std::right << std::setw(10) << stats.attempted[i]
                  << "\t" << stats.executed[i]
                  << "\t" << stats.skipped[i]
                  << "\t" << stats.passed[i]
                  << "\t" << stats.failed[i]
                  << "\t" << std::fixed << std::setprecision(1) << exec_rate << "%"
                  << std::endl;
    }
    std::cout << std::string(82, '-') << std::endl;

    uint64_t sum_attempted = 0;
    for (size_t i = 0; i < kRandomOpCount; i++)
    {
        sum_attempted += stats.attempted[i];
    }
    double total_exec_rate = sum_attempted > 0
                                 ? 100.0 * static_cast<double>(stats.total_executed()) / static_cast<double>(sum_attempted)
                                 : 0.0;
    std::cout << std::left << std::setw(22) << "合計"
              << std::right << std::setw(10) << sum_attempted
              << "\t" << stats.total_executed()
              << "\t" << stats.total_skipped()
              << "\t" << stats.total_passed()
              << "\t" << stats.total_failed()
              << "\t" << std::fixed << std::setprecision(1) << total_exec_rate << "%"
              << std::endl;
    std::cout << std::defaultfloat;
    std::cout << "========================================" << std::endl;
}

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
    throw std::runtime_error("無効なモード: " + mode_str);
}

std::vector<uint8_t> get_alphabet_by_type(uint64_t alphabet_type)
{
    switch (alphabet_type)
    {
    case 1:
        return {'a', 'b'};
    case 2:
        return {'A', 'C', 'G', 'T'};
    case 3:
        return {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
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
        throw std::runtime_error("無効なアルファベットタイプ: " + std::to_string(alphabet_type));
    }
}

std::vector<uint8_t> derive_string(
    dynRLSLP::ExplicitNonterminal explicit_nonterminal,
    const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list)
{
    std::vector<uint8_t> output;
    dynRLSLP::RLSLPRuleBody::decode_rule(explicit_nonterminal, explicit_nonterminal_rule_list)
        .decompress(explicit_nonterminal_rule_list, output);
    return output;
}

uint64_t naive_lcp(
    dynRLSLP::ExplicitNonterminal X1,
    dynRLSLP::ExplicitNonterminal X2,
    const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list)
{
    return stool::StringFunctions::lcp(
        derive_string(X1, explicit_nonterminal_rule_list),
        derive_string(X2, explicit_nonterminal_rule_list));
}

uint64_t naive_lcs(
    dynRLSLP::ExplicitNonterminal X1,
    dynRLSLP::ExplicitNonterminal X2,
    const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list)
{
    return stool::StringFunctions::lcs(
        derive_string(X1, explicit_nonterminal_rule_list),
        derive_string(X2, explicit_nonterminal_rule_list));
}

uint64_t naive_reverse_lce(const std::vector<uint8_t> &T, uint64_t i, uint64_t j)
{
    uint64_t x = 0;
    while (x <= i && x <= j && T[i - x] == T[j - x])
    {
        x++;
    }
    return x;
}

bool vectors_equal(const std::vector<uint64_t> &a, const std::vector<uint64_t> &b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

bool verify_text_equal(const std::vector<uint8_t> &T, const dynRLSLP::DynamicRLSLPString &ds)
{
    return ds.to_vector() == T;
}

std::vector<dynRLSLP::ExplicitNonterminal> collect_valid_nonterminals(
    const dynRLSLP::DynamicRLSLPString &ds)
{
    const std::vector<dynRLSLP::RLSLPRuleBody> &rules = ds.get_dictionary().get_explicit_nonterminal_rule_list();
    uint64_t count = ds.get_dictionary().count_explicit_nonterminals();
    std::vector<dynRLSLP::ExplicitNonterminal> valid;
    for (dynRLSLP::ExplicitNonterminal X = 0; X < static_cast<dynRLSLP::ExplicitNonterminal>(count); X++)
    {
        if (dynRLSLP::RLSLPRuleBody::decode_rule(X, rules).get_type() != dynRLSLP::RLSLPRuleType::Null)
        {
            valid.push_back(X);
        }
    }
    return valid;
}

struct RandomOpsContext
{
    std::mt19937_64 &mt;
    const std::vector<uint8_t> &alphabet;
    uint64_t max_pattern_length;
    uint64_t max_string_length;
};

bool try_insert(
    dynRLSLP::DynamicRLSLPString &ds,
    std::vector<uint8_t> &T,
    RandomOpsContext &ctx)
{
    if (T.size() >= ctx.max_string_length)
    {
        return false;
    }

    std::uniform_int_distribution<int64_t> pos_dist(0, static_cast<int64_t>(T.size()));
    int64_t pos = pos_dist(ctx.mt);

    uint64_t remain = ctx.max_string_length - T.size();
    uint64_t max_len = std::min(ctx.max_pattern_length, remain);
    if (max_len == 0)
    {
        return false;
    }

    std::uniform_int_distribution<uint64_t> pat_len_dist(1, max_len);
    uint64_t pat_len = pat_len_dist(ctx.mt);
    std::vector<uint8_t> pattern(pat_len);
    std::uniform_int_distribution<size_t> char_dist(0, ctx.alphabet.size() - 1);
    for (uint64_t k = 0; k < pat_len; k++)
    {
        pattern[k] = ctx.alphabet[char_dist(ctx.mt)];
    }

    ds.insert_string(pos, pattern);
    T.insert(T.begin() + pos, pattern.begin(), pattern.end());
    return true;
}

bool try_delete(
    dynRLSLP::DynamicRLSLPString &ds,
    std::vector<uint8_t> &T,
    RandomOpsContext &ctx)
{
    if (T.empty())
    {
        return false;
    }

    uint64_t max_del = std::min<uint64_t>(ctx.max_pattern_length, T.size());
    std::uniform_int_distribution<uint64_t> len_dist(1, max_del);
    uint64_t len = len_dist(ctx.mt);

    std::uniform_int_distribution<uint64_t> pos_dist(0, T.size() - len);
    uint64_t pos = pos_dist(ctx.mt);

    ds.delete_substring(pos, len);
    T.erase(T.begin() + static_cast<int64_t>(pos), T.begin() + static_cast<int64_t>(pos + len));
    return true;
}

bool try_access(
    const dynRLSLP::DynamicRLSLPString &ds,
    const std::vector<uint8_t> &T,
    RandomOpsContext &ctx,
    uint64_t &expected,
    uint64_t &actual)
{
    if (T.empty())
    {
        return false;
    }

    std::uniform_int_distribution<uint64_t> pos_dist(0, T.size() - 1);
    uint64_t pos = pos_dist(ctx.mt);
    expected = T[pos];
    actual = ds.access_char(pos);
    return true;
}

bool try_get_all_occurrences(
    const dynRLSLP::DynamicRLSLPString &ds,
    RandomOpsContext &ctx,
    std::vector<uint64_t> &expected,
    std::vector<uint64_t> &actual,
    dynRLSLP::ExplicitNonterminal &X)
{
    std::vector<dynRLSLP::ExplicitNonterminal> valid = collect_valid_nonterminals(ds);
    if (valid.empty())
    {
        return false;
    }

    std::uniform_int_distribution<size_t> idx_dist(0, valid.size() - 1);
    X = valid[idx_dist(ctx.mt)];

    const std::vector<dynRLSLP::RLSLPRuleBody> &rules = ds.get_dictionary().get_explicit_nonterminal_rule_list();
    const std::vector<uint64_t> &lengths = ds.get_dictionary().get_explicit_nonterminal_length_list();
    dynRLSLP::ExplicitNonterminal root = ds.get_dictionary().get_grammar().get_root();

    actual = ds.get_all_occurrences(X);
    std::sort(actual.begin(), actual.end());
    expected = dynRLSLP::NaiveParentQuery::get_all_occurrences(X, root, rules, lengths);
    return true;
}

bool try_lce(
    const dynRLSLP::DynamicRLSLPString &ds,
    const std::vector<uint8_t> &T,
    RandomOpsContext &ctx,
    uint64_t &expected,
    uint64_t &actual,
    uint64_t &i,
    uint64_t &j)
{
    if (T.empty())
    {
        return false;
    }

    std::uniform_int_distribution<uint64_t> pos_dist(0, T.size() - 1);
    i = pos_dist(ctx.mt);
    j = pos_dist(ctx.mt);
    expected = stool::StringFunctions::lce(T, i, j);
    actual = ds.lce(i, j);
    return true;
}

bool try_lcp(
    const dynRLSLP::DynamicRLSLPString &ds,
    RandomOpsContext &ctx,
    uint64_t &expected,
    uint64_t &actual,
    dynRLSLP::ExplicitNonterminal &X1,
    dynRLSLP::ExplicitNonterminal &X2)
{
    std::vector<dynRLSLP::ExplicitNonterminal> valid = collect_valid_nonterminals(ds);
    if (valid.empty())
    {
        return false;
    }

    std::uniform_int_distribution<size_t> idx_dist(0, valid.size() - 1);
    X1 = valid[idx_dist(ctx.mt)];
    X2 = valid[idx_dist(ctx.mt)];

    const std::vector<dynRLSLP::RLSLPRuleBody> &rules = ds.get_dictionary().get_explicit_nonterminal_rule_list();
    expected = naive_lcp(X1, X2, rules);
    actual = ds.lcp(X1, X2);
    return true;
}

bool try_lcs(
    const dynRLSLP::DynamicRLSLPString &ds,
    RandomOpsContext &ctx,
    uint64_t &expected,
    uint64_t &actual,
    dynRLSLP::ExplicitNonterminal &X1,
    dynRLSLP::ExplicitNonterminal &X2)
{
    std::vector<dynRLSLP::ExplicitNonterminal> valid = collect_valid_nonterminals(ds);
    if (valid.empty())
    {
        return false;
    }

    std::uniform_int_distribution<size_t> idx_dist(0, valid.size() - 1);
    X1 = valid[idx_dist(ctx.mt)];
    X2 = valid[idx_dist(ctx.mt)];

    const std::vector<dynRLSLP::RLSLPRuleBody> &rules = ds.get_dictionary().get_explicit_nonterminal_rule_list();
    expected = naive_lcs(X1, X2, rules);
    actual = ds.lcs(X1, X2);
    return true;
}

bool try_reverse_lce(
    const dynRLSLP::DynamicRLSLPString &ds,
    const std::vector<uint8_t> &T,
    RandomOpsContext &ctx,
    uint64_t &expected,
    uint64_t &actual,
    uint64_t &i,
    uint64_t &j)
{
    if (T.empty())
    {
        return false;
    }

    std::uniform_int_distribution<uint64_t> pos_dist(0, T.size() - 1);
    i = pos_dist(ctx.mt);
    j = pos_dist(ctx.mt);
    expected = naive_reverse_lce(T, i, j);
    actual = ds.reverse_lce(i, j);
    return true;
}

bool random_ops_test(
    bool use_restricted_block_compression,
    int64_t seed,
    uint64_t alphabet_type,
    dynRLSLP::DictionaryMode mode,
    uint64_t num_trials = 100,
    uint64_t num_operations_per_trial = 50,
    uint64_t initial_max_length = 500,
    uint64_t max_string_length = 2000,
    uint64_t max_pattern_length = 20)
{
    std::vector<uint8_t> alphabet = get_alphabet_by_type(alphabet_type);

    std::cout << "========================================" << std::endl;
    std::cout << "DynamicString ランダム操作テスト実行中" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::string mode_str = (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" :
                           (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight" : "fast";
    std::cout << "mode: " << mode_str << std::endl;
    std::cout << "seed: " << seed << std::endl;
    std::cout << "alphabet_type: " << alphabet_type << std::endl;
    std::cout << "num_trials: " << num_trials << std::endl;
    std::cout << "num_operations_per_trial: " << num_operations_per_trial << std::endl;
    std::cout << "initial_max_length: " << initial_max_length << std::endl;
    std::cout << "max_string_length: " << max_string_length << std::endl;
    std::cout << "max_pattern_length: " << max_pattern_length << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    std::mt19937_64 mt(seed);
    std::uniform_int_distribution<size_t> char_index_dist(0, alphabet.size() - 1);
    std::uniform_int_distribution<uint64_t> length_dist(1, initial_max_length);
    std::uniform_int_distribution<int> op_dist(0, 7);

    int passed_tests = 0;
    int failed_tests = 0;
    uint64_t last_length = 0;
    RandomOpStats stats;

    for (uint64_t trial = 0; trial < num_trials; trial++)
    {
        uint64_t length = length_dist(mt);
        last_length = length;
        std::vector<uint8_t> T;
        for (uint64_t k = 0; k < length; k++)
        {
            T.push_back(alphabet[char_index_dist(mt)]);
        }

        try
        {
            auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;
            dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::offline_build_from_text(T, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);

            if (!verify_text_equal(T, ds))
            {
                failed_tests++;
                stats.trial_build_failures++;
                std::cout << "✗ 試行 " << (trial + 1) << " 初期構築後の文字列不一致" << std::endl;
                continue;
            }

            RandomOpsContext ctx{mt, alphabet, max_pattern_length, max_string_length};

            for (uint64_t op_idx = 0; op_idx < num_operations_per_trial; op_idx++)
            {
                RandomOpType op = static_cast<RandomOpType>(op_dist(mt));
                stats.random_draws++;
                bool executed = false;
                bool ok = true;

                switch (op)
                {
                case RandomOpType::INSERT:
                {
                    executed = try_insert(ds, T, ctx);
                    if (executed && !verify_text_equal(T, ds))
                    {
                        ok = false;
                    }
                    break;
                }
                case RandomOpType::DELETE:
                {
                    executed = try_delete(ds, T, ctx);
                    if (executed && !verify_text_equal(T, ds))
                    {
                        ok = false;
                    }
                    break;
                }
                case RandomOpType::ACCESS:
                {
                    uint64_t expected = 0;
                    uint64_t actual = 0;
                    executed = try_access(ds, T, ctx, expected, actual);
                    if (executed && expected != actual)
                    {
                        ok = false;
                    }
                    break;
                }
                case RandomOpType::GET_ALL_OCCURRENCES:
                {
                    std::vector<uint64_t> expected;
                    std::vector<uint64_t> actual;
                    dynRLSLP::ExplicitNonterminal X = 0;
                    executed = try_get_all_occurrences(ds, ctx, expected, actual, X);
                    if (executed && !vectors_equal(expected, actual))
                    {
                        ok = false;
                    }
                    break;
                }
                case RandomOpType::LCE:
                {
                    uint64_t expected = 0;
                    uint64_t actual = 0;
                    uint64_t i = 0;
                    uint64_t j = 0;
                    executed = try_lce(ds, T, ctx, expected, actual, i, j);
                    if (executed && expected != actual)
                    {
                        ok = false;
                    }
                    break;
                }
                case RandomOpType::LCP:
                {
                    uint64_t expected = 0;
                    uint64_t actual = 0;
                    dynRLSLP::ExplicitNonterminal X1 = 0;
                    dynRLSLP::ExplicitNonterminal X2 = 0;
                    executed = try_lcp(ds, ctx, expected, actual, X1, X2);
                    if (executed && expected != actual)
                    {
                        ok = false;
                    }
                    break;
                }
                case RandomOpType::LCS:
                {
                    uint64_t expected = 0;
                    uint64_t actual = 0;
                    dynRLSLP::ExplicitNonterminal X1 = 0;
                    dynRLSLP::ExplicitNonterminal X2 = 0;
                    executed = try_lcs(ds, ctx, expected, actual, X1, X2);
                    if (executed && expected != actual)
                    {
                        ok = false;
                    }
                    break;
                }
                case RandomOpType::REVERSE_LCE:
                {
                    uint64_t expected = 0;
                    uint64_t actual = 0;
                    uint64_t i = 0;
                    uint64_t j = 0;
                    executed = try_reverse_lce(ds, T, ctx, expected, actual, i, j);
                    if (executed && expected != actual)
                    {
                        ok = false;
                    }
                    break;
                }
                }

                stats.record_attempt(op, executed, executed && ok);

                if (!executed)
                {
                    continue;
                }

                if (ok)
                {
                    passed_tests++;
                }
                else
                {
                    failed_tests++;
                    std::cout << "✗ 試行 " << (trial + 1) << " 操作 " << (op_idx + 1)
                              << " (" << random_op_name(op) << ") 失敗 (|T|=" << T.size() << ")" << std::endl;
                    if (failed_tests <= 10)
                    {
                        if (T.size() <= 80)
                        {
                            std::cout << "  T: \"" << std::string(T.begin(), T.end()) << "\"" << std::endl;
                        }
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            failed_tests++;
            stats.trial_exceptions++;
            std::cout << "✗ 試行 " << (trial + 1) << " 例外: " << e.what() << std::endl;
        }

        if ((trial + 1) % 20 == 0)
        {
            std::cout << "進捗: " << (trial + 1) << "/" << num_trials
                      << " (|T|=" << last_length
                      << ", 実行: " << stats.total_executed()
                      << ", 成功: " << passed_tests
                      << ", 失敗: " << failed_tests << ")" << std::endl;
        }
    }

    print_random_op_statistics(stats);

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
        std::cerr << "エラー: " << failed_tests << " 個の操作が失敗しました。" << std::endl;
        return false;
    }

    std::cout << "すべてのテストが成功しました！" << std::endl;
    return true;
}

int main(int argc, char *argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "DynamicRLSLPString ランダム操作テスト" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "各試行で insert / delete / access / get_all_occurrences /" << std::endl;
    std::cout << "lce / lcp / lcs / reverse_lce をランダムに実行し、参照実装と比較します。" << std::endl;
    std::cout << std::endl;

    bool use_restricted_block_compression = false;
    int64_t seed = 42;
    uint64_t alphabet_type = 0;
    dynRLSLP::DictionaryMode mode = dynRLSLP::DictionaryMode::Standard;
    uint64_t num_trials = 100;
    uint64_t num_operations_per_trial = 50;
    uint64_t initial_max_length = 5000;
    uint64_t max_string_length = 10000;
    uint64_t max_pattern_length = 100;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--use-restricted" || arg == "-r" || arg == "--x")
        {
            if (i + 1 < argc && (std::string(argv[i + 1]) == "true" || std::string(argv[i + 1]) == "false"))
            {
                use_restricted_block_compression = (std::string(argv[++i]) == "true");
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
            seed = std::stoll(argv[++i]);
            if (seed == -1)
            {
                std::random_device rd;
                seed = static_cast<int64_t>(rd());
            }
        }
        else if (arg == "--trials" || arg == "-t")
        {
            num_trials = std::stoull(argv[++i]);
        }
        else if (arg == "--operations" || arg == "-o")
        {
            num_operations_per_trial = std::stoull(argv[++i]);
        }
        else if (arg == "--alphabet-type" || arg == "-a" || arg == "--type")
        {
            alphabet_type = std::stoull(argv[++i]);
        }
        else if (arg == "--initial-max-length")
        {
            initial_max_length = std::stoull(argv[++i]);
        }
        else if (arg == "--max-length")
        {
            max_string_length = std::stoull(argv[++i]);
        }
        else if (arg == "--max-pattern-length")
        {
            max_pattern_length = std::stoull(argv[++i]);
        }
        else if (arg == "--mode" || arg == "-m")
        {
            mode = get_mode_from_string(argv[++i]);
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "使用方法: " << argv[0] << " [オプション]" << std::endl;
            std::cout << "  --seed, -s <値>                 シード (デフォルト: 42, -1でランダム)" << std::endl;
            std::cout << "  --trials, -t <値>               試行回数 (デフォルト: 100)" << std::endl;
            std::cout << "  --operations, -o <値>           各試行の操作回数 (デフォルト: 50)" << std::endl;
            std::cout << "  --initial-max-length <値>       初期文字列の最大長 (デフォルト: 500)" << std::endl;
            std::cout << "  --max-length <値>               操作中の文字列の最大長 (デフォルト: 2000)" << std::endl;
            std::cout << "  --max-pattern-length <値>       insert/delete の最大長 (デフォルト: 20)" << std::endl;
            std::cout << "  --alphabet-type, -a <1-4>       アルファベットタイプ" << std::endl;
            std::cout << "  --mode, -m <standard|fast|...>  DictionaryMode" << std::endl;
            std::cout << "  --use-restricted, -r            制限付きブロック圧縮" << std::endl;
            return 0;
        }
    }

    bool all_success = true;

    if (alphabet_type == 0)
    {
        for (uint64_t type = 1; type <= 4; type++)
        {
            bool success = random_ops_test(
                use_restricted_block_compression, seed, type, mode,
                num_trials, num_operations_per_trial,
                initial_max_length, max_string_length, max_pattern_length);
            if (!success)
            {
                all_success = false;
                if (!use_restricted_block_compression)
                {
                    return 1;
                }
            }
        }
    }
    else
    {
        all_success = random_ops_test(
            use_restricted_block_compression, seed, alphabet_type, mode,
            num_trials, num_operations_per_trial,
            initial_max_length, max_string_length, max_pattern_length);
        if (!all_success && !use_restricted_block_compression)
        {
            return 1;
        }
    }

    return all_success ? 0 : 1;
}
