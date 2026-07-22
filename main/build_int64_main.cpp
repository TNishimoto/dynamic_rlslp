#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include "stool/include/all.hpp"
#include "../include/all.hpp"
#include "cmdline/cmdline.h"

template <typename T>
static void loadVec(std::ifstream &file, std::vector<T> &vec)
{
    file.seekg(0, std::ios::end);
    auto n = (unsigned long)file.tellg();
    file.seekg(0, std::ios::beg);

    if (n % sizeof(T) != 0)
    {
        throw std::runtime_error("Input file size is not a multiple of sizeof(T).");
    }

    vec.resize(n / sizeof(T));

    if (!vec.empty())
    {
        file.read(reinterpret_cast<char *>(vec.data()), n);
    }
    file.close();
    file.clear();
}

void main_func(std::string input_filepath, std::string output_filepath, dynRLSLP::GrammarParsingType parser, bool offline_build, dynRLSLP::DictionaryMode mode, uint64_t seed)
{
    if (!offline_build)
    {
        throw std::runtime_error("build_int64_main only supports --offline_build=1 because online_build_from_text_file reads uint8_t input.");
    }

    dynRLSLP::DynamicRLSLPString ds(parser);

    auto start = std::chrono::steady_clock::now();

    std::ifstream in;
    in.open(input_filepath, std::ios::binary);

    std::vector<int64_t> text;
    loadVec<int64_t>(in, text);
    in.close();

    if (text.empty())
    {
        throw std::runtime_error("Input file is empty.");
    }

    auto tmp_ds = dynRLSLP::DynamicRLSLPString::offline_build_from_text(text, parser, mode, seed);
    ds.swap(tmp_ds);

    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    uint64_t text_size = ds.size();

    if (output_filepath.size() > 0)
    {
        std::cout << "Writing the dictionary to the file...: " << output_filepath << std::flush;
        std::ofstream os(output_filepath, std::ios::binary);
        dynRLSLP::DynamicRLSLPString::store_to_file(ds, os);
        os.close();
        std::cout << "[Done]" << std::endl;
    }

    std::cout << "Time : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    std::cout << "Finish." << std::endl;

    std::cout << "\033[36m" << std::endl;
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "File : " << input_filepath << std::endl;
    std::cout << "Output : " << output_filepath << std::endl;
    std::cout << "Input type : int64_t" << std::endl;
    std::cout << "Build mode : Offline" << std::endl;
    ds.print_statistics(1);
    double charperms = (double)text_size / elapsed;
    std::cout << "Excecution time : " << elapsed << "ms";
    std::cout << " [" << charperms << "chars/ms]" << std::endl;
    stool::Memory::print_memory_usage();
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

int main(int argc, char *argv[])
{

    std::cout << "\033[41m";
    #ifdef RELEASE_BUILD
        std::cout << "Running in Release mode";
    #elif defined(DEBUG_BUILD)
    
        std::cout << "Running in Debug mode";
    #else
        std::cout << "Running in Unknown mode";
    #endif
        std::cout << "\e[m" << std::endl;

    cmdline::parser p;

    p.add<std::string>("input_file_path", 'i', "Input text file path", true);
    p.add<std::string>("output_file_path", 'o', "File path for writing the dynamic data structure storing an RLSLP deriving the input text", false, "");
    p.add<std::string>("parser", 'p', "Grammar_parser(restricted_recompression or signature_encoding)", false, "restricted_recompression");
    p.add<uint64_t>("seed", 's', "Seed", false, 0);
    p.add<bool>("offline_build", 'f', "Offline build mode (must be true for int64_t input)", false, true);
    p.add<std::string>("mode", 'm', "If fast mode is selected, the built data structure incldues cache for fast LCE and parent queries (fast or standard)", false, "standard");

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file_path");
    std::string output_file_path = p.get<std::string>("output_file_path");
    std::string parserStr = p.get<std::string>("parser");
    dynRLSLP::GrammarParsingType parser = dynRLSLP::GrammarParsingType::RestrictedRecompression;
    if (parserStr == "restricted_recompression")
    {
        parser = dynRLSLP::GrammarParsingType::RestrictedRecompression;
    }
    else if (parserStr == "signature_encoding")
    {
        parser = dynRLSLP::GrammarParsingType::SignatureEncoding;
    }
    else
    {
        throw std::runtime_error("Invalid parser: " + parserStr);
    }

    uint64_t seed = p.get<uint64_t>("seed");
    bool offline_build = p.get<bool>("offline_build");

    std::string modeStr = p.get<std::string>("mode");
    dynRLSLP::DictionaryMode mode = dynRLSLP::DictionaryMode::Standard;
    if (modeStr == "standard")
    {
        mode = dynRLSLP::DictionaryMode::Standard;
    }
    else if (modeStr == "lightweight")
    {
        mode = dynRLSLP::DictionaryMode::Lightweight;
    }
    else if (modeStr == "fast")
    {
        mode = dynRLSLP::DictionaryMode::Fast;
    }

    if (output_file_path.size() == 0)
    {
        if (parser == dynRLSLP::GrammarParsingType::RestrictedRecompression)
        {
            if (mode == dynRLSLP::DictionaryMode::Fast)
            {
                output_file_path = input_file_path + ".int64.rr.fa.ds";
            }
            else if (mode == dynRLSLP::DictionaryMode::Standard)
            {
                output_file_path = input_file_path + ".int64.rr.st.ds";
            }
            else
            {
                output_file_path = input_file_path + ".int64.rr.lw.ds";
            }
        }
        else
        {
            if (mode == dynRLSLP::DictionaryMode::Fast)
            {
                output_file_path = input_file_path + ".int64.se.fa.ds";
            }
            else if (mode == dynRLSLP::DictionaryMode::Standard)
            {
                output_file_path = input_file_path + ".int64.se.st.ds";
            }
            else
            {
                output_file_path = input_file_path + ".int64.se.lw.ds";
            }
        }
    }

    main_func(input_file_path, output_file_path, parser, offline_build, mode, seed);
}
