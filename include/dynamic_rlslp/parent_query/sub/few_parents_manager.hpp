#pragma once
#include "./many_parents_manager.hpp"

namespace dynRLSLP
{
        /**
         * @brief XXXXXXXX
         * @ingroup ParentClasses
         */

        class FewParentsManager
        {
        private:
            std::vector<uint16_t> level_diff_list_;
            std::vector<SignatureWithRelativeLevel> secondary_parent_list_;
            ManyParentsManager *many_parents_manager_ = nullptr;
            uint64_t count_ = 0;

        public:
            FewParentsManager()
            {
            }
            ~FewParentsManager()
            {
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            uint64_t secondary_parent_count() const
            {
                return this->secondary_parent_list_.size();
            }

            const ManyParentsManager *get_many_parents_manager() const
            {
                return this->many_parents_manager_;
            }


            bool is_empty() const
            {
                return this->level_diff_list_.size() == 0 && this->secondary_parent_list_.size() == 0;
            }

            uint64_t size() const
            {
                return this->count_;
            }


            bool has_single_parent(const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
            {
                if (this->secondary_parent_list_.size() >= 2)
                {
                    return false;
                }
                else if (this->secondary_parent_list_.size() == 1)
                {
                    SignatureWithRelativeLevel parent = this->secondary_parent_list_[0];
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
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

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{


            bool get_all_type_1_primary_occurrences_of_signature(BaseSignature sig, int64_t position_offset, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list, VStack<TemporaryOccurrence> &output) const
            {
                bool b = this->secondary_parent_list_.size() > 0;
                for (auto parent : this->secondary_parent_list_)
                {
                    ManyParentsManager::get_all_type_1_primary_occurrences_of_signature_sub(sig, position_offset, parent, base_signature_rule_list, base_signature_length_list, output);
                }

                if (this->many_parents_manager_ != nullptr)
                {
                    this->many_parents_manager_->get_all_type_1_primary_occurrences_of_signature(sig, position_offset, base_signature_rule_list, base_signature_length_list, output);
                }
                return b;
            }

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

            int64_t get_pair_signature(SignatureWithRelativeLevel left_sig, SignatureWithRelativeLevel right_sig, QuaternaryKey quaternary_key, const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
            {
                uint64_t level_diff = SignatureFunctions::get_relative_level(left_sig);
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    int64_t parent = this->secondary_parent_list_[secondary_index];
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Pair && parent_item.A == left_sig && parent_item.B == right_sig)
                    {
                        return parent;
                    }
                    else if (this->many_parents_manager_ != nullptr)
                    {
                        return this->many_parents_manager_->get_pair_signature(level_diff, this->secondary_parent_list_.size(), left_sig, right_sig, quaternary_key, this->level_diff_list_, base_signature_rule_list);
                    }
                }
                return -1;
            }
            int64_t get_power_signature(SignatureWithRelativeLevel child_sig, uint64_t power, QuaternaryKey quaternary_key, const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
            {
                uint64_t level_diff = SignatureFunctions::get_relative_level(child_sig);
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    int64_t parent = this->secondary_parent_list_[secondary_index];
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                    if (parent_item.get_type() == RLSLPRuleType::Power && parent_item.A == child_sig && (uint64_t)parent_item.B == power)
                    {
                        return parent;
                    }
                    else if (this->many_parents_manager_ != nullptr)
                    {
                        return this->many_parents_manager_->get_power_signature(level_diff, this->secondary_parent_list_.size(), child_sig, power, quaternary_key, this->level_diff_list_, base_signature_rule_list);
                    }
                }
                return -1;
            }


            std::vector<std::pair<uint16_t, SignatureWithRelativeLevel>> get_all_elements() const
            {
                std::vector<std::pair<uint16_t, SignatureWithRelativeLevel>> output;
                for (uint16_t i = 0; i < this->secondary_parent_list_.size(); i++)
                {
                    output.push_back({this->level_diff_list_[i], this->secondary_parent_list_[i]});
                }
                if (this->many_parents_manager_ != nullptr)
                {
                    std::vector<std::pair<uint16_t, SignatureWithRelativeLevel>> many_parents = this->many_parents_manager_->get_all_elements(this->secondary_parent_list_.size(), this->level_diff_list_);
                    output.insert(output.end(), many_parents.begin(), many_parents.end());
                }
                return output;
            }

            void get_all_important_ancestors(std::vector<SignatureWithRelativeLevel> &output) const{
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
            uint64_t size_in_bytes() const
            {
                uint64_t total = 0;
                // Size used by secondary_parent_list_
                total += sizeof(this->secondary_parent_list_);
                total += sizeof(SignatureWithRelativeLevel) * this->secondary_parent_list_.size();

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

            void print_tree(uint64_t base_signature, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::pair<uint16_t, SignatureWithRelativeLevel>> all_elements;
                for (uint16_t i = 0; i < this->secondary_parent_list_.size(); i++)
                {
                    all_elements.push_back({this->level_diff_list_[i], this->secondary_parent_list_[i]});
                }
                for (size_t i = 0; i < all_elements.size(); i++)
                {
                    uint64_t h = all_elements[i].first;
                    SignatureWithRelativeLevel sig = SignatureFunctions::get_signature(h, base_signature);
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << SignatureFunctions::to_string(sig) << ": " << SignatureFunctions::to_string(all_elements[i].second) << std::endl;
                }
                if (this->many_parents_manager_ != nullptr)
                {
                    this->many_parents_manager_->print_tree(base_signature, this->secondary_parent_list_.size(), this->level_diff_list_, message_paragraph + 1);
                }
            }
            bool verify() const
            {
                std::set<SignatureWithRelativeLevel> diff_set;
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

            void initialize(BaseSignature descendant, const std::vector<SignatureWithRelativeLevel> &parents, const std::vector<RLSLPRuleBody> &base_signature_rule_list, bool is_restricted_recompression_mode) {
                for(auto parent : parents){
                    RLSLPRuleBody parent_item = RLSLPRuleBody::decodeRule(parent, base_signature_rule_list);
                    assert(parent_item.get_type() == RLSLPRuleType::Pair || parent_item.get_type() == RLSLPRuleType::Power);
                    ChildType child_type = ManyParentsManager::get_child_type(descendant, parent, base_signature_rule_list, true);
                    SignatureWithRelativeLevel child;

                    if(child_type == ChildType::LeftChild || child_type == ChildType::PowerChild){
                        child = parent_item.A;
                    }else if(child_type == ChildType::RightChild){
                        child = parent_item.B;
                    }else{
                        throw std::runtime_error("initialize: unknown child type");
                    }
                    QuaternaryKey quaternary_key = ManyParentsManager::get_quaternary_key(child, parent_item, is_restricted_recompression_mode);
                    uint64_t level_diff = SignatureFunctions::get_relative_level(child);
                    this->insert(level_diff, parent, quaternary_key);    
                }

            }
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
            void swap(FewParentsManager &other)
            {
                this->level_diff_list_.swap(other.level_diff_list_);
                this->secondary_parent_list_.swap(other.secondary_parent_list_);
                std::swap(this->many_parents_manager_, other.many_parents_manager_);
                std::swap(this->count_, other.count_);
            }
            SignatureWithRelativeLevel take_any_parent(uint16_t level_diff)
            {
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    SignatureWithRelativeLevel last_parent = this->secondary_parent_list_[secondary_index];
                    SignatureWithRelativeLevel next_parent = this->take_any_parent_from_many_parents_manager(level_diff);
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
            std::pair<uint16_t, SignatureWithRelativeLevel> take_any_parent_with_diff_level()
            {
                if (this->is_empty())
                {
                    return {0, ManyParentsManager::EMPTY_FLAG};
                }
                else
                {
                    uint64_t size = this->secondary_parent_list_.size();
                    int16_t last_level = this->level_diff_list_[size - 1];
                    SignatureWithRelativeLevel any_parent = this->take_any_parent(last_level);
                    return {last_level, any_parent};
                }
            }

            void erase(uint16_t level_diff, SignatureWithRelativeLevel parent, uint64_t quaternary_key)
            {
                int16_t secondary_index = this->find_secondary_list_index(level_diff);
                if (secondary_index != -1)
                {
                    if (this->secondary_parent_list_[secondary_index] == parent)
                    {
                        SignatureWithRelativeLevel next_parent = this->take_any_parent_from_many_parents_manager(level_diff);
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
            void insert(uint16_t level_diff, SignatureWithRelativeLevel parent, QuaternaryKey quaternary_key)
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
                    ifs.read(reinterpret_cast<char *>(r.secondary_parent_list_.data()), sizeof(SignatureWithRelativeLevel) * _secondary_parent_list_size);
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
            static void store_to_file(const FewParentsManager &item, std::ofstream &os)
            {
                uint64_t count = item.count_;
                os.write(reinterpret_cast<const char *>(&count), sizeof(uint64_t));


                uint64_t _level_diff_list_size = item.level_diff_list_.size();
                os.write(reinterpret_cast<const char *>(&_level_diff_list_size), sizeof(_level_diff_list_size));
                os.write(reinterpret_cast<const char *>(item.level_diff_list_.data()), sizeof(uint16_t) * _level_diff_list_size);
                uint64_t _secondary_parent_list_size = item.secondary_parent_list_.size();
                os.write(reinterpret_cast<const char *>(&_secondary_parent_list_size), sizeof(_secondary_parent_list_size));
                os.write(reinterpret_cast<const char *>(item.secondary_parent_list_.data()), sizeof(SignatureWithRelativeLevel) * _secondary_parent_list_size);

                uint8_t many_parents_manager_flag = item.many_parents_manager_ != nullptr ? 1 : 0;
                os.write(reinterpret_cast<const char *>(&many_parents_manager_flag), sizeof(uint8_t));
                if (item.many_parents_manager_ != nullptr)
                {
                    ManyParentsManager::store_to_file(*item.many_parents_manager_, os);
                }
            }
            //}@

        private:
            SignatureWithRelativeLevel take_any_parent_from_many_parents_manager(uint16_t level_diff)
            {
                if (this->many_parents_manager_ != nullptr)
                {
                    SignatureWithRelativeLevel next_parent = this->many_parents_manager_->take_any_parent(level_diff, this->secondary_parent_list_.size(), this->level_diff_list_);
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
