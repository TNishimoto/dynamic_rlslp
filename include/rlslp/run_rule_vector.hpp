#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include <stack>
#include <list>
#include "./dictionary_for_layered_rlslp.hpp"
#include "stool/include/all.hpp"

namespace dynRLSLP
{


        /**
         * @brief A list of RLSLP nonterminals to represent a substring.
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
            RunRuleVector(const DictionaryForLayeredRLSLP &dic) : left_lines(), right_lines(), dic(dic)
            {
                this->update_front_and_back_level();
            }
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

            RunRuleVector(int64_t number, const DictionaryForLayeredRLSLP &dic)
                : left_lines(), right_lines(), dic(dic)
            {
                uint64_t level = dic.get_level(number);
                RunRuleBody item = RunRuleBody(number, 1);
                this->left_lines.resize(level + 1);
                this->right_lines.resize(level + 1);
                this->left_lines[level].push_back(item);
                this->update_front_and_back_level();

                assert(this->get_string_length() == (int64_t)dic.get_length(number));
            }
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
            bool is_empty() const {
                return this->get_max_level() == -1;
            }
            uint64_t ceil_size() const
            {
                int64_t max_level = this->get_max_level();
                return this->left_lines[max_level].size() + this->right_lines[max_level].size();
            }
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

            RunRuleVector create_empty_link() const
            {
                return RunRuleVector(this->dic);
            }
            bool check_empty_left_line(int64_t h) const
            {
                return this->left_lines[h].size() == 0;
            }

            //}@

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{

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

            int64_t front_level() const
            {
                return this->_front_level;
            }
            int64_t back_level() const
            {
                return this->_back_level;
            }

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
                    assert(level == this->get_max_level());
                    return this->right_lines[level][0];
                }
            }
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
                    assert(level == this->get_max_level());
                    uint64_t size = this->left_lines[level].size();

                    return this->left_lines[level][size - 1];
                }
            }
            int64_t left_line_size(int64_t h)
            {
                return this->left_lines[h].size();
            }
            int64_t right_line_size(int64_t h)
            {
                return this->right_lines[h].size();
            }

            int64_t front_length() const
            {
                assert(!this->is_empty());
                return this->peek_front().get_length(this->dic.get_base_signature_length_list());
            }
            int64_t back_length() const
            {
                assert(!this->is_empty());
                return this->peek_back().get_length(this->dic.get_base_signature_length_list());
            }

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
            RLSLPRuleInfo get_front_info() const
            {
                assert(this->verify());
                assert(this->front_level() != -1);
                RunRuleBody p = this->peek_front();
                uint64_t length = SignatureFunctions::get_length(p.number, this->dic.get_base_signature_length_list()) * p.power;
                uint64_t level = SignatureFunctions::get_level(p.number, this->dic.get_base_signature_level_list());
                return RLSLPRuleInfo(RLSLPRuleBody::create_run_rule_body(p.number, p.power), length, level);
            }
            RLSLPRuleInfo get_back_info() const
            {
                assert(this->verify());
                assert(this->back_level() != -1);
                RunRuleBody p = this->peek_back();
                uint64_t length = SignatureFunctions::get_length(p.number, this->dic.get_base_signature_length_list()) * p.power;
                uint64_t level = SignatureFunctions::get_level(p.number, this->dic.get_base_signature_level_list());
                return RLSLPRuleInfo(RLSLPRuleBody::create_run_rule_body(p.number, p.power), length, level);
            }
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
                assert(r.get_string_length() == len);
                return r;
            }

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
            void swap_list(RunRuleVector &other)
            {
                std::swap(this->left_lines, other.left_lines);
                std::swap(this->right_lines, other.right_lines);
                std::swap(this->_front_level, other._front_level);
                std::swap(this->_back_level, other._back_level);
                assert(this->verify());
            }

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

            void push_front(RunRuleBody item, uint16_t level)
            {
                assert(this->get_max_level() >= level);
                this->left_lines[level].insert(this->left_lines[level].begin(), item);
                if (level < this->_front_level)
                {
                    this->_front_level = level;
                }
                assert(this->verify());
            }
            void push_back(RunRuleBody item, uint16_t level)
            {
                assert(this->get_max_level() >= level);
                this->right_lines[level].push_back(item);

                if (level < this->_back_level)
                {
                    this->_back_level = level;
                }
                assert(this->verify());
            }
            void break_front()
            {

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
            void break_back()
            {
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
                    assert(len <= this->get_string_length());
                    assert(this->front_level() != -1);
                    RLSLPRuleInfo frontItemInfo = this->get_front_info();
                    int64_t basicLength = frontItemInfo.length / frontItemInfo.item.B;
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
                        assert(this->front_level() > 0);
                        this->break_front();
                    }
                }
                assert(this->verify());
                // assert(this->getLength() == len);
            }
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
                    assert(len <= this->get_string_length());
                    RLSLPRuleInfo backItemInfo = this->get_back_info();
                    int64_t basicLength = backItemInfo.length / backItemInfo.item.B;
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
            RunRuleVector copy() const
            {
                RunRuleVector r(*this);
                return r;
            }
            std::vector<std::string> to_power_derivation_tree() const
            {
                throw std::runtime_error("ERROR in to_power_derivation_tree");
            }
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
            void decompress(OUTPUT_VEC_TYPE &output) const
            {
                std::vector<RunRuleBody> items = this->to_vector();
                for (auto it : items)
                {
                    it.decompress(this->dic.get_base_signature_rule_list(), output);
                    //body.decompress(this->base_signature_rule_list, output);
                }
            }
            std::string to_original_text_str() const
            {
                std::string r;
                this->decompress(r);
                return r;
            }

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
            static RunRuleVector create_empty_vector(const DictionaryForLayeredRLSLP &dic)
            {
                return RunRuleVector(dic);
            }

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

            /*
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
                assert(tmp.get_string_length() == len);
                return tmp;
            }

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