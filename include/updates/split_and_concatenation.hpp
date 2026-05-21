#pragma once
#include "../dynamic_rlslp/dynamic_grammar_for_layered_rlslp.hpp"
#include "./common_sequence_compiler.hpp"
#include "../rlslp/static_operations/fast_common_sequence_builder.hpp"
//#include "dynamic_grammar_for_layered_rlslp.hpp"
//#include "../local_parsings/locally_consistent_parsing.hpp"

// #include "./i_integer_set.hpp"

namespace dynRLSLP
{
        /**
         * @brief A class for supporting split and concatenate operations.
         * @ingroup DynamicDictionaryClasses
         */
        class SplitAndConcatenation
        {
        public:
            /**
             * @brief Splits a document nonterminal into left and right parts at a given length.
             * @tparam CALLBACK1 Callback type invoked when nonterminals are removed.
             * @tparam CALLBACK2 Callback type invoked when nonterminals are added.
             * @param node Nonterminal of the document to split.
             * @param leftLength Length of the left part in characters.
             * @param remove_node If true, remove the original document when @p process_document is set.
             * @param process_document If true, update the document set in the grammar.
             * @param dic Dynamic grammar updated during the split.
             * @param callback_for_removed_nonterminals Callback invoked for removed nonterminals.
             * @param callback_for_added_nonterminals Callback invoked for added nonterminals.
             * @return Pair of nonterminals for the left and right parts.
             */
            template <typename CALLBACK1 = decltype(no_callback), typename CALLBACK2 = decltype(no_callback)>
            static std::pair<int64_t, int64_t> split(NonterminalWithRelativeLevel node, int64_t leftLength, bool remove_node, bool process_document, DynamicGrammarForLayeredRLSLP &dic, CALLBACK1 &callback_for_removed_nonterminals, CALLBACK2 &callback_for_added_nonterminals)
            {

                const DictionaryForLayeredRLSLP &small_dic = dic.get_dictionary();

                assert(0 < leftLength && leftLength < (int64_t)NonterminalFunctions::get_length(node, small_dic.get_explicit_nonterminal_length_list()));
                FastCommonSequenceBuilder::initialize(dic.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression);

                #ifdef DEBUG
                RunRuleVector nodeVector = FastCommonSequenceBuilder::build(node, small_dic);
                //std::cout << "Node: " << std::endl;
                //nodeVector.print_derivation_tree(3, 2);
                #endif


                RunRuleVector left = FastCommonSequenceBuilder::build(node, 0, leftLength, small_dic);


                //assert(left.check_equal_sequence(left2));
                assert(left.get_string_length() == leftLength);

                RunRuleVector right = FastCommonSequenceBuilder::build(node, leftLength, small_dic);
                //assert(right.check_equal_sequence(right2));
                assert((int64_t)right.get_string_length() == (int64_t)(NonterminalFunctions::get_length(node, small_dic.get_explicit_nonterminal_length_list()) - leftLength));
                //std::cout << "Right: " << std::endl;
                //right.print_derivation_tree(3, 2);

                int64_t left1 = CommonSequenceCompiler::single_compile(left, dic, callback_for_added_nonterminals);
                int64_t right1 = CommonSequenceCompiler::single_compile(right, dic, callback_for_added_nonterminals);

                assert((int64_t)NonterminalFunctions::get_length(left1, small_dic.get_explicit_nonterminal_length_list()) == leftLength);

                if (process_document)
                {
                    dic.add_document(left1);
                    dic.add_document(right1);
                }

#ifdef SLOWDEBUG
                this->CheckDocumentKey(left1);
                this->CheckDocumentKey(right1);
#endif
                if (remove_node && process_document)
                {
                    dic.remove_document(node, callback_for_removed_nonterminals);
                }

#ifdef SLOWDEBUG
                this->CheckDocumentKey(left1);
                this->CheckDocumentKey(right1);
#endif
                assert(NonterminalFunctions::get_length(left1, small_dic.get_explicit_nonterminal_length_list()) > 0 && NonterminalFunctions::get_length(right1, small_dic.get_explicit_nonterminal_length_list()) > 0);

                // std::cout << "Split = " << left1 << ", " << right1 << std::endl;
                return std::pair<int64_t, int64_t>(left1, right1);
            }

            /**
             * @brief Concatenates two document nonterminals into one.
             * @tparam CALLBACK1 Callback type invoked when nonterminals are removed.
             * @tparam CALLBACK2 Callback type invoked when nonterminals are added.
             * @param left Nonterminal of the left document.
             * @param right Nonterminal of the right document.
             * @param process_document If true, update the document set in the grammar.
             * @param dic Dynamic grammar updated during concatenation.
             * @param callback_for_removed_nonterminals Callback invoked for removed nonterminals.
             * @param callback_for_added_nonterminals Callback invoked for added nonterminals.
             * @param message_paragraph Indentation level for progress messages.
             * @return Nonterminal of the concatenated document.
             */
            template <typename CALLBACK1 = decltype(no_callback), typename CALLBACK2 = decltype(no_callback)>
            static int64_t concatenate(int64_t left, int64_t right, bool process_document, DynamicGrammarForLayeredRLSLP &dic, CALLBACK1 &callback_for_removed_nonterminals = no_callback, CALLBACK2 &callback_for_added_nonterminals = no_callback, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                FastCommonSequenceBuilder::initialize(dic.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression);
                const DictionaryForLayeredRLSLP &small_dic = dic.get_dictionary();

#ifdef DEBUG
                int64_t leftLen = NonterminalFunctions::get_length(left, small_dic.get_explicit_nonterminal_length_list());
                int64_t rightLen = NonterminalFunctions::get_length(right, small_dic.get_explicit_nonterminal_length_list());
#endif

                RunRuleVector left2 = FastCommonSequenceBuilder::build(left, small_dic);
                RunRuleVector right2 = FastCommonSequenceBuilder::build(right, small_dic);


                NonterminalWithRelativeLevel p = CommonSequenceCompiler::LR_compile(left2, right2, dic, callback_for_added_nonterminals, stool::Message::increment_paragraph_level(message_paragraph));

#ifdef SLOWDEBUG
                this->CheckDocumentKey(p);
#endif

#ifdef DEBUG
                [[maybe_unused]] int64_t resultLen = NonterminalFunctions::get_length(p, small_dic.get_explicit_nonterminal_length_list());
                assert(leftLen + rightLen == resultLen);
                assert(resultLen > 0);
#endif

                if (process_document)
                {
                    dic.remove_document(left, callback_for_removed_nonterminals);
                    dic.remove_document(right, callback_for_removed_nonterminals);

                    dic.add_document(p);
                }
                // std::cout << "Concat = " << p << std::endl;
                return p;
            }
        };

    
}