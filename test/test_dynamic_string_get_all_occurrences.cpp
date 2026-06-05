#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include "stool/include/all.hpp"
#include "../include/all.hpp"

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
        throw std::runtime_error("Invalid mode: " + mode_str + " (specify one of standard, lightweight, fast)");
    }
}

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
        throw std::runtime_error("Invalid alphabet type: " + std::to_string(alphabet_type) + " (specify a value in the range 1-4)");
    }
}



/*
std::vector<uint64_t> naive_get_all_occurrences(
    const dynRLSLP::DynamicRLSLPString &ds,
    dynRLSLP::ExplicitNonterminal explicit_nonterminal)
{
    const dynRLSLP::DynamicGrammarForLayeredRLSLP &grammar = ds.get_dictionary();
    const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list = grammar.get_explicit_nonterminal_rule_list();
    const std::vector<uint64_t> &explicit_nonterminal_length_list = grammar.get_explicit_nonterminal_length_list();
    dynRLSLP::NonterminalWithRelativeLevel root = grammar.get_grammar().get_root();

    std::vector<uint64_t> output;
    naive_collect_occurrences(root, 0, explicit_nonterminal, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
    std::sort(output.begin(), output.end());
    return output;
}
*/

bool vectors_equal(const std::vector<uint64_t> &a, const std::vector<uint64_t> &b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (size_t i = 0; i < a.size(); i++)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }
    return true;
}

bool verify_occurrence_substrings(
    const std::vector<uint8_t> &text,
    const std::vector<uint64_t> &positions,
    dynRLSLP::ExplicitNonterminal explicit_nonterminal,
    const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list)
{
    std::vector<uint8_t> derived;
    dynRLSLP::RLSLPRuleBody::decode_rule(explicit_nonterminal, explicit_nonterminal_rule_list).decompress(explicit_nonterminal_rule_list, derived);

    if (derived.empty())
    {
        return positions.empty();
    }

    for (uint64_t pos : positions)
    {
        if (pos + derived.size() > text.size())
        {
            return false;
        }
        for (size_t k = 0; k < derived.size(); k++)
        {
            if (text[pos + k] != derived[k])
            {
                return false;
            }
        }
    }
    return true;
}

