#pragma once
#include <vector>
#include <iostream>
#include <list>
#include <stack>
#include <algorithm>
#include <limits>
#include "../rules/all.hpp"

namespace dynRLSLP
{
        /**
         * @brief Substring and context extraction on run sequences.
         * @ingroup StaticOperationsClasses
         */
        class LevelSequenceFunction
        {
        public:

        static uint64_t get_string_length(const std::vector<RunRuleBody> &sequence, const std::vector<uint64_t>& base_signature_length_list){
            uint64_t length = 0;
            for (auto it : sequence)
            {
                length += SignatureFunctions::get_length(it.number, base_signature_length_list) * it.power;
            }
            return length;
        }
        static VStack<RunRuleBody> create_stack(const std::vector<RunRuleBody> &sequence){
            VStack<RunRuleBody> st;
            int64_t size = sequence.size();
            for(int64_t i = size-1; i >= 0; i--){
                st.push(sequence[i]);
            }
            return st;
        }

        
        /**
         * @brief Build a right-context VStack for a signature.
         * @param signature Encoded signature with relative level.
         * @param base_signature_rule_list Base-signature rule list (D).
         * @return Resulting vector.
         */
        static VStack<RunRuleBody> create_right_sequence(SignatureWithRelativeLevel signature, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
        {
            RLSLPRuleBody item = RLSLPRuleBody::decodeRule(signature, base_signature_rule_list);
            VStack<RunRuleBody> st;
            if (item.get_type() == RLSLPRuleType::Pair)
            {
                st.push(RunRuleBody(item.B, 1));
            }
            else if (item.get_type() == RLSLPRuleType::Power && item.B > 1)
            {
                st.push(RunRuleBody(item.A, item.B - 1));
            }
            else
            {
                std::cout << item.get_info() << std::endl;
                throw std::runtime_error("create_right_sequence: Not implemented");
            }
            return st;
        }
        /**
         * @brief Return the rightmost run rule for a signature.
         * @param signature Encoded signature with relative level.
         * @param base_signature_rule_list Base-signature rule list (D).
         * @return Resulting vector.
         */
        static RunRuleBody create_right_run_rule(SignatureWithRelativeLevel signature, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
        {
            RLSLPRuleBody item = RLSLPRuleBody::decodeRule(signature, base_signature_rule_list);
            if (item.get_type() == RLSLPRuleType::Pair)
            {
                return RunRuleBody(item.B, 1);
            }
            else if (item.get_type() == RLSLPRuleType::Power && item.B > 1)
            {
                return RunRuleBody(item.A, item.B - 1);
            }
            else
            {
                std::cout << item.get_info() << std::endl;
                throw std::runtime_error("create_right_sequence: Not implemented");
            }
        }


        /**
         * @brief Build a left-context VStack for a signature.
         * @param signature Encoded signature with relative level.
         * @param base_signature_rule_list Base-signature rule list (D).
         * @return Resulting vector.
         */
        static VStack<RunRuleBody> create_left_sequence(SignatureWithRelativeLevel signature, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
        {

            RLSLPRuleBody item = RLSLPRuleBody::decodeRule(signature, base_signature_rule_list);
            VStack<RunRuleBody> st;
            if (item.get_type() == RLSLPRuleType::Pair)
            {
                st.push(RunRuleBody(item.A, 1));
            }
            else if (item.get_type() == RLSLPRuleType::Power && item.B > 1)
            {
                st.push(RunRuleBody(item.A, 1));
            }
            else
            {
                std::cout << item.get_info() << std::endl;
                throw std::runtime_error("create_left_sequence: Not implemented");
            }
            return st;
        }

        /**
         * @brief Return the leftmost run rule for a signature.
         * @param signature Encoded signature with relative level.
         * @param base_signature_rule_list Base-signature rule list (D).
         * @return Resulting vector.
         */
        static RunRuleBody create_left_run_rule(SignatureWithRelativeLevel signature, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
        {

            RLSLPRuleBody item = RLSLPRuleBody::decodeRule(signature, base_signature_rule_list);
            if (item.get_type() == RLSLPRuleType::Pair)
            {
                return RunRuleBody(item.A, 1);
            }
            else if (item.get_type() == RLSLPRuleType::Power && item.B > 1)
            {
                return RunRuleBody(item.A, 1);
            }
            else
            {
                std::cout << "Sig: " << signature << std::endl;
                std::cout << item.get_info() << std::endl;
                throw std::runtime_error("create_left_sequence: Not implemented");
            }
        }
        

        /**
         * @brief Extract a substring as a vector of run rules.
         * @param item Rule body item.
         * @param pos Start position in the represented string.
         * @param len Length of the substring or prefix/suffix.
         * @param dic Layered RLSLP dictionary.
         * @return Computed integer value.
         */
        static std::vector<RunRuleBody> substring(SignatureWithRelativeLevel item, int64_t pos, int64_t len, const DictionaryForLayeredRLSLP &dic)
            {
                VStack<RunRuleBody> st;
                st.push(RunRuleBody(RunRuleBody(item, 1)));
                int64_t tmp_pos = 0;

                while (tmp_pos < pos)
                {
                    RunRuleBody current = st.top();
                    int64_t baseLen = SignatureFunctions::get_length(current.number, dic.get_base_signature_length_list());
                    int64_t totalLen = baseLen * current.power;

                    if (tmp_pos + totalLen <= pos)
                    {
                        tmp_pos += totalLen;
                        st.pop();
                    }
                    else if (tmp_pos + baseLen <= pos)
                    {
                        int64_t c = (pos - tmp_pos) / baseLen;
                        tmp_pos += baseLen * c;
                        st.pop();
                        st.push(RunRuleBody(current.number, current.power - c));
                    }
                    else
                    {
                        st.pop();
                        if (current.power > 1)
                        {
                            st.push(RunRuleBody(current.number, current.power - 1));
                        }
                        std::vector<RunRuleBody> tmp;
                        RunRuleBody::y_break(current.number, dic.get_base_signature_rule_list(), dic.get_base_signature_level_list(), tmp);
                        for (int64_t i = tmp.size() - 1; i >= 0; i--)
                        {
                            st.push(RunRuleBody(tmp[i].number, tmp[i].power));
                        }
                    }
                }
                if (tmp_pos != pos)
                {
                    throw std::runtime_error("Error in preprocess: tmp_pos != pos");
                }
                int64_t totalResultLen = 0;
                std::vector<RunRuleBody> r;
                while (!st.empty())
                {
                    RunRuleBody current = st.top();
                    st.pop();
                    r.push_back(current);
                    totalResultLen += SignatureFunctions::get_length(current.number, dic.get_base_signature_length_list()) * current.power;
                }
                while (totalResultLen > len)
                {
                    RunRuleBody current = r.back();
                    int64_t baseLen = SignatureFunctions::get_length(current.number, dic.get_base_signature_length_list());
                    int64_t totalLen = baseLen * current.power;

                    if (totalResultLen - totalLen >= len)
                    {
                        totalResultLen -= totalLen;
                        r.pop_back();
                    }
                    else if (totalResultLen - baseLen >= len)
                    {
                        int64_t c = (totalResultLen - len) / baseLen;
                        totalResultLen -= baseLen * c;
                        r.pop_back();
                        r.push_back(RunRuleBody(current.number, current.power - c));
                    }
                    else
                    {
                        r.pop_back();
                        if (current.power > 1)
                        {
                            r.push_back(RunRuleBody(current.number, current.power - 1));
                        }
                        std::vector<RunRuleBody> tmp;
                        RunRuleBody::y_break(current.number, dic.get_base_signature_rule_list(), dic.get_base_signature_level_list(), tmp);
                        for (auto it : tmp)
                        {
                            r.push_back(RunRuleBody(it.number, it.power));
                        }
                    }
                }
                if (totalResultLen != len)
                {
                    throw std::runtime_error("Error in preprocess: totalResultLen != len");
                }

                return r;
            }

            /**
             * @brief Extract a substring as a vector of run rules.
             * @param item Rule body item.
             * @param pos Start position in the represented string.
             * @param dic Layered RLSLP dictionary.
             * @return Computed integer value.
             */
            static std::vector<RunRuleBody> substring(int64_t item, int64_t pos, const DictionaryForLayeredRLSLP &dic)
            {
                int64_t len = SignatureFunctions::get_length(item, dic.get_base_signature_length_list());
                if (pos >= len)
                {
                    throw std::runtime_error("Error in create: pos >= len");
                }

                return LevelSequenceFunction::substring(item, pos, len - pos, dic);
            }

        };
    
}