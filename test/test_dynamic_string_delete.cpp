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
        throw std::runtime_error("Invalid alphabet type: " + std::to_string(alphabet_type) + " (specify a value in the range 1-4)");
    }
}

/**
 * @brief Test delete_substring operations on a DynamicString built from string T
 * @param use_restricted_block_compression Whether to use restricted block compression
 * @param seed Random seed value
 * @param alphabet_type Alphabet type (1-4)
 * @param mode DictionaryMode
 * @param num_trials Number of strings T to generate (default: 1000)
 * @param num_deletes_per_string Number of delete operations per string T (default: 10)
 * @param max_length Maximum string length to generate (default: 10000)
 * @return true if all tests pass, false otherwise
 */
bool delete_test(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials = 1000, uint64_t num_deletes_per_string = 10, uint64_t max_length = 10000)
{
    // Get alphabet
    std::vector<uint8_t> alphabet = get_alphabet_by_type(alphabet_type);

    std::cout << "========================================" << std::endl;
    std::cout << "Running DynamicString delete_substring test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::string mode_str = (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" : (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight"
                                                                                                                                           : "fast";
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
    std::cout << "num_trials: " << num_trials << std::endl;
    std::cout << "num_deletes_per_string: " << num_deletes_per_string << std::endl;
    std::cout << "max_length: " << max_length << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Each trial performs the following checks:" << std::endl;
    std::cout << "  1. Generate a random string T (length uniformly random from 1 to max_length)" << std::endl;
    std::cout << "  2. Build with dynRLSLP::DynamicRLSLPString::build_from_text(T)" << std::endl;
    std::cout << "  3. If T.size() == 0, stop verification immediately" << std::endl;
    std::cout << "  4. Select random position i (0 <= i < T.size()) " << num_deletes_per_string << " times" << std::endl;
    std::cout << "  5. Each time, run ds.delete_substring(i) and T.erase(T.begin() + i)" << std::endl;
    std::cout << "  6. Verify ds.get_text_str() == T_prime (T converted to string)" << std::endl;
    std::cout << std::endl;

    std::mt19937_64 mt(seed);
    std::uniform_int_distribution<size_t> char_index_dist(0, alphabet.size() - 1);
    std::uniform_int_distribution<uint64_t> length_dist(1, max_length);

    int passed_tests = 0;
    int failed_tests = 0;
    uint64_t last_length = 0; // For progress display

    for (uint64_t trial = 0; trial < num_trials; trial++)
    {
        // Generate random string T
        uint64_t length = length_dist(mt);
        last_length = length; // Save for progress display
        std::vector<uint8_t> T;
        for (uint64_t i = 0; i < length; i++)
        {
            size_t char_index = char_index_dist(mt);
            T.push_back(alphabet[char_index]);
        }

        //try
        //{
            auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;

            // Build DynamicString
            dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::offline_build_from_text(T, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);

            // Stop verification if T.size() == 0
            if (T.size() == 0)
            {
                continue; // Next trial
            }

            // Run num_deletes_per_string delete operations for each string T
            for (uint64_t delete_op = 0; delete_op < num_deletes_per_string; delete_op++)
            {
                // Stop verification if T.size() == 0
                if (T.size() == 0)
                {
                    break; // Stop processing this string
                }

                uint64_t minLen = std::min(100ULL, static_cast<uint64_t>(T.size()));

                std::uniform_int_distribution<int64_t> len_dist(0, minLen);
                int64_t len = len_dist(mt);

                // Select random position i (int64_t in range 0 to T.size()-1)
                std::uniform_int_distribution<int64_t> pos_dist(0, static_cast<int64_t>(T.size() - len));
                int64_t i = pos_dist(mt);

                // Update ds with ds.delete_substring(i)
                ds.delete_substring(i, len);

                // Delete character at position i in T
                T.erase(T.begin() + i, T.begin() + i + len);

                // Convert std::vector<uint8_t> T to std::string T_prime
                std::string T_prime(T.begin(), T.end());

                // Verify ds.get_text_str() == T_prime
                std::string ds_str = ds.get_text_str();

                if (ds_str == T_prime)
                {
                    passed_tests++;
                }
                else
                {
                    failed_tests++;
                    std::cout << "✗ Test " << (trial + 1) << " delete operation " << (delete_op + 1) << " failed (position i: " << i << ")" << std::endl;
                    std::cout << "  Expected: \"" << T_prime << "\"" << std::endl;
                    std::cout << "  Actual: \"" << ds_str << "\"" << std::endl;
                    std::cout << "  Expected length: " << T_prime.size() << std::endl;
                    std::cout << "  Actual length: " << ds_str.size() << std::endl;

                    // Show details for only the first 10 failures
                    if (failed_tests <= 10)
                    {
                        // Also show T before deletion
                        std::vector<uint8_t> T_before_delete = T;
                        T_before_delete.insert(T_before_delete.begin() + i, 0); // Reconstruct state before deletion (character unknown)
                        std::string T_before_delete_str(T_before_delete.begin(), T_before_delete.end());
                        if (T_before_delete_str.size() <= 100)
                        {
                            std::cout << "  String T before deletion: (delete character at position " << i << ")" << std::endl;
                        }
                        else
                        {
                            std::cout << "  String T before deletion: (length " << (T_before_delete_str.size()) << ", position " << i << ")" << std::endl;
                        }
                    }
                }
            }
            ds.verify_string();
            /*
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
        */

        // Progress report (every 100 trials)
        if ((trial + 1) % 100 == 0)
        {
            uint64_t total_operations = (trial + 1) * num_deletes_per_string;
            std::cout << "Progress: " << (trial + 1) << "/" << num_trials << " strings (length: " << last_length << ", total operations: " << total_operations << ", passed: " << passed_tests << ", failed: " << failed_tests << ")" << std::endl;
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
    // Show test description
    std::cout << "========================================" << std::endl;
    std::cout << "Test for dynRLSLP::DynamicRLSLPString::delete_substring()" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "[Test purpose]" << std::endl;
    std::cout << "For a DynamicString built from string T," << std::endl;
    std::cout << "After delete_substring, verify ds.get_text_str() == T_prime" << std::endl;
    std::cout << "holds for random position i." << std::endl;
    std::cout << std::endl;
    std::cout << "[Test procedure]" << std::endl;
    std::cout << "1. Generate a random string T (length uniformly random from 1 to max_length)" << std::endl;
    std::cout << "2. Build a DynamicString instance from string T" << std::endl;
    std::cout << "3. If T.size() == 0, stop verification immediately" << std::endl;
    std::cout << "4. Select random position i (0 <= i < T.size()) for the specified number of operations" << std::endl;
    std::cout << "5. Each time, run ds.delete_substring(i) and T.erase(T.begin() + i)" << std::endl;
    std::cout << "6. Verify ds.get_text_str() == T_prime (T converted to string)" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Default values
    bool use_restricted_block_compression = false;
    int64_t seed = 42;
    uint64_t alphabet_type = 0; // 0 means unspecified (test all types)
    dynRLSLP::DictionaryMode mode = dynRLSLP::DictionaryMode::Standard;
    uint64_t num_trials = 1000;
    uint64_t num_deletes_per_string = 10;
    uint64_t max_length = 10000;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--use-restricted" || arg == "-r" || arg == "--x")
        {
            // Check if a value follows -r
            if (i + 1 < argc)
            {
                std::string next_arg = argv[i + 1];
                if (next_arg == "true" || next_arg == "false")
                {
                    use_restricted_block_compression = (next_arg == "true");
                    i++; // Skip next argument
                }
                else
                {
                    // Default to true when no value is specified
                    use_restricted_block_compression = true;
                }
            }
            else
            {
                // Default to true when no value is specified
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
                    // Generate random seed
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
        else if (arg == "--deletes-per-string" || arg == "-d")
        {
            if (i + 1 < argc)
            {
                num_deletes_per_string = std::stoull(argv[++i]);
            }
            else
            {
                std::cerr << "Error: --deletes-per-string option requires a value" << std::endl;
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
            std::cout << "  --alphabet-type, -a, --type <value> Alphabet type (test all types if omitted)" << std::endl;
            std::cout << "                                1: Σ={a, b}" << std::endl;
            std::cout << "                                2: Σ={A, C, G, T}" << std::endl;
            std::cout << "                                3: Σ={0,1,2,3,4,5,6,7,8,9}" << std::endl;
            std::cout << "                                4: Σ={a-z}" << std::endl;
            std::cout << "  --trials, -t <value>          Number of strings T to generate (default: 1000)" << std::endl;
            std::cout << "  --deletes-per-string, -d <value> Number of delete operations per string T (default: 10)" << std::endl;
            std::cout << "  --max-length <value>          Maximum string length to generate (default: 10000)" << std::endl;
            std::cout << "  --mode, -m <value>            DictionaryMode (default: standard)" << std::endl;
            std::cout << "                                standard: Standard mode" << std::endl;
            std::cout << "                                lightweight: Lightweight mode" << std::endl;
            std::cout << "                                fast: Fast mode" << std::endl;
            std::cout << "  --help, -h                    Show this help" << std::endl;
            std::cout << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --alphabet-type 2" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --trials 500 --deletes-per-string 20" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --alphabet-type 2 --trials 500 --max-length 5000" << std::endl;
            std::cout << "  " << argv[0] << " --use-restricted --seed 456 --alphabet-type 4" << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "Warning: ignoring unknown option '" << arg << "'" << std::endl;
        }
    }

    // Show effective configuration
    std::cout << "========================================" << std::endl;
    std::cout << "Test configuration" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "use_restricted_block_compression: " << (use_restricted_block_compression ? "true" : "false") << std::endl;
    std::string mode_str = (mode == dynRLSLP::DictionaryMode::Standard) ? "standard" : (mode == dynRLSLP::DictionaryMode::Lightweight) ? "lightweight"
                                                                                                                                           : "fast";
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
    std::cout << "num_deletes_per_string: " << num_deletes_per_string << std::endl;
    std::cout << "max_length: " << max_length << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Run tests
    bool all_success = true;

    if (alphabet_type == 0)
    {
        // Test all alphabet types when none is specified
        std::cout << "Alphabet type not specified; testing all types (1-4)." << std::endl;
        std::cout << std::endl;

        for (uint64_t type = 1; type <= 4; type++)
        {
            std::cout << "========================================" << std::endl;
            std::cout << "Starting tests for alphabet type " << type << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << std::endl;

            bool success = delete_test(use_restricted_block_compression, seed, type, mode, num_trials, num_deletes_per_string, max_length);

            if (!success)
            {
                all_success = false;
                if (!use_restricted_block_compression)
                {
                    // Exit with error on failure outside restricted_block_compression mode
                    return 1;
                }
            }

            std::cout << std::endl;
        }
    }
    else
    {
        // Test only the specified alphabet type
        all_success = delete_test(use_restricted_block_compression, seed, alphabet_type, mode, num_trials, num_deletes_per_string, max_length);

        if (!all_success && !use_restricted_block_compression)
        {
            // Exit with error on failure outside restricted_block_compression mode
            return 1;
        }
    }

    return all_success ? 0 : 1;
}
