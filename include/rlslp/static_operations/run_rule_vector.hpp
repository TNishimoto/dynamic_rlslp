#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include <stack>
#include <list>
#include "../components/dictionary_for_layered_rlslp.hpp"
#include "stool/include/all.hpp"

namespace dynRLSLP
{


        /**
         * @brief Level-partitioned list of runs representing a substring.
         * @ingroup RLSLPClasses
         */
        class RunRuleVector
        {
        public:
            using LINE = std::vector<RunRuleBody>;

        private:
            std::vector<LINE> left_lines;
            std::vector<LINE> right_lines;
            int32_t _front_level = -1;
            int32_t _back_level = -1;

        public:
            const DictionaryForLayeredRLSLP &dic;

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Constructors and Destructor
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Construct an empty run-rule vector bound to a dictionary.
             * @param dic Layered RLSLP dictionary.
             */
            RunRuleVector(const DictionaryForLayeredRLSLP &dic) : left_lines(), right_lines(), dic(dic)
            {
                this->update_front_and_back_level();
            }
            /**
             * @brief Construct from a flat common sequence partitioned by level.
             * @param commonSequence Flat sequence of runs.
             * @param dic Layered RLSLP dictionary.
             */
            RunRuleVector(const std::vector<RunRuleBody> &commonSequence, const DictionaryForLayeredRLSLP &dic) : left_lines(), right_lines(), dic(dic)
            {
                if(commonSequence.size() == 0){
                    throw std::runtime_error("Error in RunRuleVector: commonSequence is empty");
                }
                int64_t maxLevel = -1;
                int64_t maxLevelRightmostPos = 0;
                for(int64_t i = 0; i < (int64_t)commonSequence.size(); i++){
                    int64_t level = dic.get_level(commonSequence[i].number);
                    if(level >= maxLevel){
                        maxLevel = level;
                        maxLevelRightmostPos = i;
                    }
                }
                this->set_dummy_lines(maxLevel, true);
                this->_front_level = dic.get_level(commonSequence[0].number);
                this->_back_level = dic.get_level(commonSequence[commonSequence.size() - 1].number);
                for(int64_t i = 0; i < maxLevelRightmostPos; i++){
                    int64_t level = dic.get_level(commonSequence[i].number);
                    this->left_lines[level].push_back(commonSequence[i]);
                }
                for(int64_t i = maxLevelRightmostPos; i < (int64_t)commonSequence.size(); i++){
                    int64_t level = dic.get_level(commonSequence[i].number);
                    this->right_lines[level].push_back(commonSequence[i]);
                }

            }

            /**
             * @brief Construct a vector containing a single signature run.
             * @param number Encoded signature identifier.
             * @param dic Layered RLSLP dictionary.
             */
            RunRuleVector(int64_t number, const DictionaryForLayeredRLSLP &dic)
                : left_lines(), right_lines(), dic(dic)
            {
                uint64_t level = dic.get_level(number);
                RunRuleBody item = RunRuleBody(number, 1);
                this->left_lines.resize(level + 1);
                this->right_lines.resize(level + 1);
                this->left_lines[level].push_back(item);
                this->update_front_and_back_level();

            /**
             * @brief Return the total derived string length.
             * @return int64_t
             */
                assert(this->get_string_length() == (int64_t)dic.get_length(number));
            }
            /**
             * @brief Copy-construct from another run-rule vector.
             * @param source Source vector to copy.
             */
            RunRuleVector(const RunRuleVector &source) : left_lines(), right_lines(), dic(source.dic)
            {
                this->left_lines.resize(source.left_lines.size());
                this->right_lines.resize(source.right_lines.size());
                for (uint64_t i = 0; i < source.left_lines.size(); i++)
                {
                    this->left_lines[i] = source.left_lines[i];
                }
                for (uint64_t i = 0; i < source.right_lines.size(); i++)
                {
                    this->right_lines[i] = source.right_lines[i];
                }
                this->update_front_and_back_level();
            }
            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Return the highest level index (or -1 if empty).
             * @return int64_t
             */
            int64_t get_max_level() const
            {
                return this->left_lines.size() - 1;
            }
            /*
            size_t size2() const
            {
                int64_t max_level = this->get_max_level();
                uint64_t size = 0;
                for (int64_t i = 0; i <= max_level; i++)
                {
                    size += this->left_lines[i].size();
                    size += this->right_lines[i].size();
                }
                return size;
            }
            */
            /**
             * @brief Return whether the vector has no level lines.
             * @return bool
             */
            bool is_empty() const {
                return this->get_max_level() == -1;
            }
            /**
             * @brief Return the number of runs on the ceiling level.
             * @return uint64_t
             */
            uint64_t ceil_size() const
            {
                int64_t max_level = this->get_max_level();
                return this->left_lines[max_level].size() + this->right_lines[max_level].size();
            }
            /**
             * @brief Compare level-wise run sequences with another vector.
             * ('@param other', 'Vector to compare with.')
             * @return bool
             */
            bool check_equal_sequence(const RunRuleVector &other) const{
                if(this->get_max_level() != other.get_max_level()){
                    std::cout << "E1: this->get_max_level(): " << this->get_max_level() << ", other.get_max_level(): " << other.get_max_level() << std::endl;
                    return false;
                }
                if(this->front_level() != other.front_level()){
                    std::cout << "E2: this->front_level(): " << this->front_level() << ", other.front_level(): " << other.front_level() << std::endl;
                    return false;
                }
                if(this->back_level() != other.back_level()){
                    std::cout << "E3: this->back_level(): " << this->back_level() << ", other.back_level(): " << other.back_level() << std::endl;
                    return false;
                }
                for(int64_t i = 0; i < this->get_max_level(); i++){
                    if(this->left_lines[i].size() != other.left_lines[i].size()){
                        std::cout << "E4:, h = " << i << ": this->left_lines[i].size(): " << this->left_lines[i].size() << ", other.left_lines[i].size(): " << other.left_lines[i].size() << std::endl;
                        return false;
                    }
                    for(int64_t j = 0; j < (int64_t)this->left_lines[i].size(); j++){
                        if(this->left_lines[i][j].number != other.left_lines[i][j].number){                            
                            return false;
                        }
                        if(this->left_lines[i][j].power != other.left_lines[i][j].power){
                            return false;
                        }
                    }
                }
                for(int64_t i = 0; i < this->get_max_level(); i++){
                    if(this->right_lines[i].size() != other.right_lines[i].size()){
                        std::cout << "E5: this->right_lines[i].size(): " << this->right_lines[i].size() << ", other.right_lines[i].size(): " << other.right_lines[i].size() << std::endl;
                        return false;
                    }
                    for(int64_t j = 0; j < (int64_t)this->right_lines[i].size(); j++){
                        if(this->right_lines[i][j].number != other.right_lines[i][j].number){
                            return false;
                        }
                        if(this->right_lines[i][j].power != other.right_lines[i][j].power){
                            return false;
                        }
                    }
                }
                if(this->get_max_level() > 0){
                    int64_t h = this->get_max_level();
                    std::vector<RunRuleBody> tmp1;
                    for(int64_t i = 0; i < (int64_t)this->left_lines[h].size(); i++){
                        tmp1.push_back(this->left_lines[h][i]);
                    }
                    for(int64_t i = 0; i < (int64_t)this->right_lines[h].size(); i++){
                        tmp1.push_back(this->right_lines[h][i]);
                    }

                    std::vector<RunRuleBody> tmp2;
                    for(int64_t i = 0; i < (int64_t)other.left_lines[h].size(); i++){
                        tmp2.push_back(other.left_lines[h][i]);
                    }
                    for(int64_t i = 0; i < (int64_t)other.right_lines[h].size(); i++){
                        tmp2.push_back(other.right_lines[h][i]);
                    }

                    if(tmp1.size() != tmp2.size()){
                        std::cout << "E6: tmp1.size(): " << tmp1.size() << ", tmp2.size(): " << tmp2.size() << std::endl;
                        return false;
                    }
                    for(int64_t i = 0; i < (int64_t)tmp1.size(); i++){
                        if(tmp1[i].number != tmp2[i].number){
                            return false;
                        }
                    }


                }

                return true;
            }
            /**
             * @brief Return the text position where the ceiling substring starts.
             * @return uint64_t
             */
            uint64_t compute_ceil_string_starting_position() const
            {
                int64_t max_level = this->get_max_level();
                uint64_t size = 0;
                for (int64_t i = 0; i < max_level; i++)
                {
                    for (auto it : this->left_lines[i])
                    {
                        size += it.get_length(this->dic.get_base_signature_length_list());
                    }
                }
                return size;
            }

