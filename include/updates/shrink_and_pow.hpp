#pragma once

#include "../dynamic_rlslp/dynamic_grammar_for_layered_rlslp.hpp"
#include "../local_parsings/locally_consistent_parsing.hpp"
#include "../local_parsings/naive_parsing.hpp"
#include "../local_parsings/mu.hpp"

namespace dynRLSLP
{
        /**
         * @brief A class for shrink and pow functions.
         * @ingroup DynamicDictionaryClasses
         */
        class ShrinkAndPow
        {
        public:
            static std::vector<int8_t> compute_restricted_factor_flags(const std::deque<RunRuleBody> &items, uint16_t current_level, const DynamicGrammarForLayeredRLSLP &dic)
            {
                std::vector<int8_t> factor_flags;
                factor_flags.resize(items.size(), -2);
                const RandomBitDictionary &random_bit_dictionary = dic.get_random_bit_dictionary();
                const std::vector<uint64_t>& base_signature_length_list = dic.get_base_signature_length_list();

                for (int64_t i = 0; i < (int64_t)items.size(); i++)
                {

                    uint64_t baseLen = SignatureFunctions::get_length(items[i].number, base_signature_length_list);
                    if (baseLen <= dynRLSLP::Mu::MU_FLOOR[current_level + 1])
                    {
                        factor_flags[i] = random_bit_dictionary.get_random_bit(items[i].number) ? 1 : 0;
                    }
                    else
                    {
                        factor_flags[i] = -1;
                    }
                }
                return factor_flags;
            }
            template <typename C, typename CALLBACK = decltype(no_callback)>
            static std::deque<RunRuleBody> shrink_char(const std::vector<C> &text, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_signature, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                int64_t MB = 1000000;
                int64_t counter = MB;
                if (text.size() == 0)
                {
                    throw std::runtime_error("ShrinkAndPow::shrink_char: text.size() == 0");
                }
                if (message_paragraph != stool::Message::NO_MESSAGE)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing the level-0 sequence in the derivation tree..." << std::endl;
                }
                std::deque<RunRuleBody> r;
                RLSLPRuleBody first_char_body = RLSLPRuleBody::create_char_item(text[0]);
                SignatureWithRelativeLevel first_sig = (int64_t)dic.get_or_add_signature(first_char_body, BSignatureBottomLevel, callback_for_added_signature);
                RunRuleBody current_run = RunRuleBody(first_sig, 1);

                for (uint64_t i = 1; i < text.size(); i++)
                {
                    RLSLPRuleBody char_body = RLSLPRuleBody::create_char_item(text[i]);
                    SignatureWithRelativeLevel char_sig = (int64_t)dic.get_or_add_signature(char_body, BSignatureBottomLevel, callback_for_added_signature);
                    if (char_sig == (int64_t)current_run.number)
                    {
                        current_run.power++;
                    }
                    else
                    {
                        r.push_back(current_run);
                        current_run = RunRuleBody(char_sig, 1);
                    }

                    counter--;
                    if (counter == 0)
                    {
                        if (message_paragraph != stool::Message::NO_MESSAGE)
                        {
                            std::cout << "\r" << stool::Message::get_paragraph_string(message_paragraph + 1) << "[" << (i / MB) << "/" << (text.size() / MB) << "MB]" << std::flush;
                        }
                        counter = MB;
                    }
                }
                r.push_back(current_run);

                if (message_paragraph != stool::Message::NO_MESSAGE)
                {
                    std::cout << std::endl;
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[DONE]" << std::endl;
                }
                return r;
            }

