#pragma once
#include "./many_parents_manager.hpp"

namespace dynRLSLP
{
        /**
         * @brief Manages a bounded set of secondary parents, delegating overflow to ManyParentsManager.
         * @ingroup ParentClasses
         */

        class FewParentsManager
        {
        private:
            std::vector<uint16_t> level_diff_list_;
            std::vector<NonterminalWithRelativeLevel> secondary_parent_list_;
            ManyParentsManager *many_parents_manager_ = nullptr;
            uint64_t count_ = 0;

        public:
            /**
             * @brief Default constructor creating an empty manager.
             */
            FewParentsManager()
            {
            }
            /**
             * @brief Default destructor releasing any owned ManyParentsManager.
             */
            ~FewParentsManager()
            {
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Returns the number of entries in the secondary parent list.
             * @return Size of the inline secondary parent list.
             */
            uint64_t secondary_parent_count() const
            {
                return this->secondary_parent_list_.size();
            }

            /**
             * @brief Returns the optional overflow manager for tertiary and quaternary parents.
             * @return Pointer to ManyParentsManager, or nullptr if not allocated.
             */
            const ManyParentsManager *get_many_parents_manager() const
            {
                return this->many_parents_manager_;
            }

            /**
             * @brief Tests whether this manager stores no parents.
             * @return True if both secondary lists are empty and no overflow manager exists.
             */
            bool is_empty() const
            {
                return this->level_diff_list_.size() == 0 && this->secondary_parent_list_.size() == 0;
            }

            /**
             * @brief Returns the total number of registered parents across all tiers.
             * @return Total parent count including overflow storage.
             */
            uint64_t size() const
            {
                return this->count_;
            }

            /*
            bool has_single_parent(const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list) const
            {
                if (this->secondary_parent_list_.size() >= 2)
                {
                    return false;
                }
                else if (this->secondary_parent_list_.size() == 1)
                {
                    NonterminalWithRelativeLevel parent = this->secondary_parent_list_[0];
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Pair)
                    {
                        return this->many_parents_manager_ == nullptr;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            */

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{


            /**
             * @brief Pushes type-1 primary occurrences for all parents onto the stack.
             * @param sig Base nonterminal of the queried nonterminal.
             * @param position_offset Position offset of the occurrence within its parent.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param explicit_nonterminal_length_list Derived string lengths indexed by base nonterminal.
             * @param output Stack receiving temporary occurrences to expand further.
             * @return True if at least one parent occurrence was pushed.
             */
            bool get_all_type_1_primary_occurrences_of_nonterminal(ExplicitNonterminal sig, int64_t position_offset, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, const std::vector<uint64_t> &explicit_nonterminal_length_list, VStack<TemporaryOccurrence> &output) const
            {
                bool b = this->secondary_parent_list_.size() > 0;
                for (auto parent : this->secondary_parent_list_)
                {
                    ManyParentsManager::get_all_type_1_primary_occurrences_of_nonterminal_sub(sig, position_offset, parent, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                }

                if (this->many_parents_manager_ != nullptr)
                {
                    this->many_parents_manager_->get_all_type_1_primary_occurrences_of_nonterminal(sig, position_offset, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                }
                return b;
            }

            /**
             * @brief Finds the index of a relative level in the secondary parent list.
             * @param level_diff Relative level to search for.
             * @return Index in the secondary list, or -1 if not found.
             */
            int16_t find_secondary_list_index(uint16_t level_diff) const
            {
                uint64_t secondary_list_size = this->secondary_parent_list_.size();
                for (uint64_t i = 0; i < secondary_list_size; i++)
                {
                    if (this->level_diff_list_[i] == level_diff)
                    {
                        return i;
                    }
                }
                return -1;
            }

            /**
             * @brief Looks up a pair parent with the given children at the child's relative level.
             * @param left_sig Left child nonterminal.
             * @param right_sig Right child nonterminal.
             * @param quaternary_key Quaternary lookup key for overflow parents.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return Parent nonterminal if found, otherwise -1.
             */
            int64_t get_pair_nonterminal(NonterminalWithRelativeLevel left_sig, NonterminalWithRelativeLevel right_sig, QuaternaryKey quaternary_key, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list) const
            {
                uint64_t level_diff = NonterminalFunctions::get_relative_level(left_sig);
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    int64_t parent = this->secondary_parent_list_[secondary_index];
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.A == left_sig && parent_item.B == right_sig)
                    {
                        return parent;
                    }
                    else if (this->many_parents_manager_ != nullptr)
                    {
                        return this->many_parents_manager_->get_pair_nonterminal(level_diff, this->secondary_parent_list_.size(), left_sig, right_sig, quaternary_key, this->level_diff_list_, explicit_nonterminal_rule_list);
                    }
                }
                return -1;
            }
            /**
             * @brief Looks up a power parent with the given child and exponent at the child's relative level.
             * @param child_sig Child nonterminal.
             * @param power Exponent of the power rule.
             * @param quaternary_key Quaternary lookup key for overflow parents.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @return Parent nonterminal if found, otherwise -1.
             */
            int64_t get_power_nonterminal(NonterminalWithRelativeLevel child_sig, uint64_t power, QuaternaryKey quaternary_key, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list) const
            {
                uint64_t level_diff = NonterminalFunctions::get_relative_level(child_sig);
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    int64_t parent = this->secondary_parent_list_[secondary_index];
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && parent_item.A == child_sig && (uint64_t)parent_item.B == power)
                    {
                        return parent;
                    }
                    else if (this->many_parents_manager_ != nullptr)
                    {
                        return this->many_parents_manager_->get_power_nonterminal(level_diff, this->secondary_parent_list_.size(), child_sig, power, quaternary_key, this->level_diff_list_, explicit_nonterminal_rule_list);
                    }
                }
                return -1;
            }


