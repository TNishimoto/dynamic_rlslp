#pragma once
#include "stool/include/all.hpp"
#include "../../rlslp/run_rule_vector.hpp"
#include "../../rlslp/nonterminal_less_comparer.hpp"
#include "./many_parents_manager.hpp"
#include "./few_parents_manager.hpp"
#include "./parent_vector_manager.hpp"
#include "../../types/types.hpp"

namespace dynRLSLP
{
        /**
         * @brief XXXXXXXX
         * @ingroup ParentClasses
         */
        class FastParentDictionary
        {
        private:
            std::vector<SignatureWithRelativeLevel> primaryParents;
            std::vector<void *> sub_pointer;
            std::vector<ManagerFlag> sub_pointer_status_;
            bool is_restricted_recompression_mode = false;
            const std::vector<uint16_t> *relative_max_level_list_ = nullptr;

        public:
            inline static const int64_t EMPTY_FLAG = INT64_MAX;
            static inline uint64_t MUDA_COUNT = 0;
            static inline uint64_t NOT_MUDA_COUNT = 0;
            bool USE_CACHE = false;

            static inline std::unordered_map<BaseSignature, TemporaryOccurrence> muda_signature_map;
            static inline std::unordered_map<BaseSignature, uint64_t> muda_signature_map2;

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            uint64_t count_registed_parents(uint64_t base_signature) const
            {
                if (base_signature >= this->primaryParents.size())
                {
                    return 0;
                }
                uint64_t parent_count = 0;
                if (this->primaryParents[base_signature] != EMPTY_FLAG)
                {
                    parent_count++;
                }
                ManagerFlag manager_flag = this->sub_pointer_status_[base_signature];
                if (manager_flag == ManagerFlag::Single)
                {
                    parent_count++;
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[base_signature];
                    parent_count += few_parents_manager->size();
                }else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[base_signature];
                    parent_count += vec->size();
                }

                return parent_count;
            }