            /**
             * @brief Return a new empty vector sharing the same dictionary.
             * @return RunRuleVector
             */
            RunRuleVector create_empty_link() const
            {
                return RunRuleVector(this->dic);
            }
            /**
             * @brief Return whether the left line at level h is empty.
             * ('@param h', 'Level index.')
             * @return bool
             */
            bool check_empty_left_line(int64_t h) const
            {
                return this->left_lines[h].size() == 0;
            }

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Extract a prefix vector with optional run-count and length limits.
             * ('@param max_item_count', 'Maximum number of runs to include.')
             * ('@param max_string_length', 'Maximum derived string length to include.')
             * @return RunRuleVector
             */
            RunRuleVector get_front_items_with_level(int64_t max_item_count = INT64_MAX, int64_t max_string_length = INT64_MAX) const
            {
                // std::vector<RunRuleBodyWidthLevel> r;
                RunRuleVector tmp = RunRuleVector(this->dic);

                int64_t max_level = this->get_max_level();
                int64_t tmp_string_length = 0;
                int64_t tmp_item_count = 0;
                for (int64_t i = 0; i <= max_level; i++)
                {
                    tmp.left_lines.push_back(LINE());
                    tmp.right_lines.push_back(LINE());

                    for (int64_t j = 0; j < (int64_t)this->left_lines[i].size(); j++)
                    {
                        tmp.left_lines[i].push_back(this->left_lines[i][j]);
                        tmp_item_count++;
                        if (max_string_length != INT64_MAX)
                        {
                            tmp_string_length += this->left_lines[i][j].get_length(this->dic.get_base_signature_length_list());
                            if (tmp_string_length >= max_string_length)
                            {
                                tmp.update_front_and_back_level();
                                return tmp;
                            }
                        }
                        if (tmp_item_count >= max_item_count)
                        {
                            tmp.update_front_and_back_level();
                            return tmp;
                        }
                    }
                }
                for (int64_t i = max_level; i >= 0; i--)
                {
                    for (int64_t j = 0; j < (int64_t)this->right_lines[i].size(); j++)
                    {
                        tmp.right_lines[i].push_back(this->right_lines[i][j]);
                        tmp_item_count++;

                        if (max_string_length != INT64_MAX)
                        {
                            tmp_string_length += this->right_lines[i][j].get_length(this->dic.get_base_signature_length_list());
                            if (tmp_string_length >= max_string_length)
                            {
                                tmp.update_front_and_back_level();
                                return tmp;
                            }
                        }
                        if (tmp_item_count >= max_item_count)
                        {
                            tmp.update_front_and_back_level();
                            return tmp;
                        }
                    }
                }
                tmp.update_front_and_back_level();
                return tmp;
            }

            /**
             * @brief Extract a suffix vector with an optional run-count limit.
             * ('@param max_item_count', 'Maximum number of runs to include.')
             * @return RunRuleVector
             */
            RunRuleVector get_back_items_with_level(int64_t max_item_count = INT64_MAX) const
            {
                RunRuleVector tmp = RunRuleVector(this->dic);

                int64_t max_level = this->get_max_level();
                int64_t tmp_item_count = 0;
                for (int64_t i = 0; i <= max_level; i++)
                {
                    tmp.left_lines.push_back(LINE());
                    tmp.right_lines.push_back(LINE());
                    for (int64_t j = this->right_lines[i].size() - 1; j >= 0; j--)
                    {
                        tmp.right_lines[i].insert(tmp.right_lines[i].begin(), this->right_lines[i][j]);
                        tmp_item_count++;
                        if (tmp_item_count >= max_item_count)
                        {
                            tmp.update_front_and_back_level();
                            return tmp;
                        }
                    }
                }
                for (int64_t i = max_level; i >= 0; i--)
                {
                    for (int64_t j = this->left_lines[i].size() - 1; j >= 0; j--)
                    {
                        tmp.left_lines[i].insert(tmp.left_lines[i].begin(), this->left_lines[i][j]);
                        tmp_item_count++;
                        if (tmp_item_count >= max_item_count)
                        {
                            tmp.update_front_and_back_level();
                            return tmp;
                        }
                    }
                }
                tmp.update_front_and_back_level();
                return tmp;
            }

            /**
             * @brief Return the level of the frontmost run.
             * @return int64_t
             */
            int64_t front_level() const
            {
                return this->_front_level;
            }
            /**
             * @brief Return the level of the backmost run.
             * @return int64_t
             */
            int64_t back_level() const
            {
                return this->_back_level;
            }

            /**
             * @brief Return the frontmost run without removing it.
             * @return RunRuleBody
             */
            RunRuleBody peek_front() const
            {
                int64_t level = this->front_level();
                if (level == -1)
                {
                    throw std::runtime_error("ERROR in peek_front");
                }
                if (this->left_lines[level].size() > 0)
                {
                    return this->left_lines[level][0];
                }
                else
                {
            /**
             * @brief Return the highest level index (or -1 if empty).
             * @return int64_t
             */
                    assert(level == this->get_max_level());
                    return this->right_lines[level][0];
                }
            }
            /**
             * @brief Return the backmost run without removing it.
             * @return RunRuleBody
             */
            RunRuleBody peek_back() const
            {
                int64_t level = this->back_level();
                assert(this->left_lines[level].size() + this->right_lines[level].size() > 0);
                if (level == -1)
                {
                    throw std::runtime_error("ERROR in peek_back");
                }

                if (this->right_lines[level].size() > 0)
                {
                    uint64_t size = this->right_lines[level].size();
                    return this->right_lines[level][size - 1];
                }
                else
                {
            /**
             * @brief Return the highest level index (or -1 if empty).
             * @return int64_t
             */
                    assert(level == this->get_max_level());
                    uint64_t size = this->left_lines[level].size();

                    return this->left_lines[level][size - 1];
                }
            }
            /**
             * @brief Return the size of the left line at level h.
             * ('@param h', 'Level index.')
             * @return int64_t
             */
            int64_t left_line_size(int64_t h)
            {
                return this->left_lines[h].size();
            }
            /**
             * @brief Return the size of the right line at level h.
             * ('@param h', 'Level index.')
             * @return int64_t
             */
            int64_t right_line_size(int64_t h)
            {
                return this->right_lines[h].size();
            }

            /**
             * @brief Return the derived length of the frontmost run.
             * @return int64_t
             */
            int64_t front_length() const
            {
            /**
             * @brief Return whether the vector has no level lines.
             * @return bool
             */
                assert(!this->is_empty());
                return this->peek_front().get_length(this->dic.get_base_signature_length_list());
            }
            /**
             * @brief Return the derived length of the backmost run.
             * @return int64_t
             */
            int64_t back_length() const
            {
            /**
             * @brief Return whether the vector has no level lines.
             * @return bool
             */
                assert(!this->is_empty());
                return this->peek_back().get_length(this->dic.get_base_signature_length_list());
            }

