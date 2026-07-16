#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "stool/include/all.hpp"
#include "../include/all.hpp"
#include "./test_dynamic_string_int64_common.hpp"

bool build_test_int64(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials_per_length = 100)
{
    std::vector<int64_t> alphabet = get_int64_alphabet_by_type(alphabet_type);
    std::vector<uint64_t> lengths;
    for (uint64_t d = 1; d <= 8192; d *= 2)
    {
        lengths.push_back(d);
    }

    std::mt19937_64 mt(seed);
    std::uniform_int_distribution<size_t> char_index_dist(0, alphabet.size() - 1);

    int passed_tests = 0;
    int failed_tests = 0;

    for (uint64_t length : lengths)
    {
        std::cout << "Tests for length " << length << ": " << std::flush;
        uint64_t passed_for_length = 0;
        uint64_t failed_for_length = 0;

        for (uint64_t trial = 0; trial < num_trials_per_length; trial++)
        {
            std::vector<int64_t> text(length);
            for (uint64_t i = 0; i < length; i++)
            {
                text[i] = alphabet[char_index_dist(mt)];
            }

            try
            {
                auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;
                dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::offline_build_from_text(text, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);
                if (ds.to_vector_as<int64_t>() == text)
                {
                    passed_tests++;
                    passed_for_length++;
                }
                else
                {
                    failed_tests++;
                    failed_for_length++;
                }
            }
            catch (const std::exception &e)
            {
                failed_tests++;
                failed_for_length++;
                std::cout << "\nException at length " << length << ", trial " << (trial + 1) << ": " << e.what() << std::endl;
            }
        }

        if (failed_for_length == 0)
        {
            std::cout << "ok (" << passed_for_length << "/" << num_trials_per_length << ")" << std::endl;
        }
        else
        {
            std::cout << "failed (" << failed_for_length << " failures)" << std::endl;
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
    uint64_t num_trials_per_length = 100;

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
            num_trials_per_length = std::stoull(argv[++i]);
        }
        else if ((arg == "--alphabet-type" || arg == "-a" || arg == "--type") && i + 1 < argc)
        {
            alphabet_type = std::stoull(argv[++i]);
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
            all_success = build_test_int64(use_restricted_block_compression, seed, type, mode, num_trials_per_length) && all_success;
            if (!all_success && !use_restricted_block_compression)
            {
                return 1;
            }
        }
    }
    else
    {
        all_success = build_test_int64(use_restricted_block_compression, seed, alphabet_type, mode, num_trials_per_length);
    }

    return all_success ? 0 : 1;
}
