#pragma once
#include "../local_parsings/locally_consistent_parsing.hpp"
#include "../rlslp/components/dictionary_for_layered_rlslp.hpp"
#include "./shrink_and_pow.hpp"
#include "../rlslp/static_operations/fast_common_sequence_builder.hpp"

namespace dynRLSLP
{


        /**
         * @brief A class for compiling common sequences.
         * @ingroup DynamicDictionaryClasses
         */
        class CommonSequenceCompiler
        {

        public:
            /**
             * @brief Compiles a run-rule vector into a single root nonterminal.
             * @tparam CALLBACK Callback type invoked when new nonterminals are added.
             * @param seq Input run-rule vector to compile.
             * @param dic Dynamic grammar updated during compilation.
             * @param callback_for_added_nonterminals Callback invoked for each added nonterminal.
             * @param message_paragraph Indentation level for progress messages.
             * @return Root nonterminal of the compiled derivation.
             */
            template <typename CALLBACK = decltype(no_callback)>
            static NonterminalWithRelativeLevel single_compile(RunRuleVector &seq, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_nonterminals, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if(seq.is_empty()){
                    throw std::runtime_error("Error in single_compile: seq is empty");
                }
                //std::cout << stool::Message::get_paragraph_string(message_paragraph) << "single_compile: seq.size() = " << seq.size() << std::endl;

                CommonSequenceCompiler::left_side_semi_compile(seq, dic, callback_for_added_nonterminals, stool::Message::increment_paragraph_level(message_paragraph));
                CommonSequenceCompiler::right_side_semi_compile(seq, dic, callback_for_added_nonterminals, stool::Message::increment_paragraph_level(message_paragraph));


                std::deque<RunRuleBody> current_seq;
                int16_t level = seq.get_max_level();
                auto tmp = seq.pop_front_sequence();
                for(auto it : tmp){
                    current_seq.push_back(it);
                }
                RunRuleBody::merge_same_nonterminals(current_seq);

                std::vector<RunRuleBody> empty_left_line;
                std::vector<RunRuleBody> empty_right_line;

                assert(RunRuleBody::verify_vector(current_seq));

                while (!RunRuleBody::is_implicit_nonterminal(current_seq))
                {
                    assert(current_seq.size() > 0);
                    std::deque<RunRuleBody> new_seq = ShrinkAndPow::center_line_compile(empty_left_line, current_seq, empty_right_line, level, dic, callback_for_added_nonterminals, stool::Message::increment_paragraph_level(message_paragraph));
                    current_seq.swap(new_seq);
                    level++;
                }
                return current_seq[0].number;

            }