            /**
             * @brief Return the total derived string length.
             * @return int64_t
             */
            int64_t get_string_length() const
            {
                uint64_t length = 0;
                int64_t max_level = this->get_max_level();
                for (int64_t i = 0; i <= max_level; i++)
                {
                    for (RunRuleBody it : this->left_lines[i])
                    {
                        length += it.get_length(this->dic.get_base_signature_length_list());
                    }
                    for (RunRuleBody it : this->right_lines[i])
                    {
                        length += it.get_length(this->dic.get_base_signature_length_list());
                    }
                }
                return length;
            }
            /**
             * @brief Return rule info for the frontmost run.
             * @return RLSLPRuleInfo
             */
            RLSLPRuleInfo get_front_info() const
            {
                assert(this->verify());
            /**
             * @brief Return the level of the frontmost run.
             * @return int64_t
             */
                assert(this->front_level() != -1);
                RunRuleBody p = this->peek_front();
                uint64_t length = SignatureFunctions::get_length(p.number, this->dic.get_base_signature_length_list()) * p.power;
                uint64_t level = SignatureFunctions::get_level(p.number, this->dic.get_base_signature_level_list());
                return RLSLPRuleInfo(RLSLPRuleBody::create_run_rule_body(p.number, p.power), length, level);
            }
            /**
             * @brief Return rule info for the backmost run.
             * @return RLSLPRuleInfo
             */
            RLSLPRuleInfo get_back_info() const
            {
                assert(this->verify());
            /**
             * @brief Return the level of the backmost run.
             * @return int64_t
             */
                assert(this->back_level() != -1);
                RunRuleBody p = this->peek_back();
                uint64_t length = SignatureFunctions::get_length(p.number, this->dic.get_base_signature_length_list()) * p.power;
                uint64_t level = SignatureFunctions::get_level(p.number, this->dic.get_base_signature_level_list());
                return RLSLPRuleInfo(RLSLPRuleBody::create_run_rule_body(p.number, p.power), length, level);
            }
            /**
             * @brief Collect up to length front runs at a fixed level.
             * ('@param length', 'Number of runs to collect.')
             * ('@param level', 'Target derivation level.')
             * @return std::vector<RunRuleBody>
             */
            std::vector<RunRuleBody> get_front_sequence(int64_t length, int64_t level) const
            {
                auto tmp = this->get_front_items_with_level(length);
                std::vector<RunRuleBody> r;
                while ((int64_t)r.size() < length && !tmp.is_empty())
                {
                    if (tmp.front_level() == level)
                    {
                        r.push_back(tmp.pop_front());
                    }
                    else if (tmp.front_level() > level)
                    {
                        tmp.break_front();
                    }
                    else
                    {
                        std::cout << "tmp.front_level(): " << tmp.front_level() << ", level: " << level << std::endl;
                        this->print_info();
                        throw std::runtime_error("ERROR in get_front_sequence");
                    }
                }
                return r;
            }
            /**
             * @brief Collect up to length back runs at a fixed level.
             * ('@param length', 'Number of runs to collect.')
             * ('@param level', 'Target derivation level.')
             * @return std::vector<RunRuleBody>
             */
            std::vector<RunRuleBody> get_back_sequence(int64_t length, int64_t level) const
            {
                auto tmp = this->get_back_items_with_level(length);
                std::vector<RunRuleBody> r;
                while ((int64_t)r.size() < length && !tmp.is_empty())
                {
                    if (tmp.back_level() == level)
                    {
                        auto item = tmp.pop_back();
                        r.insert(r.begin(), item);
                    }
                    else if (tmp.back_level() > level)
                    {
                        tmp.break_back();
                    }
                    else
                    {
                        std::cout << "ERROR in RunRuleVector::get_back_sequence" << std::endl;
                        std::cout << "tmp.back_level(): " << tmp.back_level() << ", level: " << level << std::endl;
                        throw std::runtime_error("ERROR in RunRuleVector::get_back_sequence");
                    }
                }
                return r;
            }
            /**
             * @brief Return a vector for the substring [pos, pos+len).
             * ('@param pos', 'Start position in the derived string.')
             * ('@param len', 'Substring length.')
             * @return RunRuleVector
             */
            RunRuleVector create_substring_link(int64_t pos, int64_t len) const
            {
                if (pos < 0)
                    throw std::runtime_error("Error in create_substring_link: pos < 0");
                if (len < 0)
                    throw std::runtime_error("Error in create_substring_link: len < 0");
                if (pos + len > this->get_string_length())
                    throw std::runtime_error("Error in create_substring_link: pos + len > this->get_string_length()");

                RunRuleVector item = this->copy();
                item.cut_front_string(pos);
                auto r = item.copy_front(len);
            /**
             * @brief Return the total derived string length.
             * @return int64_t
             */
                assert(r.get_string_length() == len);
                return r;
            }

