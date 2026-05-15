#pragma once
#include "./common_sequence_compiler.hpp"
#include "./try_shrink_and_pow.hpp"

namespace dynRLSLP
{


        /**
         * @brief A class for compiling common sequences.
         * @ingroup DynamicDictionaryClasses
         */
        class TryCommonSequenceCompiler
        {
            public:
            static int64_t try_single_compile(RunRuleVector &seq, const DynamicGrammarForLayeredRLSLP &dic, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if(seq.is_empty()){
                    throw std::runtime_error("Error in single_compile: seq is empty");
                }
                //std::cout << stool::Message::get_paragraph_string(message_paragraph) << "single_compile: seq.size() = " << seq.size() << std::endl;

                bool b1 = TryCommonSequenceCompiler::try_left_side_semi_compile(seq, dic, stool::Message::increment_paragraph_level(message_paragraph));
                if(!b1){
                    return -1;
                }
                bool b2 = TryCommonSequenceCompiler::try_right_side_semi_compile(seq, dic, stool::Message::increment_paragraph_level(message_paragraph));
                if(!b2){
                    return -1;
                }


                std::deque<RunRuleBody> current_seq;
                int16_t level = seq.get_max_level();
                auto tmp = seq.pop_front_sequence();
                for(auto it : tmp){
                    current_seq.push_back(it);
                }
                RunRuleBody::merge_same_signatures(current_seq);

                std::vector<RunRuleBody> empty_left_line;
                std::vector<RunRuleBody> empty_right_line;

                assert(RunRuleBody::verify_vector(current_seq));

                while (!RunRuleBody::is_single_signature(current_seq))
                {
                    assert(current_seq.size() > 0);
                    std::deque<RunRuleBody> new_seq;
                    bool b1 = TryShrinkAndPow::try_center_line_compile(empty_left_line, current_seq, empty_right_line, level, dic, new_seq, stool::Message::increment_paragraph_level(message_paragraph));
                    if(!b1){
                        return -1;
                    }
                    current_seq.swap(new_seq);
                    level++;
                }
                return current_seq[0].number;

            }

            template <typename C>
            static bool try_build_common_sequence_from_text(const DynamicGrammarForLayeredRLSLP &dic, const std::vector<C> &text, RunRuleVector &output, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if(text.size() == 0){
                    throw std::runtime_error("Error in build_common_sequence_from_text: text is empty");
                }
                FastCommonSequenceBuilder::initialize(dic.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);
                std::deque<RunRuleBody> current_seq;
                bool b1 = TryShrinkAndPow::try_shrink_char(text, dic, current_seq, stool::Message::increment_paragraph_level(message_paragraph));
                if(!b1){
                    return false;
                }
                uint64_t level = 0;

                if(message_paragraph != stool::Message::NO_MESSAGE){
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Construct the derivation tree of the input text... " << std::endl;
                }


                //std::cout << stool::Message::get_paragraph_string(message_paragraph) << "build_common_sequence_from_text: current_seq.size() = " << current_seq.size() << " / " << message_paragraph << std::endl;

                while (current_seq.size() > 0)
                {
                    if(message_paragraph != stool::Message::NO_MESSAGE){
                        std::cout << "+" << std::flush;
                    }
                    std::pair<uint64_t, uint64_t> unstable_context_lengths = ShrinkAndPow::compute_unstable_context_lengths(current_seq, dic, level);
                    int64_t L = unstable_context_lengths.first;
                    int64_t R = unstable_context_lengths.second;
                    std::vector<RunRuleBody> tmp_left_line;
                    while ((int64_t)tmp_left_line.size() < L && current_seq.size() > 0)
                    {
                        tmp_left_line.push_back(current_seq.front());
                        current_seq.pop_front();
                    }

                    std::vector<RunRuleBody> tmp_right_line;
                    while ((int64_t)tmp_right_line.size() < R && current_seq.size() > 0)
                    {
                        tmp_right_line.insert(tmp_right_line.begin(), current_seq.back());
                        current_seq.pop_back();
                    }
                    if (current_seq.size() >= 1)
                    {
                        std::deque<RunRuleBody> new_seq;
                        bool b1 = TryShrinkAndPow::try_center_line_compile(tmp_left_line, current_seq, tmp_right_line, level, dic, new_seq, stool::Message::increment_paragraph_level(message_paragraph));
                        if(!b1){
                            return false;
                        }
                        current_seq.swap(new_seq);
                    }

                    if(tmp_left_line.size() > 0 || tmp_right_line.size() > 0){
                        output.push_new_ceil(tmp_left_line, tmp_right_line, level);
                    }
                    //output.swap_left_line(tmp_left_line, level);
                    //output.swap_right_line(tmp_right_line, level);
                    level++;

                }
                if(message_paragraph != stool::Message::NO_MESSAGE){
                    std::cout << std::endl;
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[DONE]" << std::endl;
                }
                assert(output.is_empty() == false);
                return true;
            }


