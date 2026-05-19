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
     * @brief Utility for visualizing RLSLP derivation trees.
     * @ingroup RLSLPClasses
     */
    class DerivationTreeVisualizer
    {

        /**
         * @brief Recursively expand nonterminals down to a target level.
         * @param items Sequence of run rules or integers.
         * @param target_level Target derivation level.
         * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
         * @param explicit_nonterminal_level_list Base-nonterminal level list (H).
         * @param explicit_nonterminal_length_list Base-nonterminal length list (L).
         * @return Resulting vector.
         */
        static std::vector<NonterminalWithRelativeLevel> compute_level_sequence(const std::vector<NonterminalWithRelativeLevel> &items,
                                                                              uint64_t target_level, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, const std::vector<uint16_t> &explicit_nonterminal_level_list, const std::vector<uint64_t> &explicit_nonterminal_length_list)
        {
            bool created = true;
            for (auto sig : items)
            {
                uint64_t height = NonterminalFunctions::get_level(sig, explicit_nonterminal_level_list);
                created = created && (height <= target_level);
            }
            if (created)
            {
                std::vector<NonterminalWithRelativeLevel> tmp;
                for (auto sig : items)
                {
                    tmp.push_back(sig);
                }
                return tmp;
            }
            else
            {
                std::vector<NonterminalWithRelativeLevel> tmp;
                for (auto sig : items)
                {
                    uint64_t height = NonterminalFunctions::get_level(sig, explicit_nonterminal_level_list);
                    if (height > target_level)
                    {
                        RLSLPRuleType type = (RLSLPRuleType)RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list).type;
                        if (type == RLSLPRuleType::Pair)
                        {
                            NonterminalWithRelativeLevel left = RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list).A;
                            NonterminalWithRelativeLevel right = RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list).B;
                            tmp.push_back(left);
                            tmp.push_back(right);
                        }
                        else if (type == RLSLPRuleType::Power)
                        {
                            for (int64_t i = 0; i < RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list).B; i++)
                            {
                                tmp.push_back(RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list).A);
                            }
                        }
                        else if (type == RLSLPRuleType::Nonterminal)
                        {
                            tmp.push_back(RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list).A);
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
                return compute_level_sequence(tmp, target_level, explicit_nonterminal_rule_list, explicit_nonterminal_level_list, explicit_nonterminal_length_list);
            }
        }
        public:

        /**
         * @brief Recursively build derivation-tree visualization lines.
         * @param items Sequence of nonterminals.
         * @param target_level Target derivation level.
         * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
         * @param explicit_nonterminal_level_list Base-nonterminal level list (H).
         * @param explicit_nonterminal_length_list Base-nonterminal length list (L).
         * @param padding Padding width for visualization.
         * @param output_strings Output lines for the visualization.
         */
        static void compute_derivation_tree_sub(const std::vector<NonterminalWithRelativeLevel> &items, uint64_t target_level, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list,
                                                const std::vector<uint16_t> &explicit_nonterminal_level_list, const std::vector<uint64_t> &explicit_nonterminal_length_list, uint64_t padding, std::vector<std::string> &output_strings)
        {
            std::vector<NonterminalWithRelativeLevel> next_items = compute_level_sequence(items, target_level, explicit_nonterminal_rule_list, explicit_nonterminal_level_list, explicit_nonterminal_length_list);
            stool::DebugPrinter::print_integers(next_items, std::to_string(target_level));
            if (target_level > 0)
            {
                compute_derivation_tree_sub(next_items, target_level - 1, explicit_nonterminal_rule_list, explicit_nonterminal_level_list, explicit_nonterminal_length_list, padding, output_strings);

                uint64_t pos = 0;
                uint64_t total_length = 0;
                for (uint64_t i = 0; i < next_items.size(); i++)
                {
                    uint64_t height = NonterminalFunctions::get_level(next_items[i], explicit_nonterminal_level_list);
                    uint64_t length = NonterminalFunctions::get_length(next_items[i], explicit_nonterminal_length_list);
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
                    RLSLPRuleBody body = RLSLPRuleBody::decode_rule(next_items[i], explicit_nonterminal_rule_list);
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

        /**
         * @brief Build a multi-line derivation-tree visualization.
         * @param items Sequence of nonterminals.
         * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
         * @param explicit_nonterminal_level_list Base-nonterminal level list (H).
         * @param explicit_nonterminal_length_list Base-nonterminal length list (L).
         * @param padding Padding width for visualization.
         * @return Resulting string.
         */
        static std::vector<std::string> compute_derivation_tree(const std::vector<NonterminalWithRelativeLevel> &items, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list,
                                                                const std::vector<uint16_t> &explicit_nonterminal_level_list, const std::vector<uint64_t> &explicit_nonterminal_length_list, uint64_t padding = 3)
        {
            uint64_t max_level = 0;
            uint64_t length = 0;
            for (auto sig : items)
            {
                max_level = std::max(max_level, (uint64_t)NonterminalFunctions::get_level(sig, explicit_nonterminal_level_list));
                length += NonterminalFunctions::get_length(sig, explicit_nonterminal_length_list);
            }
            std::vector<std::string> output_strings;
            output_strings.resize(length);
            compute_derivation_tree_sub(items, max_level, explicit_nonterminal_rule_list, explicit_nonterminal_level_list, explicit_nonterminal_length_list, padding, output_strings);
            return output_strings;
        }
    };

}
