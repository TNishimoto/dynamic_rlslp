#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include <stack>
#include <list>
#include "../rules/all.hpp"
#include "stool/include/all.hpp"

namespace dynRLSLP
{
    /**
     * @brief XXXXXXXXXXXXX
     * @ingroup RLSLPClasses
     */
    class DerivationTreeVisualizer
    {
    private:
        /*
        struct SignatureForVisualization{
            SignatureWithRelativeLevel sig;
            char padding_char;

            SignatureForVisualization(){

            }
            SignatureForVisualization(SignatureWithRelativeLevel sig, char padding_char) : sig(sig), padding_char(padding_char){

            }
        };
        */

    public:
        static std::vector<SignatureWithRelativeLevel> compute_level_sequence(const std::vector<SignatureWithRelativeLevel> &items,
                                                                              uint64_t target_level, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint16_t> &base_signature_level_list, const std::vector<uint64_t> &base_signature_length_list)
        {
            bool created = true;
            for (auto sig : items)
            {
                uint64_t height = SignatureFunctions::get_level(sig, base_signature_level_list);
                created = created && (height <= target_level);
            }
            if (created)
            {
                std::vector<SignatureWithRelativeLevel> tmp;
                for (auto sig : items)
                {
                    tmp.push_back(sig);
                }
                return tmp;
            }
            else
            {
                std::vector<SignatureWithRelativeLevel> tmp;
                for (auto sig : items)
                {
                    uint64_t height = SignatureFunctions::get_level(sig, base_signature_level_list);
                    if (height > target_level)
                    {
                        RLSLPRuleType type = (RLSLPRuleType)RLSLPRuleBody::decodeRule(sig, base_signature_rule_list).type;
                        if (type == RLSLPRuleType::Pair)
                        {
                            SignatureWithRelativeLevel left = RLSLPRuleBody::decodeRule(sig, base_signature_rule_list).A;
                            SignatureWithRelativeLevel right = RLSLPRuleBody::decodeRule(sig, base_signature_rule_list).B;
                            tmp.push_back(left);
                            tmp.push_back(right);
                        }
                        else if (type == RLSLPRuleType::Power)
                        {
                            for (int64_t i = 0; i < RLSLPRuleBody::decodeRule(sig, base_signature_rule_list).B; i++)
                            {
                                tmp.push_back(RLSLPRuleBody::decodeRule(sig, base_signature_rule_list).A);
                            }
                        }
                        else if (type == RLSLPRuleType::Signature)
                        {
                            tmp.push_back(RLSLPRuleBody::decodeRule(sig, base_signature_rule_list).A);
                        }
                        else
                        {
                            throw "error in extract function";
                        }
                    }
                    else
                    {
                        tmp.push_back(sig);
                    }
                }
                return compute_level_sequence(tmp, target_level, base_signature_rule_list, base_signature_level_list, base_signature_length_list);
            }
        }
        static void compute_derivation_tree_sub(const std::vector<SignatureWithRelativeLevel> &items, uint64_t target_level, const std::vector<RLSLPRuleBody> &base_signature_rule_list,
                                                const std::vector<uint16_t> &base_signature_level_list, const std::vector<uint64_t> &base_signature_length_list, uint64_t padding, std::vector<std::string> &output_strings)
        {
            std::vector<SignatureWithRelativeLevel> next_items = compute_level_sequence(items, target_level, base_signature_rule_list, base_signature_level_list, base_signature_length_list);
            stool::DebugPrinter::print_integers(next_items, std::to_string(target_level));
            if (target_level > 0)
            {
                compute_derivation_tree_sub(next_items, target_level - 1, base_signature_rule_list, base_signature_level_list, base_signature_length_list, padding, output_strings);

                uint64_t pos = 0;
                uint64_t total_length = 0;
                for (uint64_t i = 0; i < next_items.size(); i++)
                {
                    uint64_t height = SignatureFunctions::get_level(next_items[i], base_signature_level_list);
                    uint64_t length = SignatureFunctions::get_length(next_items[i], base_signature_length_list);
                    if (height < target_level)
                    {
                        for (uint64_t j = 0; j < length; j++)
                        {
                            output_strings[pos].push_back('*');
                            pos++;
                        }
                    }
                    else if (height == target_level)
                    {
                        output_strings[pos].push_back('-');
                        output_strings[pos].append(std::to_string(next_items[i]));
                        pos++;
                        for (uint64_t j = 1; j < length; j++)
                        {
                            output_strings[pos].push_back('-');
                            pos++;
                        }
                    }
                    else
                    {
                        for (uint64_t j = 0; j < length; j++)
                        {
                            output_strings[pos].push_back('=');
                            pos++;
                        }
                    }
                }
                total_length = pos;
                uint64_t max_length = 0;
                for (uint64_t i = 0; i < total_length; i++)
                {
                    max_length = std::max(max_length, (uint64_t)output_strings[i].size());
                }
                for (uint64_t i = 0; i < total_length; i++)
                {
                    while (output_strings[i].size() < max_length + padding)
                    {
                        output_strings[i].append("_");
                    }
                }
            }
            else
            {
                uint64_t max_length = 0;
                for (uint64_t i = 0; i < next_items.size(); i++)
                {
                    RLSLPRuleBody body = RLSLPRuleBody::decodeRule(next_items[i], base_signature_rule_list);
                    assert(body.get_type() == RLSLPRuleType::Character);
                    char c = (char)body.A;
                    output_strings[i].push_back(c);
                    output_strings[i].append(": ");
                    output_strings[i].append(std::to_string(next_items[i]));
                    max_length = std::max(max_length, (uint64_t)output_strings[i].size());
                }

                for (uint64_t i = 0; i < next_items.size(); i++)
                {
                    while (output_strings[i].size() < max_length + padding)
                    {
                        output_strings[i].append("_");
                    }
                }
            }
        }

        static std::vector<std::string> compute_derivation_tree(const std::vector<SignatureWithRelativeLevel> &items, const std::vector<RLSLPRuleBody> &base_signature_rule_list,
                                                                const std::vector<uint16_t> &base_signature_level_list, const std::vector<uint64_t> &base_signature_length_list, uint64_t padding = 3)
        {
            uint64_t max_level = 0;
            uint64_t length = 0;
            for (auto sig : items)
            {
                max_level = std::max(max_level, (uint64_t)SignatureFunctions::get_level(sig, base_signature_level_list));
                length += SignatureFunctions::get_length(sig, base_signature_length_list);
            }
            std::vector<std::string> output_strings;
            output_strings.resize(length);
            compute_derivation_tree_sub(items, max_level, base_signature_rule_list, base_signature_level_list, base_signature_length_list, padding, output_strings);
            return output_strings;
        }
    };

}