            /**
             * @brief Compiles the concatenation of left and right run-rule vectors.
             * @tparam CALLBACK Callback type invoked when new nonterminals are added.
             * @param left Left run-rule vector.
             * @param right Right run-rule vector.
             * @param dic Dynamic grammar updated during compilation.
             * @param callback_for_added_nonterminal Callback invoked for each added nonterminal.
             * @param message_paragraph Indentation level for progress messages.
             * @return Root nonterminal of the concatenated derivation.
             */
            template <typename CALLBACK = decltype(no_callback)>
            static NonterminalWithRelativeLevel LR_compile(RunRuleVector &left, RunRuleVector &right, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_nonterminal, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {

// debug
#ifdef DEBUG
                int64_t leftLen = left.get_string_length();
                int64_t rightLen = right.get_string_length();
#endif

                CommonSequenceCompiler::left_side_semi_compile(left, dic, callback_for_added_nonterminal, stool::Message::increment_paragraph_level(message_paragraph));
#ifdef DEBUG
                assert(left.get_string_length() == leftLen);
#endif

                CommonSequenceCompiler::right_side_semi_compile(right, dic, callback_for_added_nonterminal, stool::Message::increment_paragraph_level(message_paragraph));
#ifdef DEBUG
                assert(right.get_string_length() == rightLen);
#endif

                NonterminalWithRelativeLevel r = CommonSequenceCompiler::center_compile(left, right, dic, callback_for_added_nonterminal, stool::Message::increment_paragraph_level(message_paragraph));
#ifdef DEBUG
                const std::vector<uint64_t>& explicit_nonterminal_length_list = dic.get_explicit_nonterminal_length_list();
                int64_t resultLen = NonterminalFunctions::get_length(r, explicit_nonterminal_length_list);
                if (resultLen != leftLen + rightLen)
                {
                    std::cout << "ERROR in LR_compile: resultLen != leftLen + rightLen" << std::endl;
                    std::cout << "resultLen: " << resultLen << ", leftLen: " << leftLen << ", rightLen: " << rightLen << std::endl;
                    throw std::runtime_error("ERROR in LR_compile");
                }
#endif

                return r;
            }
            /**
             * @brief Builds a common sequence from text and compiles it to one nonterminal.
             * @tparam C Character type of the input text.
             * @tparam CALLBACK Callback type invoked when new nonterminals are added.
             * @param text Input character sequence.
             * @param dic Dynamic grammar updated during compilation.
             * @param callback_for_added_nonterminals Callback invoked for each added nonterminal.
             * @param message_paragraph Indentation level for progress messages.
             * @return Root nonterminal of the compiled text.
             */
            template <typename C, typename CALLBACK = decltype(no_callback)>
            static NonterminalWithRelativeLevel direct_compile_from_text(const std::vector<C> &text, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_nonterminals, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if(text.size() == 0){
                    throw std::runtime_error("Error in direct_compile_from_text: text is empty");
                }
                RunRuleVector seq = build_common_sequence_from_text(dic, text, callback_for_added_nonterminals, message_paragraph);
                assert(seq.is_empty() == false);
                return single_compile(seq, dic, callback_for_added_nonterminals, message_paragraph);
            }



            /**
             * @brief Constructs a layered run-rule vector from plain text.
             * @tparam C Character type of the input text.
             * @tparam CALLBACK Callback type invoked when new nonterminals are added.
             * @param dic Dynamic grammar used to allocate nonterminals.
             * @param text Input character sequence.
             * @param callback_for_added_nonterminal Callback invoked for each added nonterminal.
             * @param message_paragraph Indentation level for progress messages.
             * @return Run-rule vector representing the derivation tree of the text.
             */
            template <typename C, typename CALLBACK = decltype(no_callback)>
            static RunRuleVector build_common_sequence_from_text(DynamicGrammarForLayeredRLSLP &dic, const std::vector<C> &text, CALLBACK &callback_for_added_nonterminal = no_callback, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if(text.size() == 0){
                    throw std::runtime_error("Error in build_common_sequence_from_text: text is empty");
                }
                FastCommonSequenceBuilder::initialize(dic.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);
                std::deque<RunRuleBody> current_seq = ShrinkAndPow::shrink_char(text, dic, callback_for_added_nonterminal, stool::Message::increment_paragraph_level(message_paragraph));
                uint64_t level = 0;

                const DictionaryForLayeredRLSLP &small_dic = dic.get_dictionary();

                RunRuleVector output = RunRuleVector::create_empty_vector(small_dic);

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
                        std::deque<RunRuleBody> new_seq = ShrinkAndPow::center_line_compile(tmp_left_line, current_seq, tmp_right_line, level, dic, callback_for_added_nonterminal, stool::Message::increment_paragraph_level(message_paragraph));
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
                return output;
            }

        private:
            /**
             * @brief Verifies that a nonterminal matches recompilation from its decompressed text.
             * @param sig Nonterminal to verify.
             * @param dic Dynamic grammar used for recompilation.
             * @param message_paragraph Indentation level for debug output.
             * @return True if the nonterminal is consistent with recompilation.
             */
            static bool verify(NonterminalWithRelativeLevel sig, DynamicGrammarForLayeredRLSLP &dic, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                
                const DictionaryForLayeredRLSLP &small_dic = dic.get_dictionary();

                RunRuleVector seq(sig, small_dic);
                std::vector<uint8_t> text;
                seq.decompress(text);

                FastCommonSequenceBuilder::initialize(dic.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);
                std::deque<RunRuleBody> current_seq = ShrinkAndPow::shrink_char(text, dic, dynRLSLP::no_callback, message_paragraph);
                std::vector<RunRuleBody> tmp_left_line;
                std::vector<RunRuleBody> tmp_right_line;
                uint64_t level = 0;
                while (!RunRuleBody::is_implicit_nonterminal(current_seq))
                {
                    assert(current_seq.size() > 0);
                    std::deque<RunRuleBody> new_seq = ShrinkAndPow::center_line_compile(tmp_left_line, current_seq, tmp_right_line, level, dic, dynRLSLP::no_callback, stool::Message::increment_paragraph_level(message_paragraph));
                    current_seq.swap(new_seq);
                    level++;
                }
                NonterminalWithRelativeLevel correct_sig = current_seq[0].number;

                if (sig != correct_sig)
                {
                    std::cout << "ERROR in verify: sig != correct_sig" << std::endl;
                    std::cout << "sig: " << sig << ", correct_sig: " << correct_sig << std::endl;
                    stool::DebugPrinter::print_characters(text, "text");

                    dic.print_derivation_tree(sig, message_paragraph + 1);
                    dic.print_derivation_tree(correct_sig, message_paragraph + 1);
                    throw std::runtime_error("ERROR in verify");
                }
                return true;
            }


            

            /**
             * @brief Semi-compiles the center line using left and right run-rule vectors at level @p h.
             * @tparam CALLBACK Callback type invoked when new nonterminals are added.
             * @param left Left run-rule vector.
             * @param center Center deque updated in place and then compiled.
             * @param right Right run-rule vector.
             * @param h Current hierarchy level.
             * @param dic Dynamic grammar updated during compilation.
             * @param callback_for_added_nonterminal Callback invoked for each added nonterminal.
             * @param message_paragraph Indentation level for progress messages.
             * @return Compiled center sequence at the next level.
             */
            template <typename CALLBACK = decltype(no_callback)>
            static std::deque<RunRuleBody> LCR_semi_compile(RunRuleVector &left, std::deque<RunRuleBody> &center, RunRuleVector &right, int64_t h, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_nonterminal, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                assert(left.verify());
                assert(right.verify());
                //std::cout << stool::Message::get_paragraph_string(message_paragraph) << "LCR_semi_compile: h = " << h << std::endl;

                assert(RunRuleBody::verify_vector(center));

#ifdef DEBUG
                int64_t prevLeftLen = left.get_string_length();
                int64_t prevRightLen = right.get_string_length();
                const std::vector<uint64_t>& explicit_nonterminal_length_list = dic.get_explicit_nonterminal_length_list();
                int64_t prevCenterLen = RunRuleBody::compute_string_length(center, explicit_nonterminal_length_list);
#endif

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
                    std::deque<RunRuleBody> r;
                    return r;
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
                RunRuleBody::merge_same_nonterminals(center);

                std::vector<RunRuleBody> leftItems;
                std::vector<RunRuleBody> rightItems;
                if(dic.get_grammar_parsing_type() == GrammarParsingType::SignatureEncoding && h % 2 == 1){
                    std::vector<RunRuleBody> leftItems2 = left.get_back_sequence(FastCommonSequenceBuilder::left_context_length, h);
                    std::vector<RunRuleBody> rightItems2 = right.get_front_sequence(FastCommonSequenceBuilder::right_context_length, h);
                    leftItems.swap(leftItems2);
                    rightItems.swap(rightItems2);
                }

                



                assert(RunRuleBody::verify_vector(center));
                // RunRuleBody::print_vector(center, "new_center", stool::Message::increment_paragraph_level(message_paragraph));
                //assert(center.size() > 0);


                std::deque<RunRuleBody> r = ShrinkAndPow::center_line_compile(leftItems, center, rightItems, h, dic, callback_for_added_nonterminal, stool::Message::increment_paragraph_level(message_paragraph));
                // RunRuleBody::print_vector(r, "r", stool::Message::increment_paragraph_level(message_paragraph));
                assert(RunRuleBody::verify_vector(r));

#ifdef DEBUG
                int64_t centerLen = RunRuleBody::compute_string_length(center, explicit_nonterminal_length_list);
                int64_t leftLen = left.get_string_length();
                int64_t rightLen = right.get_string_length();
                int64_t rLen = RunRuleBody::compute_string_length(r, explicit_nonterminal_length_list);
                if (prevLeftLen + prevCenterLen + prevRightLen != leftLen + centerLen + rightLen)
                {
                    std::cout << "ERROR in LCR_semi_compile: prevLeftLen + prevCenterLen + prevRightLen != leftLen + centerLen + rightLen" << std::endl;
                    std::cout << "prevLeftLen: " << prevLeftLen << ", prevCenterLen: " << prevCenterLen << ", prevRightLen: " << prevRightLen << std::endl;
                    std::cout << "leftLen: " << leftLen << ", centerLen: " << centerLen << ", rightLen: " << rightLen << std::endl;
                    throw std::runtime_error("ERROR in LCR_semi_compile1");
                }
                if (centerLen != rLen)
                {
                    std::cout << "ERROR in LCR_semi_compile: centerLen != rLen" << std::endl;
                    std::cout << "centerLen: " << centerLen << ", rLen: " << rLen << std::endl;


                    std::cout << "center: " << std::endl;
                    for(auto it : center){
                        std::cout << it.to_string() << "_" << NonterminalFunctions::get_length(it.number, explicit_nonterminal_length_list) << " ";
                    }
                    std::cout << std::endl;

                    std::cout << "r: " << std::endl;
                    for(auto it : r){
                        std::cout << it.to_string() << "_" << NonterminalFunctions::get_length(it.number, explicit_nonterminal_length_list) << " ";
                    }
                    std::cout << std::endl;
                    throw std::runtime_error("ERROR in LCR_semi_compile2: centerLen != rLen");
                }

#endif

                return r;
            }
            /**
             * @brief Semi-compiles the left side of a run-rule vector level by level.
             * @tparam CALLBACK Callback type invoked when new nonterminals are added.
             * @param item Run-rule vector whose left side is semi-compiled in place.
             * @param dic Dynamic grammar updated during compilation.
             * @param callback_for_added_nonterminals Callback invoked for each added nonterminal.
             * @param message_paragraph Indentation level for progress messages.
             */
            template <typename CALLBACK = decltype(no_callback)>
            static void left_side_semi_compile(RunRuleVector &item, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_nonterminals, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                //std::cout << stool::Message::get_paragraph_string(message_paragraph) << "left_side_semi_compile: item.get_max_level() = " << item.get_max_level() << std::endl;

#ifdef DEBUG
                int64_t prevLen = item.get_string_length();
                std::string prevString;
                item.decompress(prevString);
#endif

                const DictionaryForLayeredRLSLP &small_dic = dic.get_dictionary();
                
                int64_t h = 0;
                RunRuleVector empty_left = RunRuleVector::create_empty_vector(small_dic);
                std::deque<RunRuleBody> center;
                for (int64_t i = 0; i < item.get_max_level(); i++)
                {
                    std::deque<RunRuleBody> new_center = CommonSequenceCompiler::LCR_semi_compile(empty_left, center, item, h++, dic, callback_for_added_nonterminals, stool::Message::increment_paragraph_level(message_paragraph));
                    center.swap(new_center);

#ifdef DEBUG
                    int64_t midRightLen = item.get_string_length();
                    int64_t midCenterLen = RunRuleBody::compute_string_length(center, dic.get_explicit_nonterminal_length_list());
                    if (prevLen != midCenterLen + midRightLen)
                    {
                        std::cout << "ERROR in left_side_semi_compile: prevLen != midCenterLen + midRightLen" << std::endl;
                        std::cout << "prevLen: " << prevLen << ", midCenterLen: " << midCenterLen << ", midRightLen: " << midRightLen << std::endl;
                        item.print_info();
                        throw std::runtime_error("ERROR in left_side_semi_compile");
                    }
#endif
                }
                // RunRuleBody::print_vector(center, "center", message_paragraph+1);
                while (center.size() > 0)
                {
                    item.push_front(center[center.size() - 1], h);
                    center.pop_back();
                }
#ifdef DEBUG
                int64_t newLen = item.get_string_length();
                std::string newString;
                item.decompress(newString);
                if (newString != prevString)
                {
                    std::cout << "ERROR in left_side_semi_compile: newString != prevString" << std::endl;
                    std::cout << "newString: " << newString << std::endl;
                    std::cout << "prevString: " << prevString << std::endl;
                    item.print_info();
                    throw std::runtime_error("ERROR in left_side_semi_compile(new)");
                }

                if (prevLen != newLen)
                {
                    std::cout << "ERROR in left_side_semi_compile: prevLen != newLen" << std::endl;
                    std::cout << "prevLen: " << prevLen << ", newLen: " << newLen << std::endl;
                    throw std::runtime_error("ERROR in left_side_semi_compile");
                }
#endif
            }
            /**
             * @brief Semi-compiles the right side of a run-rule vector level by level.
             * @tparam CALLBACK Callback type invoked when new nonterminals are added.
             * @param item Run-rule vector whose right side is semi-compiled in place.
             * @param dic Dynamic grammar updated during compilation.
             * @param callback_for_added_nonterminal Callback invoked for each added nonterminal.
             * @param message_paragraph Indentation level for progress messages.
             */
            template <typename CALLBACK = decltype(no_callback)>
            static void right_side_semi_compile(RunRuleVector &item, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_nonterminal, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                //std::cout << stool::Message::get_paragraph_string(message_paragraph) << "right_side_semi_compile: item.get_max_level() = " << item.get_max_level() << std::endl;

                const DictionaryForLayeredRLSLP &small_dic = dic.get_dictionary();
                

                int64_t h = item.get_max_level();
                RunRuleVector right = RunRuleVector::create_empty_vector(small_dic);
                std::deque<RunRuleBody> center;
                for (int64_t i = 0; i < h; i++)
                {
                    std::deque<RunRuleBody> new_center = CommonSequenceCompiler::LCR_semi_compile(item, center, right, i, dic, callback_for_added_nonterminal, stool::Message::increment_paragraph_level(message_paragraph));
                    center.swap(new_center);
                }
                while (center.size() > 0)
                {
                    item.push_back(center[0], h);
                    center.pop_front();
                }
            }
            /**
             * @brief Compiles the center region after left and right semi-compilation.
             * @tparam CALLBACK Callback type invoked when new nonterminals are added.
             * @param left Left run-rule vector consumed during compilation.
             * @param right Right run-rule vector consumed during compilation.
             * @param dic Dynamic grammar updated during compilation.
             * @param callback_for_added_nonterminal Callback invoked for each added nonterminal.
             * @param message_paragraph Indentation level for progress messages.
             * @return Root nonterminal of the merged center derivation.
             */
            template <typename CALLBACK = decltype(no_callback)>
            static NonterminalWithRelativeLevel center_compile(RunRuleVector &left, RunRuleVector &right, DynamicGrammarForLayeredRLSLP &dic, CALLBACK &callback_for_added_nonterminal, int64_t message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                /*
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "left: " << std::endl;
                left.print_info();
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "right: " << std::endl;
                right.print_info();
                */

                std::deque<RunRuleBody> center;
                int64_t t = 0;
                while (true)
                {
                    if (t > 10000)
                    {
                        throw std::runtime_error("ERROR in center_compile: counter > 10000");
                    }
                    std::deque<RunRuleBody> new_center = CommonSequenceCompiler::LCR_semi_compile(left, center, right, t++, dic, callback_for_added_nonterminal, stool::Message::increment_paragraph_level(message_paragraph));
                    center.swap(new_center);
                    if (RunRuleBody::is_implicit_nonterminal(center) && left.is_empty() && right.is_empty())
                        break;
                }
                return center[0].number;
            }
        };
    
}