            bool get_all_type_1_primary_occurrences_of_signature(BaseSignature sig, int64_t position_offset, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list, VStack<TemporaryOccurrence> &output) const
            {
                bool b = false;

                if (this->primaryParents[sig] != EMPTY_FLAG)
                {
                    ManyParentsManager::get_all_type_1_primary_occurrences_of_signature_sub(sig, position_offset, this->primaryParents[sig], base_signature_rule_list, base_signature_length_list, output);
                    b = true;
                }

                ManagerFlag manager_flag = this->sub_pointer_status_[sig];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<SignatureWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(sig);

                    ManyParentsManager::get_all_type_1_primary_occurrences_of_signature_sub(sig, position_offset, special_parent.first, base_signature_rule_list, base_signature_length_list, output);
                    b = true;
                }
                else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[sig];                   
                    ParentVectorManager::get_all_type_1_primary_occurrences_of_signature_sub(sig, position_offset, *vec, base_signature_rule_list, base_signature_length_list, output);
                    b = true;
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    assert(this->sub_pointer[sig] != nullptr);
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[sig];
                    bool b1 = few_parents_manager->get_all_type_1_primary_occurrences_of_signature(sig, position_offset, base_signature_rule_list, base_signature_length_list, output);
                    b = b || b1;
                }
                return b;
            }

            SignatureWithRelativeLevel take_any_important_ancestor(BaseSignature descendant) const
            {
                if(this->primaryParents[descendant] != EMPTY_FLAG){
                    return this->primaryParents[descendant];
                }else{
                    throw std::runtime_error("take_any_ancestor: the descendant is not registered");
                }
            }
            void get_all_important_ancestors(BaseSignature descendant, std::vector<SignatureWithRelativeLevel> &output) const{

                if(this->primaryParents[descendant] != EMPTY_FLAG){
                    output.push_back(this->primaryParents[descendant]);

                    ManagerFlag manager_flag = this->sub_pointer_status_[descendant];
                    if(manager_flag == ManagerFlag::Single){
                        output.push_back(this->get_special_secondary_parent(descendant).first);
                    }else if(manager_flag == ManagerFlag::FewParentsManager){
                        FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[descendant];
                        few_parents_manager->get_all_important_ancestors(output);
                    }else if(manager_flag == ManagerFlag::Vector){
                        const std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[descendant];
                        for(auto p : *vec){
                            output.push_back(p);
                        }
                    }
    
                }

            }

            bool has_single_ancestor(BaseSignature descendant, const std::vector<RLSLPRuleBody> &base_signature_rule_list) const {
                if(this->primaryParents[descendant] != EMPTY_FLAG){
                    SignatureWithRelativeLevel parent = this->primaryParents[descendant];
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                    if(parent_item.get_type() == RLSLPRuleType::Pair){
                        ManagerFlag manager_flag = this->sub_pointer_status_[descendant];
                        if(manager_flag == ManagerFlag::None){
                            return true;
                        }else{
                            return false;
                        }
                    }else{
                        return false;
                    }
                }else{
                    return false;
                }
            }
            bool is_empty(BaseSignature descendant) const {
                return this->primaryParents[descendant] == EMPTY_FLAG;
            }
 


            uint64_t count_important_ancestors(BaseSignature sig) const
            {
                uint64_t occ = 0;
                if (this->primaryParents[sig] != EMPTY_FLAG)
                {
                    occ++;
                }
                ManagerFlag manager_flag = this->sub_pointer_status_[sig];
                if (manager_flag == ManagerFlag::Single)
                {
                    occ++;
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[sig];
                    occ += few_parents_manager->size();
                }
                else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[sig];
                    occ += vec->size();
                }

                return occ;
            }

            bool has_parent(SignatureWithRelativeLevel child) const{
                BaseSignature base_signature = SignatureFunctions::get_base_signature(child);
                if(this->primaryParents[base_signature] != EMPTY_FLAG){
                    return true;
                }else{
                    uint64_t level = SignatureFunctions::get_relative_level(child);
                    return level < (*this->relative_max_level_list_)[base_signature];
                }
            }
            
            int64_t get_pair_signature(SignatureWithRelativeLevel left_sig, SignatureWithRelativeLevel right_sig, const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
            {

                uint64_t base_left_sig = SignatureFunctions::get_base_signature(left_sig);
                uint64_t level_left_sig = SignatureFunctions::get_relative_level(left_sig);

                assert(this->relative_max_level_list_ != nullptr);
                assert(base_left_sig < (*this->relative_max_level_list_).size());
                bool isTopLevel = level_left_sig == (*this->relative_max_level_list_)[base_left_sig];

                if (isTopLevel && this->primaryParents[base_left_sig] != EMPTY_FLAG)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(this->primaryParents[base_left_sig], base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.A == left_sig && parent_item.B == right_sig)
                    {
                        return this->primaryParents[base_left_sig];
                    }
                }

                ManagerFlag manager_flag = this->sub_pointer_status_[base_left_sig];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<SignatureWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(base_left_sig);
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(special_parent.first, base_signature_rule_list);

                    if (parent_item.get_type() == RLSLPRuleType::Pair && (SignatureWithRelativeLevel)parent_item.A == left_sig && (SignatureWithRelativeLevel)parent_item.B == right_sig)
                    {
                        return special_parent.first;
                    }
                    else
                    {
                        return -1;
                    }
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    assert(this->sub_pointer[base_left_sig] != nullptr);
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[base_left_sig];
                    QuaternaryKey quaternary_key = right_sig;
                    int64_t result = few_parents_manager->get_pair_signature(left_sig, right_sig, quaternary_key, base_signature_rule_list);

                    return result;
                }
                else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[base_left_sig];
                    return ParentVectorManager::get_pair_signature(left_sig, right_sig, *vec, base_signature_rule_list);
                }
                else
                {
                    return -1;
                }
            }

            int64_t get_power_signature(SignatureWithRelativeLevel child_sig, uint64_t power, const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
            {
                uint64_t base_child_sig = SignatureFunctions::get_base_signature(child_sig);
                uint64_t level_child_sig = SignatureFunctions::get_relative_level(child_sig);
                assert(this->relative_max_level_list_ != nullptr);
                assert(base_child_sig < (*this->relative_max_level_list_).size());
                bool isTopLevel = level_child_sig == (*this->relative_max_level_list_)[base_child_sig];

                if (isTopLevel && this->primaryParents[base_child_sig] != EMPTY_FLAG)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(this->primaryParents[base_child_sig], base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && parent_item.A == child_sig && (uint64_t)parent_item.B == power)
                    {
                        return this->primaryParents[base_child_sig];
                    }
                }

                ManagerFlag manager_flag = this->sub_pointer_status_[base_child_sig];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<SignatureWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(base_child_sig);
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(special_parent.first, base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && (SignatureWithRelativeLevel)parent_item.A == child_sig && (uint64_t)parent_item.B == power)
                    {
                        return special_parent.first;
                    }
                    else
                    {
                        return -1;
                    }
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    assert(this->sub_pointer[base_child_sig] != nullptr);
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[base_child_sig];

                    QuaternaryKey quaternary_key = power;
                    if (!this->is_restricted_recompression_mode)
                    {
                        quaternary_key = ((uint64_t)power) | (1ULL << 62);
                    }

                    return few_parents_manager->get_power_signature(child_sig, power, quaternary_key, base_signature_rule_list);
                }
                else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[base_child_sig];
                    return ParentVectorManager::get_power_signature(child_sig, power, *vec, base_signature_rule_list);
                }
                else
                {
                    return -1;
                }
            }


            


            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Print and verification functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            void print_tree2(uint64_t base_signature, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                uint64_t h = (*this->relative_max_level_list_)[base_signature];
                SignatureWithRelativeLevel sig = SignatureFunctions::get_signature(h, base_signature);

                if (this->primaryParents[base_signature] != EMPTY_FLAG)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << SignatureFunctions::to_string(sig) << ": " << SignatureFunctions::to_string(this->primaryParents[base_signature]) << std::endl;
                }
                else
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << SignatureFunctions::to_string(sig) << ": " << "NULL" << std::endl;
                }

                ManagerFlag manager_flag = this->sub_pointer_status_[base_signature];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<SignatureWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(base_signature);
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << SignatureFunctions::to_string(sig) << ": " << SignatureFunctions::to_string(special_parent.first) << std::endl;
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[base_signature];
                    few_parents_manager->print_tree(base_signature, message_paragraph + 1);
                }else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[base_signature];
                    stool::DebugPrinter::print_integers(*vec, "Vector");
                }
            }

            void print_tree(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== FastParentDictionary Info ====" << std::endl;
                for (size_t i = 0; i < this->primaryParents.size(); i++)
                {
                    print_tree2(i, message_paragraph + 1);
                }
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== FastParentDictionary Info[END] ====" << std::endl;
            }
            void print_statistics(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                uint64_t primary_null_count = 0;
                uint64_t secondary_list_count = 0;
                // uint64_t few_parents_manager_null_count = 0;

                uint64_t secondary_parent_list_size_sum = 0;
                uint64_t secondary_parent_list_avg = 0;
                uint64_t secondary_parent_list_size1 = 0;
                uint64_t secondary_parent_list_size2 = 0;

                uint64_t parent_vector_count = 0;
                uint64_t parent_vector_size_sum = 0;
                uint64_t parent_vector_avg = 0;
                uint64_t parent_vector_max = 0;



                uint64_t secondary_parent_list_max_size = 0;

                uint64_t tertiary_group_count = 0;
                uint64_t tertiary_group_length_sum = 0;
                uint64_t tertiary_group_length_avg = 0;
                uint64_t tertiary_group_length_max = 0;

                uint64_t tertiary_group_element_sum = 0;
                uint64_t tertiary_group_element_avg = 0;

                uint64_t quaternary_parent_group_count = 0;
                uint64_t quaternary_parent_group_length_sum = 0;
                uint64_t quaternary_parent_group_length_avg = 0;
                uint64_t quaternary_parent_group_element_sum = 0;
                uint64_t quaternary_parent_group_element_avg = 0;

                uint64_t tertiary_list_max_size = 0;
                uint64_t quaternary_list_max_size = 0;

                for (uint64_t i = 0; i < this->primaryParents.size(); i++)
                {
                    if (this->primaryParents[i] == EMPTY_FLAG)
                    {
                        primary_null_count++;
                    }
                }

                for (uint64_t i = 0; i < this->sub_pointer.size(); i++)
                {
                    ManagerFlag manager_flag = this->sub_pointer_status_[i];
                    if (manager_flag == ManagerFlag::Single)
                    {
                        // secondary_list_count++;
                    }
                    else if (manager_flag == ManagerFlag::Vector){
                        parent_vector_count++;
                        const std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[i];
                        parent_vector_size_sum += vec->size();
                        parent_vector_max = std::max<uint64_t>(parent_vector_max, vec->size());
                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        secondary_list_count++;
                        FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[i];
                        uint64_t secondary_parent_count = few_parents_manager->secondary_parent_count();
                        //uint64_t secondary_list_size = few_parents_manager->size();
                        secondary_parent_list_size_sum += secondary_parent_count;
                        secondary_parent_list_max_size = std::max(secondary_parent_list_max_size, secondary_parent_count);
                        if (secondary_parent_count == 1)
                        {
                            secondary_parent_list_size1++;
                        }
                        else if (secondary_parent_count == 2)
                        {
                            secondary_parent_list_size2++;
                        }

                        const ManyParentsManager *many_parents_manager = few_parents_manager->get_many_parents_manager();
                        if (many_parents_manager != nullptr)
                        {
                            tertiary_group_count++;
                            uint64_t tertiary_group_length = many_parents_manager->tertiary_parents_list_length();

                            tertiary_group_length_max = std::max(tertiary_group_length_max, tertiary_group_length);
                            tertiary_group_length_sum += tertiary_group_length;
                            tertiary_group_element_sum += many_parents_manager->count_tertiary_parents();
                            tertiary_list_max_size = std::max(tertiary_list_max_size, many_parents_manager->max_tertiary_parent_count());

                            if (many_parents_manager->has_quaternary_parents())
                            {
                                quaternary_parent_group_count++;
                                quaternary_parent_group_length_sum += many_parents_manager->quaternary_parents_list_length();
                                quaternary_parent_group_element_sum += many_parents_manager->count_quaternary_parents();
                                quaternary_list_max_size = std::max(quaternary_list_max_size, many_parents_manager->max_quaternary_parent_count());
                            }
                        }
                    }
                }

                secondary_parent_list_avg = secondary_parent_list_size_sum / secondary_list_count;
                tertiary_group_length_avg = tertiary_group_length_sum / tertiary_group_count;
                tertiary_group_element_avg = tertiary_group_element_sum / tertiary_group_count;
                quaternary_parent_group_length_avg = quaternary_parent_group_length_sum / quaternary_parent_group_count;
                quaternary_parent_group_element_avg = quaternary_parent_group_element_sum / quaternary_parent_group_count;
                parent_vector_avg = parent_vector_size_sum / parent_vector_count;



                uint64_t tertiary_list_avg_size = tertiary_group_element_sum / tertiary_group_length_sum;
                uint64_t quaternary_list_avg_size = quaternary_parent_group_element_sum / quaternary_parent_group_length_sum;

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(ParentDictionary):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "PrimaryList:\t\t" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|          size:\t\t" << this->primaryParents.size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|          null:\t\t" << primary_null_count << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "ParentVector: \t\t" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|              count:\t\t" << parent_vector_count << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|         total size:\t\t" << parent_vector_size_sum << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|                avg:\t\t" << parent_vector_avg << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|                max:\t\t" << parent_vector_max << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "SecondaryList: \t\t" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|              count:\t\t" << secondary_list_count << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|         total size:\t\t" << secondary_parent_list_size_sum << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|                avg:\t\t" << secondary_parent_list_avg << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|                max:\t\t" << secondary_parent_list_max_size << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|              size1:\t\t" << secondary_parent_list_size1 << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|              size2:\t\t" << secondary_parent_list_size2 << std::endl;
                // std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "   subpointer count:\t\t" << many_parents_manager_count << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "TertiaryGroup: \t\t" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|              count:\t\t" << tertiary_group_count << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|       total length:\t\t" << tertiary_group_length_sum << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|         avg length:\t\t" << tertiary_group_length_avg << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|                max:\t\t" << tertiary_group_length_max << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|      total element:\t\t" << tertiary_group_element_sum << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|        avg element:\t\t" << tertiary_group_element_avg << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "QuaternaryGroup: \t\t" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|              count:\t\t" << quaternary_parent_group_count << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|       total length:\t\t" << quaternary_parent_group_length_sum << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|         avg length:\t\t" << quaternary_parent_group_length_avg << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|      total element:\t\t" << quaternary_parent_group_element_sum << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|        avg element:\t\t" << quaternary_parent_group_element_avg << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "TertiaryList: \t\t" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|              count:\t\t" << tertiary_group_length_sum << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|           avg size:\t\t" << tertiary_list_avg_size << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|           max size:\t\t" << tertiary_list_max_size << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "QuaternaryList: \t\t" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|              count:\t\t" << quaternary_parent_group_length_sum << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|           avg size:\t\t" << quaternary_list_avg_size << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "|           max size:\t\t" << quaternary_list_max_size << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            std::vector<std::string> get_memory_usage_info([[maybe_unused]] uint64_t signature_count, [[maybe_unused]] int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> r;
                /*
                uint64_t primaryParents_byte_size = sizeof(std::vector<SignatureWithRelativeLevel>) * this->primaryParents.size();
                r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "primaryParents: \t\t\t" + std::to_string(primaryParents_byte_size) + " bytes");


                uint64_t secondaryParents_byte_size = sizeof(std::unordered_map<SignatureWithRelativeLevel, SignatureWithRelativeLevel>) * this->secondaryParents.size();
                r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "secondaryParents: \t\t\t" + std::to_string(secondaryParents_byte_size) + " bytes");

                uint64_t tertiaryParents_byte_size = 0;

                for (auto &pair : this->tertiaryParents)
                {
                    uint64_t _size = pair.second.size();
                    tertiaryParents_byte_size += ((_size + 1) * (sizeof(SignatureWithRelativeLevel) + 3 * sizeof(void *))) + sizeof(void *);
                    tertiaryParents_byte_size += sizeof(std::set<SignatureWithRelativeLevel>);
                }
                uint64_t bits_per_tertiaryParents = signature_count > 0 ? ((double)tertiaryParents_byte_size / (double)signature_count) : 0;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "tertiaryParents: \t\t\t" + std::to_string(tertiaryParents_byte_size) + " bytes" + " (" + std::to_string(bits_per_tertiaryParents) + " bytes per signature)");

                uint64_t quaternaryParents_byte_size = 0;
                for (auto &pair : this->quaternaryParents)
                {
                    uint64_t _size = pair.second.size();
                    quaternaryParents_byte_size += (sizeof(SignatureWithRelativeLevel) * _size) + sizeof(std::set<SignatureWithRelativeLevel>);
                }
                uint64_t bits_per_quaternaryParents = signature_count > 0 ? ((double)quaternaryParents_byte_size / (double)signature_count) : 0;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "quaternaryParents: \t\t\t" + std::to_string(quaternaryParents_byte_size) + " bytes" + " (" + std::to_string(bits_per_quaternaryParents) + " bytes per signature)");
                */
                return r;
            }

            bool verify() const
            {

                for (size_t i = 0; i < this->primaryParents.size(); i++)
                {
                    ManagerFlag manager_flag = this->sub_pointer_status_[i];
                    if (manager_flag == ManagerFlag::Single)
                    {
                        //
                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[i];
                        few_parents_manager->verify();
                    }
                }

                return true;
            }
            bool verify_equal(const FastParentDictionary &other) const
            {
                if (this->is_restricted_recompression_mode != other.is_restricted_recompression_mode)
                {
                    throw std::runtime_error("Error in verify_equal: The is_restricted_recompression_mode must be equal.");
                }

                if (this->primaryParents.size() != other.primaryParents.size())
                {
                    throw std::runtime_error("Error in verify_equal: The size of primaryParents must be equal.");
                }
                for (size_t i = 0; i < this->primaryParents.size(); i++)
                {
                    if (this->primaryParents[i] != other.primaryParents[i])
                    {
                        throw std::runtime_error("Error in verify_equal: The primaryParents must be equal.");
                    }
                }

                if (this->sub_pointer_status_.size() != other.sub_pointer_status_.size())
                {
                    throw std::runtime_error("Error in verify_equal: The size of sub_pointer_status_ must be equal.");
                }
                for (size_t i = 0; i < this->sub_pointer_status_.size(); i++)
                {
                    if (this->sub_pointer_status_[i] != other.sub_pointer_status_[i])
                    {
                        throw std::runtime_error("Error in verify_equal: The sub_pointer_status_ must be equal.");
                    }
                }

                if (this->sub_pointer.size() != other.sub_pointer.size())
                {
                    throw std::runtime_error("Error in verify_equal: The size of sub_pointer must be equal.");
                }
                for (size_t i = 0; i < this->sub_pointer.size(); i++)
                {
                    if (this->sub_pointer_status_[i] == ManagerFlag::Single)
                    {
                        if (this->sub_pointer[i] != other.sub_pointer[i])
                        {
                            throw std::runtime_error("Error in verify_equal: The sub_pointer must be equal.");
                        }
                    }
                    else if (this->sub_pointer_status_[i] == ManagerFlag::FewParentsManager)
                    {
                        FewParentsManager *few_parents_manager1 = (FewParentsManager *)this->sub_pointer[i];
                        FewParentsManager *few_parents_manager2 = (FewParentsManager *)other.sub_pointer[i];
                        if (!few_parents_manager1->verify_equal(*few_parents_manager2))
                        {
                            throw std::runtime_error("Error in verify_equal: The sub_pointer must be equal.");
                        }
                    }
                    else if (this->sub_pointer_status_[i] == ManagerFlag::Vector){
                        const std::vector<SignatureWithRelativeLevel> *vec1 = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[i];
                        const std::vector<SignatureWithRelativeLevel> *vec2 = (std::vector<SignatureWithRelativeLevel> *)other.sub_pointer[i];
                        if (*vec1 != *vec2){
                            throw std::runtime_error("Error in verify_equal: The sub_pointer must be equal.");
                        }
                    }
                }

                return true;
            }

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Update operations
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            void set_pointer(const std::vector<uint16_t> *relative_max_level_list_, bool is_restricted_recompression_mode)
            {
                this->relative_max_level_list_ = relative_max_level_list_;
                this->is_restricted_recompression_mode = is_restricted_recompression_mode;
            }
            void clear()
            {
                this->primaryParents.clear();
                for (uint64_t i = 0; i < this->sub_pointer.size(); i++)
                {
                    ManagerFlag manager_flag = this->sub_pointer_status_[i];
                    if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[i];
                        few_parents_manager->clear();
                        delete few_parents_manager;
                    }
                    else if (manager_flag == ManagerFlag::Vector){
                        std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[i];
                        vec->clear();
                        delete vec;
                    }

                    this->sub_pointer[i] = nullptr;
                    this->sub_pointer_status_[i] = ManagerFlag::None;
                }
                this->sub_pointer_status_.clear();
                this->sub_pointer.clear();
            }
            void swap(FastParentDictionary &other)
            {
                this->primaryParents.swap(other.primaryParents);
                this->sub_pointer_status_.swap(other.sub_pointer_status_);
                this->sub_pointer.swap(other.sub_pointer);
                std::swap(this->relative_max_level_list_, other.relative_max_level_list_);
                std::swap(this->is_restricted_recompression_mode, other.is_restricted_recompression_mode);
            }
            void add_new_element()
            {
                this->primaryParents.push_back(EMPTY_FLAG);
                this->sub_pointer_status_.push_back(ManagerFlag::None);
                this->sub_pointer.push_back(nullptr);
            }

            void insert_single_signature(BaseSignature base_signature, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {

                uint64_t previous_level_diff = (*this->relative_max_level_list_)[base_signature];
                SignatureWithRelativeLevel previous_signature = SignatureFunctions::get_signature(previous_level_diff, base_signature);
                if (this->primaryParents[base_signature] != EMPTY_FLAG)
                {

                    SignatureWithRelativeLevel moved_parent = this->primaryParents[base_signature];


                    this->insert(previous_signature, moved_parent, false, base_signature_rule_list);
                    this->primaryParents[base_signature] = EMPTY_FLAG;
                }

                /*
                if(!this->debug){
                    (*this->relative_max_level_list_)[base_signature]++;
                }
                */
            }
            void insert(SignatureWithRelativeLevel child, uint64_t parent, bool topLevelChild, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {

                uint64_t base_child_sig = SignatureFunctions::get_base_signature(child);

                if (topLevelChild && this->primaryParents[base_child_sig] == EMPTY_FLAG)
                {
                    this->primaryParents[base_child_sig] = parent;
                }
                else
                {
                    ManagerFlag manager_flag = this->sub_pointer_status_[base_child_sig];
                    if (manager_flag == ManagerFlag::None)
                    {
                        this->sub_pointer_status_[base_child_sig] = ManagerFlag::Single;
                        RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                        assert(parent_item.get_type() == RLSLPRuleType::Pair || parent_item.get_type() == RLSLPRuleType::Power);
                        uint64_t parent_with_flag = this->get_signature_with_flag(child, parent, parent_item);
                        this->sub_pointer[base_child_sig] = (void *)parent_with_flag;
                    }
                    else if (manager_flag == ManagerFlag::Single)
                    {
                        std::pair<SignatureWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(base_child_sig);
                        this->sub_pointer[base_child_sig] = new std::vector<SignatureWithRelativeLevel>();
                        this->sub_pointer_status_[base_child_sig] = ManagerFlag::Vector;
                        std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[base_child_sig];
                        ParentVectorManager::insert_signature(special_parent.first, *vec);
                        ParentVectorManager::insert_signature(parent, *vec);

                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                        QuaternaryKey quaternary_key = ManyParentsManager::get_quaternary_key(child, parent_item, this->is_restricted_recompression_mode);
                        uint64_t level_diff = SignatureFunctions::get_relative_level(child);
                        ((FewParentsManager *)this->sub_pointer[base_child_sig])->insert(level_diff, parent, quaternary_key);
                    }
                    else if(manager_flag == ManagerFlag::Vector){
                        std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[base_child_sig];
                        ParentVectorManager::insert_signature(parent, *vec);

                        if(vec->size() > ParentVectorManager::VECTOR_MAX_SIZE){
                            FewParentsManager *few_parents_manager = new FewParentsManager();
                            few_parents_manager->initialize(base_child_sig, *vec, base_signature_rule_list, this->is_restricted_recompression_mode);
                            this->sub_pointer[base_child_sig] = few_parents_manager;
                            this->sub_pointer_status_[base_child_sig] = ManagerFlag::FewParentsManager;

                            vec->clear();
                            delete vec;
                        }
                    }
                    else
                    {
                        throw std::runtime_error("insert: unknown manager flag");
                    }
                }
            }

            void erase_signature(SignatureWithRelativeLevel child, SignatureWithRelativeLevel parent, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {

                // uint64_t base_signature = SignatureFunctions::get_base_signature(parent);
                assert(SignatureFunctions::get_relative_level(parent) == 0);

                uint64_t base_child_sig = SignatureFunctions::get_base_signature(child);
                uint64_t level_child_sig = SignatureFunctions::get_relative_level(child);

                bool isTopLevel = level_child_sig == (*this->relative_max_level_list_)[base_child_sig];

                if (isTopLevel && this->primaryParents[base_child_sig] == parent)
                {
                    SignatureWithRelativeLevel last_parent = this->take_any_parent_from_few_parents_manager(child, base_signature_rule_list);
                    this->primaryParents[base_child_sig] = last_parent;
                }
                else
                {

                    ManagerFlag manager_flag = this->sub_pointer_status_[base_child_sig];
                    if (manager_flag == ManagerFlag::Single && this->get_special_secondary_parent(base_child_sig).first == parent)
                    {
                        this->sub_pointer[base_child_sig] = nullptr;
                        this->sub_pointer_status_[base_child_sig] = ManagerFlag::None;
                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                        QuaternaryKey quaternary_key = ManyParentsManager::get_quaternary_key(child, parent_item, this->is_restricted_recompression_mode);

                        uint64_t level_diff = SignatureFunctions::get_relative_level(child);
                        FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[base_child_sig];
                        few_parents_manager->erase(level_diff, parent, quaternary_key);
                        if (few_parents_manager->size() <= ParentVectorManager::VECTOR_MAX_SIZE)
                        {
                            std::vector<SignatureWithRelativeLevel> *vec = new std::vector<SignatureWithRelativeLevel>();
                            few_parents_manager->get_all_important_ancestors(*vec);
                            few_parents_manager->clear();
                            delete few_parents_manager;
                            this->sub_pointer[base_child_sig] = vec;
                            this->sub_pointer_status_[base_child_sig] = ManagerFlag::Vector;

                        }
                    }
                    else if (manager_flag == ManagerFlag::Vector){
                        std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[base_child_sig];
                        ParentVectorManager::erase_signature(parent, *vec);
                        assert(vec->size() > 0);
                        if(vec->size() == 1){
                            SignatureWithRelativeLevel parentX = vec->at(0);
                            RLSLPRuleBody parentX_item = RLSLPRuleBody::decodeRule(parentX, base_signature_rule_list);
                            assert(parentX_item.get_type() == RLSLPRuleType::Pair || parentX_item.get_type() == RLSLPRuleType::Power);

                            ChildType childX_type = ManyParentsManager::get_child_type(base_child_sig, parentX, base_signature_rule_list, true);
                            SignatureWithRelativeLevel childX;
        
                            if(childX_type == ChildType::LeftChild || childX_type == ChildType::PowerChild){
                                childX = parentX_item.A;
                            }else if(childX_type == ChildType::RightChild){
                                childX = parentX_item.B;
                            }else{
                                throw std::runtime_error("initialize: unknown child type");
                            }
        
                            uint64_t signature_with_flagX = this->get_signature_with_flag(childX, parentX, parentX_item);

                            delete vec;
                            this->sub_pointer[base_child_sig] = (void *)signature_with_flagX;
                            this->sub_pointer_status_[base_child_sig] = ManagerFlag::Single;
                        }
                    }
                    else
                    {
                        throw std::runtime_error("erase_signature: unknown manager flag");
                    }
                }
            }

            void erase_signature(BaseSignature base_signature, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {
                
                SignatureWithRelativeLevel next_signature = SignatureFunctions::get_signature((*this->relative_max_level_list_)[base_signature] - 1, base_signature);
                SignatureWithRelativeLevel last_parent_quaternary = this->take_any_parent_from_few_parents_manager(next_signature, base_signature_rule_list);
                this->primaryParents[base_signature] = last_parent_quaternary;
            }

            uint64_t size_in_bytes() const
            {
                uint64_t total = 0;
                // Size used by primaryParents
                total += sizeof(this->primaryParents);
                total += sizeof(SignatureWithRelativeLevel) * this->primaryParents.size();

                // Size used by sub_pointer (pointers, not objects themselves)
                total += sizeof(this->sub_pointer);
                total += sizeof(void *) * this->sub_pointer.size();

                // Size used by sub_pointer_status_
                total += sizeof(this->sub_pointer_status_);
                total += sizeof(ManagerFlag) * this->sub_pointer_status_.size();

                // Size used by is_restricted_recompression_mode
                total += sizeof(this->is_restricted_recompression_mode);

                // Size used by relative_max_level_list_ pointer (not vector itself, as it's external)
                total += sizeof(this->relative_max_level_list_);

                // Add the size of dynamically allocated FewParentsManager objects, if any
                for (size_t i = 0; i < this->sub_pointer.size(); ++i)
                {
                    if (this->sub_pointer_status_[i] == ManagerFlag::FewParentsManager && this->sub_pointer[i] != nullptr)
                    {
                        FewParentsManager *mgr = (FewParentsManager *)this->sub_pointer[i];
                        total += mgr->size_in_bytes();
                    }
                    else if (this->sub_pointer_status_[i] == ManagerFlag::Vector && this->sub_pointer[i] != nullptr)
                    {
                        std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[i];
                        total += sizeof(SignatureWithRelativeLevel) * vec->size();
                    }
                }
                return total;
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Load, save, and builder functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

        public:
            static FastParentDictionary load_from_file(const std::vector<uint16_t> *relative_max_level_list_, std::ifstream &ifs)
            {
                FastParentDictionary r;

                uint8_t is_restricted_recompression_mode;
                ifs.read(reinterpret_cast<char *>(&is_restricted_recompression_mode), sizeof(uint8_t));
                r.is_restricted_recompression_mode = is_restricted_recompression_mode == 1;

                // primaryParentsの復元
                uint64_t _primaryParents_size;
                ifs.read(reinterpret_cast<char *>(&_primaryParents_size), sizeof(_primaryParents_size));
                r.primaryParents.resize(_primaryParents_size);
                ifs.read(reinterpret_cast<char *>(r.primaryParents.data()), sizeof(SignatureWithRelativeLevel) * _primaryParents_size);

                // sub_pointerの存在フラグ復元
                uint64_t few_parents_sub_pointer_status_size;
                ifs.read(reinterpret_cast<char *>(&few_parents_sub_pointer_status_size), sizeof(uint64_t));
                r.sub_pointer_status_.resize(few_parents_sub_pointer_status_size);
                if (few_parents_sub_pointer_status_size > 0)
                {
                    ifs.read(reinterpret_cast<char *>(r.sub_pointer_status_.data()), sizeof(ManagerFlag) * few_parents_sub_pointer_status_size);
                }

                // sub_pointer本体の復元
                r.sub_pointer.resize(few_parents_sub_pointer_status_size, nullptr);
                for (size_t i = 0; i < few_parents_sub_pointer_status_size; i++)
                {
                    ManagerFlag manager_flag = r.sub_pointer_status_[i];
                    if (manager_flag == ManagerFlag::Single)
                    {
                        uint64_t signature_with_flag;
                        ifs.read(reinterpret_cast<char *>(&signature_with_flag), sizeof(uint64_t));
                        r.sub_pointer[i] = (void *)signature_with_flag;
                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        r.sub_pointer[i] = new FewParentsManager(FewParentsManager::load_from_file(ifs));
                    }
                    else if (manager_flag == ManagerFlag::Vector)
                    {
                        uint64_t vec_size;
                        ifs.read(reinterpret_cast<char *>(&vec_size), sizeof(uint64_t));
                        std::vector<SignatureWithRelativeLevel> *vec = new std::vector<SignatureWithRelativeLevel>();
                        vec->resize(vec_size);
                        ifs.read(reinterpret_cast<char *>(vec->data()), sizeof(SignatureWithRelativeLevel) * vec_size);
                        r.sub_pointer[i] = vec;
                    }
                    else
                    {
                        r.sub_pointer[i] = nullptr;
                    }
                }
                r.set_pointer(relative_max_level_list_, is_restricted_recompression_mode);
                return r;
            }
            static void store_to_file(const FastParentDictionary &item, std::ofstream &os)
            {

                uint8_t is_restricted_recompression_mode = item.is_restricted_recompression_mode ? 1 : 0;
                os.write(reinterpret_cast<const char *>(&is_restricted_recompression_mode), sizeof(uint8_t));

                uint64_t _primaryParents_size = item.primaryParents.size();
                os.write(reinterpret_cast<const char *>(&_primaryParents_size), sizeof(_primaryParents_size));
                os.write(reinterpret_cast<const char *>(item.primaryParents.data()), sizeof(SignatureWithRelativeLevel) * _primaryParents_size);

                uint64_t few_parents_sub_pointer_status_size = item.sub_pointer_status_.size();
                os.write(reinterpret_cast<const char *>(&few_parents_sub_pointer_status_size), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(item.sub_pointer_status_.data()), sizeof(ManagerFlag) * few_parents_sub_pointer_status_size);

                for (size_t i = 0; i < item.sub_pointer.size(); i++)
                {
                    ManagerFlag manager_flag = item.sub_pointer_status_[i];
                    if (manager_flag == ManagerFlag::Single)
                    {
                        uint64_t signature_with_flag = (uint64_t)item.sub_pointer[i];
                        os.write(reinterpret_cast<const char *>(&signature_with_flag), sizeof(uint64_t));
                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        FewParentsManager::store_to_file(*(FewParentsManager *)item.sub_pointer[i], os);
                    }
                    else if (manager_flag == ManagerFlag::Vector)
                    {
                        std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)item.sub_pointer[i];
                        uint64_t vec_size = vec->size();
                        os.write(reinterpret_cast<const char *>(&vec_size), sizeof(uint64_t));
                        os.write(reinterpret_cast<const char *>(vec->data()), sizeof(SignatureWithRelativeLevel) * vec_size);
                    }
                }
            }
            //}@

        private:
            SignatureWithRelativeLevel take_any_parent_from_few_parents_manager(SignatureWithRelativeLevel child, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {
                uint64_t base_signature = SignatureFunctions::get_base_signature(child);
                uint64_t level_diff = SignatureFunctions::get_relative_level(child);

                ManagerFlag manager_flag = this->sub_pointer_status_[base_signature];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<SignatureWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(base_signature);
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(special_parent.first, base_signature_rule_list);
                    if ((special_parent.second && parent_item.B == child) || (!special_parent.second && parent_item.A == child))
                    {
                        this->sub_pointer[base_signature] = nullptr;
                        this->sub_pointer_status_[base_signature] = ManagerFlag::None;
                        return special_parent.first;
                    }
                    else
                    {
                        return EMPTY_FLAG;
                    }
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[base_signature];
                    SignatureWithRelativeLevel next_parent = few_parents_manager->take_any_parent(level_diff);
                    if (few_parents_manager->size() <= ParentVectorManager::VECTOR_MAX_SIZE)
                    {

                        std::vector<SignatureWithRelativeLevel> *vec = new std::vector<SignatureWithRelativeLevel>();
                        few_parents_manager->get_all_important_ancestors(*vec);
                        few_parents_manager->clear();
                        delete few_parents_manager;
                        this->sub_pointer[base_signature] = vec;
                        this->sub_pointer_status_[base_signature] = ManagerFlag::Vector;

                    }
                    return next_parent;
                }
                else if (manager_flag == ManagerFlag::Vector){
                    std::vector<SignatureWithRelativeLevel> *vec = (std::vector<SignatureWithRelativeLevel> *)this->sub_pointer[base_signature];
                    SignatureWithRelativeLevel parent = ParentVectorManager::take_any_parent(child, *vec, base_signature_rule_list);
                    assert(vec->size() > 0);
                    if(vec->size() == 1){
                        SignatureWithRelativeLevel parentX = vec->at(0);
                        RLSLPRuleBody parentX_item = RLSLPRuleBody::decodeRule(parentX, base_signature_rule_list);
                        assert(parentX_item.get_type() == RLSLPRuleType::Pair || parentX_item.get_type() == RLSLPRuleType::Power);

                        ChildType childX_type = ManyParentsManager::get_child_type(base_signature, parentX, base_signature_rule_list, true);
                        SignatureWithRelativeLevel childX;
    
                        if(childX_type == ChildType::LeftChild || childX_type == ChildType::PowerChild){
                            childX = parentX_item.A;
                        }else if(childX_type == ChildType::RightChild){
                            childX = parentX_item.B;
                        }else{
                            throw std::runtime_error("initialize: unknown child type");
                        }
    
                        uint64_t signature_with_flagX = this->get_signature_with_flag(childX, parentX, parentX_item);

                        delete vec;
                        this->sub_pointer[base_signature] = (void *)signature_with_flagX;
                        this->sub_pointer_status_[base_signature] = ManagerFlag::Single;
                    }

                    return parent;

                }
                else
                {
                    return EMPTY_FLAG;
                }
            }
            std::pair<SignatureWithRelativeLevel, bool> get_special_secondary_parent(uint64_t base_signature) const
            {
                uint64_t parent_with_flag = (uint64_t)this->sub_pointer[base_signature];
                bool is_special_pair = (parent_with_flag >> 63) == 1;
                SignatureWithRelativeLevel parent = (parent_with_flag << 1) >> 1;
                return {parent, is_special_pair};
            }
            uint64_t get_signature_with_flag(SignatureWithRelativeLevel child, SignatureWithRelativeLevel parent, const RLSLPRuleBody &parent_item) const
            {
                if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.B == child)
                {
                    return (uint64_t)parent | (1ULL << 63);
                }
                else
                {
                    return parent;
                }
            }

            //}@

            
        };

    
}
