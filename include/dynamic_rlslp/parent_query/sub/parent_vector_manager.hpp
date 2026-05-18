#pragma once
#include "stool/include/all.hpp"
#include "./many_parents_manager.hpp"
#include "./few_parents_manager.hpp"
#include "../../../types/types.hpp"

namespace dynRLSLP
{
        /**
         * @brief Static utilities for managing a small vector of parent signatures.
         * @ingroup ParentClasses
         */
        class ParentVectorManager
        {
        public:
            static inline uint64_t VECTOR_MAX_SIZE = 64;
            
            /**
             * @brief Pushes type-1 primary occurrences for each parent in the vector onto the stack.
             * @param sig Base signature of the queried nonterminal.
             * @param position_offset Position offset of the occurrence within its parent.
             * @param parent_vector List of parent signatures to enumerate.
             * @param base_signature_rule_list Base-signature rule list (D).
             * @param base_signature_length_list The derived string lengths of DictionaryForLayeredRLSLP.
             * @param output Stack receiving temporary occurrences to expand further.
             */
            static void get_all_type_1_primary_occurrences_of_signature_sub(BaseSignature sig, int64_t position_offset, const std::vector<SignatureWithRelativeLevel> &parent_vector,
                                                                            const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list, VStack<TemporaryOccurrence> &output)
            {
                for (auto parent : parent_vector)
                {
                    ManyParentsManager::get_all_type_1_primary_occurrences_of_signature_sub(sig, position_offset, parent, base_signature_rule_list, base_signature_length_list, output);
                }
            }

            /**
             * @brief Searches the parent vector for a pair rule with the given children.
             * @param left_sig Left child signature.
             * @param right_sig Right child signature.
             * @param parent_vector List of parent signatures to search.
             * @param base_signature_rule_list Base-signature rule list (D).
             * @return Parent signature if found, otherwise -1.
             */
            static int64_t get_pair_signature(SignatureWithRelativeLevel left_sig, SignatureWithRelativeLevel right_sig, const std::vector<SignatureWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {
                for (auto parent : parent_vector)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.A == left_sig && parent_item.B == right_sig)
                    {
                        return parent;
                    }
                }
                return -1;
            }
            /**
             * @brief Searches the parent vector for a power rule with the given child and exponent.
             * @param child_sig Child signature.
             * @param power Exponent of the power rule.
             * @param parent_vector List of parent signatures to search.
             * @param base_signature_rule_list Base-signature rule list (D).
             * @return Parent signature if found, otherwise -1.
             */
            static int64_t get_power_signature(SignatureWithRelativeLevel child_sig, uint64_t power, const std::vector<SignatureWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {
                for (auto parent : parent_vector)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && parent_item.A == child_sig && (uint64_t)parent_item.B == power)
                    {
                        return parent;
                    }
                }
                return -1;
            }

            /**
             * @brief Appends a parent signature to the parent vector.
             * @param parent Parent signature to insert.
             * @param parent_vector Parent vector to modify.
             */
            static void insert_signature(SignatureWithRelativeLevel parent, std::vector<SignatureWithRelativeLevel> &parent_vector){
                parent_vector.push_back(parent);
            }
            /**
             * @brief Removes all entries equal to the given parent signature from the vector.
             * @param parent Parent signature to erase.
             * @param parent_vector Parent vector to modify.
             */
            static void erase_signature(SignatureWithRelativeLevel parent, std::vector<SignatureWithRelativeLevel> &parent_vector){
                parent_vector.erase(std::remove(parent_vector.begin(), parent_vector.end(), parent), parent_vector.end());
            }

            /**
             * @brief Removes and returns one parent of the given child from the vector.
             * @param child Child signature whose parent is requested.
             * @param parent_vector Parent vector to modify.
             * @param base_signature_rule_list Base-signature rule list (D).
             * @return A parent signature, or EMPTY_FLAG if none matches the child.
             */
            static SignatureWithRelativeLevel take_any_parent(SignatureWithRelativeLevel child, std::vector<SignatureWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &base_signature_rule_list){
                int64_t idx = -1;
                for(uint64_t i = 0; i < parent_vector.size(); i++){
                    ChildType child_type = ManyParentsManager::get_child_type(child, parent_vector[i], base_signature_rule_list, false);
                    if(child_type != ChildType::None){
                        idx = i;
                        break;
                    }
                }
                if(idx != -1){
                    SignatureWithRelativeLevel parent = parent_vector[idx];
                    parent_vector.erase(parent_vector.begin() + idx);
                    return parent;
                }else{
                    return ManyParentsManager::EMPTY_FLAG;
                }

            }

            /*
            static uint64_t get_parent_count(SignatureWithRelativeLevel child, const std::vector<SignatureWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &base_signature_rule_list){
                uint64_t parent_count = 0;
                for (auto parent : parent_vector)
                {
                    ChildType child_type = ManyParentsManager::get_child_type(child, parent, base_signature_rule_list, false);
                    if (child_type != ChildType::None)
                    {
                        parent_count++;
                    }
                }
            }
            */

        };
    
}
