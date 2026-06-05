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
 * @brief Build a DynamicString y from a fixed-length string X and verify y.to_string() equals X
 * @param use_restricted_block_compression Whether to use restricted block compression
 * @param seed Random seed value
 * @param alphabet_type Alphabet type (1-4)
 * @param mode DictionaryMode
 * @param num_trials_per_length Number of tests per length (default: 100)
 * @return true if all tests pass, false otherwise
 */
bool build_test(bool use_restricted_block_compression, int64_t seed, uint64_t alphabet_type, dynRLSLP::DictionaryMode mode, uint64_t num_trials_per_length = 100)
{
    // Get alphabet
    std::vector<uint8_t> alphabet = get_alphabet_by_type(alphabet_type);
    
    std::cout << "========================================" << std::endl;
    std::cout << "Running DynamicString constructor test" << std::endl;
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
    std::cout << "String lengths: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Each trial performs the following checks:" << std::endl;
    std::cout << "  1. Generate a random string X (fixed length, from the specified alphabet)" << std::endl;
    std::cout << "  2. Build with dynRLSLP::DynamicRLSLPString::build_from_text(X)" << std::endl;
    std::cout << "  3. Compare dynRLSLP::DynamicRLSLPString::to_string() with X" << std::endl;
    std::cout << std::endl;

    // Fixed string lengths: 1, 2, 4, ..., 8192
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

    // Run tests for each length
    for (uint64_t length : lengths)
    {
        std::cout << "Tests for length " << length << ": " << std::flush;
        uint64_t passed_for_length = 0;
        uint64_t failed_for_length = 0;

        for (uint64_t trial = 0; trial < num_trials_per_length; trial++)
        {
            // Generate random string X (fixed length, from the specified alphabet)
            std::vector<uint8_t> X;
            for (uint64_t i = 0; i < length; i++)
            {
                size_t char_index = char_index_dist(mt);
                X.push_back(alphabet[char_index]);
            }

            try
            {
                auto parser = use_restricted_block_compression ? dynRLSLP::GrammarParsingType::RestrictedRecompression : dynRLSLP::GrammarParsingType::SignatureEncoding;
                // Build DynamicString instance y
                dynRLSLP::DynamicRLSLPString y = dynRLSLP::DynamicRLSLPString::offline_build_from_text(X, parser, alphabet, mode, seed + trial, stool::Message::NO_MESSAGE);
                // Get y.to_string()
                std::string y_string = y.to_string();
                std::string X_string = std::string(X.begin(), X.end());
                // Verify y.to_string() equals X
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
                    std::cout << "✗ Test failed (length: " << length << ", trial: " << (trial + 1) << ")" << std::endl;
                    std::cout << "  Expected length: " << X_string.size() << std::endl;
                    std::cout << "  Actual length: " << y_string.size() << std::endl;
                    if (X_string.size() <= 100 && y_string.size() <= 100)
                    {
                        std::cout << "  Expected: \"" << X_string << "\"" << std::endl;
                        std::cout << "  Actual: \"" << y_string << "\"" << std::endl;
                    }
                }
                
            }
            catch (const std::logic_error &e)
            {
                failed_tests++;
                failed_for_length++;
                std::cout << std::endl;
                std::cout << "✗ Test logic error exception (length: " << length << ", trial: " << (trial + 1) << "): " << e.what() << std::endl;
                //throw e;

            }
            catch (const std::exception &e)
            {
                failed_tests++;
                failed_for_length++;
                std::cout << std::endl;
                std::cout << "✗ Test exception (length: " << length << ", trial: " << (trial + 1) << "): " << e.what() << std::endl;
                //throw e;
            }
            catch (...)
            {
                failed_tests++;
                failed_for_length++;
                std::cout << std::endl;
                std::cout << "✗ Test unknown exception (length: " << length << ", trial: " << (trial + 1) << ")" << std::endl;
                throw std::runtime_error("Unknown exception");
            }
            
            
            total_tests++;
        }

        // Show results for each length
        if (failed_for_length == 0)
        {
            std::cout << "✓ " << passed_for_length << "/" << num_trials_per_length << " passed" << std::endl;
        }
        else
        {
            std::cout << "✗ " << passed_for_length << "/" << num_trials_per_length << " passed, " << failed_for_length << " failed" << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Test results" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    std::cout << "Total: " << total_tests << std::endl;
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
    std::cout << "DynamicString test suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "[Test purpose]" << std::endl;
    std::cout << "Build a DynamicString y from a fixed-length string X and" << std::endl;
    std::cout << "verify that y.to_string() equals X." << std::endl;
    std::cout << std::endl;
    std::cout << "[Test procedure]" << std::endl;
    std::cout << "1. Test multiple string lengths (1, 2, 4, 8, ..., 8192)" << std::endl;
    std::cout << "2. For each length, generate a random string X" << std::endl;
    std::cout << "3. Build with dynRLSLP::DynamicRLSLPString::build_from_text(X)" << std::endl;
    std::cout << "4. Compare dynRLSLP::DynamicRLSLPString::to_string() with X" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Default values
    bool use_restricted_block_compression = false;
    int64_t seed = 42;
    uint64_t alphabet_type = 0; // 0 means unspecified (test all types)
    dynRLSLP::DictionaryMode mode = dynRLSLP::DictionaryMode::Standard;
    uint64_t num_trials_per_length = 100;

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
                num_trials_per_length = std::stoull(argv[++i]);
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
            std::cout << "  --trials, -t <value>          Number of tests per length (default: 100)" << std::endl;
            std::cout << "  --mode, -m <value>            DictionaryMode (default: standard)" << std::endl;
            std::cout << "                                standard: Standard mode" << std::endl;
            std::cout << "                                lightweight: Lightweight mode" << std::endl;
            std::cout << "                                fast: Fast mode" << std::endl;
            std::cout << "  --help, -h                    Show this help" << std::endl;
            std::cout << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --alphabet-type 2" << std::endl;
            std::cout << "  " << argv[0] << " --use-restricted --seed 456 --alphabet-type 4" << std::endl;
            std::cout << "  " << argv[0] << " --seed 123 --trials 50 --alphabet-type 3" << std::endl;
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
    std::cout << "num_trials_per_length: " << num_trials_per_length << std::endl;
    std::cout << "String lengths: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Run tests
    bool all_success = true;
    
    // Run build_test
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
                
                bool success = build_test(use_restricted_block_compression, seed, type, mode, num_trials_per_length);
                
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
            all_success = build_test(use_restricted_block_compression, seed, alphabet_type, mode, num_trials_per_length);
            
            if (!all_success && !use_restricted_block_compression)
            {
                // Exit with error on failure outside restricted_block_compression mode
                return 1;
            }
        }

    return all_success ? 0 : 1;
}

