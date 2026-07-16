#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "stool/include/all.hpp"
#include "../include/all.hpp"
#include "./test_dynamic_string_int64_common.hpp"

bool access_test_int64(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials = 1000, uint64_t max_length = 10000)
{
    std::vector<int64_t> alphabet = get_int64_alphabet_by_type(alphabet_type);

    std::cout << "========================================" << std::endl;
    std::cout << "Running DynamicString int64 access test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::cout << "mode: " << mode_to_string_int64(mode) << std::endl;
    std::cout << "seed: " << seed << std::endl;
    std::cout << "alphabet_type: " << alphabet_type << std::endl;
    std::cout << "alphabet: " << int64_vector_to_string(alphabet) << std::endl;
    std::cout << "num_trials: " << num_trials << std::endl;
    std::cout << "max_length: " << max_length << std::endl;

    std::mt19937_64 mt(seed);
    std::uniform_int_distribution<size_t> char_index_dist(0, alphabet.size() - 1);
    std::uniform_int_distribution<uint64_t> length_dist(1, max_length);

    int passed_tests = 0;
    int failed_tests = 0;

    for (uint64_t trial = 0; trial < num_trials; trial++)
    {
        uint64_t length = length_dist(mt);
        std::vector<int64_t> text(length);
        for (uint64_t i = 0; i < length; i++)
        {
            text[i] = alphabet[char_index_dist(mt)];
        }

        try
        {
            auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;
            dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::offline_build_from_text(text, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);

            for (uint64_t i = 0; i < length; i++)
            {
                int64_t actual = ds.access_value<int64_t>(i);
                int64_t expected = text[i];
                if (actual == expected)
                {
                    passed_tests++;
                }
                else
                {
                    failed_tests++;
                    std::cout << "Mismatch at trial " << (trial + 1) << ", position " << i << ": expected " << expected << ", actual " << actual << std::endl;
                    break;
                }
            }
        }
        catch (const std::exception &e)
        {
            failed_tests++;
            std::cout << "Exception at trial " << (trial + 1) << ": " << e.what() << std::endl;
        }

        if ((trial + 1) % 100 == 0)
        {
            std::cout << "Progress: " << (trial + 1) << "/" << num_trials << " (passed: " << passed_tests << ", failed: " << failed_tests << ")" << std::endl;
        }
    }

    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    return failed_tests == 0;
}

int main(int argc, char *argv[])
{
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
            use_restricted_block_compression = true;
        }
        else if (arg == "--no-restricted" || arg == "-n")
        {
            use_restricted_block_compression = false;
        }
        else if ((arg == "--seed" || arg == "-s") && i + 1 < argc)
        {
            seed = std::stoll(argv[++i]);
        }
        else if ((arg == "--trials" || arg == "-t") && i + 1 < argc)
        {
            num_trials = std::stoull(argv[++i]);
        }
        else if ((arg == "--alphabet-type" || arg == "-a" || arg == "--type") && i + 1 < argc)
        {
            alphabet_type = std::stoull(argv[++i]);
        }
        else if (arg == "--max-length" && i + 1 < argc)
        {
            max_length = std::stoull(argv[++i]);
        }
        else if ((arg == "--mode" || arg == "-m") && i + 1 < argc)
        {
            mode = get_mode_from_string_int64(argv[++i]);
        }
    }

    bool all_success = true;
    if (alphabet_type == 0)
    {
        for (uint64_t type = 1; type <= 4; type++)
        {
            all_success = access_test_int64(use_restricted_block_compression, seed, type, mode, num_trials, max_length) && all_success;
            if (!all_success && !use_restricted_block_compression)
            {
                return 1;
            }
        }
    }
    else
    {
        all_success = access_test_int64(use_restricted_block_compression, seed, alphabet_type, mode, num_trials, max_length);
    }

    return all_success ? 0 : 1;
}