            /**
             * @brief Return whether any run decodes to a null rule.
             * ('@param base_signature_rule_list', 'Base-signature rule list.')
             * @return bool
             */
            bool has_null_item([[maybe_unused]] const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
            {
                int64_t max_level = this->get_max_level();
                for (int64_t i = 0; i <= max_level; i++)
                {
                    for (RunRuleBody it : this->left_lines[i])
                    {
                        if (RLSLPRuleBody::decodeRule(it.number, base_signature_rule_list).get_type() == RLSLPRuleType::Null)
                        {
                            return true;
                        }
                    }
                    for (RunRuleBody it : this->right_lines[i])
                    {
                        if (RLSLPRuleBody::decodeRule(it.number, base_signature_rule_list).get_type() == RLSLPRuleType::Null)
                        {
                            return true;
                        }
                    }
                }
                return false;
            }

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Update operations
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Swap level-line storage with another vector.
             * ('@param other', 'Other vector.')
             */
            void swap_list(RunRuleVector &other)
            {
                std::swap(this->left_lines, other.left_lines);
                std::swap(this->right_lines, other.right_lines);
                std::swap(this->_front_level, other._front_level);
                std::swap(this->_back_level, other._back_level);
                assert(this->verify());
            }

            /**
             * @brief Replace this vector with a copy of other.
             * ('@param other', 'Source vector.')
             */
            void copy_from(const RunRuleVector &other)
            {
                this->left_lines.clear();
                this->right_lines.clear();
                this->left_lines.resize(other.left_lines.size());
                this->right_lines.resize(other.right_lines.size());
                this->_front_level = other._front_level;
                this->_back_level = other._back_level;
                for (uint64_t i = 0; i < other.left_lines.size(); i++)
                {
                    for (RunRuleBody it : other.left_lines[i])
                    {
                        this->left_lines[i].push_back(it);
                    }
                }
                for (uint64_t i = 0; i < other.right_lines.size(); i++)
                {
                    for (RunRuleBody it : other.right_lines[i])
                    {
                        this->right_lines[i].push_back(it);
                    }
                }
                assert(this->verify());
            }

            /**
             * @brief Remove and return the frontmost run.
             * @return RunRuleBody
             */
            RunRuleBody pop_front()
            {
                assert(this->verify());
                int64_t level = this->front_level();
                if (level == -1)
                {
                    throw std::runtime_error("Error in pop_front: level == -1");
                }
                int64_t max_level = this->get_max_level();
                uint64_t sizeL = this->left_lines[level].size();
                // uint64_t sizeR = this->right_lines[level].size();
                RunRuleBody top;
                assert(level <= max_level);

                if (level < max_level)
                {
                    top = this->left_lines[level][0];
                    this->left_lines[level].erase(this->left_lines[level].begin());
                    sizeL--;
                    if (sizeL == 0)
                    {
                        this->update_front_level(level + 1);
                    }
                }
                else
                {
                    if (sizeL > 0)
                    {
                        top = this->left_lines[level][0];
                        this->left_lines[level].erase(this->left_lines[level].begin());
                        sizeL--;
                    }
                    else
                    {
            /**
             * @brief Return the highest level index (or -1 if empty).
             * @return int64_t
             */
                        assert(level == this->get_max_level());
                        top = this->right_lines[level][0];
                        this->right_lines[level].erase(this->right_lines[level].begin());
                        // sizeR--;
                    }

                    this->update_max_level(true, false);
                }
                assert(this->verify());

                return top;
            }
            /**
             * @brief Remove and return the backmost run.
             * @return RunRuleBody
             */
            RunRuleBody pop_back()
            {
                int64_t level = this->back_level();
                if (level == -1)
                {
                    throw std::runtime_error("Error in pop_front: level == -1");
                }
                int64_t max_level = this->get_max_level();
                // uint64_t sizeL = this->left_lines[level].size();
                uint64_t sizeR = this->right_lines[level].size();
                RunRuleBody top;

                if (level < max_level)
                {
                    top = this->right_lines[level][this->right_lines[level].size() - 1];
                    this->right_lines[level].pop_back();
                    sizeR--;
                    if (sizeR == 0)
                    {
                        this->update_back_level(level + 1);
                    }
                }
                else
                {
                    if (sizeR > 0)
                    {
                        top = this->right_lines[level][this->right_lines[level].size() - 1];
                        this->right_lines[level].pop_back();
                        sizeR--;
                    }
                    else
                    {
            /**
             * @brief Return the highest level index (or -1 if empty).
             * @return int64_t
             */
                        assert(level == this->get_max_level());
                        top = this->left_lines[level][this->left_lines[level].size() - 1];
                        this->left_lines[level].pop_back();
                        // sizeL--;
                    }
                    this->update_max_level(false, true);
                }
                assert(this->verify());

                return top;
            }

            /**
             * @brief Remove and return all runs on the current front level.
             * @return std::vector<RunRuleBody>
             */
            std::vector<RunRuleBody> pop_front_sequence()
            {
                std::vector<RunRuleBody> r;
                int64_t level = this->front_level();
                int64_t max_level = this->get_max_level();
                if(max_level == -1){
                    throw std::runtime_error("Error in pop_front_sequence: max_level == -1");
                }
                if (level < max_level)
                {
                    for (uint64_t i = 0; i < this->left_lines[level].size(); i++)
                    {
                        r.push_back(this->left_lines[level][i]);
                    }
                    this->left_lines[level].clear();
                    this->update_front_level(level + 1);
                }
                else
                {
                    for (uint64_t i = 0; i < this->left_lines[level].size(); i++)
                    {
                        r.push_back(this->left_lines[level][i]);
                    }
                    for (uint64_t i = 0; i < this->right_lines[level].size(); i++)
                    {
                        r.push_back(this->right_lines[level][i]);
                    }
                    this->left_lines[level].clear();
                    this->right_lines[level].clear();
                    this->update_max_level(true, false);
                }
                assert(this->verify());
                return r;
            }
            /**
             * @brief Remove and return all runs on the current back level.
             * @return std::vector<RunRuleBody>
             */
            std::vector<RunRuleBody> pop_back_sequence()
            {
                std::vector<RunRuleBody> r;
                int64_t level = this->back_level();
                int64_t max_level = this->get_max_level();
                if(max_level == -1){
                    throw std::runtime_error("Error in pop_front_sequence: max_level == -1");
                }

                if (level < max_level)
                {
                    for (uint64_t i = 0; i < this->right_lines[level].size(); i++)
                    {
                        r.push_back(this->right_lines[level][i]);
                    }
                    this->right_lines[level].clear();
                    this->update_back_level(level + 1);
                }
                else
                {
                    for (uint64_t i = 0; i < this->left_lines[level].size(); i++)
                    {
                        r.push_back(this->left_lines[level][i]);
                    }
                    for (uint64_t i = 0; i < this->right_lines[level].size(); i++)
                    {
                        r.push_back(this->right_lines[level][i]);
                    }
                    this->left_lines[level].clear();
                    this->right_lines[level].clear();
                    this->update_max_level(false, true);
                }
                assert(this->verify());
                return r;
            }

            /**
             * @brief Install new ceiling-level left and right lines.
             * ('@param new_left_line', 'Runs for the left ceiling line.')
             * ('@param new_right_line', 'Runs for the right ceiling line.')
             * ('@param level', 'Ceiling level index.')
             */
            void push_new_ceil(std::vector<RunRuleBody> &new_left_line, std::vector<RunRuleBody> &new_right_line, int64_t level)
            {


                if (new_left_line.size() == 0 && new_right_line.size() == 0)
                {
                    throw std::runtime_error("Error in push_new_ceil: new_left_line and new_right_line are empty");
                }
                if(this->get_max_level() >= level){
                    throw std::runtime_error("Error in push_new_ceil: level < this->get_max_level()");
                }
                this->set_dummy_lines(level, true);

                this->left_lines[level].swap(new_left_line);
                this->right_lines[level].swap(new_right_line);

                assert(this->verify());
            }

            /**
             * @brief Insert a run at the front of a level line.
             * ('@param item', 'Run to insert.')
             * ('@param level', 'Target level.')
             */
            void push_front(RunRuleBody item, uint16_t level)
            {
            /**
             * @brief Return the highest level index (or -1 if empty).
             * @return int64_t
             */
                assert(this->get_max_level() >= level);
                this->left_lines[level].insert(this->left_lines[level].begin(), item);
                if (level < this->_front_level)
                {
                    this->_front_level = level;
                }
                assert(this->verify());
            }
            /**
             * @brief Append a run at the back of a level line.
             * ('@param item', 'Run to append.')
             * ('@param level', 'Target level.')
             */
            void push_back(RunRuleBody item, uint16_t level)
            {
            /**
             * @brief Return the highest level index (or -1 if empty).
             * @return int64_t
             */
                assert(this->get_max_level() >= level);
                this->right_lines[level].push_back(item);

                if (level < this->_back_level)
                {
                    this->_back_level = level;
                }
                assert(this->verify());
            }
            /**
             * @brief Expand the frontmost run into lower-level runs.
             */
            void break_front()
            {

            /**
             * @brief Return the level of the frontmost run.
             * @return int64_t
             */
                assert(this->front_level() > 0);
                int64_t level = this->front_level();
                RunRuleBody pop = this->pop_front();
                int64_t child_level = SignatureFunctions::get_level(pop.number, this->dic.get_base_signature_level_list());

                assert(child_level <= level);
                if (pop.power > 1)
                {
                    RunRuleBody tail = RunRuleBody(pop.number, pop.power - 1);
                    if (this->get_max_level() < level)
                    {
                        this->set_dummy_lines(level, true);
                    }
                    this->left_lines[level].insert(this->left_lines[level].begin(), tail);
                    this->update_front_level_by_insertion(level, true);
                }

                if (child_level == level)
                {

                    std::vector<RunRuleBody> pairs;
                    // RLSLPRuleBody child = itemList[pop.number];
                    RunRuleBody::y_break(pop.number, this->dic.get_base_signature_rule_list(), this->dic.get_base_signature_level_list(), pairs);
                    assert(pairs.size() > 0);

                    if (this->get_max_level() < level - 1)
                    {
                        this->set_dummy_lines(level - 1, true);
                    }

                    for (RunRuleBody it : pairs)
                    {
                        assert((int64_t)SignatureFunctions::get_level(it.number, this->dic.get_base_signature_level_list()) <= level - 1);
                        this->left_lines[level - 1].push_back(it);
                    }
                    this->update_front_level_by_insertion(level - 1, true);
                    assert(this->verify());
                }
                else
                {
                    // assert(child_level < level);
                    if (this->get_max_level() < level - 1)
                    {
                        this->set_dummy_lines(level - 1, true);
                    }
                    assert(SignatureFunctions::get_level(pop.number, this->dic.get_base_signature_level_list()) <= level - 1);
                    assert((int64_t)level - 1 >= 0 && (int64_t)level - 1 < (int64_t)this->left_lines.size());
                    this->left_lines[level - 1].push_back(RunRuleBody(pop.number, 1));
                    this->update_front_level_by_insertion(level - 1, true);
                    assert(this->verify());
                }
            }
            /**
             * @brief Expand the backmost run into lower-level runs.
             */
            void break_back()
            {
            /**
             * @brief Return the level of the backmost run.
             * @return int64_t
             */
                assert(this->back_level() > 0);
                int64_t level = this->back_level();
                RunRuleBody pop = this->pop_back();
                uint16_t child_level = SignatureFunctions::get_level(pop.number, this->dic.get_base_signature_level_list());

                assert(child_level <= level);
                if (pop.power > 1)
                {
                    RunRuleBody head = RunRuleBody(pop.number, pop.power - 1);
                    if (this->get_max_level() < level)
                    {
                        this->set_dummy_lines(level, true);
                    }
                    this->right_lines[level].push_back(head);
                    this->update_back_level_by_insertion(level, true);
                }

                if (child_level == level)
                {
                    std::vector<RunRuleBody> pairs;
                    // RLSLPRuleBody child = itemList[pop.number];
                    RunRuleBody::y_break(pop.number, this->dic.get_base_signature_rule_list(), this->dic.get_base_signature_level_list(), pairs);
                    assert(pairs.size() > 0);

                    if (this->get_max_level() < level - 1)
                    {
                        this->set_dummy_lines(level - 1, true);
                    }
                    for (RunRuleBody it : pairs)
                    {
                        assert(SignatureFunctions::get_level(it.number, this->dic.get_base_signature_level_list()) <= level - 1);
                        assert((int64_t)level - 1 >= 0 && (int64_t)level - 1 < (int64_t)this->right_lines.size());
                        this->right_lines[level - 1].push_back(it);
                    }
                    this->update_back_level_by_insertion(level - 1, true);
                    assert(this->verify());
                }
                else
                {
                    if (this->get_max_level() < level - 1)
                    {
                        this->set_dummy_lines(level - 1, true);
                    }
                    this->right_lines[level - 1].push_back(RunRuleBody(pop.number, 1));
                    this->update_back_level_by_insertion(level - 1, true);
                    assert(this->verify());
                }
            }

            /**
             * @brief Decrease or remove power from the frontmost run.
             * ('@param removePower', 'Power units to remove.')
             */
            void pop_front_power(uint64_t removePower)
            {
                int64_t level = this->front_level();
                // int64_t front_level_cache = this->_front_level;
                // int64_t back_level_cache = this->_back_level;
                RunRuleBody top = this->pop_front();
                if (top.power < removePower)
                {
                    throw std::runtime_error("ERROR in pop_front_power");
                }
                else if (top.power == removePower)
                {
                }
                else
                {
                    int64_t k = top.power - removePower;
                    RunRuleBody newTop = RunRuleBody(top.number, k);
                    if (this->get_max_level() < level)
                    {
                        this->set_dummy_lines(level, true);
                    }
                    this->left_lines[level].insert(this->left_lines[level].begin(), newTop);
                    this->update_front_level_by_insertion(level, true);
                    // this->_front_level = front_level_cache;
                    // this->_back_level = back_level_cache;
                }
                assert(this->verify());
            }
            /**
             * @brief Decrease or remove power from the backmost run.
             * ('@param removePower', 'Power units to remove.')
             */
            void pop_back_power(uint64_t removePower)
            {
                int64_t level = this->back_level();
                // int64_t front_level_cache = this->_front_level;
                // int64_t back_level_cache = this->_back_level;

                RunRuleBody top = this->pop_back();
                if (top.power < removePower)
                {
                    throw std::runtime_error("ERROR in pop_back_power");
                }
                else if (top.power == removePower)
                {
                }
                else
                {
                    int64_t k = top.power - removePower;
                    RunRuleBody newTop = RunRuleBody(top.number, k);
                    if (this->get_max_level() < level)
                    {
                        this->set_dummy_lines(level, true);
                    }
                    this->right_lines[level].push_back(newTop);
                    this->update_back_level_by_insertion(level, true);
                    // this->_front_level = front_level_cache;
                    // this->_back_level = back_level_cache;
                }
                assert(this->verify());
            }
            /**
             * @brief Remove the first len characters from the derived string.
             * ('@param len', 'Number of characters to remove.')
             */
            void cut_front_string(int64_t len)
            {
                assert(this->verify());

                if (len < 0)
                    throw std::runtime_error("Error in cut_front_string: len < 0");
                if (len > this->get_string_length())
                {
                    throw std::runtime_error("Error in cut_front_string: len > this->get_string_length()");
                }
                assert(this->verify());
                while (len > 0)
                {
            /**
             * @brief Return the total derived string length.
             * @return int64_t
             */
                    assert(len <= this->get_string_length());
            /**
             * @brief Return the level of the frontmost run.
             * @return int64_t
             */
                    assert(this->front_level() != -1);
                    RLSLPRuleInfo frontItemInfo = this->get_front_info();
                    int64_t basicLength = frontItemInfo.length / frontItemInfo.body.B;
                    if ((int64_t)frontItemInfo.length <= len)
                    {
                        len -= frontItemInfo.length;
                        this->pop_front();
                    }
                    else if ((int64_t)basicLength <= len && len < (int64_t)frontItemInfo.length)
                    {
                        int64_t c = len / basicLength;
                        this->pop_front_power(c);
                        len -= basicLength * c;
                    }
                    else
                    {
            /**
             * @brief Return the level of the frontmost run.
             * @return int64_t
             */
                        assert(this->front_level() > 0);
                        this->break_front();
                    }
                }
                assert(this->verify());
                // assert(this->getLength() == len);
            }
            /**
             * @brief Remove the last len characters from the derived string.
             * ('@param len', 'Number of characters to remove.')
             */
            void cut_back_string(int64_t len)
            {
                assert(this->verify());

                if (len < 0)
                    throw std::runtime_error("Error in cut_back_string: len < 0");
                if (len > this->get_string_length())
                {
                    throw std::runtime_error("Error in cut_back_string: len > this->get_string_length()");
                }
                while (len > 0)
                {
                    assert(this->verify());
            /**
             * @brief Return the total derived string length.
             * @return int64_t
             */
                    assert(len <= this->get_string_length());
                    RLSLPRuleInfo backItemInfo = this->get_back_info();
                    int64_t basicLength = backItemInfo.length / backItemInfo.body.B;
                    if ((int64_t)backItemInfo.length <= len)
                    {
                        len -= backItemInfo.length;
                        this->pop_back();
                    }
                    else if (basicLength <= len && len < (int64_t)backItemInfo.length)
                    {
                        int64_t c = len / basicLength;
                        this->pop_back_power(c);
                        len -= basicLength * c;
                    }
                    else
                    {
            /**
             * @brief Return the level of the backmost run.
             * @return int64_t
             */
                        assert(this->back_level() > 0);
                        this->break_back();
                    }
                }
            }

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Convert functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Return a deep copy of this vector.
             * @return RunRuleVector
             */
            RunRuleVector copy() const
            {
                RunRuleVector r(*this);
                return r;
            }
            /**
             * @brief Build a power-aware derivation-tree representation (unimplemented).
             * @return std::vector<std::string>
             */
            std::vector<std::string> to_power_derivation_tree() const
            {
                throw std::runtime_error("ERROR in to_power_derivation_tree");
            }
            /**
             * @brief Return a string of run labels in left-to-right order.
             * @return std::string
             */
            std::string to_string() const
            {
                auto items = this->to_vector();
                std::string r = "";
                for (RunRuleBody it : items)
                {
                    r.append(it.to_string());
                }
                return r;
            }
            /**
             * @brief Return run labels concatenated with their derived lengths.
             * @return std::string
             */
            std::string to_string_with_length() const
            {
                auto items = this->to_vector();
                std::string r = "";
                for (RunRuleBody it : items)
                {
                    r.append(it.to_string());
                    r.append(std::to_string(it.get_length(this->dic.get_base_signature_length_list())));
                }
                return r;
            }
            /**
             * @brief Flatten level lines into a left-to-right run sequence.
             * @return std::vector<RunRuleBody>
             */
            std::vector<RunRuleBody> to_vector() const
            {
                std::vector<RunRuleBody> r;

                int64_t max_level = this->get_max_level();

                for (int64_t i = 0; i <= max_level; i++)
                {
                    for (uint64_t j = 0; j < this->left_lines[i].size(); j++)
                    {
                        r.push_back(this->left_lines[i][j]);
                    }
                }
                for (int64_t i = max_level; i >= 0; i--)
                {
                    for (uint64_t j = 0; j < this->right_lines[i].size(); j++)
                    {
                        r.push_back(this->right_lines[i][j]);
                    }
                }
                return r;
            }
            /**
             * @brief Flatten level lines into a right-to-left run sequence.
             * @return std::vector<RunRuleBody>
             */
            std::vector<RunRuleBody> to_backward_vector() const
            {
                std::vector<RunRuleBody> r;

                int64_t max_level = this->get_max_level();

                for (int64_t i = 0; i <= max_level; i++)
                {
                    int64_t size = this->right_lines[i].size();
                    for (int64_t j = size-1; j >= 0; j--)
                    {
                        r.push_back(this->right_lines[i][j]);
                    }
                }
                for (int64_t i = max_level; i >= 0; i--)
                {
                    int64_t size = this->left_lines[i].size();
                    for (int64_t j = size-1; j >= 0; j--)
                    {
                        r.push_back(this->left_lines[i][j]);
                    }
                }
                return r;

            }

            //}@

            /**
             * @brief Build a multi-line derivation-tree visualization.
             * @param padding Padding width for visualization.
             * @return Computed integer value.
             */
            std::vector<std::string> compute_derivation_tree(uint64_t padding = 3) const
            {
                std::vector<std::string> output_strings;

                if (this->is_empty())
                {
                    output_strings.push_back("[EMPTY]");
                    return output_strings;
                }
                else
                {
                    output_strings.resize(this->get_string_length());
                    uint16_t current_level = this->get_max_level();
                    compute_derivation_tree_sub(*this, current_level, padding, output_strings);
                    return output_strings;
                }
            }
            /**
             * @brief Print the derivation-tree visualization.
             * @param padding Padding width for visualization.
             * @param message_paragraph Indentation level for formatted output.
             */
            void print_derivation_tree(uint64_t padding = 3, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                auto r = this->compute_derivation_tree(padding);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== Derivation Tree ====" << std::endl;
                for (auto it : r)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << it << std::endl;
                }
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== Derivation Tree[END] ====" << std::endl;
            }
            /**
             * @brief Print level-line contents to standard output.
             * ('@param message_paragraph', 'Indentation level for formatted output.')
             */
            void print_info(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== RunRuleVector Info ====" << std::endl;
                int64_t max_level = this->get_max_level();
                if (max_level >= 0)
                {
                    for (int64_t h = max_level; h >= 0; h--)
                    {
                        std::cout << stool::Message::get_paragraph_string(message_paragraph) << "level " << h << " : ";
                        for (auto it : this->left_lines[h])
                        {
                            std::cout << it.to_string() << " ";
                        }
                        std::cout << " | ";
                        for (auto it : this->right_lines[h])
                        {
                            std::cout << it.to_string() << " ";
                        }
                        std::cout << std::endl;
                    }
                }
                else
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[EMPTY]" << std::endl;
                }
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== RunRuleVector Info[END] ====" << std::endl;
            }
            /**
             * @brief Verify internal level-line invariants.
             * @return True if the check succeeds.
             */
            bool verify() const
            {
                int64_t max_level = this->get_max_level();
                for (int64_t i = 0; i <= max_level; i++)
                {
                    for (int64_t j = 0; j < (int64_t)this->left_lines[i].size(); j++)
                    {
                        if (i < SignatureFunctions::get_level(this->left_lines[i][j].number, this->dic.get_base_signature_level_list()))
                        {
                            throw std::runtime_error("Error in verify: i < SignatureFunctions::get_level(this->left_lines[i][j].number, this->base_signature_level_list)");
                        }
                    }
                    for (int64_t j = 0; j < (int64_t)this->right_lines[i].size(); j++)
                    {
                        if (i < SignatureFunctions::get_level(this->right_lines[i][j].number, this->dic.get_base_signature_level_list()))
                        {
                            throw std::runtime_error("Error in verify: i < SignatureFunctions::get_level(this->right_lines[i][j].number, this->base_signature_level_list)");
                        }
                    }
                }
                if (max_level >= 0)
                {
                    if (this->left_lines[max_level].size() + this->right_lines[max_level].size() == 0)
                    {
                        this->print_info();
                        throw std::runtime_error("Error in verify: this->left_lines[max_level].size() + this->right_lines[max_level].size() == 0");
                    }

                    int32_t exact_front_level = max_level;
                    int32_t exact_back_level = max_level;

                    for (int64_t i = 0; i <= max_level; i++)
                    {
                        if (this->left_lines[i].size() > 0)
                        {
                            exact_front_level = i;
                            break;
                        }
                    }
                    for (int64_t i = 0; i <= max_level; i++)
                    {
                        if (this->right_lines[i].size() > 0)
                        {
                            exact_back_level = i;
                            break;
                        }
                    }

                    if (exact_front_level != this->_front_level)
                    {
                        throw std::runtime_error("Error in verify: exact_front_level != this->_front_level");
                    }
                    if (exact_back_level != this->_back_level)
                    {
                        std::cout << "exact_back_level: " << exact_back_level << ", this->_back_level: " << this->_back_level << std::endl;
                        this->print_info();

                        throw std::runtime_error("Error in verify: exact_back_level != this->_back_level");
                    }
                }
                else
                {
                    if (this->_front_level != -1)
                    {
                        throw std::runtime_error("Error in verify: this->_front_level != -1");
                    }
                    if (this->_back_level != -1)
                    {
                        throw std::runtime_error("Error in verify: this->_back_level != -1");
                    }
                }

                return true;
            }
            /**
             * @brief Verify minimum run counts per level for context lengths.
             * @param left_length Required left context length.
             * @param right_length Required right context length.
             * @return True if the check succeeds.
             */
            bool verify_sequence_length(uint64_t left_length, uint64_t right_length)
            {
                int64_t max_level = this->get_max_level();
                for (int64_t h = 0; h < max_level; h++)
                {
                    int64_t counterL = 0;
                    int64_t counterR = 0;
                    for(int64_t i = 0; i < (int64_t)this->left_lines[h].size(); i++){
                        counterL += this->left_lines[h][i].power;
                    }
                    for(int64_t i = 0; i < (int64_t)this->right_lines[h].size(); i++){
                        counterR += this->right_lines[h][i].power;
                    }
                    if ((uint64_t)counterL < left_length)
                    {
                        throw std::runtime_error("Error in verify_sequence_length: this->left_lines[h].size() < left_length");
                    }
                    if ((uint64_t)counterR < right_length)
                    {
                        throw std::runtime_error("Error in verify_sequence_length: this->right_lines[h].size() < right_length");
                    }
                }
                return true;
            }

