#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/all.hpp"
#include "../include/all.hpp"

int64_t count_r(uint64_t len, const dynRLSLP::DynamicRLSLPString &ds){
    const std::vector<uint64_t>& explicit_nonterminal_length_list = ds.get_dictionary().get_explicit_nonterminal_length_list();
    const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list = ds.get_dictionary().get_explicit_nonterminal_rule_list();

    uint64_t size = ds.get_dictionary().count_explicit_nonterminals();
    std::unordered_set<uint64_t> nonterminal_set_pair;
    std::unordered_set<uint64_t> nonterminal_set_power;
    for(uint64_t i = 0; i < size; i++){
        dynRLSLP::RLSLPRuleBody item = dynRLSLP::RLSLPRuleBody::decode_rule(i, explicit_nonterminal_rule_list);
        if(item.get_type() == dynRLSLP::RLSLPRuleType::Pair){
            dynRLSLP::NonterminalWithRelativeLevel left = item.A;
                dynRLSLP::NonterminalWithRelativeLevel right = item.B;
                uint64_t left_length = dynRLSLP::NonterminalFunctions::get_length(left, explicit_nonterminal_length_list);
            uint64_t right_length = dynRLSLP::NonterminalFunctions::get_length(right, explicit_nonterminal_length_list);
            if(left_length < len && left_length + right_length <= len){
                nonterminal_set_pair.insert(i);
            }
        }
        else if(item.get_type() == dynRLSLP::RLSLPRuleType::Power){            
            dynRLSLP::NonterminalWithRelativeLevel child = item.A;
            uint64_t child_length = dynRLSLP::NonterminalFunctions::get_length(child, explicit_nonterminal_length_list);
            uint64_t power = item.B;
            uint64_t power_length = child_length * power;
            if(child_length < len && power_length <= len){
                nonterminal_set_power.insert(child);
            }
        }
    }
    return nonterminal_set_pair.size() + nonterminal_set_power.size();
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

    p.add<std::string>("input_file_path", 'i', "The file path to an input text", true);
    p.add<std::string>("output_json_file_path", 'j', "The file path to an output JSON file", false, "");
    p.add<std::string>("output_derivation_tree_file_path", 'd', "The file path to an output derivation tree file", false, "");
    p.add<std::string>("output_canonized_rlslp_file_path", 'c', "The file path to an output static rlslp file", false, "");
    p.add<std::string>("output_rlslp_file_path", 'r', "The file path to an output static rlslp file", false, "");

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file_path");
    std::string output_json_file_path = p.get<std::string>("output_json_file_path");
    std::string output_derivation_tree_file_path = p.get<std::string>("output_derivation_tree_file_path");
    std::string output_canonized_rlslp_file_path = p.get<std::string>("output_canonized_rlslp_file_path");
    std::string output_rlslp_file_path = p.get<std::string>("output_rlslp_file_path");

    if (!std::filesystem::exists(input_file_path))
    {
        std::cerr << "Error: The specified input file does not exist: " << input_file_path << std::endl;
        return 1;
    }

    // std::string output_file_path = p.get<std::string>("output_index_path");

    std::cout << "Loading the dynamic string from the file..." << std::flush;
    std::ifstream ifs(input_file_path, std::ios::binary);
    dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::load_from_file(ifs);
ifs.close();
    std::cout << "Loading the dynamic string from the file...[DONE]" << std::endl;

    if(output_json_file_path.size() > 0){
        std::ofstream ofs(output_json_file_path);
        ds.write_content_as_json_format(ofs);
        ofs.close();
    
    }

    if(output_derivation_tree_file_path.size() > 0){
        std::ofstream ofs_derivation_tree(output_derivation_tree_file_path);
        std::cout << "Writing the derivation tree to the file...: " << output_derivation_tree_file_path << std::endl;
        std::vector<std::string> derivation_tree = ds.get_derivation_tree_as_plain_text();
        for(auto it : derivation_tree){
            ofs_derivation_tree << it << std::endl;
        }
        ofs_derivation_tree.close();    
    
    }



    if(output_canonized_rlslp_file_path.size() > 0){
        ds.convert_to_canonized_rlslp().write_to_file_as_json(output_canonized_rlslp_file_path);
    }

    if(output_rlslp_file_path.size() > 0){
        ds.convert_to_rlslp().write_to_file_as_json(output_rlslp_file_path);
    }








    /*

    ds.get_dictionary().print_statistics(1);

    std::cout << "The number of R explicit_nonterminal_rule_list(1):\t" << count_r(1, ds) << std::endl;
    std::cout << "The number of R explicit_nonterminal_rule_list(10):\t" << count_r(10, ds) << std::endl;
    std::cout << "The number of R explicit_nonterminal_rule_list(100):\t" << count_r(100, ds) << std::endl;
    std::cout << "The number of R explicit_nonterminal_rule_list(1000):\t" << count_r(1000, ds) << std::endl;
    std::cout << "The number of R explicit_nonterminal_rule_list(10000):\t" << count_r(10000, ds) << std::endl;



    const dynRLSLP::DynamicGrammarForLayeredRLSLP& dynamic_grammar = ds.get_dictionary();
    const dynRLSLP::DictionaryForLayeredRLSLP& dictionary = dynamic_grammar.get_dictionary();
    dictionary.print_detailed_statistics(1);
    */



}
