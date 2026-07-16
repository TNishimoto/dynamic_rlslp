#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "stool/include/all.hpp"
#include "../include/all.hpp"
#include "./test_dynamic_string_int64_common.hpp"

bool insert_test_int64(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials = 1000, uint64_t num_inserts_per_string = 10, uint64_t max_length = 10000)
{
    std::vector<int64_t> alphabet = get_int64_alphabet_by_type(alphabet_type);
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

        auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;
        dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::offline_build_from_text(text, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);

        for (uint64_t insert_op = 0; insert_op < num_inserts_per_string; insert_op++)
        {
            std::uniform_int_distribution<int64_t> pos_dist(0, static_cast<int64_t>(text.size()));
            std::uniform_int_distribution<int64_t> len_dist(0, 100);
            int64_t pos = pos_dist(mt);
            int64_t len2 = len_dist(mt);

            std::vector<int64_t> pattern(len2 + 1);
            for (int64_t j = 0; j < len2 + 1; j++)
            {
                pattern[j] = alphabet[char_index_dist(mt)];
            }

            ds.insert_string(pos, pattern);
            text.insert(text.begin() + pos, pattern.begin(), pattern.end());

            if (ds.to_vector_as<int64_t>() == text)
            {
                passed_tests++;
            }
            else
            {
                failed_tests++;
                std::cout << "Insert mismatch at trial " << (trial + 1) << ", op " << (insert_op + 1) << ", pos " << pos << std::endl;
                std::cout << "Expected: " << int64_vector_to_string(text) << std::endl;
                std::cout << "Actual:   " << int64_vector_to_string(ds.to_vector_as<int64_t>()) << std::endl;
                break;
            }
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
    uint64_t num_inserts_per_string = 10;
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
        else if ((arg == "--inserts-per-string" || arg == "-i") && i + 1 < argc)
        {
            num_inserts_per_string = std::stoull(argv[++i]);
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
            all_success = insert_test_int64(use_restricted_block_compression, seed, type, mode, num_trials, num_inserts_per_string, max_length) && all_success;
            if (!all_success && !use_restricted_block_compression)
            {
                return 1;
            }
        }
    }
    else
    {
        all_success = insert_test_int64(use_restricted_block_compression, seed, alphabet_type, mode, num_trials, num_inserts_per_string, max_length);
    }

    return all_success ? 0 : 1;
}
