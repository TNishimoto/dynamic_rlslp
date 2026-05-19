#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <stack>
#include <exception>
#include <set>
#include <cassert>
#include <random>

#include "stool/include/basic/byte.hpp"
#include "stool/include/basic/log.hpp"
#include "stool/include/debug/message.hpp"

#include "../dynamic_rlslp/dynamic_grammar_for_layered_rlslp.hpp"
#include "../local_parsings/locally_consistent_parsing.hpp"
#include "../local_parsings/naive_parsing.hpp"
#include "./shrink_and_pow.hpp"

namespace dynRLSLP
{

		/**
		 * @brief A class for computing a compressed grammar from a text
         * @ingroup DynamicDictionaryClasses
		 */
		class Compress
		{
		public:
			/**
			 * @brief Compresses a single character into the dynamic grammar.
			 * @tparam C Character type of the input.
			 * @tparam CALLBACK Callback type invoked when new nonterminals are added.
			 * @param dic Dynamic grammar to update.
			 * @param input_character Single input character.
			 * @param callback_for_added_nonterminal Callback invoked for each added nonterminal.
			 * @param message_paragraph Indentation level for progress messages.
			 * @return Root nonterminal of the compressed document.
			 */
			template <typename C, typename CALLBACK = decltype(no_callback)>
			static NonterminalWithRelativeLevel compress(DynamicGrammarForLayeredRLSLP &dic, C input_character, CALLBACK &callback_for_added_nonterminal = no_callback, int message_paragraph = stool::Message::SHOW_MESSAGE)
			{
				std::vector<C> text = {input_character};
				return compress(dic, text, callback_for_added_nonterminal, message_paragraph);
			}

			/**
			 * @brief Compresses a text into the dynamic grammar.
			 * @tparam C Character type of the input text.
			 * @tparam CALLBACK Callback type invoked when new nonterminals are added.
			 * @param dic Dynamic grammar to update.
			 * @param text Input character sequence.
			 * @param callback_for_added_nonterminal Callback invoked for each added nonterminal.
			 * @param message_paragraph Indentation level for progress messages.
			 * @return Root nonterminal of the compressed document.
			 */
			template <typename C, typename CALLBACK = decltype(no_callback)>
			static NonterminalWithRelativeLevel compress(DynamicGrammarForLayeredRLSLP &dic, const std::vector<C> &text, CALLBACK &callback_for_added_nonterminal = no_callback, int message_paragraph = stool::Message::SHOW_MESSAGE)
			{
				dynRLSLP::NonterminalWithRelativeLevel root_sig = dynRLSLP::CommonSequenceCompiler::direct_compile_from_text(text, dic, callback_for_added_nonterminal, message_paragraph);
				dic.add_document(root_sig);
				return root_sig;
				/*
				if (text.size() == 0)
				{
					throw std::runtime_error("DynamicString::compress: text.size() == 0");
				}

				// int h = 0;
				std::vector<int64_t> nonterminal_vec, output2;
				std::vector<RunRuleBody> nonterminal_runs;
				std::vector<bool> factorBits;

				ShrinkAndPow::shrink_char(text, dic, callback_for_added_nonterminal, nonterminal_vec, message_paragraph);
				uint64_t current_sequence_level = NonterminalBottomLevel;				

				RunRuleBody::pow(nonterminal_vec, nonterminal_runs);
				ShrinkAndPow::pow(nonterminal_runs, current_sequence_level, dic, callback_for_added_nonterminal, nonterminal_vec);
				current_sequence_level++;


				while (nonterminal_vec.size() > 1)
				{
					if (message_paragraph != stool::Message::NO_MESSAGE)
					{
						std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing the level-" << current_sequence_level << " sequence in the derivation tree..." << std::endl;
					}

					// my::MyStrings::print(output);
					LocallyConsistentParsing::compute_factor_bits(nonterminal_vec, factorBits);
					// NaiveParsing::compute_factor_bits(output, factorBits);

					ShrinkAndPow::shrink(nonterminal_vec, factorBits, current_sequence_level, dic,callback_for_added_nonterminal, output2);
					current_sequence_level++;

					if (message_paragraph != stool::Message::NO_MESSAGE)
					{
						std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing the level-" << current_sequence_level << " sequence in the derivation tree..." << std::endl;
					}

					RunRuleBody::pow(output2, nonterminal_runs);
					ShrinkAndPow::pow(nonterminal_runs, current_sequence_level, dic, callback_for_added_nonterminal, nonterminal_vec);
					current_sequence_level++;
				}
				assert(nonterminal_vec.size() == 1);
				dic.add_document(nonterminal_vec[0]);
				return nonterminal_vec[0];
				*/
			}
			/**
			 * @brief Prints text length and nonterminal count for the grammar root.
			 * @param dic Dynamic grammar whose static grammar is summarized.
			 */
			static void print_performance(DynamicGrammarForLayeredRLSLP &dic)
			{

                const std::vector<uint64_t>& explicit_nonterminal_length_list = dic.get_explicit_nonterminal_length_list();
				const GrammarForLayeredRLSLP &static_grammar = dic.get_grammar();
				NonterminalWithRelativeLevel root = static_grammar.get_root();

				uint64_t len = NonterminalFunctions::get_length(root, explicit_nonterminal_length_list);
				uint64_t size = static_grammar.nonterminal_count();
				std::cout << "text len = " << len << "/ sig count = " << size << std::endl;

			}
		};
	

}