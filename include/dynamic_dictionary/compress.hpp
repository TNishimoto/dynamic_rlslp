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

#include "./dynamic_grammar_for_layered_rlslp.hpp"
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
			
			
			

			template <typename C, typename CALLBACK = decltype(no_callback)>
			static SignatureWithRelativeLevel compress(DynamicGrammarForLayeredRLSLP &dic, C input_character, CALLBACK &callback_for_added_signature = no_callback, int message_paragraph = stool::Message::SHOW_MESSAGE)
			{
				std::vector<C> text = {input_character};
				return compress(dic, text, callback_for_added_signature, message_paragraph);
			}

			template <typename C, typename CALLBACK = decltype(no_callback)>
			static SignatureWithRelativeLevel compress(DynamicGrammarForLayeredRLSLP &dic, const std::vector<C> &text, CALLBACK &callback_for_added_signature = no_callback, int message_paragraph = stool::Message::SHOW_MESSAGE)
			{
				dynRLSLP::SignatureWithRelativeLevel root_sig = dynRLSLP::CommonSequenceCompiler::direct_compile_from_text(text, dic, callback_for_added_signature, message_paragraph);
				dic.add_document(root_sig);
				return root_sig;
				/*
				if (text.size() == 0)
				{
					throw std::runtime_error("DynamicString::compress: text.size() == 0");
				}

				// int h = 0;
				std::vector<int64_t> signature_vec, output2;
				std::vector<RunRuleBody> signature_runs;
				std::vector<bool> factorBits;

				ShrinkAndPow::shrink_char(text, dic, callback_for_added_signature, signature_vec, message_paragraph);
				uint64_t current_sequence_level = BSignatureBottomLevel;				

				RunRuleBody::pow(signature_vec, signature_runs);
				ShrinkAndPow::pow(signature_runs, current_sequence_level, dic, callback_for_added_signature, signature_vec);
				current_sequence_level++;


				while (signature_vec.size() > 1)
				{
					if (message_paragraph != stool::Message::NO_MESSAGE)
					{
						std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing the level-" << current_sequence_level << " sequence in the derivation tree..." << std::endl;
					}

					// my::MyStrings::print(output);
					LocallyConsistentParsing::compute_factor_bits(signature_vec, factorBits);
					// NaiveParsing::compute_factor_bits(output, factorBits);

					ShrinkAndPow::shrink(signature_vec, factorBits, current_sequence_level, dic,callback_for_added_signature, output2);
					current_sequence_level++;

					if (message_paragraph != stool::Message::NO_MESSAGE)
					{
						std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing the level-" << current_sequence_level << " sequence in the derivation tree..." << std::endl;
					}

					RunRuleBody::pow(output2, signature_runs);
					ShrinkAndPow::pow(signature_runs, current_sequence_level, dic, callback_for_added_signature, signature_vec);
					current_sequence_level++;
				}
				assert(signature_vec.size() == 1);
				dic.add_document(signature_vec[0]);
				return signature_vec[0];
				*/
			}
			static void print_performance(DynamicGrammarForLayeredRLSLP &dic)
			{

                const std::vector<uint64_t>& base_signature_length_list = dic.get_base_signature_length_list();
				const GrammarForLayeredRLSLP &static_grammar = dic.get_grammar();
				SignatureWithRelativeLevel root = static_grammar.get_root();

				uint64_t len = SignatureFunctions::get_length(root, base_signature_length_list);
				uint64_t size = static_grammar.signature_count();
				std::cout << "text len = " << len << "/ sig count = " << size << std::endl;

			}
		};
	

}