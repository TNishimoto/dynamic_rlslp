#pragma once
#include "./rlslp_rule_body.hpp"
#include "./signature_functions.hpp"
#include "stool/include/all.hpp"
namespace dynRLSLP
{
        /**
         * @brief A representation of a run *X^k* of RLSLP nonterminals
         * @ingroup RLSLPClasses
         */
        struct RunRuleBody
        {
        public:
            /**
            * X.
            */
            uint64_t number;

            /**
            * k.
            */
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

            /**
             * @brief Return *X^{k+k'}* for a given run *X^{k'}*.
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

            /**
             * @brief Return @ref term_val "|\val(X^{k})|".
             * 
             * @param base_signature_length_list The length list of DictionaryForLayeredRLSLP.
             * @return |\val(X^{k})|
             */
            uint64_t get_length(const std::vector<uint64_t>& base_signature_length_list) const
            {
                return base_signature_length_list[SignatureFunctions::get_base_signature(this->number)] * this->power;
            }

            /**
             * @brief Return the string representation of this run rule body.
             */
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

            /**
             * @brief Return the RLSLP rule body corresponding to this run rule body.
             */
            RLSLPRuleBody to_rlslp_rule_body()
            {
                return RLSLPRuleBody::create_run_rule_body(this->number, this->power);
            }

            /**
             * @brief Return the string derived by this run rule body.
             * 
             * @param base_signature_rule_list The rule list of DictionaryForLayeredRLSLP.
             * @param output The outputted string is stored in this vector.
             */
            template <typename OUTPUT_VEC_TYPE = std::vector<uint8_t>>
            void decompress(const std::vector<RLSLPRuleBody> &base_signature_rule_list, OUTPUT_VEC_TYPE &output) const {
                for(int64_t i = 0; i < (int64_t)this->power; i++){
                    RLSLPRuleBody item = RLSLPRuleBody::decodeRule(this->number, base_signature_rule_list);
                    item.decompress(base_signature_rule_list, output);
                }
            }

            /**
             * @brief Converts a vector of integers into a vector of RunRuleBody by counting consecutive equal values.
             *        Each run of the same number will become a RunRuleBody(number, count).
             * 
             * @tparam OUTPUT_VEC_TYPE The type of the output vector, defaults to std::vector<RunRuleBody>
             * @param items Input vector of integers
             * @param output Output vector of RunRuleBody objects (cleared on entry)
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

            /**
             * @brief Checks if the given sequence contains exactly one RunRuleBody and its power is 1.
             * 
             * @tparam SEQ_TYPE The type of the vector, defaults to std::vector<RunRuleBody>
             * @param seq Sequence to check
             * @return true if the sequence has one element and its power is 1, false otherwise
             */
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

            /**
             * @brief Prints the content of a vector of RunRuleBody as a string to standard output.
             * 
             * @tparam VEC_TYPE The type of the vector, defaults to std::vector<RunRuleBody>
             * @param items Vector of RunRuleBody to print
             * @param name Name to appear before the contents
             * @param message_paragraph Paragraph style for the output (default: stool::Message::SHOW_MESSAGE)
             */
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

            /**
             * @brief Verifies that no two consecutive elements in the vector have the same 'number'.
             *        Throws an exception if duplicate consecutive numbers are found.
             * 
             * @tparam VEC_TYPE The type of the vector, defaults to std::vector<RunRuleBody>
             * @param items Vector of RunRuleBody to verify
             * @return true if verification passes, otherwise throws an exception
             */
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

            /**
             * @brief Merges adjacent RunRuleBody elements in the vector that have the same 'number' by combining their 'power'.
             * 
             * @tparam VEC_TYPE The type of the vector, defaults to std::vector<RunRuleBody>
             * @param items Vector of RunRuleBody to merge (modified in place)
             */
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

            /**
             * @brief Computes the total length of the string represented by a vector of RunRuleBody, using the base signature length list.
             * 
             * @tparam VEC_TYPE The type of the vector, defaults to std::vector<RunRuleBody>
             * @param items Vector of RunRuleBody
             * @param base_signature_length_list The length vector for base signatures
             * @return The total length represented by the combination of items
             */
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

            /**
             * @brief Recursively decomposes a signature into a sequence of RunRuleBody objects, depending on the signature structure.
             * 
             * Power: Pushes RunRuleBody(child.A, child.B) to output.
             * Pair: Decomposes child.A if its level is not smaller than the current level, otherwise pushes both children as RunRuleBody(..., 1).
             * Signature: Handles indirection via the level and decomposes if appropriate.
             * Other types throw an exception.
             * 
             * @param signature The signature to decompose
             * @param base_signature_rule_list The rule list for decoding signatures
             * @param base_signature_level_list List of levels for signatures
             * @param output The output vector to which RunRuleBody objects will be appended
             */
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


    
}