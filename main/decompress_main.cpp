#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
//#include "qgram_tree.h"
#include "stool/include/third_party/cmdline.h"
#include "../include/all.hpp"

//using namespace std;

template <typename C>
void main_func(std::string input_filepath, std::string output_filepath)
{

    dynRLSLP::DynamicGrammarForLayeredRLSLP dic;
    std::vector<C> text;

    /*
    dic.load(input_filepath);

    dic.decompress(text);
    */

    std::ofstream out(output_filepath, std::ios::out | std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("Cannot open the output file!");
    }


    out.write(reinterpret_cast<const char *>(&text[0]), text.size() * sizeof(C));
    out.close();
    std::cout << "Wrote: " << output_filepath << std::endl;
}

int main(int argc, char *argv[])
{
    cmdline::parser p;

    p.add<std::string>("input_file", 'i', "input file name", true);
    p.add<std::string>("char_type", 'c', "char_type", false, "int8_t");
    p.add<std::string>("output_file", 'o', "output file path", false, "");

    p.parse_check(argc, argv);
    std::string filename = p.get<std::string>("input_file");
    std::string char_type = p.get<std::string>("char_type");
    std::string outputfile = p.get<std::string>("output_file");

    if (outputfile.size() == 0)
    {
        outputfile = filename + ".txt";
    }

    std::ifstream tmpInputStream;
    tmpInputStream.open(filename, std::ios::binary);

    if (char_type == "int8_t")
    {
        main_func<int8_t>(filename, outputfile);
    }
    else if (char_type == "uint8_t")
    {
        main_func<uint8_t>(filename, outputfile);
    }
    else if (char_type == "int32_t")
    {
        main_func<int32_t>(filename, outputfile);
    }
    else if (char_type == "int64_t")
    {
        main_func<int64_t>(filename, outputfile);
    }
    else
    {
        std::cout << "Error: " << char_type << std::endl;
    }
}
