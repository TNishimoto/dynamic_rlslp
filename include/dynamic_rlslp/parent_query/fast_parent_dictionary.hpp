#pragma once
#include "stool/include/all.hpp"
#include "./sub/many_parents_manager.hpp"
#include "./sub/few_parents_manager.hpp"
#include "./sub/parent_vector_manager.hpp"
#include "../../types/types.hpp"
#include "../../json_helper.hpp"

namespace dynRLSLP
{
        /**
         * @brief Compact parent dictionary for layered RLSLP grammars with tiered parent storage.
         * @ingroup ParentClasses
         */
        class FastParentDictionary
        {
        private:
            std::vector<NonterminalWithRelativeLevel> primaryParents;
            std::vector<void *> sub_pointer;
            std::vector<ManagerFlag> sub_pointer_status_;
            bool is_restricted_recompression_mode = false;
            const std::vector<uint16_t> *relative_max_level_list_ = nullptr;

        public:
            inline static const int64_t EMPTY_FLAG = INT64_MAX;
            static inline uint64_t MUDA_COUNT = 0;
            static inline uint64_t NOT_MUDA_COUNT = 0;
            bool USE_CACHE = false;

            static inline std::unordered_map<ExplicitNonterminal, TemporaryOccurrence> muda_nonterminal_map;
            static inline std::unordered_map<ExplicitNonterminal, uint64_t> muda_nonterminal_map2;

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Counts all registered parents of a base nonterminal across storage tiers [Debug function].
             * @param explicit_nonterminal Base nonterminal whose parents are counted.
             * @return Total number of registered parents.
             */
            uint64_t count_registed_parents(uint64_t explicit_nonterminal) const
            {
                if (explicit_nonterminal >= this->primaryParents.size())
                {
                    return 0;
                }
                uint64_t parent_count = 0;
                if (this->primaryParents[explicit_nonterminal] != EMPTY_FLAG)
                {
                    parent_count++;
                }
                ManagerFlag manager_flag = this->sub_pointer_status_[explicit_nonterminal];
                if (manager_flag == ManagerFlag::Single)
                {
                    parent_count++;
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[explicit_nonterminal];
                    parent_count += few_parents_manager->size();
                }else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[explicit_nonterminal];
                    parent_count += vec->size();
                }

                return parent_count;
            }

