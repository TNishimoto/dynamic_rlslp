#pragma once
#include "./shrink_and_pow.hpp"

namespace dynRLSLP
{
        /**
         * @brief A class for shrink and pow functions.
         * @ingroup DynamicDictionaryClasses
         */
        class TryShrinkAndPow
        {
        public:
            template <typename C, typename CALLBACK = decltype(no_callback)>
            static bool try_shrink_char(const std::vector<C> &text, const DynamicGrammarForLayeredRLSLP &dic, std::deque<RunRuleBody> &output, int message_paragraph = stool::Message::SHOW_MESSAGE)
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
                RLSLPRuleBody first_char_body = RLSLPRuleBody::create_char_item(text[0]);
                int64_t first_sig = dic.try_get_signature(first_char_body);
                if (first_sig == -1)
                {
                    return false;
                }
                RunRuleBody current_run = RunRuleBody(first_sig, 1);

                for (uint64_t i = 1; i < text.size(); i++)
                {
                    RLSLPRuleBody char_body = RLSLPRuleBody::create_char_item(text[i]);
                    int64_t char_sig = dic.try_get_signature(char_body);
                    if (char_sig == -1)
                    {
                        return false;
                    }
                    else if (char_sig == (int64_t)current_run.number)
                    {
                        current_run.power++;
                    }
                    else
                    {
                        output.push_back(current_run);
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
                output.push_back(current_run);

                if (message_paragraph != stool::Message::NO_MESSAGE)
                {
                    std::cout << std::endl;
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[DONE]" << std::endl;
                }
                return true;
            }


            static bool try_restricted_pow(const std::deque<RunRuleBody> &items, uint16_t current_level, const DynamicGrammarForLayeredRLSLP &dic, std::deque<RunRuleBody> &output)
            {
                assert(current_level < LEVEL_LIMIT);
                const std::vector<uint64_t>& base_signature_length_list = dic.get_base_signature_length_list();
                for (uint64_t i = 0; i < items.size(); i++)
                {

                    uint64_t baseLen = SignatureFunctions::get_length(items[i].number, base_signature_length_list);
                    if (baseLen <= dynRLSLP::Mu::MU_FLOOR[current_level + 1] && items[i].power > 1)
                    {
                        int64_t sig = dic.try_get_signature(RLSLPRuleBody::create_run_rule_body(items[i].number, items[i].power));
                        if(sig == -1){
                            return false;
                        }
                        output.push_back(RunRuleBody(sig, 1));
                    }
                    else
                    {
                        RLSLPRuleBody newBody = RLSLPRuleBody::create_signature_item(items[i].number);
                        int64_t newSignature = dic.try_get_signature(newBody);
                        if(newSignature == -1){
                            return false;
                        }
                        output.push_back(RunRuleBody(newSignature, items[i].power));
                    }
                }
                return true;
            }
            static bool try_center_line_compile(const std::vector<RunRuleBody> &left_context, const std::deque<RunRuleBody> &center, const std::vector<RunRuleBody> &right_context,
                                                uint64_t current_level, const DynamicGrammarForLayeredRLSLP &dic, std::deque<RunRuleBody> &output, [[maybe_unused]] int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (center.size() == 0)
                {
                    throw std::runtime_error("ERROR in center_line_compile: center.size() == 0");
                }
                assert(RunRuleBody::verify_vector(center));

                if (dic.get_grammar_parsing_type() == GrammarParsingType::SignatureEncoding)
                {
                    if (current_level % 2 == 0)
                    {
                        bool b1 = TryShrinkAndPow::try_pow(center, dic, output);
                        if (!b1)
                        {
                            return false;
                        }
                        assert(RunRuleBody::verify_vector(output));
                    }
                    else
                    {
                        bool b1 = TryShrinkAndPow::try_shrink_with_context(left_context, center, right_context, dic, output);
                        if (!b1)
                        {
                            return false;
                        }
                        assert(RunRuleBody::verify_vector(output));
                    }
                }
                else if (dic.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression)
                {
                    if (current_level % 2 == 0)
                    {
                        bool b1 = TryShrinkAndPow::try_restricted_pow(center, current_level, dic, output);
                        if (!b1)
                        {
                            return false;
                        }
                        assert(RunRuleBody::verify_vector(output));
                    }
                    else
                    {
                        bool b1 = TryShrinkAndPow::try_restricted_shrink(center, current_level, dic, output);
                        if (!b1)
                        {
                            return false;
                        }
                        assert(RunRuleBody::verify_vector(output));
                    }
                }
                else
                {
                    throw std::runtime_error("ERROR in center_line_compile: unknown grammar parsing type");
                }

                return true;
            }

            static bool try_pow(const std::deque<RunRuleBody> &items, const DynamicGrammarForLayeredRLSLP &dic, std::deque<RunRuleBody> &output)
            {
                for (uint64_t i = 0; i < items.size(); i++)
                {
                    if (items[i].power > 1)
                    {
                        int64_t sig = dic.try_get_signature(RLSLPRuleBody::create_run_rule_body(items[i].number, items[i].power));
                        if (sig == -1)
                        {
                            return false;
                        }
                        output.push_back(RunRuleBody(sig, 1));
                    }
                    else if (items[i].power == 1)
                    {
                        int64_t sig = dic.try_get_signature(RLSLPRuleBody::create_signature_item(items[i].number));
                        if (sig == -1)
                        {
                            return false;
                        }
                        output.push_back(RunRuleBody(sig, 1));
                    }
                    else
                    {
                        throw std::runtime_error("ERROR in center_line_compile: center[i].power < 0");
                    }
                }
                return true;
            }
            static bool try_shrink_with_context(const std::vector<RunRuleBody> &left_context, const std::deque<RunRuleBody> &center, const std::vector<RunRuleBody> &right_context,
                                                const DynamicGrammarForLayeredRLSLP &dic, std::deque<RunRuleBody> &output)
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
                bool b2 = TryShrinkAndPow::try_shrink(center, center_bools, dic, output);
                if (!b2)
                {
                    return false;
                }
                assert(RunRuleBody::verify_vector(output));

                return true;
            }

