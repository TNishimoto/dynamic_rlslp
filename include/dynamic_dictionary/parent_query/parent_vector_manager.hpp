#pragma once
#include "stool/include/all.hpp"
#include "../../rlslp/run_rule_vector.hpp"
#include "../../rlslp/nonterminal_less_comparer.hpp"
#include "./many_parents_manager.hpp"
#include "./few_parents_manager.hpp"
#include "../../types/types.hpp"

namespace dynRLSLP
{
        class ParentVectorManager
        {
        public:
            static inline uint64_t VECTOR_MAX_SIZE = 64;
            
            static void get_all_type_1_primary_occurrences_of_signature_sub(BaseSignature sig, int64_t position_offset, const std::vector<SignatureWithRelativeLevel> &parent_vector,
                                                                            const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list, VStack<TemporaryOccurrence> &output)
            {
                for (auto parent : parent_vector)
                {
                    ManyParentsManager::get_all_type_1_primary_occurrences_of_signature_sub(sig, position_offset, parent, base_signature_rule_list, base_signature_length_list, output);
                }
            }

            static int64_t get_pair_signature(SignatureWithRelativeLevel left_sig, SignatureWithRelativeLevel right_sig, const std::vector<SignatureWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {
                for (auto parent : parent_vector)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.A == left_sig && parent_item.B == right_sig)
                    {
                        return parent;
                    }
                }
                return -1;
            }
            static int64_t get_power_signature(SignatureWithRelativeLevel child_sig, uint64_t power, const std::vector<SignatureWithRelativeLevel> &parent_vector, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {
                for (auto parent : parent_vector)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && parent_item.A == child_sig && (uint64_t)parent_item.B == power)
                    {
                        return parent;
                    }
                }
                return -1;
            }

            static void insert_signature(SignatureWithRelativeLevel parent, std::vector<SignatureWithRelativeLevel> &parent_vector){
                parent_vector.push_back(parent);
            }
            static void erase_signature(SignatureWithRelativeLevel parent, std::vector<SignatureWithRelativeLevel> &parent_vector){
                parent_vector.erase(std::remove(parent_vector.begin(), parent_vector.end(), parent), parent_vector.end());
            }

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