            static std::pair<uint64_t, uint64_t> compute_unstable_context_lengths(const std::deque<RunRuleBody> &text, const DynamicGrammarForLayeredRLSLP &dic, uint16_t level)
            {
                if (dic.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression)
                {
                    if (level % 2 == 0)
                    {
                        if (text.size() == 1)
                        {
                            return std::make_pair(1, 1);
                        }
                        else
                        {
                            return std::make_pair(1, 1);
                        }
                    }
                    else
                    {
                        std::vector<int8_t> factor_flags = compute_restricted_factor_flags(text, level, dic);
                        int64_t L = 0, R = 0;
                        if (factor_flags[0] == -1 || factor_flags[0] == 1)
                        {
                            L = 0;
                        }
                        else
                        {
                            L = 1;
                        }
                        int64_t last_index = factor_flags.size() - 1;
                        if (factor_flags[last_index] == -1 || factor_flags[last_index] == 0)
                        {
                            R = 0;
                        }
                        else
                        {
                            R = 1;
                        }
                        return std::make_pair(L, R);
                    }
                }
                else if (dic.get_grammar_parsing_type() == GrammarParsingType::SignatureEncoding)
                {
                    if (level % 2 == 0)
                    {
                        if (text.size() == 1)
                        {
                            return std::make_pair(1, 1);
                        }
                        else
                        {
                            return std::make_pair(1, 1);
                        }
                    }
                    else
                    {
                        if ((int64_t)text.size() <= (int64_t)LocallyConsistentParsing::get_consistent_L() + LocallyConsistentParsing::get_consistent_R())
                        {
                            return std::make_pair(LocallyConsistentParsing::get_consistent_L(), LocallyConsistentParsing::get_consistent_R());
                        }
                        else
                        {
                            std::vector<SignatureWithRelativeLevel> tmp;
                            for (RunRuleBody it : text)
                            {
                                tmp.push_back(it.number);
                            }
                            std::vector<bool> factor_bits;
                            dynRLSLP::LocallyConsistentParsing::compute_factor_bits(tmp, factor_bits);
                            int64_t L = LocallyConsistentParsing::get_consistent_L(), R = LocallyConsistentParsing::get_consistent_R();
                            while (L < (int64_t)factor_bits.size() && !factor_bits[L])
                            {
                                L++;
                            }
                            while (R < (int64_t)factor_bits.size() && !factor_bits[factor_bits.size() - R - 1])
                            {
                                R++;
                            }
                            return std::make_pair(L, R + 1);
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("ERROR in compute_unstable_context_lengths: unknown grammar parsing type");
                }
            }

            template <typename CALLBACK = decltype(no_callback)>
            static std::deque<RunRuleBody> center_line_compile(const std::vector<RunRuleBody> &left_context, const std::deque<RunRuleBody> &center, const std::vector<RunRuleBody> &right_context,
                                                               uint64_t current_level, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_signatures, [[maybe_unused]] int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                // std::cout << stool::Message::get_paragraph_string(message_paragraph) << "center_line_compile: current_level = " << (int)current_level << std::endl;
                if (center.size() == 0)
                {
                    throw std::runtime_error("ERROR in center_line_compile: center.size() == 0");
                }
                assert(RunRuleBody::verify_vector(center));
                std::deque<RunRuleBody> r;

                if (dic.get_grammar_parsing_type() == GrammarParsingType::SignatureEncoding)
                {
                    if (current_level % 2 == 0)
                    {
                        auto tmp_seq = ShrinkAndPow::pow(center, current_level, dic, callback_for_added_signatures);
                        r.swap(tmp_seq);
                        assert(RunRuleBody::verify_vector(r));
                    }
                    else
                    {
                        auto tmp_seq = ShrinkAndPow::shrink_with_context(left_context, center, right_context, current_level, dic, callback_for_added_signatures, stool::Message::increment_paragraph_level(message_paragraph));
                        r.swap(tmp_seq);
                        assert(RunRuleBody::verify_vector(r));
                    }
                }
                else if (dic.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression)
                {
                    if (current_level % 2 == 0)
                    {
                        auto tmp_seq = ShrinkAndPow::restricted_pow(center, current_level, dic, callback_for_added_signatures);
                        r.swap(tmp_seq);
                        assert(RunRuleBody::verify_vector(r));
                    }
                    else
                    {
                        auto tmp_seq = ShrinkAndPow::restricted_shrink(center, current_level, dic, callback_for_added_signatures, stool::Message::increment_paragraph_level(message_paragraph));
                        r.swap(tmp_seq);
                        assert(RunRuleBody::verify_vector(r));
                    }
                }
                else
                {
                    throw std::runtime_error("ERROR in center_line_compile: unknown grammar parsing type");
                }

                return r;
            }

        private:
            template <typename CALLBACK = decltype(no_callback)>
            static std::deque<RunRuleBody> pow(const std::deque<RunRuleBody> &items, uint16_t current_level, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_signature)
            {
                std::deque<RunRuleBody> output;
                for (uint64_t i = 0; i < items.size(); i++)
                {
                    if (items[i].power > 1)
                    {
                        SignatureWithRelativeLevel sig = dic.get_or_add_signature(RLSLPRuleBody::create_run_rule_body(items[i].number, items[i].power), current_level + 1, callback_for_added_signature);
                        output.push_back(RunRuleBody(sig, 1));
                    }
                    else if (items[i].power == 1)
                    {
                        SignatureWithRelativeLevel sig = dic.get_or_add_signature(RLSLPRuleBody::create_signature_item(items[i].number), current_level + 1, callback_for_added_signature);
                        output.push_back(RunRuleBody(sig, 1));
                        // output.push_back(items[i]);
                    }
                    else
                    {
                        throw std::runtime_error("ERROR in center_line_compile: center[i].power < 0");
                    }
                }
                return output;
            }

            template <typename CALLBACK = decltype(no_callback)>
            static std::deque<RunRuleBody> restricted_pow(const std::deque<RunRuleBody> &items, uint16_t current_level, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_signature)
            {
                assert(current_level < LEVEL_LIMIT);
                std::deque<RunRuleBody> output;
                const std::vector<uint64_t>& base_signature_length_list = dic.get_base_signature_length_list();
                for (uint64_t i = 0; i < items.size(); i++)
                {
                    uint64_t baseLen = SignatureFunctions::get_length(items[i].number, base_signature_length_list);
                    if (baseLen <= dynRLSLP::Mu::MU_FLOOR[current_level + 1] && items[i].power > 1)
                    {
                        SignatureWithRelativeLevel sig = dic.get_or_add_signature(RLSLPRuleBody::create_run_rule_body(items[i].number, items[i].power), current_level + 1, callback_for_added_signature);
                        output.push_back(RunRuleBody(sig, 1));
                    }
                    else
                    {
                        RLSLPRuleBody newBody = RLSLPRuleBody::create_signature_item(items[i].number);
                        SignatureWithRelativeLevel newSignature = (int64_t)dic.get_or_add_signature(newBody, current_level + 1, callback_for_added_signature);
                        output.push_back(RunRuleBody(newSignature, items[i].power));
                    }
                }
                return output;
            }

            template <typename CALLBACK = decltype(no_callback)>
            static std::deque<RunRuleBody> shrink_with_context(const std::vector<RunRuleBody> &left_context, const std::deque<RunRuleBody> &center, const std::vector<RunRuleBody> &right_context,
                                                               uint64_t current_level, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_signatures, [[maybe_unused]] int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (center.size() == 0)
                {
                    throw std::runtime_error("ERROR in shrink_with_context: center.size() == 0");
                }

                std::vector<SignatureWithRelativeLevel> tmp;

                for (RunRuleBody it : left_context)
                    tmp.push_back(it.number);
                for (RunRuleBody it : center)
                {
                    tmp.push_back(it.number);
                }
                for (RunRuleBody it : right_context)
                    tmp.push_back(it.number);
                assert(dynRLSLP::LocallyConsistentParsing::verify_input(tmp));

                std::vector<bool> longBools;
                dynRLSLP::LocallyConsistentParsing::compute_factor_bits(tmp, longBools);
                std::vector<bool> center_bools = std::vector<bool>(center.size());
                for (uint64_t i = 0; i < center.size(); i++)
                {
                    center_bools[i] = (longBools)[i + left_context.size()];
                }
#ifdef DEBUG
                if (center.size() > 0 && !center_bools[0])
                {
                    throw std::runtime_error("ERROR in center_line_compile: center_bools[0] is false");
                }
#endif
                std::deque<RunRuleBody> shrink_seq = ShrinkAndPow::shrink(center, center_bools, current_level, dic, callback_for_added_signatures);
                assert(RunRuleBody::verify_vector(shrink_seq));

#ifdef DEBUG
const std::vector<uint64_t>& base_signature_length_list = dic.get_base_signature_length_list();

                int64_t centerLen = RunRuleBody::compute_string_length(center, base_signature_length_list);
                int64_t rLen = RunRuleBody::compute_string_length(shrink_seq, base_signature_length_list);
                if (rLen != centerLen)
                {
                    std::cout << "ERROR in center_line_compile: rLen != centerLen" << std::endl;
                    std::cout << "rLen: " << rLen << ", centerLen: " << centerLen << std::endl;
                    throw std::runtime_error("ERROR in center_line_compile: rLen != centerLen");
                }
#endif
                return shrink_seq;
            }

            template <typename CALLBACK = decltype(no_callback)>
            static std::deque<RunRuleBody> shrink(const std::deque<RunRuleBody> &items, const std::vector<bool> &bools, uint16_t current_level, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &preprocessor)
            {
                std::deque<RunRuleBody> output;
                if (items.size() == 0)
                {
                    return output;
                }

                assert(items.size() == bools.size());
                assert(bools[0]);

                SignatureWithRelativeLevel tmp = items[0].number;
                int64_t pairLength = 1;
                for (int64_t i = 1; i < (int64_t)items.size(); i++)
                {
                    if (bools[i])
                    {
                        assert(pairLength > 1);
                        output.push_back(RunRuleBody(tmp, 1));
                        tmp = items[i].number;
                        pairLength = 1;
                    }
                    else
                    {
                        RLSLPRuleBody newItem = RLSLPRuleBody::create_pair_item(tmp, items[i].number);
                        SignatureWithRelativeLevel newSignature = (int64_t)dic.get_or_add_signature(newItem, current_level + 1, preprocessor);
                        tmp = newSignature;
                        pairLength++;
                    }
                }
                if (pairLength == 1)
                    throw std::runtime_error("Shrink: pairLength == 1");
                output.push_back(RunRuleBody(tmp, 1));
                tmp = -1;
                pairLength = 1;
                RunRuleBody::merge_same_signatures(output);
                return output;
            }

            template <typename CALLBACK = decltype(no_callback)>
            static std::deque<RunRuleBody> restricted_shrink(const std::deque<RunRuleBody> &items, uint16_t current_level, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &preprocessor, [[maybe_unused]] int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                std::deque<RunRuleBody> output;
                if (items.size() == 0)
                {
                    return output;
                }

                std::vector<int8_t> factor_flags = compute_restricted_factor_flags(items, current_level, dic);

                assert(items.size() == factor_flags.size());
                int64_t i = 0;
                int64_t vec_size = items.size();
                while (i + 1 <= vec_size)
                {
                    if (i + 1 < vec_size && factor_flags[i] == 1 && factor_flags[i + 1] == 0)
                    {
                        RLSLPRuleBody newItem = RLSLPRuleBody::create_pair_item(items[i].number, items[i + 1].number);
                        SignatureWithRelativeLevel newSignature = (int64_t)dic.get_or_add_signature(newItem, current_level + 1, preprocessor);
                        output.push_back(RunRuleBody(newSignature, 1));
                        i += 2;
                    }
                    else
                    {
                        if (factor_flags[i] != -1)
                        {
                            RLSLPRuleBody newItem = RLSLPRuleBody::create_signature_item(items[i].number);
                            SignatureWithRelativeLevel newSignature = (int64_t)dic.get_or_add_signature(newItem, current_level + 1, preprocessor);
                            output.push_back(RunRuleBody(newSignature, 1));
                        }
                        else
                        {
                            RLSLPRuleBody newBody = RLSLPRuleBody::create_signature_item(items[i].number);
                            SignatureWithRelativeLevel newSignature = (int64_t)dic.get_or_add_signature(newBody, current_level + 1, preprocessor);
                            output.push_back(RunRuleBody(newSignature, items[i].power));
                        }

                        i++;
                    }
                }
                RunRuleBody::merge_same_signatures(output);
                return output;
            }
        };
    
}