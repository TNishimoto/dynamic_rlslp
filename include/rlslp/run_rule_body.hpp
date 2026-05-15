#pragma once
#include "./rlslp_rule_body.hpp"
#include "./signature_functions.hpp"
#include "stool/include/all.hpp"
namespace dynRLSLP
{
        /**
         * @brief A representation of a run of RLSLP nonterminals
         * @ingroup RLSLPClasses
         */
        struct RunRuleBody
        {
        public:
            uint64_t number;
            uint64_t power;

            RunRuleBody(uint64_t number, uint64_t power)
                : number(number), power(power)
            {
            }
            RunRuleBody()
                : number(0), power(0)
            {
            }

            RunRuleBody(const RunRuleBody &) = default;
            RunRuleBody &operator=(const RunRuleBody &) = default;
            RunRuleBody(RunRuleBody &&) = default;
            RunRuleBody &operator=(RunRuleBody &&) = default;
            /*
            bool is_dummy_run() const {
                return this->power == UINT64_MAX;
            }
            */
            RunRuleBody operator+(RunRuleBody item2) const
            {
                if (this->number == item2.number)
                {
                    return RunRuleBody(this->number, this->power + item2.power);
                }
                else
                {
                    throw std::logic_error("RunRuleBody::operator+: this->number != item2.number");
                }
            }
            static RunRuleBody construct_repair_grammar(int64_t number, const std::vector<RLSLPRuleBody> &itemList)
            {
                RLSLPRuleBody item = itemList[number];
                if (item.get_type() == RLSLPRuleType::Power)
                {
                    return RunRuleBody(item.A, item.B);
                }
                else if (item.get_type() == RLSLPRuleType::Pair)
                {
                    return RunRuleBody(number, 1);
                }
                else if (item.get_type() == RLSLPRuleType::Character)
                {
                    return RunRuleBody(number, 1);
                }
                else
                {
                    throw std::logic_error("RunRuleBody::construct_repair_grammar: unknown rule type");
                }
            }
            
            uint64_t get_length(const std::vector<uint64_t>& base_signature_length_list) const
            {
                return base_signature_length_list[SignatureFunctions::get_base_signature(this->number)] * this->power;
            }
            /*
            uint64_t get_height(const std::vector<uint16_t> &base_signature_level_list) const
            {
                return SignatureFunctions::get_level(this->number, base_signature_level_list);
            }
            */
            
            template <typename OUTPUT_VEC_TYPE = std::vector<RunRuleBody>>
            static void pow(const std::vector<int64_t> &items, OUTPUT_VEC_TYPE &output)
            {
                output.clear();
                if (items.size() == 0)
                    return;
                int64_t k = 0;
                int64_t tmp = items[0];
                for (int64_t i = 0; i < (int64_t)items.size(); i++)
                {
                    if (tmp == items[i])
                    {
                        k++;
                    }
                    else
                    {
                        RunRuleBody r = RunRuleBody(tmp, k);
                        output.push_back(r);
                        k = 1;
                        tmp = items[i];
                    }
                }
                RunRuleBody r = RunRuleBody(tmp, k);
                output.push_back(r);
                k = 0;
            }

            std::string to_string() const
            {
                if (this->power == 1)
                {
                    return "R(" + std::to_string(this->number) + ")";
                }
                else
                {
                    return "R(" + std::to_string(this->number) + "^" + std::to_string(this->power) + ")";
                }
            }

            template <typename SEQ_TYPE = std::vector<RunRuleBody>>
            static bool is_single_signature(SEQ_TYPE &seq)
            {
                if (seq.size() != 1)
                {
                    return false;
                }
                else
                {
                    return seq[0].power == 1;
                }
            }

            RLSLPRuleBody to_signature_item()
            {
                return RLSLPRuleBody::create_run_rule_body(this->number, this->power);
            }

