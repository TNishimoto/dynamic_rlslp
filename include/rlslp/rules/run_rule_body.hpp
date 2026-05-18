#pragma once
#include "./rlslp_rule_body.hpp"
#include "./signature_functions.hpp"
#include "stool/include/all.hpp"
namespace dynRLSLP
{
        /**
         * @brief A run *X^k* of RLSLP nonterminals.
         * @ingroup RLSLPClasses
         */
        struct RunRuleBody
        {
        public:
            /** Nonterminal signature *X*. */
            uint64_t number;

            /** Repetition count *k*. */
            uint64_t power;

            /**
             * @brief Construct a run from a nonterminal and repetition count.
             * @param number Nonterminal signature (X).
             * @param power Repetition count (k).
             */
            RunRuleBody(uint64_t number, uint64_t power)
                : number(number), power(power)
            {
            }
            /**
             * @brief Default constructor; constructs a zero-initialized run.
             */
            RunRuleBody()
                : number(0), power(0)
            {
            }

            /** @brief Copy constructor. */
            RunRuleBody(const RunRuleBody &) = default;
            /** @brief Copy assignment operator. */
            RunRuleBody &operator=(const RunRuleBody &) = default;
            /** @brief Move constructor. */
            RunRuleBody(RunRuleBody &&) = default;
            /** @brief Move assignment operator. */
            RunRuleBody &operator=(RunRuleBody &&) = default;

            /**
             * @brief Concatenate two runs with the same nonterminal.
             * @param item2 Second run to add.
             * @return Run *X^{k+k'}* when both runs share the same \p number.
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
             * @brief Return the string length of a signature using the base length list.
             * @param base_signature_length_list Base-signature length list (L).
             * @return Computed integer value.
             */
            uint64_t get_length(const std::vector<uint64_t>& base_signature_length_list) const
            {
                return base_signature_length_list[SignatureFunctions::get_base_signature(this->number)] * this->power;
            }

            /**
             * @brief Return a human-readable string representation of this run.
             * @return String of the form R(sig) or R(sig^k).
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
             * @brief Convert this run to an RLSLPRuleBody power rule.
             * @return Rule body.
             */
            RLSLPRuleBody to_rlslp_rule_body()
            {
                return RLSLPRuleBody::create_run_rule_body(this->number, this->power);
            }

            /**
             * @brief Append the expanded string of this run to an output container.
             * @param OUTPUT_VEC_TYPE Output container type.
             * @param base_signature_rule_list Base-signature rule list (D).
             * @param output Output container for decompressed bytes.
             */
            template <typename OUTPUT_VEC_TYPE = std::vector<uint8_t>>
            void decompress(const std::vector<RLSLPRuleBody> &base_signature_rule_list, OUTPUT_VEC_TYPE &output) const {
                for(int64_t i = 0; i < (int64_t)this->power; i++){
                    RLSLPRuleBody item = RLSLPRuleBody::decodeRule(this->number, base_signature_rule_list);
                    item.decompress(base_signature_rule_list, output);
                }
            }

            /**
             * @brief Group consecutive equal integers into RunRuleBody runs.
             * @tparam OUTPUT_VEC_TYPE Output vector type.
             * @param items Input integer sequence.
             * @param output Output vector of runs (cleared on entry).
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

            template <typename SEQ_TYPE = std::vector<RunRuleBody>>
            /**
             * @brief Return whether a sequence is a single run of power one.
             * @param seq Input run sequence.
             * @return True if the check succeeds.
             */
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

            template <typename VEC_TYPE = std::vector<RunRuleBody>>
            /**
             * @brief Print a vector of runs to standard output.
             * @param items Sequence of run rules or integers.
             * @param name Label for printed output.
             * @param message_paragraph Indentation level for formatted output.
             */
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
            /**
             * @brief Verify no adjacent runs share the same nonterminal.
             * @param items Sequence of run rules or integers.
             * @return True if the check succeeds.
             */
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
            /**
             * @brief Merge adjacent runs with the same nonterminal.
             * @param items Sequence of run rules or integers.
             */
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
            /**
             * @brief Return total string length of a run vector.
             * @param items Sequence of run rules or integers.
             * @param base_signature_length_list Base-signature length list (L).
             * @return Computed integer value.
             */
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
             * @brief Recursively decompose a signature into a sequence of RunRuleBody objects.
             * @param signature Signature to decompose.
             * @param base_signature_rule_list Base-signature rule list (D).
             * @param base_signature_level_list Base-signature level list (H).
             * @param output Output vector of runs.
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