            /**
             * @brief Returns all (relative level, parent nonterminal) pairs stored in this manager.
             * @return Combined list from secondary and overflow tiers.
             */
            std::vector<std::pair<uint16_t, NonterminalWithRelativeLevel>> get_all_elements() const
            {
                std::vector<std::pair<uint16_t, NonterminalWithRelativeLevel>> output;
                for (uint16_t i = 0; i < this->secondary_parent_list_.size(); i++)
                {
                    output.push_back({this->level_diff_list_[i], this->secondary_parent_list_[i]});
                }
                if (this->many_parents_manager_ != nullptr)
                {
                    std::vector<std::pair<uint16_t, NonterminalWithRelativeLevel>> many_parents = this->many_parents_manager_->get_all_elements(this->secondary_parent_list_.size(), this->level_diff_list_);
                    output.insert(output.end(), many_parents.begin(), many_parents.end());
                }
                return output;
            }

            /**
             * @brief Appends all parent nonterminals stored in this manager to the output vector.
             * @param output Vector receiving parent nonterminals.
             */
            void get_all_important_ancestors(std::vector<NonterminalWithRelativeLevel> &output) const{
                for(auto parent : this->secondary_parent_list_){
                    output.push_back(parent);
                }
                if(this->many_parents_manager_ != nullptr){
                    this->many_parents_manager_->get_all_important_ancestors(output);
                }
            }

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Print and verification functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Returns the memory usage of this manager in bytes.
             * @return Memory footprint including any owned ManyParentsManager.
             */
            uint64_t size_in_bytes() const
            {
                uint64_t total = 0;
                // Size used by secondary_parent_list_
                total += sizeof(this->secondary_parent_list_);
                total += sizeof(NonterminalWithRelativeLevel) * this->secondary_parent_list_.size();

                // Size used by level_diff_list_
                total += sizeof(this->level_diff_list_);
                total += sizeof(uint16_t) * this->level_diff_list_.size();

                // Size used by pointer to many_parents_manager_
                total += sizeof(this->many_parents_manager_);

                // If many_parents_manager_ is not null, add its size as well
                if (this->many_parents_manager_ != nullptr)
                {
                    total += this->many_parents_manager_->size_in_bytes();
                }
                return total;
            }

