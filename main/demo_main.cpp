#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
// #include "qgram_tree.h"
#include "stool/include/third_party/cmdline.h"
#include "../include/all.hpp"

std::vector<std::string> parse_command(const std::string &command)
{
    std::vector<std::string> tokens;
    std::string token;
    for (char c : command)
    {
        if (c == ' ')
        {
            tokens.push_back(token);
            token = "";
        }
        else
        {
            token += c;
        }
    }
    if (token.size() > 0)
    {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char *argv[])
{
    cmdline::parser p;
    dynRLSLP::DynamicRLSLPString ds;

    {
        std::string intial_text = "cococacococacococaaaaaaaaaaad";
        dynRLSLP::DynamicRLSLPString tmp = dynRLSLP::DynamicRLSLPString::build_from_text_for_debug(intial_text);
        ds.swap(tmp);    
    }


    p.add<bool>("random_parsing", 'r', "random parsing", false, false);
    p.parse_check(argc, argv);
    //bool random_parsing = p.get<bool>("random_parsing");

    std::string command;
    std::vector<std::string> tokens;

    while (true)
    {
        /*
        std::cout << "text \t\t\t = " << text << std::endl;
        std::cout << "position \t\t = " << stool::dynamic_r_index::NaiveOperations::value_with_index(position, true) << std::endl;
        std::cout << "deletion_length \t = " << length << std::endl;
        */
        //ds.get_dictionary().print_info();
        std::cout << "text: " << ds.to_string() << std::endl;
        std::cout << "Command? (Press h to display the list of commands.)" << std::endl;

        std::getline(std::cin, command);
        tokens = parse_command(command);
        std::cout << "tokens: " << tokens.size() << std::endl;
        if (tokens.size() >= 1)
        {
            if (tokens[0] == "exit")
            {
                break;
            }
            else if (command == "h" || command == "help")
            {
                std::cout << "--------------[Command list]------------------" << std::endl;
                std::cout << "snapshot \t\t = Display the snapshot of the insertion." << std::endl;
                std::cout << "exit \t\t\t = Exit the program." << std::endl;
                std::cout << "--------------------------------" << std::endl;
                std::cout << std::endl;
            }
            else if (tokens[0] == "print")
            {
                ds.get_dictionary().print_info();
                ds.print_derivation_tree();
            }
            else if (tokens[0] == "build" && tokens.size() >= 2)
            {
                std::cout << "text: " << tokens[1] << std::endl;
                auto tmp = dynRLSLP::DynamicRLSLPString::build_from_text_for_debug(tokens[1]);
                ds.swap(tmp);
                std::cout << "Build completed." << std::endl;
            }
        }
    }
    std::cout << "Goodbye!" << std::endl;
}