            /**
             * @brief Pushes type-1 primary occurrences of a nonterminal onto the stack.
             * @param sig Base nonterminal to enumerate.
             * @param position_offset Position offset of the occurrence within its parent.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param explicit_nonterminal_length_list Derived string lengths indexed by base nonterminal.
             * @param output Stack receiving temporary occurrences to expand further.
             * @return True if at least one parent occurrence was pushed.
             */
            bool get_all_type_1_primary_occurrences_of_nonterminal(ExplicitNonterminal sig, int64_t position_offset, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, const std::vector<uint64_t> &explicit_nonterminal_length_list, VStack<TemporaryOccurrence> &output) const
            {
                bool b = false;

                if (this->primaryParents[sig] != EMPTY_FLAG)
                {
                    ManyParentsManager::get_all_type_1_primary_occurrences_of_nonterminal_sub(sig, position_offset, this->primaryParents[sig], explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                    b = true;
                }

                ManagerFlag manager_flag = this->sub_pointer_status_[sig];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<NonterminalWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(sig);

                    ManyParentsManager::get_all_type_1_primary_occurrences_of_nonterminal_sub(sig, position_offset, special_parent.first, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                    b = true;
                }
                else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[sig];                   
                    ParentVectorManager::get_all_type_1_primary_occurrences_of_nonterminal_sub(sig, position_offset, *vec, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                    b = true;
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    assert(this->sub_pointer[sig] != nullptr);
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[sig];
                    bool b1 = few_parents_manager->get_all_type_1_primary_occurrences_of_nonterminal(sig, position_offset, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                    b = b || b1;
                }
                return b;
            }

            /**
             * @brief Returns the primary parent of a base nonterminal.
             * @param descendant Base nonterminal whose primary parent is requested.
             * @return Primary parent nonterminal.
             * @throws std::runtime_error if no primary parent is registered.
             */
            NonterminalWithRelativeLevel take_any_important_ancestor(ExplicitNonterminal descendant) const
            {
                if(this->primaryParents[descendant] != EMPTY_FLAG){
                    return this->primaryParents[descendant];
                }else{
                    throw std::runtime_error("take_any_ancestor: the descendant is not registered");
                }
            }
            /**
             * @brief Appends all important ancestors of a base nonterminal to the output vector.
             * @param descendant Base nonterminal whose ancestors are collected.
             * @param output Vector receiving ancestor nonterminals.
             */
            void get_all_important_ancestors(ExplicitNonterminal descendant, std::vector<NonterminalWithRelativeLevel> &output) const{

                if(this->primaryParents[descendant] != EMPTY_FLAG){
                    output.push_back(this->primaryParents[descendant]);

                    ManagerFlag manager_flag = this->sub_pointer_status_[descendant];
                    if(manager_flag == ManagerFlag::Single){
                        output.push_back(this->get_special_secondary_parent(descendant).first);
                    }else if(manager_flag == ManagerFlag::FewParentsManager){
                        FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[descendant];
                        few_parents_manager->get_all_important_ancestors(output);
                    }else if(manager_flag == ManagerFlag::Vector){
                        const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[descendant];
                        for(auto p : *vec){
                            output.push_back(p);
                        }
                    }
    
                }

            }

            /**
             * @brief Tests whether a base nonterminal has exactly one pair parent and no secondary parents.
             * @param descendant Base nonterminal to test.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return True if the nonterminal has a unique pair parent chain head.
             */
            bool has_single_ancestor(ExplicitNonterminal descendant, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list) const {
                if(this->primaryParents[descendant] != EMPTY_FLAG){
                    NonterminalWithRelativeLevel parent = this->primaryParents[descendant];
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
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
            /**
             * @brief Tests whether a base nonterminal has no registered primary parent.
             * @param descendant Base nonterminal to test.
             * @return True if no primary parent is stored.
             */
            bool is_empty(ExplicitNonterminal descendant) const {
                return this->primaryParents[descendant] == EMPTY_FLAG;
            }
 


            /**
             * @brief Counts all important ancestors of a base nonterminal across storage tiers [Debug function].
             * @param sig Base nonterminal whose ancestors are counted.
             * @return Total number of important ancestors.
             */
            uint64_t count_important_ancestors(ExplicitNonterminal sig) const
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
                    const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[sig];
                    occ += vec->size();
                }

                return occ;
            }

            /**
             * @brief Tests whether a nonterminal has any registered or implicit parent.
             * @param child Child nonterminal to test.
             * @return True if a parent exists in the dictionary or at a higher relative level.
             */
            bool has_parent(NonterminalWithRelativeLevel child) const{
                ExplicitNonterminal explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(child);
                if(this->primaryParents[explicit_nonterminal] != EMPTY_FLAG){
                    return true;
                }else{
                    uint64_t level = NonterminalFunctions::get_relative_level(child);
                    return level < (*this->relative_max_level_list_)[explicit_nonterminal];
                }
            }
            
            /**
             * @brief Looks up the nonterminal of a pair rule with the given children.
             * @param left_sig Left child nonterminal.
             * @param right_sig Right child nonterminal.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return Parent nonterminal if found, otherwise -1.
             */
            int64_t get_pair_nonterminal(NonterminalWithRelativeLevel left_sig, NonterminalWithRelativeLevel right_sig, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list) const
            {

                uint64_t base_left_sig = NonterminalFunctions::get_explicit_nonterminal(left_sig);
                uint64_t level_left_sig = NonterminalFunctions::get_relative_level(left_sig);

                assert(this->relative_max_level_list_ != nullptr);
                assert(base_left_sig < (*this->relative_max_level_list_).size());
                bool isTopLevel = level_left_sig == (*this->relative_max_level_list_)[base_left_sig];

                if (isTopLevel && this->primaryParents[base_left_sig] != EMPTY_FLAG)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(this->primaryParents[base_left_sig], explicit_nonterminal_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.A == left_sig && parent_item.B == right_sig)
                    {
                        return this->primaryParents[base_left_sig];
                    }
                }

                ManagerFlag manager_flag = this->sub_pointer_status_[base_left_sig];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<NonterminalWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(base_left_sig);
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(special_parent.first, explicit_nonterminal_rule_list);

                    if (parent_item.get_type() == RLSLPRuleType::Pair && (NonterminalWithRelativeLevel)parent_item.A == left_sig && (NonterminalWithRelativeLevel)parent_item.B == right_sig)
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
                    int64_t result = few_parents_manager->get_pair_nonterminal(left_sig, right_sig, quaternary_key, explicit_nonterminal_rule_list);

                    return result;
                }
                else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[base_left_sig];
                    return ParentVectorManager::get_pair_nonterminal(left_sig, right_sig, *vec, explicit_nonterminal_rule_list);
                }
                else
                {
                    return -1;
                }
            }

            /**
             * @brief Looks up the nonterminal of a power rule with the given child and exponent.
             * @param child_sig Child nonterminal.
             * @param power Exponent of the power rule.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return Parent nonterminal if found, otherwise -1.
             */
            int64_t get_power_nonterminal(NonterminalWithRelativeLevel child_sig, uint64_t power, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list) const
            {
                uint64_t base_child_sig = NonterminalFunctions::get_explicit_nonterminal(child_sig);
                uint64_t level_child_sig = NonterminalFunctions::get_relative_level(child_sig);
                assert(this->relative_max_level_list_ != nullptr);
                assert(base_child_sig < (*this->relative_max_level_list_).size());
                bool isTopLevel = level_child_sig == (*this->relative_max_level_list_)[base_child_sig];

                if (isTopLevel && this->primaryParents[base_child_sig] != EMPTY_FLAG)
                {
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(this->primaryParents[base_child_sig], explicit_nonterminal_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && parent_item.A == child_sig && (uint64_t)parent_item.B == power)
                    {
                        return this->primaryParents[base_child_sig];
                    }
                }

                ManagerFlag manager_flag = this->sub_pointer_status_[base_child_sig];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<NonterminalWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(base_child_sig);
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(special_parent.first, explicit_nonterminal_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && (NonterminalWithRelativeLevel)parent_item.A == child_sig && (uint64_t)parent_item.B == power)
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

                    return few_parents_manager->get_power_nonterminal(child_sig, power, quaternary_key, explicit_nonterminal_rule_list);
                }
                else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[base_child_sig];
                    return ParentVectorManager::get_power_nonterminal(child_sig, power, *vec, explicit_nonterminal_rule_list);
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
            /**
             * @brief Prints parent information for one base nonterminal.
             * @param explicit_nonterminal Base nonterminal to print.
             * @param message_paragraph Indentation depth for log output.
             */
            void print_tree2(uint64_t explicit_nonterminal, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                uint64_t h = (*this->relative_max_level_list_)[explicit_nonterminal];
                NonterminalWithRelativeLevel sig = NonterminalFunctions::get_nonterminal(h, explicit_nonterminal);

                if (this->primaryParents[explicit_nonterminal] != EMPTY_FLAG)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << NonterminalFunctions::to_string(sig) << ": " << NonterminalFunctions::to_string(this->primaryParents[explicit_nonterminal]) << std::endl;
                }
                else
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << NonterminalFunctions::to_string(sig) << ": " << "NULL" << std::endl;
                }

                ManagerFlag manager_flag = this->sub_pointer_status_[explicit_nonterminal];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<NonterminalWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(explicit_nonterminal);
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << NonterminalFunctions::to_string(sig) << ": " << NonterminalFunctions::to_string(special_parent.first) << std::endl;
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[explicit_nonterminal];
                    few_parents_manager->print_tree(explicit_nonterminal, message_paragraph + 1);
                }else if (manager_flag == ManagerFlag::Vector){
                    const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[explicit_nonterminal];
                    stool::DebugPrinter::print_integers(*vec, "Vector");
                }
            }

            /**
             * @brief Prints the full parent tree for all base nonterminals.
             * @param message_paragraph Indentation depth for log output.
             */
            void print_tree(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== FastParentDictionary Info ====" << std::endl;
                for (size_t i = 0; i < this->primaryParents.size(); i++)
                {
                    print_tree2(i, message_paragraph + 1);
                }
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== FastParentDictionary Info[END] ====" << std::endl;
            }
            /**
             * @brief Prints storage statistics for all parent tiers.
             * @param message_paragraph Indentation depth for log output.
             */
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
                        const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[i];
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

            void write_content_as_json_format(std::ofstream &ofs, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const{
                ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "{" << std::endl;
                ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "\"data_structure\": " << "\"FastParentDictionary\"," << std::endl;
                ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "\"content\": " << "{" << std::endl;

                JsonHelper::write_content_as_json_format<NonterminalWithRelativeLevel>(
                    "primaryParents(std::vector<NonterminalWithRelativeLevel>)",
                    this->primaryParents,
                    [](const NonterminalWithRelativeLevel &value){ return NonterminalFunctions::to_string(value); },
                    false,
                    ofs,
                    message_paragraph+2
                );

                ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "\"sub_pointer\": [" << std::endl;

                for(uint64_t i = 0; i < this->sub_pointer.size(); i++){
                    ManagerFlag manager_flag = this->sub_pointer_status_[i];
                    if(manager_flag == ManagerFlag::Single){
                        std::pair<NonterminalWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(i);

                        ofs << stool::Message::get_paragraph_string(message_paragraph+2) << "\"(" << NonterminalFunctions::to_string(special_parent.first) << ", " << special_parent.second << ")\"";
                    }else if(manager_flag == ManagerFlag::FewParentsManager){
                        FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[i];
                        few_parents_manager->write_content_as_json_format(ofs, message_paragraph+2);
                    }
                    else if(manager_flag == ManagerFlag::Vector){
                        const std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[i];
                        JsonHelper::write_content_as_json_format<NonterminalWithRelativeLevel>(
                            "vec(std::vector<NonterminalWithRelativeLevel>)",
                            *vec,
                            [](const NonterminalWithRelativeLevel &value){ return NonterminalFunctions::to_string(value); },
                            false,
                            ofs,
                            message_paragraph+2
                        );
                    }
                    else if(manager_flag == ManagerFlag::None){
                        ofs << stool::Message::get_paragraph_string(message_paragraph+2)<< "null";
                    }

                    if(i != this->sub_pointer.size() - 1){
                        ofs << "," << std::endl;
                    }
                }
                ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "]" << std::endl;
                
                
                JsonHelper::write_content_as_json_format<ManagerFlag>(
                    "sub_pointer_status_(std::vector<ManagerFlag>)",
                    this->sub_pointer_status_,
                    [](const ManagerFlag &value){ return std::to_string((uint8_t)value); },
                    false,
                    ofs,
                    message_paragraph+2
                );
                
                ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "\"is_restricted_recompression_mode\": " << this->is_restricted_recompression_mode << ", " << std::endl;
                if(this->relative_max_level_list_ != nullptr){
                    JsonHelper::write_content_as_json_format<uint16_t>(
                        "relative_max_level_list_(std::vector<uint16_t>)",
                        *this->relative_max_level_list_,
                        [](const uint16_t &value){ return std::to_string(value); },
                        false,
                        ofs,
                        message_paragraph+2
                    );
                }else{
                    ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "\"relative_max_level_list_\": ";
                    ofs << "null";
                }
                ofs << std::endl;
            }

            /**
             * @brief Returns human-readable memory usage lines for this dictionary.
             * @param nonterminal_count Number of nonterminals in the owning grammar (for per-nonterminal averages).
             * @param message_paragraph Indentation depth for log output.
             * @return Lines describing memory usage (may be empty when detailed accounting is disabled).
             */
            std::vector<std::string> get_memory_usage_info([[maybe_unused]] uint64_t nonterminal_count, [[maybe_unused]] int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> r;
                /*
                uint64_t primaryParents_byte_size = sizeof(std::vector<NonterminalWithRelativeLevel>) * this->primaryParents.size();
                r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "primaryParents: \t\t\t" + std::to_string(primaryParents_byte_size) + " bytes");


                uint64_t secondaryParents_byte_size = sizeof(std::unordered_map<NonterminalWithRelativeLevel, NonterminalWithRelativeLevel>) * this->secondaryParents.size();
                r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "secondaryParents: \t\t\t" + std::to_string(secondaryParents_byte_size) + " bytes");

                uint64_t tertiaryParents_byte_size = 0;

                for (auto &pair : this->tertiaryParents)
                {
                    uint64_t _size = pair.second.size();
                    tertiaryParents_byte_size += ((_size + 1) * (sizeof(NonterminalWithRelativeLevel) + 3 * sizeof(void *))) + sizeof(void *);
                    tertiaryParents_byte_size += sizeof(std::set<NonterminalWithRelativeLevel>);
                }
                uint64_t bits_per_tertiaryParents = nonterminal_count > 0 ? ((double)tertiaryParents_byte_size / (double)nonterminal_count) : 0;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "tertiaryParents: \t\t\t" + std::to_string(tertiaryParents_byte_size) + " bytes" + " (" + std::to_string(bits_per_tertiaryParents) + " bytes per nonterminal)");

                uint64_t quaternaryParents_byte_size = 0;
                for (auto &pair : this->quaternaryParents)
                {
                    uint64_t _size = pair.second.size();
                    quaternaryParents_byte_size += (sizeof(NonterminalWithRelativeLevel) * _size) + sizeof(std::set<NonterminalWithRelativeLevel>);
                }
                uint64_t bits_per_quaternaryParents = nonterminal_count > 0 ? ((double)quaternaryParents_byte_size / (double)nonterminal_count) : 0;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "quaternaryParents: \t\t\t" + std::to_string(quaternaryParents_byte_size) + " bytes" + " (" + std::to_string(bits_per_quaternaryParents) + " bytes per nonterminal)");
                */
                return r;
            }

            /**
             * @brief Verifies internal consistency of all parent managers.
             * @return True if verification succeeds.
             * @throws std::runtime_error on validation failure.
             */
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
            /**
             * @brief Verifies that this dictionary is equal to another instance.
             * @param other Dictionary to compare against.
             * @return True if all fields and sub-managers match.
             * @throws std::runtime_error on mismatch.
             */
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
                        const std::vector<NonterminalWithRelativeLevel> *vec1 = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[i];
                        const std::vector<NonterminalWithRelativeLevel> *vec2 = (std::vector<NonterminalWithRelativeLevel> *)other.sub_pointer[i];
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
            /**
             * @brief Attaches the relative max-level table and recompression mode flag.
             * @param relative_max_level_list_ Pointer to per-base-nonterminal max relative levels.
             * @param is_restricted_recompression_mode True when restricted block compression is active.
             */
            void set_pointer(const std::vector<uint16_t> *relative_max_level_list_, bool is_restricted_recompression_mode)
            {
                this->relative_max_level_list_ = relative_max_level_list_;
                this->is_restricted_recompression_mode = is_restricted_recompression_mode;
            }
            /**
             * @brief Clears all parent entries and releases dynamically allocated sub-managers.
             */
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
                        std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[i];
                        vec->clear();
                        delete vec;
                    }

                    this->sub_pointer[i] = nullptr;
                    this->sub_pointer_status_[i] = ManagerFlag::None;
                }
                this->sub_pointer_status_.clear();
                this->sub_pointer.clear();
            }
            /**
             * @brief Swaps contents with another dictionary instance.
             * @param other Dictionary to swap with.
             */
            void swap(FastParentDictionary &other)
            {
                this->primaryParents.swap(other.primaryParents);
                this->sub_pointer_status_.swap(other.sub_pointer_status_);
                this->sub_pointer.swap(other.sub_pointer);
                std::swap(this->relative_max_level_list_, other.relative_max_level_list_);
                std::swap(this->is_restricted_recompression_mode, other.is_restricted_recompression_mode);
            }
            /**
             * @brief Appends an empty parent slot for a new base nonterminal.
             */
            void add_new_element()
            {
                this->primaryParents.push_back(EMPTY_FLAG);
                this->sub_pointer_status_.push_back(ManagerFlag::None);
                this->sub_pointer.push_back(nullptr);
            }

            /**
             * @brief Promotes the current primary parent when a nonterminal rule gains a new relative level.
             * @param explicit_nonterminal Base nonterminal whose level is increasing.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             */
            void insert_implicit_nonterminal(ExplicitNonterminal explicit_nonterminal, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
            {

                uint64_t previous_level_diff = (*this->relative_max_level_list_)[explicit_nonterminal];
                NonterminalWithRelativeLevel previous_nonterminal = NonterminalFunctions::get_nonterminal(previous_level_diff, explicit_nonterminal);
                if (this->primaryParents[explicit_nonterminal] != EMPTY_FLAG)
                {

                    NonterminalWithRelativeLevel moved_parent = this->primaryParents[explicit_nonterminal];


                    this->insert(previous_nonterminal, moved_parent, false, explicit_nonterminal_rule_list);
                    this->primaryParents[explicit_nonterminal] = EMPTY_FLAG;
                }

                /*
                if(!this->debug){
                    (*this->relative_max_level_list_)[explicit_nonterminal]++;
                }
                */
            }
            /**
             * @brief Registers a parent for a child nonterminal, upgrading storage tier when needed.
             * @param child Child nonterminal receiving the parent link.
             * @param parent Parent nonterminal to register.
             * @param topLevelChild True if the child is at its current max relative level.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             */
            void insert(NonterminalWithRelativeLevel child, uint64_t parent, bool topLevelChild, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
            {

                uint64_t base_child_sig = NonterminalFunctions::get_explicit_nonterminal(child);

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
                        RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                        assert(parent_item.get_type() == RLSLPRuleType::Pair || parent_item.get_type() == RLSLPRuleType::Power);
                        uint64_t parent_with_flag = this->get_nonterminal_with_flag(child, parent, parent_item);
                        this->sub_pointer[base_child_sig] = (void *)parent_with_flag;
                    }
                    else if (manager_flag == ManagerFlag::Single)
                    {
                        std::pair<NonterminalWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(base_child_sig);
                        this->sub_pointer[base_child_sig] = new std::vector<NonterminalWithRelativeLevel>();
                        this->sub_pointer_status_[base_child_sig] = ManagerFlag::Vector;
                        std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[base_child_sig];
                        ParentVectorManager::insert_nonterminal(special_parent.first, *vec);
                        ParentVectorManager::insert_nonterminal(parent, *vec);

                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                        QuaternaryKey quaternary_key = ManyParentsManager::get_quaternary_key(child, parent_item, this->is_restricted_recompression_mode);
                        uint64_t level_diff = NonterminalFunctions::get_relative_level(child);
                        ((FewParentsManager *)this->sub_pointer[base_child_sig])->insert(level_diff, parent, quaternary_key);
                    }
                    else if(manager_flag == ManagerFlag::Vector){
                        std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[base_child_sig];
                        ParentVectorManager::insert_nonterminal(parent, *vec);

                        if(vec->size() > ParentVectorManager::VECTOR_MAX_SIZE){
                            FewParentsManager *few_parents_manager = new FewParentsManager();
                            few_parents_manager->initialize(base_child_sig, *vec, explicit_nonterminal_rule_list, this->is_restricted_recompression_mode);
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

            /**
             * @brief Removes one parent link for a child nonterminal, downgrading storage tier when needed.
             * @param child Child nonterminal whose parent is removed.
             * @param parent Parent nonterminal to erase.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             */
            void erase_nonterminal(NonterminalWithRelativeLevel child, NonterminalWithRelativeLevel parent, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
            {

                // uint64_t explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(parent);
                assert(NonterminalFunctions::get_relative_level(parent) == 0);

                uint64_t base_child_sig = NonterminalFunctions::get_explicit_nonterminal(child);
                uint64_t level_child_sig = NonterminalFunctions::get_relative_level(child);

                bool isTopLevel = level_child_sig == (*this->relative_max_level_list_)[base_child_sig];

                if (isTopLevel && this->primaryParents[base_child_sig] == parent)
                {
                    NonterminalWithRelativeLevel last_parent = this->take_any_parent_from_few_parents_manager(child, explicit_nonterminal_rule_list);
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
                        RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                        QuaternaryKey quaternary_key = ManyParentsManager::get_quaternary_key(child, parent_item, this->is_restricted_recompression_mode);

                        uint64_t level_diff = NonterminalFunctions::get_relative_level(child);
                        FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[base_child_sig];
                        few_parents_manager->erase(level_diff, parent, quaternary_key);
                        if (few_parents_manager->size() <= ParentVectorManager::VECTOR_MAX_SIZE)
                        {
                            std::vector<NonterminalWithRelativeLevel> *vec = new std::vector<NonterminalWithRelativeLevel>();
                            few_parents_manager->get_all_important_ancestors(*vec);
                            few_parents_manager->clear();
                            delete few_parents_manager;
                            this->sub_pointer[base_child_sig] = vec;
                            this->sub_pointer_status_[base_child_sig] = ManagerFlag::Vector;

                        }
                    }
                    else if (manager_flag == ManagerFlag::Vector){
                        std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[base_child_sig];
                        ParentVectorManager::erase_nonterminal(parent, *vec);
                        assert(vec->size() > 0);
                        if(vec->size() == 1){
                            NonterminalWithRelativeLevel parentX = vec->at(0);
                            RLSLPRuleBody parentX_item = RLSLPRuleBody::decode_rule(parentX, explicit_nonterminal_rule_list);
                            assert(parentX_item.get_type() == RLSLPRuleType::Pair || parentX_item.get_type() == RLSLPRuleType::Power);

                            ChildType childX_type = ManyParentsManager::get_child_type(base_child_sig, parentX, explicit_nonterminal_rule_list, true);
                            NonterminalWithRelativeLevel childX;
        
                            if(childX_type == ChildType::LeftChild || childX_type == ChildType::PowerChild){
                                childX = parentX_item.A;
                            }else if(childX_type == ChildType::RightChild){
                                childX = parentX_item.B;
                            }else{
                                throw std::runtime_error("initialize: unknown child type");
                            }
        
                            uint64_t nonterminal_with_flagX = this->get_nonterminal_with_flag(childX, parentX, parentX_item);

                            delete vec;
                            this->sub_pointer[base_child_sig] = (void *)nonterminal_with_flagX;
                            this->sub_pointer_status_[base_child_sig] = ManagerFlag::Single;
                        }
                    }
                    else
                    {
                        throw std::runtime_error("erase_nonterminal: unknown manager flag");
                    }
                }
            }

            /**
             * @brief Restores the primary parent after a nonterminal rule loses its top relative level.
             * @param explicit_nonterminal Base nonterminal whose level is decreasing.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             */
            void erase_nonterminal(ExplicitNonterminal explicit_nonterminal, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
            {
                
                NonterminalWithRelativeLevel next_nonterminal = NonterminalFunctions::get_nonterminal((*this->relative_max_level_list_)[explicit_nonterminal] - 1, explicit_nonterminal);
                NonterminalWithRelativeLevel last_parent_quaternary = this->take_any_parent_from_few_parents_manager(next_nonterminal, explicit_nonterminal_rule_list);
                this->primaryParents[explicit_nonterminal] = last_parent_quaternary;
            }

            /**
             * @brief Returns the total memory usage of this dictionary in bytes.
             * @return Memory footprint including dynamically allocated sub-managers.
             */
            uint64_t size_in_bytes() const
            {
                uint64_t total = 0;
                // Size used by primaryParents
                total += sizeof(this->primaryParents);
                total += sizeof(NonterminalWithRelativeLevel) * this->primaryParents.size();

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
                        std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[i];
                        total += sizeof(NonterminalWithRelativeLevel) * vec->size();
                    }
                }
                return total;
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Load, save, and builder functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

        public:
            /**
             * @brief Loads a parent dictionary from a binary input stream.
             * @param relative_max_level_list_ Pointer to per-base-nonterminal max relative levels.
             * @param ifs Input stream positioned at the dictionary data.
             * @return Restored FastParentDictionary instance.
             */
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
                ifs.read(reinterpret_cast<char *>(r.primaryParents.data()), sizeof(NonterminalWithRelativeLevel) * _primaryParents_size);

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
                        uint64_t nonterminal_with_flag;
                        ifs.read(reinterpret_cast<char *>(&nonterminal_with_flag), sizeof(uint64_t));
                        r.sub_pointer[i] = (void *)nonterminal_with_flag;
                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        r.sub_pointer[i] = new FewParentsManager(FewParentsManager::load_from_file(ifs));
                    }
                    else if (manager_flag == ManagerFlag::Vector)
                    {
                        uint64_t vec_size;
                        ifs.read(reinterpret_cast<char *>(&vec_size), sizeof(uint64_t));
                        std::vector<NonterminalWithRelativeLevel> *vec = new std::vector<NonterminalWithRelativeLevel>();
                        vec->resize(vec_size);
                        ifs.read(reinterpret_cast<char *>(vec->data()), sizeof(NonterminalWithRelativeLevel) * vec_size);
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
            /**
             * @brief Writes a parent dictionary to a binary output stream.
             * @param item Dictionary to serialize.
             * @param os Output stream to write to.
             */
            static void store_to_file(const FastParentDictionary &item, std::ofstream &os)
            {

                uint8_t is_restricted_recompression_mode = item.is_restricted_recompression_mode ? 1 : 0;
                os.write(reinterpret_cast<const char *>(&is_restricted_recompression_mode), sizeof(uint8_t));

                uint64_t _primaryParents_size = item.primaryParents.size();
                os.write(reinterpret_cast<const char *>(&_primaryParents_size), sizeof(_primaryParents_size));
                os.write(reinterpret_cast<const char *>(item.primaryParents.data()), sizeof(NonterminalWithRelativeLevel) * _primaryParents_size);

                uint64_t few_parents_sub_pointer_status_size = item.sub_pointer_status_.size();
                os.write(reinterpret_cast<const char *>(&few_parents_sub_pointer_status_size), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(item.sub_pointer_status_.data()), sizeof(ManagerFlag) * few_parents_sub_pointer_status_size);

                for (size_t i = 0; i < item.sub_pointer.size(); i++)
                {
                    ManagerFlag manager_flag = item.sub_pointer_status_[i];
                    if (manager_flag == ManagerFlag::Single)
                    {
                        uint64_t nonterminal_with_flag = (uint64_t)item.sub_pointer[i];
                        os.write(reinterpret_cast<const char *>(&nonterminal_with_flag), sizeof(uint64_t));
                    }
                    else if (manager_flag == ManagerFlag::FewParentsManager)
                    {
                        FewParentsManager::store_to_file(*(FewParentsManager *)item.sub_pointer[i], os);
                    }
                    else if (manager_flag == ManagerFlag::Vector)
                    {
                        std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)item.sub_pointer[i];
                        uint64_t vec_size = vec->size();
                        os.write(reinterpret_cast<const char *>(&vec_size), sizeof(uint64_t));
                        os.write(reinterpret_cast<const char *>(vec->data()), sizeof(NonterminalWithRelativeLevel) * vec_size);
                    }
                }
            }
            //}@

        private:
            /**
             * @brief Removes and returns one secondary parent, downgrading storage tier when the list shrinks.
             * @param child Child nonterminal whose secondary parent is taken.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return Removed parent nonterminal, or EMPTY_FLAG if none remains.
             */
            NonterminalWithRelativeLevel take_any_parent_from_few_parents_manager(NonterminalWithRelativeLevel child, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
            {
                uint64_t explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(child);
                uint64_t level_diff = NonterminalFunctions::get_relative_level(child);

                ManagerFlag manager_flag = this->sub_pointer_status_[explicit_nonterminal];
                if (manager_flag == ManagerFlag::Single)
                {
                    std::pair<NonterminalWithRelativeLevel, bool> special_parent = this->get_special_secondary_parent(explicit_nonterminal);
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(special_parent.first, explicit_nonterminal_rule_list);
                    if ((special_parent.second && parent_item.B == child) || (!special_parent.second && parent_item.A == child))
                    {
                        this->sub_pointer[explicit_nonterminal] = nullptr;
                        this->sub_pointer_status_[explicit_nonterminal] = ManagerFlag::None;
                        return special_parent.first;
                    }
                    else
                    {
                        return EMPTY_FLAG;
                    }
                }
                else if (manager_flag == ManagerFlag::FewParentsManager)
                {
                    FewParentsManager *few_parents_manager = (FewParentsManager *)this->sub_pointer[explicit_nonterminal];
                    NonterminalWithRelativeLevel next_parent = few_parents_manager->take_any_parent(level_diff);
                    if (few_parents_manager->size() <= ParentVectorManager::VECTOR_MAX_SIZE)
                    {

                        std::vector<NonterminalWithRelativeLevel> *vec = new std::vector<NonterminalWithRelativeLevel>();
                        few_parents_manager->get_all_important_ancestors(*vec);
                        few_parents_manager->clear();
                        delete few_parents_manager;
                        this->sub_pointer[explicit_nonterminal] = vec;
                        this->sub_pointer_status_[explicit_nonterminal] = ManagerFlag::Vector;

                    }
                    return next_parent;
                }
                else if (manager_flag == ManagerFlag::Vector){
                    std::vector<NonterminalWithRelativeLevel> *vec = (std::vector<NonterminalWithRelativeLevel> *)this->sub_pointer[explicit_nonterminal];
                    NonterminalWithRelativeLevel parent = ParentVectorManager::take_any_parent(child, *vec, explicit_nonterminal_rule_list);
                    assert(vec->size() > 0);
                    if(vec->size() == 1){
                        NonterminalWithRelativeLevel parentX = vec->at(0);
                        RLSLPRuleBody parentX_item = RLSLPRuleBody::decode_rule(parentX, explicit_nonterminal_rule_list);
                        assert(parentX_item.get_type() == RLSLPRuleType::Pair || parentX_item.get_type() == RLSLPRuleType::Power);

                        ChildType childX_type = ManyParentsManager::get_child_type(explicit_nonterminal, parentX, explicit_nonterminal_rule_list, true);
                        NonterminalWithRelativeLevel childX;
    
                        if(childX_type == ChildType::LeftChild || childX_type == ChildType::PowerChild){
                            childX = parentX_item.A;
                        }else if(childX_type == ChildType::RightChild){
                            childX = parentX_item.B;
                        }else{
                            throw std::runtime_error("initialize: unknown child type");
                        }
    
                        uint64_t nonterminal_with_flagX = this->get_nonterminal_with_flag(childX, parentX, parentX_item);

                        delete vec;
                        this->sub_pointer[explicit_nonterminal] = (void *)nonterminal_with_flagX;
                        this->sub_pointer_status_[explicit_nonterminal] = ManagerFlag::Single;
                    }

                    return parent;

                }
                else
                {
                    return EMPTY_FLAG;
                }
            }
            /**
             * @brief Decodes the single secondary parent stored with an embedded right-child flag.
             * @param explicit_nonterminal Base nonterminal whose secondary parent is decoded.
             * @return Pair of parent nonterminal and whether the child is the right child of a pair rule.
             */
            std::pair<NonterminalWithRelativeLevel, bool> get_special_secondary_parent(uint64_t explicit_nonterminal) const
            {
                uint64_t parent_with_flag = (uint64_t)this->sub_pointer[explicit_nonterminal];
                bool is_special_pair = (parent_with_flag >> 63) == 1;
                NonterminalWithRelativeLevel parent = (parent_with_flag << 1) >> 1;
                return {parent, is_special_pair};
            }
            /**
             * @brief Encodes a parent nonterminal with a high bit marking a right-child pair parent.
             * @param child Child nonterminal used to determine which side of a pair rule is stored.
             * @param parent Parent nonterminal to encode.
             * @param parent_item Decoded rule body of the parent.
             * @return Parent nonterminal optionally OR-ed with the right-child flag bit.
             */
            uint64_t get_nonterminal_with_flag(NonterminalWithRelativeLevel child, NonterminalWithRelativeLevel parent, const RLSLPRuleBody &parent_item) const
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