            /**
             * @brief Prints all parents managed for one base nonterminal.
             * @param explicit_nonterminal Base nonterminal used to reconstruct child nonterminals for display.
             * @param message_paragraph Indentation depth for log output.
             */
            void print_tree(uint64_t explicit_nonterminal, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::pair<uint16_t, NonterminalWithRelativeLevel>> all_elements;
                for (uint16_t i = 0; i < this->secondary_parent_list_.size(); i++)
                {
                    all_elements.push_back({this->level_diff_list_[i], this->secondary_parent_list_[i]});
                }
                for (size_t i = 0; i < all_elements.size(); i++)
                {
                    uint64_t h = all_elements[i].first;
                    NonterminalWithRelativeLevel sig = NonterminalFunctions::get_nonterminal(h, explicit_nonterminal);
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << NonterminalFunctions::to_string(sig) << ": " << NonterminalFunctions::to_string(all_elements[i].second) << std::endl;
                }
                if (this->many_parents_manager_ != nullptr)
                {
                    this->many_parents_manager_->print_tree(explicit_nonterminal, this->secondary_parent_list_.size(), this->level_diff_list_, message_paragraph + 1);
                }
            }
            /**
             * @brief Verifies that relative levels in the secondary list are unique.
             * @return True if verification succeeds.
             * @throws std::runtime_error on duplicate relative levels.
             */
            bool verify() const
            {
                std::set<NonterminalWithRelativeLevel> diff_set;
                for (uint16_t i = 0; i < this->secondary_parent_list_.size(); i++)
                {
                    uint64_t diff = this->level_diff_list_[i];
                    uint64_t size1 = diff_set.size();
                    diff_set.insert(diff);
                    uint64_t size2 = diff_set.size();
                    if (size1 == size2)
                    {
                        throw std::runtime_error("Error in verify: The diff is not unique.");
                    }
                }
                return true;
            }
            /**
             * @brief Verifies that this manager is equal to another instance.
             * @param other Manager to compare against.
             * @return True if all fields and sub-managers match.
             * @throws std::runtime_error on mismatch.
             */
            bool verify_equal(const FewParentsManager &other) const
            {
                if (this->level_diff_list_.size() != other.level_diff_list_.size())
                {
                    throw std::runtime_error("Error in verify_equal: The size of level_diff_list_ must be equal.");
                }
                for (size_t i = 0; i < this->level_diff_list_.size(); i++)
                {
                    if (this->level_diff_list_[i] != other.level_diff_list_[i])
                    {
                        throw std::runtime_error("Error in verify_equal: The level_diff_list_ must be equal.");
                    }
                }

                if (this->secondary_parent_list_.size() != other.secondary_parent_list_.size())
                {
                    throw std::runtime_error("Error in verify_equal: The size of secondary_parent_list_ must be equal.");
                }
                for (size_t i = 0; i < this->secondary_parent_list_.size(); i++)
                {
                    if (this->secondary_parent_list_[i] != other.secondary_parent_list_[i])
                    {
                        throw std::runtime_error("Error in verify_equal: The secondary_parent_list_ must be equal.");
                    }
                }
                bool b1 = this->many_parents_manager_ == nullptr;
                bool b2 = other.many_parents_manager_ == nullptr;
                if (b1 != b2)
                {
                    throw std::runtime_error("Error in verify_equal: The many_parents_manager_ must be equal.");
                }
                if (this->many_parents_manager_ != nullptr)
                {
                    if (!this->many_parents_manager_->verify_equal(*other.many_parents_manager_))
                    {
                        throw std::runtime_error("Error in verify_equal: The many_parents_manager_ must be equal.");
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
             * @brief Populates this manager from a vector of parent nonterminals.
             * @param descendant Base nonterminal of the child nonterminal.
             * @param parents Initial parent nonterminals to register.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param is_restricted_recompression_mode True when restricted block compression is active.
             */
            void initialize(ExplicitNonterminal descendant, const std::vector<NonterminalWithRelativeLevel> &parents, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, bool is_restricted_recompression_mode) {
                for(auto parent : parents){
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decode_rule(parent, explicit_nonterminal_rule_list);
                    assert(parent_item.get_type() == RLSLPRuleType::Pair || parent_item.get_type() == RLSLPRuleType::Power);
                    ChildType child_type = ManyParentsManager::get_child_type(descendant, parent, explicit_nonterminal_rule_list, true);
                    NonterminalWithRelativeLevel child;

                    if(child_type == ChildType::LeftChild || child_type == ChildType::PowerChild){
                        child = parent_item.A;
                    }else if(child_type == ChildType::RightChild){
                        child = parent_item.B;
                    }else{
                        throw std::runtime_error("initialize: unknown child type");
                    }
                    QuaternaryKey quaternary_key = ManyParentsManager::get_quaternary_key(child, parent_item, is_restricted_recompression_mode);
                    uint64_t level_diff = NonterminalFunctions::get_relative_level(child);
                    this->insert(level_diff, parent, quaternary_key);    
                }

            }
            /**
             * @brief Clears all parent entries and releases the overflow manager.
             */
            void clear()
            {
                this->level_diff_list_.clear();
                this->secondary_parent_list_.clear();
                if (this->many_parents_manager_ != nullptr)
                {
                    this->many_parents_manager_->clear();
                    delete this->many_parents_manager_;
                    this->many_parents_manager_ = nullptr;
                }
                this->count_ = 0;
            }
            /**
             * @brief Swaps contents with another manager instance.
             * @param other Manager to swap with.
             */
            void swap(FewParentsManager &other)
            {
                this->level_diff_list_.swap(other.level_diff_list_);
                this->secondary_parent_list_.swap(other.secondary_parent_list_);
                std::swap(this->many_parents_manager_, other.many_parents_manager_);
                std::swap(this->count_, other.count_);
            }
            /**
             * @brief Removes and returns one parent at the given relative level.
             * @param level_diff Relative level of the parent to take.
             * @return Removed parent nonterminal, or EMPTY_FLAG if none exists at that level.
             */
            NonterminalWithRelativeLevel take_any_parent(uint16_t level_diff)
            {
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    NonterminalWithRelativeLevel last_parent = this->secondary_parent_list_[secondary_index];
                    NonterminalWithRelativeLevel next_parent = this->take_any_parent_from_many_parents_manager(level_diff);
                    if (next_parent != ManyParentsManager::EMPTY_FLAG)
                    {
                        this->secondary_parent_list_[secondary_index] = next_parent;
                    }
                    else
                    {
                        this->secondary_parent_list_.erase(this->secondary_parent_list_.begin() + secondary_index);
                        this->level_diff_list_.erase(this->level_diff_list_.begin() + secondary_index);
                        this->count_--;
                    }
                    return last_parent;
                }
                else
                {
                    return ManyParentsManager::EMPTY_FLAG;
                }
            }

            /*
            std::pair<uint16_t, NonterminalWithRelativeLevel> take_any_parent_with_diff_level()
            {
                if (this->is_empty())
                {
                    return {0, ManyParentsManager::EMPTY_FLAG};
                }
                else
                {
                    uint64_t size = this->secondary_parent_list_.size();
                    int16_t last_level = this->level_diff_list_[size - 1];
                    NonterminalWithRelativeLevel any_parent = this->take_any_parent(last_level);
                    return {last_level, any_parent};
                }
            }
            */

            /**
             * @brief Removes one parent at the given relative level.
             * @param level_diff Relative level of the parent to erase.
             * @param parent Parent nonterminal to remove.
             * @param quaternary_key Quaternary lookup key for overflow parents.
             */
            void erase(uint16_t level_diff, NonterminalWithRelativeLevel parent, uint64_t quaternary_key)
            {
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    if (this->secondary_parent_list_[secondary_index] == parent)
                    {
                        NonterminalWithRelativeLevel next_parent = this->take_any_parent_from_many_parents_manager(level_diff);
                        if (next_parent != ManyParentsManager::EMPTY_FLAG)
                        {
                            this->secondary_parent_list_[secondary_index] = next_parent;
                        }
                        else
                        {
                            this->secondary_parent_list_.erase(this->secondary_parent_list_.begin() + secondary_index);
                            this->level_diff_list_.erase(this->level_diff_list_.begin() + secondary_index);
                            this->count_--;
                        }
                    }
                    else if (this->many_parents_manager_ != nullptr)
                    {
                        this->many_parents_manager_->erase(level_diff, this->secondary_parent_list_.size(), this->level_diff_list_, parent, quaternary_key);
                        this->count_--;
                        if (this->many_parents_manager_->is_empty())
                        {
                            delete this->many_parents_manager_;
                            this->many_parents_manager_ = nullptr;
                        }
                    }
                    else
                    {
                        throw std::runtime_error("erase: parent is not found");
                    }
                }
                else
                {
                    throw std::runtime_error("erase: secondary_index is not found");
                }
            }
            /**
             * @brief Inserts a parent at the given relative level, creating overflow storage when needed.
             * @param level_diff Relative level of the child within the parent rule.
             * @param parent Parent nonterminal to register.
             * @param quaternary_key Quaternary lookup key for overflow parents.
             */
            void insert(uint16_t level_diff, NonterminalWithRelativeLevel parent, QuaternaryKey quaternary_key)
            {
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    if (this->many_parents_manager_ != nullptr)
                    {
                        this->many_parents_manager_->insert(level_diff, this->secondary_parent_list_.size(), this->level_diff_list_, parent, quaternary_key);
                    }
                    else
                    {
                        this->many_parents_manager_ = new ManyParentsManager();
                        this->many_parents_manager_->insert(level_diff, this->secondary_parent_list_.size(), this->level_diff_list_, parent, quaternary_key);
                    }
                }
                else
                {
                    uint64_t size1 = this->secondary_parent_list_.size();
                    this->secondary_parent_list_.push_back(parent);
                    this->level_diff_list_.insert(this->level_diff_list_.begin() + size1, level_diff);
                }
                this->count_++;
            }
            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Load, save, and builder functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