            template <typename VEC_TYPE = std::vector<RunRuleBody>>
            static void print_vector(const VEC_TYPE &items, const std::string name, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << name << ": " << std::flush;
                for (auto it : items)
                {
                    std::cout << it.to_string();
                }
                std::cout << std::endl;
            }
            template <typename VEC_TYPE = std::vector<RunRuleBody>>
            static bool verify_vector(const VEC_TYPE &items)
            {
                for (int64_t i = 1; i < (int64_t)items.size(); i++)
                {
                    if (items[i].number == items[i - 1].number)
                    {
                        throw std::runtime_error("Error in verify_vector: items[i].number == items[i-1].number");
                    }
                }
                return true;
            }
            template <typename VEC_TYPE = std::vector<RunRuleBody>>
            static void merge_same_signatures(VEC_TYPE &items)
            {
                VEC_TYPE tmp;
                if (items.size() > 0)
                {
                    tmp.push_back(items[0]);
                    for (int64_t i = 1; i < (int64_t)items.size(); i++)
                    {
                        if (items[i].number == tmp.back().number)
                        {
                            auto last = tmp.back();
                            tmp.pop_back();
                            tmp.push_back(RunRuleBody(last.number, last.power + items[i].power));
                        }
                        else
                        {
                            tmp.push_back(items[i]);
                        }
                    }
                    items.swap(tmp);
                }
            }
            template <typename VEC_TYPE = std::vector<RunRuleBody>>
            static int64_t compute_string_length(const VEC_TYPE &items, const std::vector<uint64_t>& base_signature_length_list)
            {
                uint64_t length = 0;
                for (int64_t i = 0; i < (int64_t)items.size(); i++)
                {
                    length += items[i].get_length(base_signature_length_list);
                }
                return length;
            }

            template <typename OUTPUT_VEC_TYPE = std::vector<uint8_t>>
            void decompress(const std::vector<RLSLPRuleBody> &base_signature_rule_list, OUTPUT_VEC_TYPE &output) const {
                for(int64_t i = 0; i < (int64_t)this->power; i++){
                    RLSLPRuleBody item = RLSLPRuleBody::decodeRule(this->number, base_signature_rule_list);
                    item.decompress(base_signature_rule_list, output);
                }
            }
			static void y_break(SignatureWithRelativeLevel signature, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint16_t> &base_signature_level_list, std::vector<RunRuleBody> &output)
            {
                uint16_t child_level = SignatureFunctions::get_level(signature, base_signature_level_list);
                RLSLPRuleBody child = RLSLPRuleBody::decodeRule(signature, base_signature_rule_list);
                if(child.get_type() == RLSLPRuleType::Power){
                    output.push_back(RunRuleBody(child.A, child.B));
                }else if(child.get_type() == RLSLPRuleType::Pair){
                    uint64_t left_level = SignatureFunctions::get_level(child.A, base_signature_level_list);
                    if(left_level < child_level){
                        output.push_back(RunRuleBody(child.A, 1));
                        output.push_back(RunRuleBody(child.B, 1));
                    }else{
                        y_break(child.A, base_signature_rule_list, base_signature_level_list, output);
                        output.push_back(RunRuleBody(child.B, 1));
                    }
                }
                else if(child.get_type() == RLSLPRuleType::Signature){
                    uint64_t level = SignatureFunctions::get_level(child.A, base_signature_level_list);
                    if(level < child_level){
                        output.push_back(RunRuleBody(child.A, 1));
                    }else{
                        y_break(child.A, base_signature_rule_list, base_signature_level_list, output);
                    }

                }
                else{
                    throw std::runtime_error("Error in break_item: child.get_type() is not Power, Pair, or Character");
                }
            }
        };

        struct RunRuleBodyWidthLevel
        {
            RunRuleBody body;
            uint16_t level;

            RunRuleBodyWidthLevel(const RunRuleBody &body, uint16_t level) : body(body), level(level) {}
            RunRuleBodyWidthLevel(uint64_t number, uint64_t power, uint16_t level) : body(number, power), level(level) {}

            RunRuleBodyWidthLevel() {}

            std::string to_string() const
            {
                if (this->body.power == 1)
                {
                    return "L" + std::to_string(this->level) + "[" + std::to_string(this->body.number) + "]";
                }
                else
                {
                    return "L" + std::to_string(this->level) + "[" + std::to_string(this->body.number) + "^" + std::to_string(this->body.power) + "]";
                }
            }
        };

    
}