            template <typename OUTPUT_VEC_TYPE = std::vector<uint8_t>>
            /**
             * @brief Append the expanded string of this rule to an output container.
             * @param output Output container for decompressed data.
             */
            void decompress(OUTPUT_VEC_TYPE &output) const
            {
                std::vector<RunRuleBody> items = this->to_vector();
                for (auto it : items)
                {
                    it.decompress(this->dic.get_base_signature_rule_list(), output);
                    //body.decompress(this->base_signature_rule_list, output);
                }
            }
            /**
             * @brief Decompress and return the original text as a string.
             * @return std::string
             */
            std::string to_original_text_str() const
            {
                std::string r;
                this->decompress(r);
                return r;
            }

            /**
             * @brief Return cumulative end positions of each run.
             * @return Computed integer value.
             */
            std::vector<uint64_t> to_starting_position_vector() const
            {
                std::vector<uint64_t> r;
                auto items = this->to_vector();

                const std::vector<uint64_t>& base_signature_length_list = this->dic.get_base_signature_length_list();
                if (items.size() > 0)
                {
                    uint64_t p = 0;
                    for (RunRuleBody it : items)
                    {
                        p += SignatureFunctions::get_length(it.number, base_signature_length_list) * it.power;
                        r.push_back(p);
                    }
                    r.pop_back();
                }

                return r;
            }

