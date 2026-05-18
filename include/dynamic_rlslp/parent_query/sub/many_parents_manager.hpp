#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "../../../rlslp/static_operations/run_rule_vector.hpp"


namespace dynRLSLP
{
        /**
         * @brief A signature together with a position offset during occurrence enumeration.
         * @ingroup ParentClasses
         */
        struct TemporaryOccurrence
        {
            BaseSignature signature;
            uint64_t position;
            /**
             * @brief Constructs a temporary occurrence for a signature at a position offset.
             * @param signature Base signature of the occurrence.
             * @param position Position offset within the expanded string.
             */
            TemporaryOccurrence(BaseSignature signature, uint64_t position) : signature(signature), position(position)
            {
            }
            /**
             * @brief Default constructor initializing fields to sentinel maximum values.
             */
            TemporaryOccurrence() : signature(std::numeric_limits<BaseSignature>::max()), position(std::numeric_limits<uint64_t>::max())
            {
            }

            /**
             * @brief Creates a null sentinel occurrence.
             * @return Temporary occurrence with maximum signature and position values.
             */
            static TemporaryOccurrence create_null_occurrence()
            {
                return TemporaryOccurrence(std::numeric_limits<BaseSignature>::max(), std::numeric_limits<uint64_t>::max());
            }

            /**
             * @brief Tests whether this occurrence is the null sentinel.
             * @return True if both fields hold their maximum sentinel values.
             */
            bool is_null() const
            {
                return this->signature == std::numeric_limits<BaseSignature>::max() && this->position == std::numeric_limits<uint64_t>::max();
            }
        };

        /**
         * @brief Manages large numbers of tertiary and quaternary parents for one base signature.
         * @ingroup ParentClasses
         */
        class ManyParentsManager
        {
        private:
            std::vector<std::vector<SignatureWithRelativeLevel>> tertiary_parent_list_;
            std::vector<std::unordered_map<QuaternaryKey, SignatureWithRelativeLevel>> quaternary_parent_list_;

