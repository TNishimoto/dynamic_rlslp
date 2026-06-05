#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "../../rlslp/static_operations/run_rule_vector.hpp"
#include "../../json_helper.hpp"


namespace dynRLSLP
{
        class NaiveParentQuery
        {
            public:

            static std::vector<uint64_t> get_all_occurrences(ExplicitNonterminal explicit_nonterminal, 
                const ExplicitNonterminal root,
                const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, const std::vector<uint64_t> &explicit_nonterminal_length_list)
            {
                std::vector<uint64_t> output;
                get_all_occurrences_recursive(explicit_nonterminal, root, 0, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);

                std::sort(output.begin(), output.end());
                return output;
            }

            private: 

            static void get_all_occurrences_recursive(ExplicitNonterminal explicit_nonterminal, ExplicitNonterminal current_nonterminal, 
                uint64_t current_offset, 
                const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, const std::vector<uint64_t> &explicit_nonterminal_length_list, std::vector<uint64_t> &output)
            {
                RLSLPRuleBody rule = RLSLPRuleBody::decode_rule(current_nonterminal, explicit_nonterminal_rule_list);
                if(current_nonterminal == explicit_nonterminal){
                    output.push_back(current_offset);
                }


                if(rule.get_type() == RLSLPRuleType::Pair){
                    ExplicitNonterminal left_explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(rule.A);
                    ExplicitNonterminal right_explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(rule.B);
                    uint64_t left_length = NonterminalFunctions::get_length(rule.A, explicit_nonterminal_length_list);
                    get_all_occurrences_recursive(explicit_nonterminal, left_explicit_nonterminal, current_offset, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                    get_all_occurrences_recursive(explicit_nonterminal, right_explicit_nonterminal, current_offset + left_length, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);

                }
                else if(rule.get_type() == RLSLPRuleType::Power){
                    ExplicitNonterminal base_explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(rule.A);
                    uint64_t base_length = NonterminalFunctions::get_length(rule.A, explicit_nonterminal_length_list);
                    for(uint64_t i = 0; i < (uint64_t)rule.B; i++){
                        get_all_occurrences_recursive(explicit_nonterminal, base_explicit_nonterminal, current_offset + i * base_length, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                    }
                }
                else if(rule.get_type() == RLSLPRuleType::Nonterminal){
                    ExplicitNonterminal base_explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(rule.A);
                    get_all_occurrences_recursive(explicit_nonterminal, base_explicit_nonterminal, current_offset, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                }
                else if(rule.get_type() == RLSLPRuleType::Character){
                    
                }
                else{
                    throw std::runtime_error("get_all_occurrences_recursive: unknown rule type");
                }
            }
        };
}