        private:
            /**
             * @brief Grow level lines up to a target level with empty slots.
             * @param level Derivation-tree level.
             * @param insertionFlag Whether dummy lines are grown by insertion.
             */
            void set_dummy_lines(int64_t level, bool insertionFlag)
            {
                if (level <= this->get_max_level())
                {
                    throw std::runtime_error("Error in set_dummy_lines: level < this->get_max_level()");
                }
                else if (insertionFlag)
                {
                    if (this->get_max_level() != -1)
                    {
                        bool updateFlagL = (this->_front_level == this->get_max_level()) && (this->left_lines[this->get_max_level()].size() == 0);
                        bool updateFlagR = (this->_back_level == this->get_max_level()) && (this->right_lines[this->get_max_level()].size() == 0);

                        if (updateFlagL)
                        {
                            this->_front_level = level;
                        }

                        if (updateFlagR)
                        {
                            this->_back_level = level;
                        }
                    }
                    else
                    {
                        this->_front_level = level;
                        this->_back_level = level;
                    }
                    while (this->get_max_level() < level)
                    {
                        this->left_lines.push_back(LINE());
                        this->right_lines.push_back(LINE());
                    }
                }
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Static functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{
        public:
            /**
             * @brief Create an empty vector bound to dic.
             * ('@param dic', 'Layered RLSLP dictionary.')
             * @return static RunRuleVector
             */
            static RunRuleVector create_empty_vector(const DictionaryForLayeredRLSLP &dic)
            {
                return RunRuleVector(dic);
            }

            /**
             * @brief Create a substring vector for signature item over [pos, pos+len).
             * ('@param item', 'Encoded signature.')
             * ('@param pos', 'Start offset.')
             * ('@param len', 'Length.')
             * ('@param dic', 'Dictionary.')
             * @return Substring run-rule vector.
             */
            static RunRuleVector create(int64_t item, int64_t pos, int64_t len, const DictionaryForLayeredRLSLP &dic)
            {
                assert(len >= 0);
                RunRuleVector r(item, dic);

                int64_t item_len = SignatureFunctions::get_length(item, dic.get_base_signature_length_list());
                if (pos >= item_len)
                {
                    throw std::runtime_error("Error in create: pos >= item_len");
                }
                if (pos + len > item_len)
                {
                    throw std::runtime_error("Error in create: pos + len > item_len");
                }

                return r.create_substring_link(pos, len);
            }
            /**
             * @brief Create a suffix vector for signature item starting at pos.
             * ('@param item', 'Encoded signature.')
             * ('@param pos', 'Start offset.')
             * ('@param dic', 'Dictionary.')
             * @return Suffix run-rule vector.
             */
            static RunRuleVector create(int64_t item, int64_t pos, const DictionaryForLayeredRLSLP &dic)
            {
                int64_t len = SignatureFunctions::get_length(item, dic.get_base_signature_length_list());
                if (pos >= len)
                {
                    throw std::runtime_error("Error in create: pos >= len");
                }

                return RunRuleVector::create(item, pos, len - pos, dic);
            }

            //}@

        private:
            /**
             * @brief Break the ceiling level into the level below.
             */
            void break_ceil()
            {
                int32_t max_level = this->get_max_level();

                if (max_level > 0)
                {
                    std::vector<RunRuleBody> tmp;
                    for (auto it : this->left_lines[max_level])
                    {
                        break_item(it, max_level, this->dic.get_base_signature_rule_list(), this->dic.get_base_signature_level_list(), tmp);
                    }
                    for (auto it : this->right_lines[max_level])
                    {
                        break_item(it, max_level, this->dic.get_base_signature_rule_list(), this->dic.get_base_signature_level_list(), tmp);
                    }
                    for (auto it : tmp)
                    {
                        this->left_lines[max_level - 1].push_back(it);
                    }
                    this->left_lines.pop_back();
                    this->right_lines.pop_back();
                    if (this->_front_level == max_level)
                    {
                        this->_front_level = max_level - 1;
                    }
                    if (this->_back_level == max_level)
                    {
                        this->_back_level = max_level - 1;
                    }
                }
                else
                {
                    throw std::runtime_error("Error in break_ceil: max_level == 0");
                }
            }

            /**
             * @brief Replace the left line at a level and update the front level.
             * @param line New left-line content.
             * @param level Derivation-tree level.
             */
            void swap_left_line(LINE &line, uint16_t level)
            {
                #ifdef DEBUG
                int64_t max_level = this->get_max_level();
                assert((int64_t)level <= max_level);
                #endif



                if (this->_front_level < (int32_t)level && line.size() > 0)
                {
                    this->_front_level = level;
                }
                else if (this->_front_level == (int32_t)level && line.size() == 0)
                {
                    this->update_front_level(level + 1);
                }

                this->left_lines[level].swap(line);
            }

            /**
             * @brief swap_right_line operation.
             * @param line line.
             * @param level Derivation-tree level.
             */
            void swap_right_line(LINE &line, uint16_t level)
            {
                #ifdef DEBUG
                int64_t max_level = this->get_max_level();
                assert((int64_t)level <= max_level);
                #endif

                if (this->_back_level < (int32_t)level && line.size() > 0)
                {
                    this->_back_level = level;
                }
                else if (this->_back_level == (int32_t)level && line.size() == 0)
                {
                    this->update_back_level(level + 1);
                }

                this->right_lines[level].swap(line);
            }
            /**
             * @brief Adjust front level after inserting runs.
             * @param insertion_level Level at which insertion occurred.
             * @param is_front_insertion Whether the insertion is at the front.
             */
            void update_front_level_by_insertion(int64_t insertion_level, bool is_front_insertion)
            {
                if (is_front_insertion)
                {
                    if (insertion_level < this->_front_level || this->_front_level == -1)
                    {
                        this->_front_level = insertion_level;
                    }
                }
                else
                {
                    if (this->_front_level == -1)
                    {
                        this->_front_level = insertion_level;
                    }
                }
                /*
                else
                {
                    if (insertion_level < this->_back_level)
                    {
                        this->_back_level = insertion_level;
                    }
                }
                */
            }
            /**
             * @brief Adjust back level after inserting runs.
             * @param insertion_level Level at which insertion occurred.
             * @param is_back_insertion Whether the insertion is at the back.
             */
            void update_back_level_by_insertion(int64_t insertion_level, bool is_back_insertion)
            {
                if (is_back_insertion)
                {
                    if (insertion_level < this->_back_level || this->_back_level == -1)
                    {
                        this->_back_level = insertion_level;
                    }
                }
                else
                {
                    int64_t max_level = this->get_max_level();
                    if (this->_back_level == -1)
                    {
                        this->_back_level = max_level;
                    }
                }
            }
            /**
             * @brief Recompute the front level from a minimum level.
             * @param min_level Minimum level to consider when updating.
             */
            void update_front_level(int64_t min_level)
            {
                int64_t max_level = this->get_max_level();
                if (max_level >= 0)
                {
                    this->_front_level = max_level;
                    for (int64_t i = min_level; i <= max_level; i++)
                    {
                        if (this->left_lines[i].size() > 0)
                        {
                            this->_front_level = i;
                            break;
                        }
                    }
                }
                else
                {
                    this->_front_level = -1;
                }
            }
            /**
             * @brief Recompute the back level from a minimum level.
             * @param min_level Minimum level to consider when updating.
             */
            void update_back_level(int64_t min_level)
            {
                int64_t max_level = this->get_max_level();
                if (max_level >= 0)
                {
                    this->_back_level = max_level;
                    for (int64_t i = min_level; i <= max_level; i++)
                    {
                        if (this->right_lines[i].size() > 0)
                        {
                            this->_back_level = i;
                            break;
                        }
                    }
                }
                else
                {
                    this->_back_level = -1;
                }
            }

            /**
             * @brief Recompute both front and back levels.
             */
            void update_front_and_back_level()
            {
                int64_t max_level = this->get_max_level();

                if (max_level >= 0)
                {
                    this->_front_level = max_level;
                    this->_back_level = max_level;
                    for (int64_t i = 0; i <= max_level; i++)
                    {
                        if (this->left_lines[i].size() > 0)
                        {
                            this->_front_level = i;
                            break;
                        }
                    }
                    for (int64_t i = 0; i <= max_level; i++)
                    {
                        if (this->right_lines[i].size() > 0)
                        {
                            this->_back_level = i;
                            break;
                        }
                    }
                    if (this->_front_level == -1)
                    {
                        throw std::runtime_error("Error in update_front_and_back_level: this->_front_level == -1");
                    }
                    if (this->_back_level == -1)
                    {
                        this->print_info();
                        throw std::runtime_error("Error in update_front_and_back_level: this->_back_level == -1");
                    }
                }
                else
                {
                    this->_front_level = -1;
                    this->_back_level = -1;
                }
            }
            /**
             * @brief Shrink level lines after removing ceiling runs.
             * @param left_empty Whether the left ceiling line became empty.
             * @param right_empty Whether the right ceiling line became empty.
             */
            void update_max_level(bool left_empty, bool right_empty)
            {

                int64_t max_level = this->get_max_level();
                if (max_level == -1)
                {
                    throw std::runtime_error("Error in update_max_level: max_level == -1");
                }

                int64_t current_level = max_level;
                while (current_level >= 0)
                {
                    if (this->left_lines[current_level].size() > 0 || this->right_lines[current_level].size() > 0)
                    {
                        break;
                    }
                    else
                    {
                        this->left_lines.pop_back();
                        this->right_lines.pop_back();
                        current_level--;
                    }
                }

                if (left_empty && !right_empty)
                {
                    if (current_level >= 0)
                    {
                        this->_front_level = current_level;
                    }
                    else
                    {
                        this->_front_level = -1;
                        this->_back_level = -1;
                    }
                }
                else if (!left_empty && right_empty)
                {
                    if (current_level >= 0)
                    {
                        this->_back_level = current_level;
                    }
                    else
                    {
                        this->_front_level = -1;
                        this->_back_level = -1;
                    }
                }
                else
                {
                    throw std::runtime_error("Error in update_max_level: left_empty && right_empty");
                }
            }
            /**
             * @brief Return a copy containing only the first len characters.
             * @param len Length of the substring or prefix/suffix.
             * @return Computed integer value.
             */
            RunRuleVector copy_front(int64_t len) const
            {
                if (len < 0)
                    throw std::runtime_error("Error in copy_front: len < 0");
                if (len > this->get_string_length())
                    throw std::runtime_error("Error in copy_front: len > this->get_length()");

                RunRuleVector tmp = this->get_front_items_with_level(INT64_MAX, len);
                int64_t tmpLen = tmp.get_string_length();
                if (tmpLen < len)
                {
                    std::cout << "tmpLen: " << tmpLen << std::endl;
                    std::cout << "len: " << len << std::endl;
                    std::cout << "tmp.to_string_with_length(): " << tmp.to_string_with_length() << std::endl;
                    std::cout << "this->to_string_with_length(): " << this->to_string_with_length() << std::endl;
                    throw std::runtime_error("Error in copy_front: tmpLen < len");
                }
                int64_t remainCutLength = tmpLen - len;
                tmp.cut_back_string(remainCutLength);
            /**
             * @brief Return the total derived string length.
             * @return int64_t
             */
                assert(tmp.get_string_length() == len);
                return tmp;
            }

            /**
             * @brief Decompose a run at a given level into lower-level runs.
             * @param item Rule body item.
             * @param level Derivation-tree level.
             * @param base_signature_rule_list Base-signature rule list (D).
             * @param base_signature_level_list Base-signature level list (H).
             * @param output Output container for decompressed data.
             */
            static void break_item(const RunRuleBody &item, uint64_t level, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint16_t> &base_signature_level_list, std::vector<RunRuleBody> &output)
            {
                uint16_t child_level = SignatureFunctions::get_level(item.number, base_signature_level_list);

                assert(child_level <= level);

                if (child_level == level)
                {
                    std::vector<RunRuleBody> tmp;
                    RunRuleBody::y_break(item.number, base_signature_rule_list, base_signature_level_list, tmp);
                    for (uint64_t i = 0; i < item.power; i++)
                    {
                        for (RunRuleBody it : tmp)
                        {
                            output.push_back(it);
                        }
                    }
                }
                else
                {
                    output.push_back(item);
                }
            }
            /**
             * @brief Recursively build derivation-tree visualization lines.
             * @param vec RunRuleVector to visualize.
             * @param current_level Current level in the derivation tree.
             * @param padding Padding width for visualization.
             * @param output_strings Output lines for the visualization.
             */
            static void compute_derivation_tree_sub(const RunRuleVector &vec, uint16_t current_level, uint64_t padding, std::vector<std::string> &output_strings)
            {
                int64_t max_level = vec.get_max_level();
                std::vector<RunRuleBody> items;
                for (auto it : vec.left_lines[max_level])
                {
                    assert(SignatureFunctions::get_level(it.number, vec.dic.get_base_signature_level_list()) <= current_level);
                    items.push_back(it);
                }
                for (auto it : vec.right_lines[max_level])
                {
                    assert(SignatureFunctions::get_level(it.number, vec.dic.get_base_signature_level_list()) <= current_level);
                    items.push_back(it);
                }

                if (max_level == 0)
                {
                    uint64_t p = 0;
                    for (auto it : items)
                    {
                        int64_t power = it.power;
                        char c = RLSLPRuleBody::decodeRule(it.number, vec.dic.get_base_signature_rule_list()).A;
                        for (int64_t i = 0; i < power; i++)
                        {
                            assert(p < output_strings.size());
                            output_strings[p].push_back(c);
                            output_strings[p++].append(": ");
                        }
                    }
                }
                else
                {
                    RunRuleVector next_vec = vec.copy();
                    next_vec.break_ceil();
                    compute_derivation_tree_sub(next_vec, current_level - 1, padding, output_strings);
                }
                std::cout << std::flush;

                uint64_t starting_position = vec.compute_ceil_string_starting_position();
                uint64_t p = 0;
                const std::vector<uint64_t>& base_signature_length_list = vec.dic.get_base_signature_length_list();
                for (uint64_t i = 0; i < starting_position; i++)
                {
                    assert(p < output_strings.size());
                    output_strings[p++].push_back('*');
                }
                for (auto it : items)
                {
                    assert(p < output_strings.size());
                    output_strings[p].push_back('-');

                    output_strings[p++].append(it.to_string());
                    uint64_t length = it.get_length(base_signature_length_list);
                    for (uint64_t j = 1; j < length; j++)
                    {
                        output_strings[p++].push_back('-');
                    }
                }
                while (p < output_strings.size())
                {
                    assert(p < output_strings.size());
                    output_strings[p++].push_back('*');
                }

                uint64_t max_width = 0;
                for (uint64_t i = 0; i < output_strings.size(); i++)
                {
                    max_width = std::max(max_width, (uint64_t)output_strings[i].size());
                }
                for (uint64_t i = 0; i < output_strings.size(); i++)
                {
                    while (output_strings[i].size() < max_width + padding)
                    {
                        output_strings[i].append("_");
                    }
                }
            }
        };

}