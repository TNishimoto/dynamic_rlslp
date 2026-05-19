#pragma once
#include "stool/include/all.hpp"
#include "./many_parents_manager.hpp"
#include "./few_parents_manager.hpp"
#include "../../../types/types.hpp"

namespace dynRLSLP
{
        /**
         * @brief Static utilities for managing a small vector of parent nonterminals.
         * @ingroup ParentClasses
         */
        class ParentVectorManager
        {
        public:
            static inline uint64_t VECTOR_MAX_SIZE = 64;
            
            /**
             * @brief Pushes type-1 primary occurrences for each parent in the vector onto the stack.
             * @param sig Base nonterminal of the queried nonterminal.
             * @param position_offset Position offset of the occurrence within its parent.
             * @param parent_vector List of parent nonterminals to enumerate.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param explicit_nonterminal_length_list The derived string lengths of DictionaryForLayeredRLSLP.
             * @param output Stack receiving temporary occurrences to expand further.
             */
            static void get_all_type_1_primary_occurrences_of_nonterminal_sub(ExplicitNonterminal sig, int64_t position_offset, const std::vector<NonterminalWithRelativeLevel> &parent_vector,
                                                                            const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, const std::vector<uint64_t> &explicit_nonterminal_length_list, VStack<TemporaryOccurrence> &output)
            {
                for (auto parent : parent_vector)
                {
                    ManyParentsManager::get_all_type_1_primary_occurrences_of_nonterminal_sub(sig, position_offset, parent, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                }
            }

            /**
             * @brief Searches the parent vector for a pair rule with the given children.
             * @param left_sig Left child nonterminal.
             * @param right_sig Right child nonterminal.
             * @param parent_vector List of parent nonterminals to search.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return Parent nonterminal if found, otherwise -1.
             */
            static int64_t get_pair_nonterminal(NonterminalWithRelativeLevel left_sig, NonterminalWithRelativeLevel right_sig, const std::vector<NonterminalWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
            {
                for (auto parent : parent_vector)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.A == left_sig && parent_item.B == right_sig)
                    {
                        return parent;
                    }
                }
                return -1;
            }
            /**
             * @brief Searches the parent vector for a power rule with the given child and exponent.
             * @param child_sig Child nonterminal.
             * @param power Exponent of the power rule.
             * @param parent_vector List of parent nonterminals to search.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return Parent nonterminal if found, otherwise -1.
             */
            static int64_t get_power_nonterminal(NonterminalWithRelativeLevel child_sig, uint64_t power, const std::vector<NonterminalWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
            {
                for (auto parent : parent_vector)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && parent_item.A == child_sig && (uint64_t)parent_item.B == power)
                    {
                        return parent;
                    }
                }
                return -1;
            }

            /**
             * @brief Appends a parent nonterminal to the parent vector.
             * @param parent Parent nonterminal to insert.
             * @param parent_vector Parent vector to modify.
             */
            static void insert_nonterminal(NonterminalWithRelativeLevel parent, std::vector<NonterminalWithRelativeLevel> &parent_vector){
                parent_vector.push_back(parent);
            }
            /**
             * @brief Removes all entries equal to the given parent nonterminal from the vector.
             * @param parent Parent nonterminal to erase.
             * @param parent_vector Parent vector to modify.
             */
            static void erase_nonterminal(NonterminalWithRelativeLevel parent, std::vector<NonterminalWithRelativeLevel> &parent_vector){
                parent_vector.erase(std::remove(parent_vector.begin(), parent_vector.end(), parent), parent_vector.end());
            }

            /**
             * @brief Removes and returns one parent of the given child from the vector.
             * @param child Child nonterminal whose parent is requested.
             * @param parent_vector Parent vector to modify.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return A parent nonterminal, or EMPTY_FLAG if none matches the child.
             */
            static NonterminalWithRelativeLevel take_any_parent(NonterminalWithRelativeLevel child, std::vector<NonterminalWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list){
                int64_t idx = -1;
                for(uint64_t i = 0; i < parent_vector.size(); i++){
                    ChildType child_type = ManyParentsManager::get_child_type(child, parent_vector[i], explicit_nonterminal_rule_list, false);
                    if(child_type != ChildType::None){
                        idx = i;
                        break;
                    }
                }
                if(idx != -1){
                    NonterminalWithRelativeLevel parent = parent_vector[idx];
                    parent_vector.erase(parent_vector.begin() + idx);
                    return parent;
                }else{
                    return ManyParentsManager::EMPTY_FLAG;
                }

            }

            /*
            static uint64_t get_parent_count(NonterminalWithRelativeLevel child, const std::vector<NonterminalWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list){
                uint64_t parent_count = 0;
                for (auto parent : parent_vector)
                {
                    ChildType child_type = ManyParentsManager::get_child_type(child, parent, explicit_nonterminal_rule_list, false);
                    if (child_type != ChildType::None)
                    {
                        parent_count++;
                    }
                }
            }
            */

        };
    
}
