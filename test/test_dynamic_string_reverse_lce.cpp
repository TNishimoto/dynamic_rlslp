#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <random>
#include "stool/include/all.hpp"
#include "../include/all.hpp"

/**
 * @brief Get DictionaryMode from a string
 * @param mode_str Mode string ("standard", "lightweight", "fast")
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
        throw std::runtime_error("Invalid mode: " + mode_str + " (specify one of standard, lightweight, fast)");
    }
}

/**
 * @brief Get alphabet characters for the given alphabet type
 * @param alphabet_type Alphabet type (1-4)
 * @return Vector of alphabet characters
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
        throw std::runtime_error("Invalid alphabet type: " + std::to_string(alphabet_type) + " (specify a value in the range 1-4)");
    }
}

/**
 * @brief Return the longest common suffix length of prefixes T[0..i] and T[0..j] (naive implementation)
 * @param T String
 * @param i End position of the first prefix (0-indexed, inclusive)
 * @param j End position of the second prefix (0-indexed, inclusive)
 * @return Length of the longest common suffix
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
 * @brief Verify dynRLSLP::DynamicRLSLPString::reverse_lce(i, j) == naive_reverse_lce(T, i, j) for random i and j on a DynamicString built from string T
 */
bool reverse_lce_test(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials = 1000, uint64_t num_queries_per_string = 10, uint64_t max_length = 10000)
{
    std::vector<uint8_t> alphabet = get_alphabet_by_type(alphabet_type);

    std::cout << "========================================" << std::endl;
    std::cout << "Running DynamicString reverse_lce test" << std::endl;
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
    std::cout << "Each trial performs the following checks:" << std::endl;
    std::cout << "  1. Generate a random string T (length uniformly random from 1 to max_length)" << std::endl;
    std::cout << "  2. Build with dynRLSLP::DynamicRLSLPString::build_from_text(T)" << std::endl;
    std::cout << "  3. Select random positions i and j (0 <= i, j < T.size()) " << num_queries_per_string << " times" << std::endl;
    std::cout << "  4. Each time, verify dynRLSLP::DynamicRLSLPString::reverse_lce(i, j) == naive_reverse_lce(T, i, j)" << std::endl;
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
                    std::cout << "✗ Test " << (trial + 1) << " query " << (query + 1) << " failed (length: " << length << ", position i: " << i << ", position j: " << j << ")" << std::endl;
                    std::cout << "  Expected: " << correct_reverse_lce << std::endl;
                    std::cout << "  Actual: " << ds_reverse_lce << std::endl;

                    if (failed_tests <= 10)
                    {
                        std::string T_str = std::string(T.begin(), T.end());
                        if (T_str.size() <= 100)
                        {
                            std::cout << "  String T: \"" << T_str << "\"" << std::endl;
                        }
                        else
                        {
                            std::cout << "  String T: (length " << T_str.size() << ")" << std::endl;
                        }
                    }
                }
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
            uint64_t total_queries = (trial + 1) * num_queries_per_string;
            std::cout << "Progress: " << (trial + 1) << "/" << num_trials << " strings (length: " << last_length << ", total queries: " << total_queries << ", passed: " << passed_tests << ", failed: " << failed_tests << ")" << std::endl;
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
            std::cerr << "This mode may not work under certain conditions." << std::endl;
        }
        else
        {
            std::cerr << "Error: " << failed_tests << " test(s) failed." << std::endl;
        }
        return false;
    }
    else
    {
        std::cout << "All tests passed!" << std::endl;
        return true;
    }
}

int main(int argc, char *argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "Test for dynRLSLP::DynamicRLSLPString::reverse_lce()" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "[Test purpose]" << std::endl;
    std::cout << "For a DynamicString built from string T," << std::endl;
    std::cout << "verify that dynRLSLP::DynamicRLSLPString::reverse_lce(i, j)" << std::endl;
    std::cout << "equals the longest common suffix length of prefixes T[0..i] and T[0..j]." << std::endl;
    std::cout << std::endl;
    std::cout << "[Test procedure]" << std::endl;
    std::cout << "1. Generate a random string T (length uniformly random from 1 to max_length)" << std::endl;
    std::cout << "2. Build a DynamicString instance from string T" << std::endl;
    std::cout << "3. Select random positions i and j (0 <= i, j < T.size()) for the specified number of queries" << std::endl;
    std::cout << "4. Each time, compare dynRLSLP::DynamicRLSLPString::reverse_lce(i, j) with naive_reverse_lce(T, i, j)" << std::endl;
    std::cout << "5. Repeat the above for num_trials trials" << std::endl;
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
        else if (arg == "--queries-per-string" || arg == "-q")
        {
            if (i + 1 < argc)
            {
                num_queries_per_string = std::stoull(argv[++i]);
            }
            else
            {
                std::cerr << "Error: --queries-per-string option requires a value" << std::endl;
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
            std::cout << "  --alphabet-type, -a, --type <value> Alphabet type (test all types if omitted)" << std::endl;
            std::cout << "                                1: Σ={a, b}" << std::endl;
            std::cout << "                                2: Σ={A, C, G, T}" << std::endl;
            std::cout << "                                3: Σ={0,1,2,3,4,5,6,7,8,9}" << std::endl;
            std::cout << "                                4: Σ={a-z}" << std::endl;
            std::cout << "  --trials, -t <value>          Number of strings T to generate (default: 1000)" << std::endl;
            std::cout << "  --queries-per-string, -q <value> Number of reverse_lce queries per string T (default: 10)" << std::endl;
            std::cout << "  --max-length <value>          Maximum string length to generate (default: 10000)" << std::endl;
            std::cout << "  --mode, -m <value>            DictionaryMode (default: standard)" << std::endl;
            std::cout << "                                standard: Standard mode" << std::endl;
            std::cout << "                                lightweight: Lightweight mode" << std::endl;
            std::cout << "                                fast: Fast mode" << std::endl;
            std::cout << "  --help, -h                    Show this help" << std::endl;
            std::cout << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --alphabet-type 2 --trials 500" << std::endl;
            std::cout << "  " << argv[0] << " --use-restricted --seed 456 --alphabet-type 4" << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "Warning: ignoring unknown option '" << arg << "'" << std::endl;
        }
    }

    std::cout << "========================================" << std::endl;
    std::cout << "Test configuration" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::string mode_str = (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" :
                           (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight" : "fast";
    std::cout << "mode: " << mode_str << std::endl;
    std::cout << "seed: " << seed << std::endl;
    if (alphabet_type == 0)
    {
        std::cout << "alphabet_type: unspecified (test all types 1-4)" << std::endl;
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
        std::cout << "Alphabet type not specified; testing all types (1-4)." << std::endl;
        std::cout << std::endl;

        for (uint64_t type = 1; type <= 4; type++)
        {
            std::cout << "========================================" << std::endl;
            std::cout << "Starting tests for alphabet type " << type << std::endl;
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