            static bool try_shrink(const std::deque<RunRuleBody> &items, const std::vector<bool> &bools, const DynamicGrammarForLayeredRLSLP &dic, std::deque<RunRuleBody> &output)
            {
                if (items.size() == 0)
                {
                    return true;
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
                        int64_t newSignature = dic.try_get_signature(newItem);
                        if(newSignature == -1){
                            return false;
                        }
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
                return true;

            }

            static bool try_restricted_shrink(const std::deque<RunRuleBody> &items, uint16_t current_level, const DynamicGrammarForLayeredRLSLP &dic, std::deque<RunRuleBody> &output, [[maybe_unused]] int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (items.size() == 0)
                {
                    return true;
                }

                std::vector<int8_t> factor_flags = ShrinkAndPow::compute_restricted_factor_flags(items, current_level, dic);

                assert(items.size() == factor_flags.size());
                int64_t i = 0;
                int64_t vec_size = items.size();
                while (i + 1 <= vec_size)
                {
                    if (i + 1 < vec_size && factor_flags[i] == 1 && factor_flags[i + 1] == 0)
                    {
                        RLSLPRuleBody newItem = RLSLPRuleBody::create_pair_item(items[i].number, items[i + 1].number);
                        int64_t newSignature = dic.try_get_signature(newItem);
                        if(newSignature == -1){
                            return false;
                        }
                        output.push_back(RunRuleBody(newSignature, 1));
                        i += 2;
                    }
                    else
                    {
                        if (factor_flags[i] != -1)
                        {
                            RLSLPRuleBody newItem = RLSLPRuleBody::create_signature_item(items[i].number);
                            int64_t newSignature = dic.try_get_signature(newItem);
                            if(newSignature == -1){
                                return false;
                            }
                            output.push_back(RunRuleBody(newSignature, 1));
                        }
                        else
                        {
                            RLSLPRuleBody newBody = RLSLPRuleBody::create_signature_item(items[i].number);
                            int64_t newSignature = dic.try_get_signature(newBody);
                            if(newSignature == -1){
                                return false;
                            }
                            output.push_back(RunRuleBody(newSignature, items[i].power));
                        }

                        i++;
                    }
                }
                RunRuleBody::merge_same_signatures(output);
                return true;
            }
        };
    
}