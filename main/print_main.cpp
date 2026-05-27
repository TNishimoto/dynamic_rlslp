#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/all.hpp"
#include "../include/all.hpp"

int64_t count_r(uint64_t len, const dynRLSLP::DynamicRLSLPString &ds)
{
    const std::vector<uint64_t> &explicit_nonterminal_length_list = ds.get_dictionary().get_explicit_nonterminal_length_list();
    const std::vector<dynRLSLP::RLSLPRuleBody> &explicit_nonterminal_rule_list = ds.get_dictionary().get_explicit_nonterminal_rule_list();

    uint64_t size = ds.get_dictionary().count_explicit_nonterminals();
    std::unordered_set<uint64_t> nonterminal_set_pair;
    std::unordered_set<uint64_t> nonterminal_set_power;
    for (uint64_t i = 0; i < size; i++)
    {
        dynRLSLP::RLSLPRuleBody item = dynRLSLP::RLSLPRuleBody::decode_rule(i, explicit_nonterminal_rule_list);
        if (item.get_type() == dynRLSLP::RLSLPRuleType::Pair)
        {
            dynRLSLP::NonterminalWithRelativeLevel left = item.A;
            dynRLSLP::NonterminalWithRelativeLevel right = item.B;
            uint64_t left_length = dynRLSLP::NonterminalFunctions::get_length(left, explicit_nonterminal_length_list);
            uint64_t right_length = dynRLSLP::NonterminalFunctions::get_length(right, explicit_nonterminal_length_list);
            if (left_length < len && left_length + right_length <= len)
            {
                nonterminal_set_pair.insert(i);
            }
        }
        else if (item.get_type() == dynRLSLP::RLSLPRuleType::Power)
        {
            dynRLSLP::NonterminalWithRelativeLevel child = item.A;
            uint64_t child_length = dynRLSLP::NonterminalFunctions::get_length(child, explicit_nonterminal_length_list);
            uint64_t power = item.B;
            uint64_t power_length = child_length * power;
            if (child_length < len && power_length <= len)
            {
                nonterminal_set_power.insert(child);
            }
        }
    }
    return nonterminal_set_pair.size() + nonterminal_set_power.size();
}

int main(int argc, char *argv[])
{
    cmdline::parser p;

    p.add<std::string>("input_file_path", 'i', "The file path to a dynamic data structure storing an RLSLP", true);
    p.add<std::string>("output_dynamic_data_structure_file_path", 'j', "File path for writing the given dynamic data structure in JSON format", false, "");
    p.add<std::string>("output_rlslp_file_path", 'r', "File path for writing the RLSLP in JSON format", false, "");
    p.add<std::string>("output_derivation_tree_file_path", 'd', "File path for writing the derivation tree of the RLSLP", false, "");
    p.add<std::string>("output_canonized_rlslp_file_path", 'c', "File path for writing the canonized RLSLP corresponding to the RLSLP in JSON format", false, "");

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file_path");
    std::string output_dynamic_data_structure_file_path = p.get<std::string>("output_dynamic_data_structure_file_path");
    std::string output_derivation_tree_file_path = p.get<std::string>("output_derivation_tree_file_path");
    std::string output_canonized_rlslp_file_path = p.get<std::string>("output_canonized_rlslp_file_path");
    std::string output_rlslp_file_path = p.get<std::string>("output_rlslp_file_path");

    if (!std::filesystem::exists(input_file_path))
    {
        std::cerr << "Error: The specified input file does not exist: " << input_file_path << std::endl;
        return 1;
    }

    // std::string output_file_path = p.get<std::string>("output_index_path");

    std::cout << "Loading the dynamic string from the file..." << std::endl;
    std::ifstream ifs(input_file_path, std::ios::binary);
    dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::load_from_file(ifs);
    ifs.close();
    std::cout << "[DONE]" << std::endl;

    ds.print_statistics(0);
    std::cout << std::endl;
    ds.print_memory_breakdown(0);


    if (output_dynamic_data_structure_file_path.size() > 0)
    {
        std::ofstream ofs(output_dynamic_data_structure_file_path);
        ds.write_content_as_json_format(ofs);
        ofs.close();
    }
    else
    {
        std::cout << "No output file path for the dynamic data structure is specified. Skipping..." << std::endl;
    }

    if (output_derivation_tree_file_path.size() > 0)
    {
        std::ofstream ofs_derivation_tree(output_derivation_tree_file_path);
        std::cout << "Writing the derivation tree to the file...: " << output_derivation_tree_file_path << std::endl;
        std::vector<std::string> derivation_tree = ds.get_derivation_tree_as_plain_text();
        for (auto it : derivation_tree)
        {
            ofs_derivation_tree << it << std::endl;
        }
        ofs_derivation_tree.close();
    }
    else
    {
        std::cout << "No output file path for the derivation tree is specified. Skipping..." << std::endl;
    }

    if (output_canonized_rlslp_file_path.size() > 0)
    {
        ds.convert_to_canonized_rlslp().write_to_file_as_json(output_canonized_rlslp_file_path);
    }
    else
    {
        std::cout << "No output file path for the canonized RLSLP is specified. Skipping..." << std::endl;
    }

    if (output_rlslp_file_path.size() > 0)
    {
        ds.convert_to_rlslp().write_to_file_as_json(output_rlslp_file_path);
    }
    else
    {
        std::cout << "No output file path for the RLSLP is specified. Skipping..." << std::endl;
    }
}