            static bool try_left_side_semi_compile(RunRuleVector &item, const DynamicGrammarForLayeredRLSLP &dic, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                
                const DictionaryForLayeredRLSLP &small_dic = dic.get_dictionary();

                int64_t h = 0;
                RunRuleVector empty_left = RunRuleVector::create_empty_vector(small_dic);
                std::deque<RunRuleBody> center;
                for (int64_t i = 0; i < item.get_max_level(); i++)
                {
                    std::deque<RunRuleBody> new_center;
                    bool b1 = TryCommonSequenceCompiler::try_LCR_semi_compile(empty_left, center, item, h++, dic, new_center, stool::Message::increment_paragraph_level(message_paragraph));
                    if(!b1){
                        return false;
                    }
                    center.swap(new_center);
                }
                while (center.size() > 0)
                {
                    item.push_front(center[center.size() - 1], h);
                    center.pop_back();
                }
                return true;

            }
            static bool try_right_side_semi_compile(RunRuleVector &item, const DynamicGrammarForLayeredRLSLP &dic, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                //std::cout << stool::Message::get_paragraph_string(message_paragraph) << "right_side_semi_compile: item.get_max_level() = " << item.get_max_level() << std::endl;

                const DictionaryForLayeredRLSLP &small_dic = dic.get_dictionary();

                int64_t h = item.get_max_level();
                RunRuleVector right = RunRuleVector::create_empty_vector(small_dic);
                std::deque<RunRuleBody> center;
                for (int64_t i = 0; i < h; i++)
                {
                    std::deque<RunRuleBody> new_center;
                    bool b1 = TryCommonSequenceCompiler::try_LCR_semi_compile(item, center, right, i, dic, new_center, stool::Message::increment_paragraph_level(message_paragraph));
                    if(!b1){
                        return false;
                    }
                    center.swap(new_center);
                }
                while (center.size() > 0)
                {
                    item.push_back(center[0], h);
                    center.pop_front();
                }
                return true;
            }


            static bool try_LCR_semi_compile(RunRuleVector &left, std::deque<RunRuleBody> &center, RunRuleVector &right, int64_t h, const DynamicGrammarForLayeredRLSLP &dic, std::deque<RunRuleBody> &output, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                assert(left.verify());
                assert(right.verify());

                assert(RunRuleBody::verify_vector(center));


                std::vector<RunRuleBody> tmp_left_items;
                if (left.back_level() == h)
                {
                    auto tmp = left.pop_back_sequence();
                    tmp_left_items.swap(tmp);
                }
                std::vector<RunRuleBody> tmp_right_items;
                if (right.front_level() == h)
                {
                    auto tmp = right.pop_front_sequence();
                    tmp_right_items.swap(tmp);
                }

                if(center.size() == 0 && tmp_left_items.size() == 0 && tmp_right_items.size() == 0){
                    return true;
                }

                
                for (int64_t i = tmp_left_items.size() - 1; i >= 0; i--)
                {
                    center.insert(center.begin(), tmp_left_items[i]);
                }
                for (int64_t i = 0; i < (int64_t)tmp_right_items.size(); i++)
                {
                    center.push_back(tmp_right_items[i]);
                }
                assert(center.size() > 0);
                RunRuleBody::merge_same_signatures(center);

                std::vector<RunRuleBody> leftItems;
                std::vector<RunRuleBody> rightItems;
                if(dic.get_grammar_parsing_type() == GrammarParsingType::SignatureEncoding && h % 2 == 1){
                    std::vector<RunRuleBody> leftItems2 = left.get_back_sequence(FastCommonSequenceBuilder::left_context_length, h);
                    std::vector<RunRuleBody> rightItems2 = right.get_front_sequence(FastCommonSequenceBuilder::right_context_length, h);
                    leftItems.swap(leftItems2);
                    rightItems.swap(rightItems2);
                }

                assert(RunRuleBody::verify_vector(center));

                bool b1 = TryShrinkAndPow::try_center_line_compile(leftItems, center, rightItems, h, dic, output, stool::Message::increment_paragraph_level(message_paragraph));
                if(!b1){
                    return false;
                }
                assert(RunRuleBody::verify_vector(output));
                return true;

            }

        };
    
}