        public:
            /**
             * @brief Loads a manager from a binary input stream.
             * @param ifs Input stream positioned at the manager data.
             * @return Restored FewParentsManager instance.
             */
            static FewParentsManager load_from_file(std::ifstream &ifs)
            {
                FewParentsManager r;

                uint64_t count;
                ifs.read(reinterpret_cast<char *>(&count), sizeof(uint64_t));
                r.count_ = count;
                
                uint64_t _level_diff_list_size;
                ifs.read(reinterpret_cast<char *>(&_level_diff_list_size), sizeof(_level_diff_list_size));
                r.level_diff_list_.resize(_level_diff_list_size);
                if (_level_diff_list_size > 0)
                {
                    ifs.read(reinterpret_cast<char *>(r.level_diff_list_.data()), sizeof(uint16_t) * _level_diff_list_size);
                }
                uint64_t _secondary_parent_list_size;
                ifs.read(reinterpret_cast<char *>(&_secondary_parent_list_size), sizeof(_secondary_parent_list_size));
                r.secondary_parent_list_.resize(_secondary_parent_list_size);
                if (_secondary_parent_list_size > 0)
                {
                    ifs.read(reinterpret_cast<char *>(r.secondary_parent_list_.data()), sizeof(NonterminalWithRelativeLevel) * _secondary_parent_list_size);
                }

                uint8_t many_parents_manager_flag;
                ifs.read(reinterpret_cast<char *>(&many_parents_manager_flag), sizeof(uint8_t));
                if (many_parents_manager_flag != 0)
                {
                    r.many_parents_manager_ = new ManyParentsManager(ManyParentsManager::load_from_file(ifs));
                }
                else
                {
                    r.many_parents_manager_ = nullptr;
                }
                return r;
            }
            /**
             * @brief Writes a manager to a binary output stream.
             * @param item Manager to serialize.
             * @param os Output stream to write to.
             */
            static void store_to_file(const FewParentsManager &item, std::ofstream &os)
            {
                uint64_t count = item.count_;
                os.write(reinterpret_cast<const char *>(&count), sizeof(uint64_t));


                uint64_t _level_diff_list_size = item.level_diff_list_.size();
                os.write(reinterpret_cast<const char *>(&_level_diff_list_size), sizeof(_level_diff_list_size));
                os.write(reinterpret_cast<const char *>(item.level_diff_list_.data()), sizeof(uint16_t) * _level_diff_list_size);
                uint64_t _secondary_parent_list_size = item.secondary_parent_list_.size();
                os.write(reinterpret_cast<const char *>(&_secondary_parent_list_size), sizeof(_secondary_parent_list_size));
                os.write(reinterpret_cast<const char *>(item.secondary_parent_list_.data()), sizeof(NonterminalWithRelativeLevel) * _secondary_parent_list_size);

                uint8_t many_parents_manager_flag = item.many_parents_manager_ != nullptr ? 1 : 0;
                os.write(reinterpret_cast<const char *>(&many_parents_manager_flag), sizeof(uint8_t));
                if (item.many_parents_manager_ != nullptr)
                {
                    ManyParentsManager::store_to_file(*item.many_parents_manager_, os);
                }
            }
            //}@

        private:
            /**
             * @brief Removes one parent from the overflow manager at the given relative level.
             * @param level_diff Relative level of the parent to take.
             * @return Removed parent nonterminal, or EMPTY_FLAG if the overflow manager is absent or empty.
             */
            NonterminalWithRelativeLevel take_any_parent_from_many_parents_manager(uint16_t level_diff)
            {
                if (this->many_parents_manager_ != nullptr)
                {
                    NonterminalWithRelativeLevel next_parent = this->many_parents_manager_->take_any_parent(level_diff, this->secondary_parent_list_.size(), this->level_diff_list_);
                    if (this->many_parents_manager_->is_empty())
                    {
                        delete this->many_parents_manager_;
                        this->many_parents_manager_ = nullptr;
                    }
                    this->count_--;
                    return next_parent;
                }
                else
                {
                    return ManyParentsManager::EMPTY_FLAG;
                }
            }
        };

    
}