bool get_all_occurrences_test(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials = 1000, uint64_t max_length = 10000)
{
    std::vector<uint8_t> alphabet = get_alphabet_by_type(alphabet_type);

    std::cout << "========================================" << std::endl;
    std::cout << "Running DynamicString get_all_occurrences test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::string mode_str = (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" :
                           (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight" : "fast";
    std::cout << "mode: " << mode_str << std::endl;
    std::cout << "seed: " << seed << std::endl;
    std::cout << "alphabet_type: " << alphabet_type << std::endl;
    std::cout << "num_trials: " << num_trials << std::endl;
    std::cout << "max_length: " << max_length << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Each trial performs the following checks:" << std::endl;
    std::cout << "  1. Generate a random string T" << std::endl;
    std::cout << "  2. Build DynamicRLSLPString" << std::endl;
    std::cout << "  3. For each explicit nonterminal X, call get_all_occurrences(X)" << std::endl;
    std::cout << "  4. Verify results match the naive implementation via derivation-tree traversal" << std::endl;
    std::cout << "  5. Verify the substring of T at each occurrence matches the string derived by X" << std::endl;
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
            T.push_back(alphabet[char_index_dist(mt)]);
        }

        try
        {
            auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;
            dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::offline_build_from_text(T, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);

            std::vector<uint8_t> decompressed = ds.to_vector();
            if (decompressed != T)
            {
                failed_tests++;
                std::cout << "✗ Test " << (trial + 1) << " failed: restored string mismatch (length: " << length << ")" << std::endl;
                continue;
            }

            const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list = ds.get_dictionary().get_explicit_nonterminal_rule_list();
            uint64_t explicit_nonterminal_count = ds.get_dictionary().count_explicit_nonterminals();

            for (uint64_t xi = 0; xi < 100; xi++)
            {
                // INSERT_YOUR_CODE
                // ExplicitNonterminal values range from 0 to explicit_nonterminal_count-1
                std::uniform_int_distribution<uint64_t> X_dist(0, explicit_nonterminal_count - 1);
                dynRLSLP::ExplicitNonterminal X = X_dist(mt);
    

                dynRLSLP::RLSLPRuleBody body = dynRLSLP::RLSLPRuleBody::decode_rule(X, explicit_nonterminal_rule_list);
                if (body.get_type() == dynRLSLP::RLSLPRuleType::Null)
                {
                    continue;
                }

                std::vector<uint64_t> actual = ds.get_all_occurrences(X);
                std::sort(actual.begin(), actual.end());
                const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list = ds.get_dictionary().get_explicit_nonterminal_rule_list();
                const std::vector<uint64_t> &explicit_nonterminal_length_list = ds.get_dictionary().get_explicit_nonterminal_length_list();
                std::vector<uint64_t> expected = dynRLSLP::NaiveParentQuery::get_all_occurrences(X, ds.get_dictionary().get_grammar().get_root(), explicit_nonterminal_rule_list, explicit_nonterminal_length_list);

                if (!vectors_equal(actual, expected))
                {
                    failed_tests++;
                    std::cout << "✗ Test " << (trial + 1) << " nonterminal " << X << " failed (length: " << length << ")" << std::endl;
                    std::cout << "  Expected count: " << expected.size() << std::endl;
                    std::cout << "  Actual count: " << actual.size() << std::endl;
                    if (failed_tests <= 10)
                    {
                        size_t show = std::min<size_t>(10, expected.size());
                        std::cout << "  Expected (first " << show << "): ";
                        for (size_t i = 0; i < show; i++)
                        {
                            std::cout << expected[i] << (i + 1 < show ? ", " : "");
                        }
                        std::cout << std::endl;
                        show = std::min<size_t>(10, actual.size());
                        std::cout << "  Actual (first " << show << "): ";
                        for (size_t i = 0; i < show; i++)
                        {
                            std::cout << actual[i] << (i + 1 < show ? ", " : "");
                        }
                        std::cout << std::endl;
                    }
                    continue;
                }

                if (!verify_occurrence_substrings(T, actual, X, explicit_nonterminal_rule_list))
                {
                    failed_tests++;
                    std::cout << "✗ Test " << (trial + 1) << " nonterminal " << X << " substring verification failed (length: " << length << ")" << std::endl;
                    continue;
                }

                passed_tests++;
            }
        }
        catch (const std::exception &e)
        {
            failed_tests++;
            std::cout << "✗ Test " << (trial + 1) << " exception (length: " << length << "): " << e.what() << std::endl;
        }
        catch (...)
        {
            failed_tests++;
            std::cout << "✗ Test " << (trial + 1) << " unknown exception (length: " << length << ")" << std::endl;
        }

        if ((trial + 1) % 100 == 0)
        {
            std::cout << "Progress: " << (trial + 1) << "/" << num_trials << " strings (length: " << last_length << ", passed: " << passed_tests << ", failed: " << failed_tests << ")" << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Test results" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    std::cout << "Total: " << (passed_tests + failed_tests) << std::endl;
    std::cout << "========================================" << std::endl;

    if (failed_tests > 0)
    {
        if (use_restricted_block_compression)
        {
            std::cerr << "Warning: " << failed_tests << " test(s) failed in restricted_block_compression mode." << std::endl;
        }
        else
        {
            std::cerr << "Error: " << failed_tests << " test(s) failed." << std::endl;
        }
        return false;
    }

    std::cout << "All tests passed!" << std::endl;
    return true;
}

int main(int argc, char *argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "Test for dynRLSLP::DynamicRLSLPString::get_all_occurrences()" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    bool use_restricted_block_compression = false;
    int64_t seed = 42;
    uint64_t alphabet_type = 0;
    dynRLSLP::DictionaryMode mode = dynRLSLP::DictionaryMode::Standard;
    uint64_t num_trials = 1000;
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
                    std::cout << "Generated random seed: " << seed << std::endl;
                }
            }
            else
            {
                std::cerr << "Error: --seed option requires a value" << std::endl;
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
                std::cerr << "Error: --trials option requires a value" << std::endl;
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
                    std::cerr << "Error: alphabet type must be in the range 1-4" << std::endl;
                    return 1;
                }
            }
            else
            {
                std::cerr << "Error: --alphabet-type option requires a value" << std::endl;
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
                std::cerr << "Error: --max-length option requires a value" << std::endl;
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
                    std::cerr << "Error: " << e.what() << std::endl;
                    return 1;
                }
            }
            else
            {
                std::cerr << "Error: --mode option requires a value" << std::endl;
                return 1;
            }
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --use-restricted, -r, --x    Use restricted block compression (default: false)" << std::endl;
            std::cout << "  --no-restricted, -n           Do not use restricted block compression (default)" << std::endl;
            std::cout << "  --seed, -s <value>            Random seed (default: 42, -1 to generate randomly)" << std::endl;
            std::cout << "  --alphabet-type, -a, --type <value> Alphabet type (test all types 1-4 if omitted)" << std::endl;
            std::cout << "  --trials, -t <value>          Number of trials (default: 1000)" << std::endl;
            std::cout << "  --max-length <value>          Maximum string length to generate (default: 10000)" << std::endl;
            std::cout << "  --mode, -m <value>            DictionaryMode (default: standard)" << std::endl;
            std::cout << "  --help, -h                    Show this help" << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "Warning: ignoring unknown option '" << arg << "'" << std::endl;
        }
    }

    bool all_success = true;

    if (alphabet_type == 0)
    {
        for (uint64_t type = 1; type <= 4; type++)
        {
            std::cout << "========================================" << std::endl;
            std::cout << "Starting tests for alphabet type " << type << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << std::endl;

            bool success = get_all_occurrences_test(use_restricted_block_compression, seed, type, mode, num_trials, max_length);
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
        all_success = get_all_occurrences_test(use_restricted_block_compression, seed, alphabet_type, mode, num_trials, max_length);
        if (!all_success && !use_restricted_block_compression)
        {
            return 1;
        }
    }

    return all_success ? 0 : 1;
}