        public:
            inline static const uint64_t PARENT_NUMBER_THRESHOLD = 64;
            inline static const int64_t EMPTY_FLAG = INT64_MAX;
            /**
             * @brief Default constructor creating an empty manager.
             */
            ManyParentsManager()
            {
            }
            /**
             * @brief Default destructor.
             */
            ~ManyParentsManager()
            {
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Tests whether any quaternary parent groups are stored.
             * @return True if the quaternary parent list is non-empty.
             */
            bool has_quaternary_parents() const
            {
                return this->quaternary_parent_list_.size() > 0;
            }
            /**
             * @brief Returns the total number of parents across tertiary and quaternary tiers.
             * @return Total parent count.
             */
            uint64_t size() const
            {
                uint64_t size = 0;
                for (uint64_t i = 0; i < this->tertiary_parent_list_.size(); i++)
                {
                    size += this->tertiary_parent_list_[i].size();
                }
                for (uint64_t i = 0; i < this->quaternary_parent_list_.size(); i++)
                {
                    size += this->quaternary_parent_list_[i].size();
                }
                return size;
            }
            /**
             * @brief Returns the number of tertiary parent groups.
             * @return Length of the tertiary parent list.
             */
            uint64_t tertiary_parents_list_length() const
            {
                return this->tertiary_parent_list_.size();
            }
            /**
             * @brief Returns the number of quaternary parent groups.
             * @return Length of the quaternary parent list.
             */
            uint64_t quaternary_parents_list_length() const
            {
                return this->quaternary_parent_list_.size();
            }
            /**
             * @brief Counts all parents stored in tertiary groups.
             * @return Total tertiary parent count.
             */
            uint64_t count_tertiary_parents() const
            {
                uint64_t count = 0;
                for (uint64_t i = 0; i < this->tertiary_parent_list_.size(); i++)
                {
                    count += this->tertiary_parent_list_[i].size();
                }
                return count;
            }
            /**
             * @brief Counts all parents stored in quaternary hash maps.
             * @return Total quaternary parent count.
             */
            uint64_t count_quaternary_parents() const
            {
                uint64_t count = 0;
                for (uint64_t i = 0; i < this->quaternary_parent_list_.size(); i++)
                {
                    count += this->quaternary_parent_list_[i].size();
                }
                return count;
            }
            /**
             * @brief Returns the maximum size of any single tertiary parent group.
             * @return Largest tertiary group length.
             */
            uint64_t max_tertiary_parent_count() const
            {
                uint64_t max_count = 0;
                for (uint64_t i = 0; i < this->tertiary_parent_list_.size(); i++)
                {
                    max_count = std::max(max_count, (uint64_t)this->tertiary_parent_list_[i].size());
                }
                return max_count;
            }
            /**
             * @brief Returns the maximum size of any single quaternary parent map.
             * @return Largest quaternary map size.
             */
            uint64_t max_quaternary_parent_count() const
            {
                uint64_t max_count = 0;
                for (uint64_t i = 0; i < this->quaternary_parent_list_.size(); i++)
                {
                    max_count = std::max(max_count, (uint64_t)this->quaternary_parent_list_[i].size());
                }
                return max_count;
            }

            /**
             * @brief Tests whether this manager stores no parents.
             * @return True if both tertiary and quaternary lists are empty.
             */
            bool is_empty() const
            {
                return this->tertiary_parent_list_.size() == 0 && this->quaternary_parent_list_.size() == 0;
            }

            /**
             * @brief Tests whether the parent group at a relative level is empty.
             * @param level_diff Relative level to test.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table.
             * @return True if no parents exist at the given level.
             */
            bool is_empty(uint16_t level_diff, uint16_t list_offset, const std::vector<uint16_t> &level_diff_list_)
            {
                int16_t tertiary_index = this->find_tertiary_list_index(level_diff, list_offset, level_diff_list_);
                if (tertiary_index != -1)
                {
                    return this->tertiary_parent_list_[tertiary_index].size() == 0;
                }
                else
                {
                    int16_t quaternary_index = this->find_quaternary_list_index(level_diff, list_offset, level_diff_list_);
                    if (quaternary_index != -1)
                    {
                        return this->quaternary_parent_list_[quaternary_index].size() == 0;
                    }
                }
                return true;
            }
            /*
            bool has_single_parent(const std::vector<RLSLPRuleBody> &base_signature_rule_list) const {
                if(this->tertiary_parent_list_.size() >= 2){
                    return false;
                }else if (this->tertiary_parent_list_.size() == 1){
                    if(this->tertiary_parent_list_[0].size() >= 2){
                        return false;
                    }else{
                        assert(this->tertiary_parent_list_[0].size() == 1);
                        SignatureWithRelativeLevel parent = this->tertiary_parent_list_[0][0];
                        RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                        return parent_item.get_type() == RLSLPRuleType::Pair;    
                    }
                }else{
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
             * @brief Appends all parent signatures to the output vector.
             * @param output Vector receiving parent signatures.
             */
            void get_all_important_ancestors(std::vector<SignatureWithRelativeLevel> &output) const{
                for(const std::vector<SignatureWithRelativeLevel> &parent : this->tertiary_parent_list_){
                    for(auto p : parent){
                        output.push_back(p);
                    }
                }
                for(const std::unordered_map<QuaternaryKey, SignatureWithRelativeLevel> &parent : this->quaternary_parent_list_){
                    for(auto key_value : parent){
                        output.push_back(key_value.second);
                    }
                }
            }

            /**
             * @brief Pushes type-1 primary occurrences for all parents onto the stack.
             * @param sig Base signature of the queried nonterminal.
             * @param position_offset Position offset of the occurrence within its parent.
             * @param base_signature_rule_list Rule bodies indexed by base signature.
             * @param base_signature_length_list Derived string lengths indexed by base signature.
             * @param output Stack receiving temporary occurrences to expand further.
             * @return True if at least one parent occurrence was pushed.
             */
            bool get_all_type_1_primary_occurrences_of_signature(BaseSignature sig, int64_t position_offset, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list, VStack<TemporaryOccurrence> &output) const
            {
                bool b = this->tertiary_parent_list_.size() > 0;
                for (const auto &element : this->tertiary_parent_list_)
                {
                    for (auto parent : element)
                    {
                        get_all_type_1_primary_occurrences_of_signature_sub(sig, position_offset, parent, base_signature_rule_list, base_signature_length_list, output);
                    }
                }
                for (const auto &element : this->quaternary_parent_list_)
                {
                    for (auto key_value : element)
                    {
                        uint64_t parent = key_value.second;
                        get_all_type_1_primary_occurrences_of_signature_sub(sig, position_offset, parent, base_signature_rule_list, base_signature_length_list, output);
                    }
                }
                return b;
            }

            /**
             * @brief Finds the tertiary group index for a relative level.
             * @param level_diff Relative level to search for.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table.
             * @return Tertiary group index, or -1 if not found.
             */
            int16_t find_tertiary_list_index(uint16_t level_diff, uint16_t list_offset, const std::vector<uint16_t> &level_diff_list_) const
            {
                uint64_t tertiary_size = this->tertiary_parent_list_.size();
                for (uint16_t i = 0; i < tertiary_size; i++)
                {
                    if (level_diff_list_[i + list_offset] == level_diff)
                    {
                        return i;
                    }
                }
                return -1;
            }
            /**
             * @brief Finds the quaternary group index for a relative level.
             * @param level_diff Relative level to search for.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table.
             * @return Quaternary group index, or -1 if not found.
             */
            int16_t find_quaternary_list_index(uint16_t level_diff, uint16_t list_offset, const std::vector<uint16_t> &level_diff_list_) const
            {
                uint64_t tertiary_size = this->tertiary_parent_list_.size();
                uint64_t quaternary_size = this->quaternary_parent_list_.size();
                for (uint16_t i = 0; i < quaternary_size; i++)
                {
                    if (level_diff_list_[i + list_offset + tertiary_size] == level_diff)
                    {
                        return i;
                    }
                }
                return -1;
            }


            /**
             * @brief Looks up a pair parent with the given children at a relative level.
             * @param level_diff Relative level of the left child.
             * @param list_offset Offset into the shared level-diff list.
             * @param left_child Left child signature.
             * @param right_child Right child signature.
             * @param quaternary_key Quaternary lookup key for hash-map parents.
             * @param level_diff_list_ Shared relative-level index table.
             * @param base_signature_rule_list Rule bodies indexed by base signature.
             * @return Parent signature if found, otherwise -1.
             */
            int64_t get_pair_signature(uint16_t level_diff, uint16_t list_offset, SignatureWithRelativeLevel left_child, SignatureWithRelativeLevel right_child, QuaternaryKey quaternary_key, 
                const std::vector<uint16_t> &level_diff_list_, const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
            {
                int16_t tertiary_index = this->find_tertiary_list_index(level_diff, list_offset, level_diff_list_);
                if (tertiary_index != -1)
                {
                    for (BaseSignature parent : this->tertiary_parent_list_[tertiary_index])
                    {
                        const RLSLPRuleBody& parent_item = RLSLPRuleBody::refer_body_of_base_signature(parent, base_signature_rule_list);
                        if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.A == left_child && parent_item.B == right_child)
                        {
                            return parent;
                        }
                    }
                    int16_t quaternary_index = this->find_quaternary_list_index(level_diff, list_offset, level_diff_list_);
                    if (quaternary_index != -1)
                    {
                        auto f = this->quaternary_parent_list_[quaternary_index].find(quaternary_key);
                        if (f != this->quaternary_parent_list_[quaternary_index].end())
                        {
                            return f->second;
                        }
                        else
                        {
                            return -1;
                        }
                    }
                }
                return -1;
            }
            /**
             * @brief Looks up a power parent with the given child and exponent at a relative level.
             * @param level_diff Relative level of the child.
             * @param list_offset Offset into the shared level-diff list.
             * @param child Child signature.
             * @param power Exponent of the power rule.
             * @param quaternary_key Quaternary lookup key for hash-map parents.
             * @param level_diff_list_ Shared relative-level index table.
             * @param base_signature_rule_list Rule bodies indexed by base signature.
             * @return Parent signature if found, otherwise -1.
             */
            int64_t get_power_signature(uint16_t level_diff, uint16_t list_offset, SignatureWithRelativeLevel child, uint64_t power, QuaternaryKey quaternary_key, const std::vector<uint16_t> &level_diff_list_, const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
            {
                int16_t tertiary_index = this->find_tertiary_list_index(level_diff, list_offset, level_diff_list_);
                if (tertiary_index != -1)
                {
                    for (BaseSignature parent : this->tertiary_parent_list_[tertiary_index])
                    {
                        const RLSLPRuleBody& parent_item = RLSLPRuleBody::refer_body_of_base_signature(parent, base_signature_rule_list);
                        if (parent_item.get_type() == RLSLPRuleType::Power && parent_item.A == child && (uint64_t)parent_item.B == power)
                        {
                            return parent;
                        }
                    }
                    int16_t quaternary_index = this->find_quaternary_list_index(level_diff, list_offset, level_diff_list_);
                    if (quaternary_index != -1)
                    {
                        auto f = this->quaternary_parent_list_[quaternary_index].find(quaternary_key);
                        if (f != this->quaternary_parent_list_[quaternary_index].end())
                        {
                            return f->second;
                        }
                        else
                        {
                            return -1;
                        }
                    }
                }
                return -1;
            }


            /**
             * @brief Returns all (relative level, parent signature) pairs stored in this manager.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table.
             * @return List of level-parent pairs from tertiary and quaternary tiers.
             */
            std::vector<std::pair<uint16_t, SignatureWithRelativeLevel>> get_all_elements(uint16_t list_offset, const std::vector<uint16_t> &level_diff_list_) const
            {
                std::vector<std::pair<uint16_t, SignatureWithRelativeLevel>> output;
                for (uint16_t i = 0; i < this->tertiary_parent_list_.size(); i++)
                {
                    uint16_t level_diff = level_diff_list_[i + list_offset];
                    for (SignatureWithRelativeLevel parent : this->tertiary_parent_list_[i])
                    {
                        output.push_back({level_diff, parent});
                    }
                }
                for (uint16_t i = 0; i < this->quaternary_parent_list_.size(); i++)
                {
                    uint16_t level_diff = level_diff_list_[i + list_offset + this->tertiary_parent_list_.size()];
                    for (auto key_value : this->quaternary_parent_list_[i])
                    {
                        output.push_back({level_diff, key_value.second});
                    }
                }
                return output;
            }

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Print and verification functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Returns the memory usage of this manager in bytes.
             * @return Memory footprint of tertiary vectors and quaternary hash maps.
             */
            uint64_t size_in_bytes() const
            {
                size_t total = 0;
                // Calculate size used by tertiary_parent_list_
                total += sizeof(this->tertiary_parent_list_);
                for (const auto &vec : this->tertiary_parent_list_)
                {
                    total += sizeof(vec);
                    total += sizeof(SignatureWithRelativeLevel) * vec.size();
                }
                // Calculate size used by quaternary_parent_list_
                total += sizeof(this->quaternary_parent_list_);
                for (const auto &m : this->quaternary_parent_list_)
                {
                    total += sizeof(m);
                    total += sizeof(typename std::decay<decltype(m)>::type::value_type) * m.size();
                }
                return total;
            }

            /**
             * @brief Prints all parents managed for one base signature.
             * @param base_signature Base signature used to reconstruct child signatures for display.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table.
             * @param message_paragraph Indentation depth for log output.
             */
            void print_tree(uint64_t base_signature, uint16_t list_offset, const std::vector<uint16_t> &level_diff_list_, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                for (uint64_t i = 0; i < this->tertiary_parent_list_.size(); i++)
                {
                    uint64_t diff_level = level_diff_list_[i + list_offset];
                    SignatureWithRelativeLevel sig = SignatureFunctions::get_signature(diff_level, base_signature);
                    for (SignatureWithRelativeLevel parent : this->tertiary_parent_list_[i])
                    {
                        std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << SignatureFunctions::to_string(sig) << " <- " << SignatureFunctions::to_string(parent) << std::endl;
                    }
                }
                for (uint64_t i = 0; i < this->quaternary_parent_list_.size(); i++)
                {
                    uint64_t diff_level = level_diff_list_[i + list_offset + this->tertiary_parent_list_.size()];
                    for (auto key_value : this->quaternary_parent_list_[i])
                    {
                        bool b = (key_value.first >> 63) == 1;
                        SignatureWithRelativeLevel key2 = (key_value.first << 1) >> 1;
                        SignatureWithRelativeLevel sig = SignatureFunctions::get_signature(diff_level, base_signature);
                        if (b)
                        {
                            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << SignatureFunctions::to_string(sig) << " <- " << SignatureFunctions::to_string(key_value.second) << ", key: L" << SignatureFunctions::to_string(key2) << std::endl;
                        }
                        else
                        {
                            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << SignatureFunctions::to_string(sig) << " <- " << SignatureFunctions::to_string(key_value.second) << ", key: R" << SignatureFunctions::to_string(key2) << std::endl;
                        }
                    }
                }

                /*
                std::vector<std::pair<uint16_t, SignatureWithRelativeLevel>> all_elements = this->get_all_elements(list_offset, level_diff_list_);
                for (size_t i = 0; i < all_elements.size(); i++)
                {
                    uint64_t h = all_elements[i].first;
                    SignatureWithRelativeLevel sig = SignatureFunctions::get_signature(h, base_signature);
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << SignatureFunctions::to_string(sig) << ": " << SignatureFunctions::to_string(all_elements[i].second) << std::endl;
                }
                */
            }
            /**
             * @brief Verifies that this manager is equal to another instance.
             * @param other Manager to compare against.
             * @return True if all tertiary and quaternary storage match.
             * @throws std::runtime_error on mismatch.
             */
            bool verify_equal(const ManyParentsManager &other) const
            {
                if (this->tertiary_parent_list_.size() != other.tertiary_parent_list_.size())
                {
                    throw std::runtime_error("Error in verify_equal: The size of tertiary_parent_list_ must be equal.");
                }
                for (size_t i = 0; i < this->tertiary_parent_list_.size(); i++)
                {
                    if (this->tertiary_parent_list_[i] != other.tertiary_parent_list_[i])
                    {
                        throw std::runtime_error("Error in verify_equal: The tertiary_parent_list_ must be equal.");
                    }
                }
                if (this->quaternary_parent_list_.size() != other.quaternary_parent_list_.size())
                {
                    throw std::runtime_error("Error in verify_equal: The size of quaternary_parent_list_ must be equal.");
                }
                for (size_t i = 0; i < this->quaternary_parent_list_.size(); i++)
                {
                    if (this->quaternary_parent_list_[i].size() != other.quaternary_parent_list_[i].size())
                    {
                        throw std::runtime_error("Error in verify_equal: The size of quaternary_parent_list_ must be equal.");
                    }

                    for (auto pair : this->quaternary_parent_list_[i])
                    {
                        auto f = other.quaternary_parent_list_[i].find(pair.first);
                        if (f == other.quaternary_parent_list_[i].end())
                        {
                            throw std::runtime_error("Error in verify_equal: The quaternary_parent_list_ must be equal.");
                        }
                        if (pair.second != f->second)
                        {
                            throw std::runtime_error("Error in verify_equal: The quaternary_parent_list_ must be equal.");
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
             * @brief Clears all tertiary and quaternary parent storage.
             */
            void clear()
            {
                this->tertiary_parent_list_.clear();
                this->quaternary_parent_list_.clear();
            }
            /**
             * @brief Swaps contents with another manager instance.
             * @param other Manager to swap with.
             */
            void swap(ManyParentsManager &other)
            {
                this->tertiary_parent_list_.swap(other.tertiary_parent_list_);
                this->quaternary_parent_list_.swap(other.quaternary_parent_list_);
            }
            /**
             * @brief Inserts a parent at the given relative level, promoting to quaternary storage when a tertiary group is full.
             * @param level_diff Relative level of the child within the parent rule.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table to update when new groups are created.
             * @param parent Parent signature to register.
             * @param quaternary_key Quaternary lookup key used when the tertiary group overflows.
             */
            void insert(uint16_t level_diff, uint16_t list_offset, std::vector<uint16_t> &level_diff_list_, SignatureWithRelativeLevel parent, QuaternaryKey quaternary_key)
            {
                int16_t tertiary_index = this->find_tertiary_list_index(level_diff, list_offset, level_diff_list_);
                if (tertiary_index != -1)
                {
                    if (this->tertiary_parent_list_[tertiary_index].size() < PARENT_NUMBER_THRESHOLD)
                    {
                        this->tertiary_parent_list_[tertiary_index].push_back(parent);
                    }
                    else
                    {
                        this->insert_quaternary(level_diff, list_offset, level_diff_list_, quaternary_key, parent);
                    }
                }
                else
                {
                    uint64_t tertiary_size = this->tertiary_parent_list_.size();
                    level_diff_list_.insert(level_diff_list_.begin() + list_offset + tertiary_size, level_diff);
                    this->tertiary_parent_list_.push_back(std::vector<SignatureWithRelativeLevel>());
                    this->tertiary_parent_list_[tertiary_size].push_back(parent);
                }
            }

            /**
             * @brief Removes one parent at the given relative level from tertiary or quaternary storage.
             * @param level_diff Relative level of the parent to erase.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table to update when groups become empty.
             * @param parent Parent signature to remove.
             * @param quaternary_key Quaternary lookup key for hash-map parents.
             */
            void erase(uint16_t level_diff, uint16_t list_offset, std::vector<uint16_t> &level_diff_list_, SignatureWithRelativeLevel parent, QuaternaryKey quaternary_key)
            {
                int16_t tertiary_index = this->find_tertiary_list_index(level_diff, list_offset, level_diff_list_);
                if (tertiary_index != -1)
                {
                    auto f1 = std::find(this->tertiary_parent_list_[tertiary_index].begin(), this->tertiary_parent_list_[tertiary_index].end(), parent);
                    if (f1 != this->tertiary_parent_list_[tertiary_index].end())
                    {
                        SignatureWithRelativeLevel any_element = this->take_any_quaternary(level_diff, list_offset, level_diff_list_);
                        if (any_element != EMPTY_FLAG)
                        {
                            *f1 = any_element;
                        }
                        else
                        {
                            this->tertiary_parent_list_[tertiary_index].erase(f1);
                            if (this->tertiary_parent_list_[tertiary_index].size() == 0)
                            {
                                // uint64_t tertiary_size = this->tertiary_parent_list_.size();
                                level_diff_list_.erase(level_diff_list_.begin() + list_offset + tertiary_index);
                                this->tertiary_parent_list_.erase(this->tertiary_parent_list_.begin() + tertiary_index);
                            }
                        }
                    }
                    else
                    {
                        this->erase_quaternary(level_diff, list_offset, level_diff_list_, quaternary_key);
                    }
                }
                else
                {
                    throw std::runtime_error("erase_tertiary_or_quaternary: tertiary_index is not found");
                }
            }
            /**
             * @brief Removes and returns one parent at the given relative level.
             * @param level_diff Relative level of the parent to take.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table to update when groups become empty.
             * @return Removed parent signature, or EMPTY_FLAG if none exists at that level.
             */
            SignatureWithRelativeLevel take_any_parent(uint16_t level_diff, uint16_t list_offset, std::vector<uint16_t> &level_diff_list_)
            {
                int16_t tertiary_index = this->find_tertiary_list_index(level_diff, list_offset, level_diff_list_);
                if (tertiary_index != -1)
                {
                    SignatureWithRelativeLevel last_parent = this->tertiary_parent_list_[tertiary_index].back();
                    SignatureWithRelativeLevel next_parent = this->take_any_quaternary(level_diff, list_offset, level_diff_list_);
                    if (next_parent != EMPTY_FLAG)
                    {
                        uint64_t idx = this->tertiary_parent_list_[tertiary_index].size() - 1;
                        this->tertiary_parent_list_[tertiary_index][idx] = next_parent;
                    }
                    else
                    {
                        this->tertiary_parent_list_[tertiary_index].pop_back();
                        if (this->tertiary_parent_list_[tertiary_index].size() == 0)
                        {
                            level_diff_list_.erase(level_diff_list_.begin() + list_offset + tertiary_index);
                            this->tertiary_parent_list_.erase(this->tertiary_parent_list_.begin() + tertiary_index);
                        }
                    }
                    return last_parent;
                }
                else
                {
                    return EMPTY_FLAG;
                }
                throw std::runtime_error("take_any_tertiary is not implemented");
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
             * @return Restored ManyParentsManager instance.
             */
            static ManyParentsManager load_from_file(std::ifstream &ifs)
            {
                // INSERT_YOUR_CODE
                ManyParentsManager r;
                uint64_t _tertiaryParents_count = 0;
                ifs.read(reinterpret_cast<char *>(&_tertiaryParents_count), sizeof(_tertiaryParents_count));

                r.tertiary_parent_list_.resize(_tertiaryParents_count);
                for (size_t i = 0; i < _tertiaryParents_count; i++)
                {
                    uint64_t size = 0;
                    ifs.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
                    r.tertiary_parent_list_[i].resize(size);
                    if (size > 0)
                    {
                        ifs.read(reinterpret_cast<char *>(r.tertiary_parent_list_[i].data()), sizeof(SignatureWithRelativeLevel) * size);
                    }
                }

                uint64_t _quaternaryParents_count = 0;
                ifs.read(reinterpret_cast<char *>(&_quaternaryParents_count), sizeof(_quaternaryParents_count));

                r.quaternary_parent_list_.resize(_quaternaryParents_count);
                for (size_t i = 0; i < _quaternaryParents_count; i++)
                {
                    uint64_t size = 0;
                    ifs.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
                    std::vector<SignatureWithRelativeLevel> key_vec(size);
                    std::vector<SignatureWithRelativeLevel> value_vec(size);

                    if (size > 0)
                    {
                        ifs.read(reinterpret_cast<char *>(key_vec.data()), sizeof(SignatureWithRelativeLevel) * size);
                        ifs.read(reinterpret_cast<char *>(value_vec.data()), sizeof(SignatureWithRelativeLevel) * size);

                        for (size_t j = 0; j < size; j++)
                        {
                            r.quaternary_parent_list_[i][key_vec[j]] = value_vec[j];
                        }
                    }
                }

                return r;
            }

            /**
             * @brief Writes a manager to a binary output stream.
             * @param item Manager to serialize.
             * @param os Output stream to write to.
             */
            static void store_to_file(const ManyParentsManager &item, std::ofstream &os)
            {
                uint64_t _tertiaryParents_count = item.tertiary_parent_list_.size();
                os.write(reinterpret_cast<const char *>(&_tertiaryParents_count), sizeof(_tertiaryParents_count));

                for (size_t i = 0; i < item.tertiary_parent_list_.size(); i++)
                {
                    uint64_t size = item.tertiary_parent_list_[i].size();
                    os.write(reinterpret_cast<const char *>(&size), sizeof(uint64_t));
                    os.write(reinterpret_cast<const char *>(item.tertiary_parent_list_[i].data()), sizeof(SignatureWithRelativeLevel) * size);
                }

                uint64_t _quaternaryParents_count = item.quaternary_parent_list_.size();
                os.write(reinterpret_cast<const char *>(&_quaternaryParents_count), sizeof(_quaternaryParents_count));
                for (size_t i = 0; i < item.quaternary_parent_list_.size(); i++)
                {
                    uint64_t size = item.quaternary_parent_list_[i].size();
                    os.write(reinterpret_cast<const char *>(&size), sizeof(uint64_t));

                    std::vector<SignatureWithRelativeLevel> key_vec;
                    std::vector<SignatureWithRelativeLevel> value_vec;
                    for (auto pair : item.quaternary_parent_list_[i])
                    {
                        key_vec.push_back(pair.first);
                        value_vec.push_back(pair.second);
                    }
                    os.write(reinterpret_cast<const char *>(key_vec.data()), sizeof(SignatureWithRelativeLevel) * size);
                    os.write(reinterpret_cast<const char *>(value_vec.data()), sizeof(SignatureWithRelativeLevel) * size);
                }
            }
            //}@

            /**
             * @brief Builds the quaternary lookup key for a parent rule and main child.
             * @param main_child Child signature used as the lookup anchor.
             * @param parent_item Decoded rule body of the parent.
             * @param is_restricted_recompression_mode True when restricted block compression is active.
             * @return Encoded quaternary key distinguishing left/right pair children or power exponents.
             */
            static QuaternaryKey get_quaternary_key(SignatureWithRelativeLevel main_child, RLSLPRuleBody parent_item, bool is_restricted_recompression_mode)
            {
                if (parent_item.get_type() == RLSLPRuleType::Pair)
                {
                    if (parent_item.A == main_child)
                    {
                        return parent_item.B;
                    }
                    else if (parent_item.B == main_child)
                    {
                        return ((uint64_t)parent_item.A) | (1ULL << 63);
                    }
                    else
                    {
                        throw std::runtime_error("get_quaternary_key: main_child is not found");
                    }
                }
                else if (parent_item.get_type() == RLSLPRuleType::Power)
                {
                    if (is_restricted_recompression_mode)
                    {
                        return parent_item.B;
                    }
                    else
                    {
                        return ((uint64_t)parent_item.B) | (1ULL << 62);
                    }
                }
                else
                {
                    std::cout << "main_child: " << SignatureFunctions::to_string(main_child) << std::endl;
                    std::cout << "parent_item: " << parent_item.get_info() << std::endl;
                    throw std::runtime_error("get_quaternary_key: unknown rule type");
                }
            }
            /**
             * @brief Pushes one type-1 primary occurrence of a child within a parent rule onto the stack.
             * @param sig Base signature of the child occurrence.
             * @param position_offset Position offset of the occurrence within its parent.
             * @param parent Base signature of the parent rule.
             * @param base_signature_rule_list Rule bodies indexed by base signature.
             * @param base_signature_length_list Derived string lengths indexed by base signature.
             * @param output Stack receiving the temporary occurrence.
             */
            static void get_all_type_1_primary_occurrences_of_signature_sub(BaseSignature sig, int64_t position_offset, BaseSignature parent,
                                                          const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list, VStack<TemporaryOccurrence> &output)
            {
                const RLSLPRuleBody& parent_item = RLSLPRuleBody::refer_body_of_base_signature(parent, base_signature_rule_list);
                assert(parent_item.get_type() == RLSLPRuleType::Pair || parent_item.get_type() == RLSLPRuleType::Power);
                if (parent_item.get_type() == RLSLPRuleType::Pair)
                {
                    BaseSignature left_base_signature = SignatureFunctions::get_base_signature(parent_item.A);
                    uint64_t diff_len = SignatureFunctions::get_length(parent_item.A, base_signature_length_list);
                    uint64_t new_offset = left_base_signature == sig ? position_offset : position_offset + diff_len;
                    output.push(TemporaryOccurrence(parent, new_offset));
                }
                else if (parent_item.get_type() == RLSLPRuleType::Power)
                {
                    assert(SignatureFunctions::get_base_signature(parent_item.A) == (uint64_t)sig);
                    uint64_t base_len = SignatureFunctions::get_length(parent_item.A, base_signature_length_list);
                    uint64_t diff_len = 0;
                    for (int64_t i = 0; i < parent_item.B; i++)
                    {
                        output.push(TemporaryOccurrence(parent, position_offset + diff_len));
                        diff_len += base_len;
                    }
                }
                else
                {
                    throw std::runtime_error("get_all_important_occurrences: unknown rule type");
                }
            }
            /**
             * @brief Returns one important occurrence of a child within a parent rule.
             * @param sig Base signature of the child occurrence.
             * @param position_offset Position offset of the occurrence within its parent.
             * @param parent Base signature of the parent rule.
             * @param base_signature_rule_list Rule bodies indexed by base signature.
             * @param base_signature_length_list Derived string lengths indexed by base signature.
             * @return A single temporary occurrence for the child within the parent.
             */
            static TemporaryOccurrence take_any_important_occurrence_sub(BaseSignature sig, int64_t position_offset, BaseSignature parent,
                                                          const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list)
            {
                const RLSLPRuleBody& parent_item = RLSLPRuleBody::refer_body_of_base_signature(parent, base_signature_rule_list);
                assert(parent_item.get_type() == RLSLPRuleType::Pair || parent_item.get_type() == RLSLPRuleType::Power);
                if (parent_item.get_type() == RLSLPRuleType::Pair)
                {
                    BaseSignature left_base_signature = SignatureFunctions::get_base_signature(parent_item.A);
                    if (left_base_signature == sig)
                    {
                        return TemporaryOccurrence(parent, position_offset);
                    }
                    else
                    {
                        assert(SignatureFunctions::get_base_signature(parent_item.B) == (uint64_t)sig);
                        uint64_t diff_len = SignatureFunctions::get_length(parent_item.A, base_signature_length_list);
                        return TemporaryOccurrence(parent, position_offset+ diff_len);
                    }
                }
                else if (parent_item.get_type() == RLSLPRuleType::Power)
                {
                    assert(SignatureFunctions::get_base_signature(parent_item.A) == (uint64_t)sig);
                    assert(parent_item.B >= 1);
                    TemporaryOccurrence temp(parent, position_offset);
                    return temp;
                }
                else
                {
                    throw std::runtime_error("get_all_important_occurrences: unknown rule type");
                }
            }
            /**
             * @brief Determines which child slot of a parent rule contains the given child.
             * @param child Child signature to locate.
             * @param parent Base signature of the parent rule.
             * @param base_signature_rule_list Rule bodies indexed by base signature.
             * @param useBaseSignature True to compare base signatures only; false for full signature equality.
             * @return ChildType indicating left, right, power, signature child, or none.
             */
            static ChildType get_child_type(SignatureWithRelativeLevel child, BaseSignature parent, const std::vector<RLSLPRuleBody> &base_signature_rule_list, bool useBaseSignature){
                const RLSLPRuleBody& parent_item = RLSLPRuleBody::refer_body_of_base_signature(parent, base_signature_rule_list);
                if(!useBaseSignature){
                    if (parent_item.get_type() == RLSLPRuleType::Pair)
                    {
                        if(parent_item.A == child){
                            return ChildType::LeftChild;
                        }else if (parent_item.B == child){
                            return ChildType::RightChild;
                        }else{
                            return ChildType::None;
                        }
                    }
                    else if (parent_item.get_type() == RLSLPRuleType::Power)
                    {
                        if(parent_item.A == child){
                            return ChildType::PowerChild;
                        }else{
                            return ChildType::None;
                        }
                    }
                    else if (parent_item.get_type() == RLSLPRuleType::Signature){
                        if(parent_item.A == child){
                            return ChildType::SignatureChild;
                        }else{
                            return ChildType::None;
                        }
                    }else{
                        throw std::runtime_error("get_child_type: unknown rule type");
                    }    
                }else{
                    BaseSignature base_child = SignatureFunctions::get_base_signature(child);
                    if (parent_item.get_type() == RLSLPRuleType::Pair)
                    {
                        if(SignatureFunctions::get_base_signature(parent_item.A) == (uint64_t)base_child){
                            return ChildType::LeftChild;
                        }else if (SignatureFunctions::get_base_signature(parent_item.B) == (uint64_t)base_child){
                            return ChildType::RightChild;
                        }else{
                            return ChildType::None;
                        }
                    }
                    else if (parent_item.get_type() == RLSLPRuleType::Power)
                    {
                        if(SignatureFunctions::get_base_signature(parent_item.A) == (uint64_t)base_child){
                            return ChildType::PowerChild;
                        }else{
                            return ChildType::None;
                        }
                    }
                    else if (parent_item.get_type() == RLSLPRuleType::Signature){
                        if(SignatureFunctions::get_base_signature(parent_item.A) == (uint64_t)base_child){
                            return ChildType::SignatureChild;
                        }else{
                            return ChildType::None;
                        }
                    }else{
                        throw std::runtime_error("get_child_type: unknown rule type");
                    }    
                }
            }
            /**
             * @brief Returns the start position of a child occurrence within the string derived from a parent.
             * @param child Child signature to locate.
             * @param parent Base signature of the parent rule.
             * @param base_signature_rule_list Rule bodies indexed by base signature.
             * @param base_signature_length_list Derived string lengths indexed by base signature.
             * @param useBaseSignature True to compare base signatures only; false for full signature equality.
             * @return Position offset of the child within the parent's derived string.
             */
            static uint64_t get_any_occurrence_position(SignatureWithRelativeLevel child, BaseSignature parent, const std::vector<RLSLPRuleBody> &base_signature_rule_list , const std::vector<uint64_t> &base_signature_length_list, bool useBaseSignature){
                const RLSLPRuleBody& parent_item = RLSLPRuleBody::refer_body_of_base_signature(parent, base_signature_rule_list);
                if(!useBaseSignature){
                    if (parent_item.get_type() == RLSLPRuleType::Pair)
                    {
                        if(parent_item.A == child){
                            return 0;
                        }else if (parent_item.B == child){
                            return SignatureFunctions::get_length(parent_item.A, base_signature_length_list);
                        }else{
                            throw std::runtime_error("get_any_occurrence_position: child is not found");
                        }
                    }
                    else if (parent_item.get_type() == RLSLPRuleType::Power)
                    {
                        if(parent_item.A == child){
                            return 0;
                        }else{
                            throw std::runtime_error("get_any_occurrence_position: child is not found");
                        }
                    }
                    else if (parent_item.get_type() == RLSLPRuleType::Signature){
                        if(parent_item.A == child){
                            return 0;
                        }else{
                            throw std::runtime_error("get_any_occurrence_position: child is not found");
                        }
                    }else{
                        throw std::runtime_error("get_any_occurrence_position: unknown rule type");
                    }    
                }else{
                    BaseSignature base_child = SignatureFunctions::get_base_signature(child);
                    if (parent_item.get_type() == RLSLPRuleType::Pair)
                    {
                        if(SignatureFunctions::get_base_signature(parent_item.A) == (uint64_t)base_child){
                            return 0;
                        }else if (SignatureFunctions::get_base_signature(parent_item.B) == (uint64_t)base_child){
                            return SignatureFunctions::get_length(parent_item.A, base_signature_length_list);
                        }else{
                            throw std::runtime_error("get_any_occurrence_position: child is not found");
                        }
                    }
                    else if (parent_item.get_type() == RLSLPRuleType::Power)
                    {
                        if(SignatureFunctions::get_base_signature(parent_item.A) == (uint64_t)base_child){
                            return 0;
                        }else{
                            throw std::runtime_error("get_any_occurrence_position: child is not found");
                        }
                    }
                    else if (parent_item.get_type() == RLSLPRuleType::Signature){
                        if(SignatureFunctions::get_base_signature(parent_item.A) == (uint64_t)base_child){
                            return 0;
                        }else{
                            throw std::runtime_error("get_any_occurrence_position: child is not found");
                        }
                    }else{
                        throw std::runtime_error("get_any_occurrence_position: unknown rule type");
                    }    
                }
            }

            private:
            /**
             * @brief Removes and returns one quaternary parent at the given relative level.
             * @param level_diff Relative level of the parent to take.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table to update when a map becomes empty.
             * @return Removed parent signature, or EMPTY_FLAG if no quaternary group exists.
             */
            SignatureWithRelativeLevel take_any_quaternary(uint16_t level_diff, uint16_t list_offset, std::vector<uint16_t> &level_diff_list_)
            {
                int16_t quaternary_index = this->find_quaternary_list_index(level_diff, list_offset, level_diff_list_);
                if (quaternary_index != -1)
                {
                    assert(this->quaternary_parent_list_[quaternary_index].size() > 0);
                    auto f = this->quaternary_parent_list_[quaternary_index].begin();
                    SignatureWithRelativeLevel any_parent = f->second;
                    this->quaternary_parent_list_[quaternary_index].erase(f);
                    if (this->quaternary_parent_list_[quaternary_index].size() == 0)
                    {
                        level_diff_list_.erase(level_diff_list_.begin() + list_offset + this->tertiary_parent_list_.size() + quaternary_index);
                        this->quaternary_parent_list_.erase(this->quaternary_parent_list_.begin() + quaternary_index);
                    }
                    return any_parent;
                }
                else
                {
                    return EMPTY_FLAG;
                }
            }

            /**
             * @brief Removes one quaternary parent entry at the given relative level.
             * @param level_diff Relative level of the parent to erase.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table to update when a map becomes empty.
             * @param quaternary_key Quaternary lookup key of the entry to erase.
             */
            void erase_quaternary(uint16_t level_diff, uint16_t list_offset, std::vector<uint16_t> &level_diff_list_, uint64_t quaternary_key)
            {
                int16_t quaternary_index = this->find_quaternary_list_index(level_diff, list_offset, level_diff_list_);
                if (quaternary_index != -1)
                {
                    auto f2 = this->quaternary_parent_list_[quaternary_index].find(quaternary_key);
                    if (f2 != this->quaternary_parent_list_[quaternary_index].end())
                    {
                        this->quaternary_parent_list_[quaternary_index].erase(f2);
                        if (this->quaternary_parent_list_[quaternary_index].size() == 0)
                        {
                            uint64_t tertiary_size = this->tertiary_parent_list_.size();
                            level_diff_list_.erase(level_diff_list_.begin() + list_offset + tertiary_size + quaternary_index);
                            this->quaternary_parent_list_.erase(this->quaternary_parent_list_.begin() + quaternary_index);
                        }
                    }
                    else
                    {
                        std::cout << "quaternary_key: 0b" << std::bitset<64>(quaternary_key) << std::endl;
                        throw std::runtime_error("erase_quaternary: quaternary_key is not found1");
                    }
                }
                else
                {
                    throw std::runtime_error("erase_quaternary: quaternary_index is not found2");
                }
            }

            /**
             * @brief Inserts one quaternary parent entry at the given relative level.
             * @param level_diff Relative level of the child within the parent rule.
             * @param list_offset Offset into the shared level-diff list.
             * @param level_diff_list_ Shared relative-level index table to extend when a new map is created.
             * @param key Quaternary lookup key for the new entry.
             * @param new_parent Parent signature to register.
             */
            void insert_quaternary(uint16_t level_diff, uint16_t list_offset, std::vector<uint16_t> &level_diff_list_, QuaternaryKey key, SignatureWithRelativeLevel new_parent)
            {
                int16_t quaternary_index = this->find_quaternary_list_index(level_diff, list_offset, level_diff_list_);
                if (quaternary_index != -1)
                {
                    assert(this->quaternary_parent_list_[quaternary_index].find(key) == this->quaternary_parent_list_[quaternary_index].end());

                    this->quaternary_parent_list_[quaternary_index][key] = new_parent;
                }
                else
                {
                    uint64_t quaternary_size = this->quaternary_parent_list_.size();
                    this->quaternary_parent_list_.push_back(std::unordered_map<uint64_t, SignatureWithRelativeLevel>());
                    this->quaternary_parent_list_[quaternary_size][key] = new_parent;
                    level_diff_list_.push_back(level_diff);
                }
            }
        };

    
}
