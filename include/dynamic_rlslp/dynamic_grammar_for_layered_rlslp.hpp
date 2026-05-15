#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "stool/include/all.hpp"
#include "../local_parsings/mu.hpp"
#include "../rlslp/grammar_for_layered_rlslp.hpp"
#include "./parent_query/fast_parent_dictionary.hpp"
#include "./parent_query/node_occurrence_query.hpp"
// #include "./i_integer_set.hpp"

namespace dynRLSLP
{

		enum class DictionaryMode : uint8_t
		{
			Standard = 0,
			Lightweight = 1,
			Fast = 2,
			StandardWithImprovement = 3
		};

		/**
		 * @brief A dynamic data structure for storing the infomation about a compressed grammar \p G of \p g nonterminals representing \p d derivation trees
		 * This data structure consists of following nine data structures:
		 * @li \p std::vector<RLSLPRuleBody> \p D[0..m-1]: \p D[i] stores the rule body of the nonterminal \p v_{i} if it exists.
		 * @li \p std::vector<uint8_t> \p H[0..m-1]: \p H[i] stores the height of the subtree derived from the nonterminal \p v_{i} if it exists.
		 * @li \p std::vector<uint64_t> \p L[0..m-1]: \p L[i] stores the length of the string derived from the nonterminal \p v_{i} if it exists.
		 * @li \p std::vector<SignatureWithRelativeLevel> \p Z[0..g-m-1]: This vector stores unused signatures (identifiers) in any order.
		 * @li \p std::vector<bool> \p B[0..m-1]: \p B[i] stores a random bit assigned to the nonterminal \p v_{i} if it exists.
		 * @li \p std::vector<std::vector<SignatureWithRelativeLevel> \p F: \p F[i] stores the list \p P_{i} of parents of the nonterminal \p v_{i} in the DAG correspnding to the grammar \p G if \p v_{i} exists and the length of the list is at most \p PARENT_NUMBER_THRESHOLD.
		 * @li \p std::map<int64_t, \p SignatureWithRelativeLevel> \p M_{char}: \p M_{char}[c] stores the signature deriving the character \p c.
		 * @li \p std::unordered_map<SignatureWithRelativeLevel, \p std::set<SignatureWithRelativeLevel, \p NonterminalLessComparer>> \p M_{pair}: \p M_{pair}[i] stores the list \p P_{i} of parents of the nonterminal \p v_{i} if the list is longer than \p PARENT_NUMBER_THRESHOLD.
		 * @li \p std::unordered_map<SignatureWithRelativeLevel, \p uint64_t> \p M_{doc}: \p M_{doc}[i] stores the number of derivation trees rooted at the nonterminal \p v_{i}
		 * @ingroup DynamicDictionaryClasses
		 * @note @li \p v_{i} is the nonterminal with the signature (identifier) \p i in {0, 1, ..., m-1}. \p v_{i} does not exist if \p G does not contain the nonterminal.
		 * @note @li \p m >= g and m = Ω(g)
		 */
		class DynamicGrammarForLayeredRLSLP
		{

		private:
			GrammarForLayeredRLSLP grammar;
			FastParentDictionary fastParentDictionary;
			std::vector<SignatureWithRelativeLevel> unused_signatures;					// Z
			std::map<int64_t, SignatureWithRelativeLevel> character_signature_item_map; // M_{char}
			std::unordered_map<int64_t, uint64_t> character_id_map;

		public:
			inline static const uint64_t FINGERPRINT = 8937492749;
			////////////////////////////////////////////////////////////////////////////////
			///   @name Constructors and Destructor
			////////////////////////////////////////////////////////////////////////////////
			//@{
			/**
			 * @brief Default constructor.
			 */
			DynamicGrammarForLayeredRLSLP()
			{
				NonterminalLessComparer::base_signature_rule_list = &this->grammar.get_rlslp_dictionary().get_base_signature_rule_list();

				this->clear();
			}
			/**
			 * @brief Default destructor.
			 */
			~DynamicGrammarForLayeredRLSLP()
			{
				NonterminalLessComparer::base_signature_rule_list = nullptr;
			}
			/**
			 * @brief Deleted copy constructor.
			 */
			DynamicGrammarForLayeredRLSLP(const DynamicGrammarForLayeredRLSLP &) = delete;
			/**
			 * @brief Move constructor.
			 */
			DynamicGrammarForLayeredRLSLP(DynamicGrammarForLayeredRLSLP &&other) noexcept : grammar(std::move(other.grammar)),
																							fastParentDictionary(std::move(other.fastParentDictionary)),
																							unused_signatures(std::move(other.unused_signatures)),
																							character_signature_item_map(std::move(other.character_signature_item_map)),
																							character_id_map(std::move(other.character_id_map))
			{
				// this->parentDictionary.set_pointer(&this->relative_max_level_list_);
				this->fastParentDictionary.set_pointer(&this->grammar.get_rlslp_dictionary().get_relative_max_level_list(), this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);
				NonterminalLessComparer::base_signature_rule_list = &this->grammar.get_rlslp_dictionary().get_base_signature_rule_list();
			}

			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Operators
			////////////////////////////////////////////////////////////////////////////////
			//@{
			/**
			 * @brief Deleted copy assignment operator.
			 */
			DynamicGrammarForLayeredRLSLP &operator=(const DynamicGrammarForLayeredRLSLP &) = delete;
			/**
			 * @brief Move assignment operator.
			 */
			DynamicGrammarForLayeredRLSLP &operator=(DynamicGrammarForLayeredRLSLP &&other) noexcept
			{
				if (this != &other)
				{
					this->grammar = std::move(other.grammar);
					this->unused_signatures = std::move(other.unused_signatures);
					this->character_signature_item_map = std::move(other.character_signature_item_map);
					this->fastParentDictionary = std::move(other.fastParentDictionary);
					this->character_id_map = std::move(other.character_id_map);

					this->fastParentDictionary.set_pointer(&this->grammar.get_rlslp_dictionary().get_relative_max_level_list(), this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);
					NonterminalLessComparer::base_signature_rule_list = &this->grammar.get_rlslp_dictionary().get_base_signature_rule_list();
				}
				return *this;
			}

			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Lightweight functions for accessing to properties of this class
			////////////////////////////////////////////////////////////////////////////////
			//@{
		public:
			const GrammarForLayeredRLSLP &get_grammar() const
			{
				return this->grammar;
			}
			const FastParentDictionary &get_parent_dictionary() const
			{
				return this->fastParentDictionary;
			}
			const RandomBitDictionary &get_random_bit_dictionary() const
			{
				return this->grammar.get_random_bit_dictionary();
			}
			const std::vector<RLSLPRuleBody> &get_base_signature_rule_list() const
			{
				return this->grammar.get_rlslp_dictionary().get_base_signature_rule_list();
			}
			const std::vector<uint64_t> &get_base_signature_length_list() const
			{
				return this->grammar.get_rlslp_dictionary().get_base_signature_length_list();
			}
			const std::vector<uint16_t> &get_base_signature_level_list() const
			{
				return this->grammar.get_rlslp_dictionary().get_base_signature_level_list();
			}
			const std::vector<uint16_t> &get_relative_max_level_list() const
			{
				return this->grammar.get_rlslp_dictionary().get_relative_max_level_list();
			}
			const DictionaryForLayeredRLSLP &get_dictionary() const
			{
				return this->grammar.get_rlslp_dictionary();
			}
			GrammarParsingType get_grammar_parsing_type() const
			{
				return this->grammar.get_grammar_parsing_type();
			}
			uint64_t get_distinct_document_count() const
			{
				return this->grammar.get_distinct_document_count();
			}
			uint64_t get_document_count() const
			{
				return this->grammar.get_document_count();
			}


			uint64_t base_signature_count() const
			{
				return this->grammar.get_rlslp_dictionary().base_signature_count();
			}

			/**
			 * @brief Return the number of distinct signatures in \p G
			 */
			uint64_t signature_count_without_null_signatures() const
			{
				return this->grammar.signature_count() - this->unused_signatures.size();
			}

			
			int64_t base_signature_count_without_null_signatures() const
			{
				return this->base_signature_count() - this->unused_signatures.size();
			}

			/**
			 * @brief Return \p true if |D| = 0. Otherwise, return \p false.
			 */
			bool is_empty() const
			{
				return this->base_signature_count() == 0;
			}


			/**
			 * @brief Returns the total memory usage in bytes
			 * @param only_dynamic_memory If true, only the size of the dynamic memory is returned
			 */
			uint64_t size_in_bytes([[maybe_unused]] bool only_dynamic_memory = false) const
			{
				uint64_t size_in_bytes = 0;
				uint64_t _capacity = this->grammar.get_rlslp_dictionary().get_base_signature_rule_list().capacity();
				size_in_bytes += sizeof(std::vector<RLSLPRuleBody>) + (sizeof(RLSLPRuleBody) * _capacity);
				size_in_bytes += sizeof(std::vector<uint8_t>) + (sizeof(unsigned char) * _capacity);
				size_in_bytes += sizeof(std::vector<uint64_t>) + (sizeof(uint64_t) * _capacity);

				uint64_t character_signature_item_map_size = this->character_signature_item_map.size();
				size_in_bytes += ((character_signature_item_map_size + 1) * (sizeof(std::pair<int64_t, SignatureWithRelativeLevel>) + 3 * sizeof(void *))) + sizeof(void *);

				size_in_bytes += this->fastParentDictionary.size_in_bytes();

				uint64_t doc_size = this->grammar.get_document_counter().size();
				size_in_bytes += ((doc_size + 1) * (sizeof(std::pair<SignatureWithRelativeLevel, uint64_t>) + 3 * sizeof(void *))) + sizeof(void *);

				uint64_t _capacity2 = this->unused_signatures.capacity();
				size_in_bytes += sizeof(std::vector<SignatureWithRelativeLevel>) + (sizeof(SignatureWithRelativeLevel) * _capacity2);

				return size_in_bytes;
			}

			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Main queries
			////////////////////////////////////////////////////////////////////////////////
			//@{

			std::vector<uint64_t> faster_get_all_occurrences(const std::vector<TemporaryOccurrence> &input) const
			{
				return NodeOccurrenceQuery::faster_get_all_occurrences(input, this->fastParentDictionary, this->grammar.get_rlslp_dictionary().get_base_signature_rule_list(), this->grammar.get_rlslp_dictionary().get_base_signature_length_list());
			}
			std::vector<uint64_t> faster_get_all_occurrences_using_memory(const std::vector<TemporaryOccurrence> &input, const std::vector<TemporaryOccurrence> *occCacheList) const
			{
				return NodeOccurrenceQuery::faster_get_all_occurrences_using_low_memory(input, this->fastParentDictionary,
																						this->grammar.get_rlslp_dictionary(), this->grammar.get_max_level(), occCacheList);
			}
			/**
			 * @brief Return the signature \p i of v_{i} -> c if such a nonterminal exists. Otherwise, return -1.
			 */
			int64_t get_character_signatuere(uint8_t c) const
			{
				auto f = this->character_signature_item_map.find(c);
				if (f == this->character_signature_item_map.end())
				{
					return -1;
				}
				else
				{
					return f->second;
				}
			}

			/**
			 * @brief Return the alphabet of the grammar \p G as a vector of characters
			 */
			std::vector<uint8_t> get_alphabet() const
			{
				std::vector<uint8_t> r;

				if (this->has_explicit_alphabet())
				{
					for (const auto &pair : this->character_id_map)
					{
						r.push_back(pair.first);
					}
				}
				else
				{
					for (auto &pair : this->character_signature_item_map)
					{
						r.push_back(pair.first);
					}
				}
				return r;
			}
			bool has_explicit_alphabet() const
			{
				return this->character_id_map.size() > 0;
			}
			uint64_t get_alphabet_bit_size() const
			{

				uint64_t alphabet_size = 0;
				if (this->has_explicit_alphabet())
				{
					alphabet_size = this->character_id_map.size();
				}
				else
				{
					alphabet_size = this->character_signature_item_map.size();
				}
				if (alphabet_size <= 2)
				{
					return 1;
				}
				else if (alphabet_size <= 4)
				{
					return 2;
				}
				else if (alphabet_size <= 16)
				{
					return 4;
				}
				else if (alphabet_size <= 256)
				{
					return 8;
				}
				else
				{
					return 64;
				}
			}
			const std::unordered_map<int64_t, uint64_t> &get_character_id_map() const
			{
				return this->character_id_map;
			}

			/**
			 * @brief Return the signature of the nonterminal \p v_{i} -> \p body if such a nonterminal exists. Otherwise, return -1.
			 */
			int64_t get_signature(const RLSLPRuleBody body) const
			{
				const std::vector<RLSLPRuleBody> &base_signature_rule_list = this->get_base_signature_rule_list();
				if (body.get_type() == RLSLPRuleType::Character)
				{
					int64_t c = body.A;
					auto f = this->character_signature_item_map.find(c);
					if (f == this->character_signature_item_map.end())
					{
						return -1;
					}
					else
					{
						return f->second;
					}
				}
				else if (body.get_type() == RLSLPRuleType::Pair)
				{
					int64_t searchResult = this->fastParentDictionary.get_pair_signature(body.A, body.B, base_signature_rule_list);
					return searchResult;
				}
				else if (body.get_type() == RLSLPRuleType::Power)
				{
					int64_t searchResult = this->fastParentDictionary.get_power_signature(body.A, body.B, base_signature_rule_list);
					return searchResult;
				}
				else if (body.get_type() == RLSLPRuleType::Signature)
				{
					return this->get_signature_single(body);
				}
				else
				{
					throw std::runtime_error("Error in get_signature: Unexpected rule type");
				}
			}

			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Update operations
			////////////////////////////////////////////////////////////////////////////////
			//@{
		public:
			void initialize(bool restricted_recompression = false, uint64_t seed = 0)
			{
				this->clear();
				this->grammar.initialize(restricted_recompression, seed);
				this->fastParentDictionary.set_pointer(&this->grammar.get_rlslp_dictionary().get_relative_max_level_list(), this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);
			}
			void initialize(bool restricted_recompression, const std::vector<uint8_t> &alphabet, uint64_t seed)
			{
				this->initialize(restricted_recompression, seed);
				std::vector<uint8_t> tmp = alphabet;
				std::sort(tmp.begin(), tmp.end());
				for (uint64_t i = 0; i < tmp.size(); i++)
				{
					this->character_id_map[tmp[i]] = i;
				}
			}

			/**
			 * @brief Clear the grammar \p G
			 */
			void clear()
			{
				this->grammar.clear();
				this->fastParentDictionary.clear();
				this->unused_signatures.clear();
				this->character_signature_item_map.clear();
				this->character_id_map.clear();
			}
			/**
			 * @brief Swap operation
			 */
			void swap(DynamicGrammarForLayeredRLSLP &other)
			{
				this->grammar.swap(other.grammar);
				this->unused_signatures.swap(other.unused_signatures);
				this->character_signature_item_map.swap(other.character_signature_item_map);
				this->fastParentDictionary.swap(other.fastParentDictionary);
				this->character_id_map.swap(other.character_id_map);

				this->fastParentDictionary.set_pointer(&this->grammar.get_rlslp_dictionary().get_relative_max_level_list(), this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);
				other.fastParentDictionary.set_pointer(&other.grammar.get_rlslp_dictionary().get_relative_max_level_list(), other.grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);
			}

			int64_t try_get_signature(RLSLPRuleBody body) const
			{
				NonterminalLessComparer::base_signature_rule_list = &this->grammar.get_rlslp_dictionary().get_base_signature_rule_list();

				if (body.get_type() == RLSLPRuleType::Power && body.B == 1)
				{
					throw std::runtime_error("get_or_add_signature: body.get_type() == RLSLPRuleType::Power && body.B == 1");
				}

				if (body.get_type() == RLSLPRuleType::Signature)
				{
					int64_t child_sig = body.A;
					int64_t base_sig = SignatureFunctions::get_base_signature(child_sig);
					int64_t child_level = SignatureFunctions::get_relative_level(child_sig);
					int64_t body_signature = SignatureFunctions::get_signature(child_level + 1, base_sig);
					bool is_new_signature = child_level >= this->grammar.get_rlslp_dictionary().get_relative_max_level_list()[base_sig];
					if (is_new_signature)
					{
						return -1;
					}
					else
					{
						return body_signature;
					}
				}
				else
				{
					auto index = this->get_signature(body);
					return index;
				}
			}

			/**
			 * @brief Return the signature \p i of the rule v_{i} -> \p body. If such a nonterminal does not exist, add it to the grammar \p G and execute the callback function \p callback_for_added_signature.
			 */
			template <typename CALLBACK>
			SignatureWithRelativeLevel get_or_add_signature(RLSLPRuleBody body, uint16_t new_level, CALLBACK &callback_for_added_signature)
			{
				NonterminalLessComparer::base_signature_rule_list = &this->get_base_signature_rule_list();

				SignatureWithRelativeLevel registedSignature = -1;

				if (body.get_type() == RLSLPRuleType::Power && body.B == 1)
				{
					throw std::runtime_error("get_or_add_signature: body.get_type() == RLSLPRuleType::Power && body.B == 1");
				}

				if (body.get_type() == RLSLPRuleType::Signature)
				{
					int64_t child_sig = body.A;
					int64_t base_sig = SignatureFunctions::get_base_signature(child_sig);
					int64_t child_level = SignatureFunctions::get_relative_level(child_sig);
					int64_t body_signature = SignatureFunctions::get_signature(child_level + 1, base_sig);
					bool is_new_signature = child_level >= this->get_relative_max_level_list()[base_sig];
					if (is_new_signature)
					{
						this->insert_new_item_into_list(body_signature, body, new_level);
					}
					registedSignature = body_signature;
				}
				else
				{
					auto index = this->get_signature(body);
					if (index == -1)
					{
						auto newNumber = this->get_new_number();
						this->insert_new_item_into_list(newNumber, body, new_level);
						registedSignature = newNumber;
						callback_for_added_signature(newNumber);
					}
					else
					{
						registedSignature = index;
					}
				}

				return registedSignature;
			}

			/**
			 * @brief Add a new document to the grammar \p G, where the derivation tree of the document is rooted at the nonterminal \p v_{i}
			 */
			void add_document(SignatureWithRelativeLevel i)
			{
				this->grammar.add_document(i);
			}
			/**
			 * @brief If \p G contains a document rooted at the nonterminal \p v_{i}, remove it from \p G and execute the callback function \p preprocessor. Otherwise, throw an error.
			 */
			template <typename PREPROCESSOR = decltype(no_callback)>
			bool remove_document(SignatureWithRelativeLevel i, PREPROCESSOR &preprocessor)
			{
				bool b = this->grammar.remove_document(i);
				if (b)
				{
					__remove_document_sub(i, GrammarForLayeredRLSLP::DOCUMENT_MARK, preprocessor);
				}
				return b;
			}

			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Print and verification functions
			////////////////////////////////////////////////////////////////////////////////
			//@{

			void print_statistics(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
			{

				std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicGrammarForLayeredRLSLP):" << std::endl;
				this->grammar.print_statistics(message_paragraph + 1);

				if (this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression)
				{
					this->grammar.get_random_bit_dictionary().print_statistics(message_paragraph + 1);
				}

				this->fastParentDictionary.print_statistics(message_paragraph + 1);
			}

			/**
			 * @brief Print the information of the grammar \p G
			 */
			void print_info(bool print_parents = false, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
			{
				this->grammar.print_info(message_paragraph);

				if (print_parents)
				{
					this->fastParentDictionary.print_tree(message_paragraph + 1);
				}
			}
			void print_derivation_tree(SignatureWithRelativeLevel sig, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
			{
				const DictionaryForLayeredRLSLP &rlslp_dictionary = this->grammar.get_rlslp_dictionary();
				RunRuleVector item1(sig, rlslp_dictionary);
				item1.print_derivation_tree(3, message_paragraph);
			}

			/**
			 * @brief Return the memory usage information of this grammar as a vector of strings
			 * @param message_paragraph The paragraph depth of message logs
			 */
			std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
			{

				std::vector<std::string> r;
				uint64_t size_in_bytes = this->size_in_bytes();
				uint64_t signature_count = this->signature_count_without_null_signatures();
				uint64_t unused_signature_count = this->unused_signatures.size();

				double bits_per_element = signature_count > 0 ? ((double)size_in_bytes / (double)signature_count) : 0;

				r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicGrammarForLayeredRLSLP: " + std::to_string(this->size_in_bytes()) + " bytes, " + std::to_string(signature_count) + " signatures, " + std::to_string(unused_signature_count) + " unused signatures, " + std::to_string(bits_per_element) + " bytes per signature =");

				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "base_signature_rule_list: \t\t\t" + std::to_string(this->get_base_signature_rule_list().size() * sizeof(RLSLPRuleBody)) + " bytes" + " (" + std::to_string(sizeof(RLSLPRuleBody)) + " bytes per signature)");
				// r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "heightList: \t\t\t" + std::to_string(this->heightList.size() * sizeof(unsigned char)) + " bytes" + " (" + std::to_string(sizeof(unsigned char)) + " bytes per signature)");
				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "base_signature_length_list_: \t\t\t" + std::to_string(this->get_base_signature_length_list().size() * sizeof(uint64_t)) + " bytes" + " (" + std::to_string(sizeof(uint64_t)) + " bytes per signature)");

				uint64_t character_signature_item_map_byte_size = ((this->character_signature_item_map.size() + 1) * (sizeof(std::pair<int64_t, SignatureWithRelativeLevel>) + 3 * sizeof(void *))) + sizeof(void *);
				uint64_t bits_per_character_signature_item_map = this->character_signature_item_map.size() > 0 ? ((double)character_signature_item_map_byte_size / (double)this->character_signature_item_map.size()) : 0;
				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "character_signature_item_map: \t" + std::to_string(character_signature_item_map_byte_size) + " bytes" + " (" + std::to_string(bits_per_character_signature_item_map) + " bytes per character)");

				auto r2 = this->fastParentDictionary.get_memory_usage_info(signature_count, message_paragraph + 1);
				for (auto s : r2)
				{
					r.push_back(s);
				}

				uint64_t documentCounter_byte_size = 0;
				documentCounter_byte_size += ((this->grammar.get_document_counter().size() + 1) * (sizeof(std::pair<SignatureWithRelativeLevel, uint64_t>) + 3 * sizeof(void *))) + sizeof(void *);
				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "documentCounter: \t\t\t" + std::to_string(documentCounter_byte_size) + " bytes");

				uint64_t unused_signatures_byte_size = 0;
				unused_signatures_byte_size += sizeof(std::vector<SignatureWithRelativeLevel>) + (sizeof(SignatureWithRelativeLevel) * this->unused_signatures.size());
				uint64_t bits_per_unused_signatures = this->unused_signatures.size() > 0 ? ((double)unused_signatures_byte_size / (double)this->unused_signatures.size()) : 0;
				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "unused_signatures: \t\t" + std::to_string(unused_signatures_byte_size) + " bytes" + " (" + std::to_string(bits_per_unused_signatures) + " bytes per unused signature)");

				r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");

				return r;
			}
			bool verify() const
			{
				if (!this->fastParentDictionary.verify())
				{
					throw std::runtime_error("Error in verify: The parent dictionary must be valid.");
				}
				return true;
			}
			/**
			 * @brief Verify if the grammar \p G is nearly equal to the grammar \p other
			 */
			bool verify_nearly_equal(const DynamicGrammarForLayeredRLSLP &other) const
			{
				/*
				Checklist
				*/
				// Code 0: GrammarParsingType grammarParsingType = GrammarParsingType::SignatureEncoding;
				// Code 4: std::map<int64_t, SignatureWithRelativeLevel> character_signature_item_map;
				// Code 5: std::vector<std::vector<SignatureWithRelativeLevel>> parentVectorList;
				// Code 5: std::unordered_map<SignatureWithRelativeLevel, std::set<SignatureWithRelativeLevel, NonterminalLessComparer>> parentMap;
				// Code 6: std::unordered_map<SignatureWithRelativeLevel, uint64_t> documentCounter;
				// Code 7: std::vector<SignatureWithRelativeLevel> unused_signatures;
				// Code 8: std::vector<uint8_t> randomBitList;

				// Code 0

				if (!this->grammar.verify_nearly_equal(other.grammar))

					// Code 4
					if (this->character_signature_item_map.size() != other.character_signature_item_map.size())
					{
						throw std::runtime_error("Error in verify_nearly_equal: The size of character signature item map must be equal.");
					}
				std::vector<std::pair<int64_t, SignatureWithRelativeLevel>> character_signatuere_item_vec1 = this->create_character_signatuere_item_vec();
				std::vector<std::pair<int64_t, SignatureWithRelativeLevel>> character_signatuere_item_vec2 = other.create_character_signatuere_item_vec();

				for (uint64_t i = 0; i < character_signatuere_item_vec1.size(); i++)
				{
					if (character_signatuere_item_vec1[i].first != character_signatuere_item_vec2[i].first || character_signatuere_item_vec1[i].second != character_signatuere_item_vec2[i].second)
					{
						throw std::runtime_error("Error in verify_nearly_equal: The character signature item map must be equal.");
					}
				}

				// Code 5
				/*
				if (!this->parentDictionary.verify_equal(other.parentDictionary))
				{
					throw std::runtime_error("Error in verify_nearly_equal: The parent dictionary must be equal.");
				}
				*/
				if (!this->fastParentDictionary.verify_equal(other.fastParentDictionary))
				{
					throw std::runtime_error("Error in verify_nearly_equal: The fast parent dictionary must be equal.");
				}

				// Code 7
				if (this->unused_signatures.size() != other.unused_signatures.size())
				{
					throw std::runtime_error("Error in verify_nearly_equal: The size of unused signatures must be equal.");
				}
				for (uint64_t i = 0; i < this->unused_signatures.size(); i++)
				{
					if (this->unused_signatures[i] != other.unused_signatures[i])
					{
						throw std::runtime_error("Error in verify_nearly_equal: The unused signatures must be equal.");
					}
				}

				return true;
			}
			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Load, save, and builder functions
			////////////////////////////////////////////////////////////////////////////////
			//@{
		public:
			/**
			 * @brief Returns the DynamicGrammarForLayeredRLSLP instance loaded from a file stream \p ifs
			 */
			static DynamicGrammarForLayeredRLSLP load_from_file(std::ifstream &ifs)
			{
				/*
				Checklist
				*/
				// Code 0: GrammarParsingType grammarParsingType = GrammarParsingType::SignatureEncoding;
				// Code 2: std::map<int64_t, SignatureWithRelativeLevel> character_signature_item_map;
				// Code 3: std::vector<std::vector<SignatureWithRelativeLevel>> parentVectorList;
				// Code 3: std::unordered_map<SignatureWithRelativeLevel, std::set<SignatureWithRelativeLevel, NonterminalLessComparer>> parentMap;
				// Code 4: std::unordered_map<SignatureWithRelativeLevel, uint64_t> documentCounter;
				// Code 5: std::vector<SignatureWithRelativeLevel> unused_signatures;
				// Code 6: std::vector<uint8_t> randomBitList;

				uint64_t fingerprint;
				ifs.read(reinterpret_cast<char *>(&fingerprint), sizeof(uint64_t));
				if (fingerprint != FINGERPRINT)
				{
					throw std::runtime_error("DynamicGrammarForLayeredRLSLP::load_from_file: Fingerprint mismatch");
				}

				DynamicGrammarForLayeredRLSLP r;

				GrammarForLayeredRLSLP grammar = GrammarForLayeredRLSLP::load_from_file(ifs);
				r.grammar.swap(grammar);

				// Code 2
				uint64_t _character_signature_item_map_size;
				ifs.read(reinterpret_cast<char *>(&_character_signature_item_map_size), sizeof(_character_signature_item_map_size));
				{
					std::vector<std::pair<int64_t, SignatureWithRelativeLevel>> character_signatuere_item_vec;
					character_signatuere_item_vec.resize(_character_signature_item_map_size);
					ifs.read(reinterpret_cast<char *>(character_signatuere_item_vec.data()), sizeof(std::pair<int64_t, SignatureWithRelativeLevel>) * _character_signature_item_map_size);

					std::map<int64_t, SignatureWithRelativeLevel> _character_signature_item_map(character_signatuere_item_vec.begin(), character_signatuere_item_vec.end());
					r.character_signature_item_map.swap(_character_signature_item_map);
				}

				// Code 3
				// r.parentDictionary = ParentDictionary::load_from_file(&r.get_relative_max_level_list(), ifs);
				r.fastParentDictionary = FastParentDictionary::load_from_file(&r.grammar.get_rlslp_dictionary().get_relative_max_level_list(), ifs);

				// Code 5
				uint64_t _unused_signatures_size;
				ifs.read(reinterpret_cast<char *>(&_unused_signatures_size), sizeof(_unused_signatures_size));
				r.unused_signatures.resize(_unused_signatures_size);
				ifs.read(reinterpret_cast<char *>(r.unused_signatures.data()), sizeof(SignatureWithRelativeLevel) * _unused_signatures_size);

				// Initialize decoders and parentDictionary
				// r.levelDecoder = LevelDecoder(&r.small_dic.get_base_signature_level_list());
				const std::vector<RLSLPRuleBody> &base_signature_rule_list = r.grammar.get_rlslp_dictionary().get_base_signature_rule_list();
				NonterminalLessComparer::base_signature_rule_list = &base_signature_rule_list;

				// Code 8
				uint64_t character_id_map_size;
				ifs.read(reinterpret_cast<char *>(&character_id_map_size), sizeof(uint64_t));
				for (uint64_t i = 0; i < character_id_map_size; i++)
				{
					int64_t character_id;
					ifs.read(reinterpret_cast<char *>(&character_id), sizeof(int64_t));
					int64_t signature_id;
					ifs.read(reinterpret_cast<char *>(&signature_id), sizeof(int64_t));
					r.character_id_map[character_id] = signature_id;
				}

				return r;
			}
			/**
			 * @brief Save the given instance \p item to a file stream \p os
			 */
			static void store_to_file(const DynamicGrammarForLayeredRLSLP &item, std::ofstream &os)
			{
				/*
				Checklist
				*/
				// Code 0: GrammarParsingType grammarParsingType = GrammarParsingType::SignatureEncoding;
				// Code 2: std::map<int64_t, SignatureWithRelativeLevel> character_signature_item_map;
				// Code 3: std::vector<std::vector<SignatureWithRelativeLevel>> parentVectorList;
				// Code 3: std::unordered_map<SignatureWithRelativeLevel, std::set<SignatureWithRelativeLevel, NonterminalLessComparer>> parentMap;
				// Code 4: std::unordered_map<SignatureWithRelativeLevel, uint64_t> documentCounter;
				// Code 5: std::vector<SignatureWithRelativeLevel> unused_signatures;
				// Code 6: std::vector<uint8_t> randomBitList;

				os.write(reinterpret_cast<const char *>(&FINGERPRINT), sizeof(uint64_t));
				GrammarForLayeredRLSLP::store_to_file(item.grammar, os);

				// Code 2
				uint64_t character_signature_item_map_size = item.character_signature_item_map.size();
				os.write(reinterpret_cast<const char *>(&character_signature_item_map_size), sizeof(uint64_t));
				{
					std::vector<std::pair<int64_t, SignatureWithRelativeLevel>> character_signatuere_item_vec = item.create_character_signatuere_item_vec();
					os.write(reinterpret_cast<const char *>(character_signatuere_item_vec.data()), sizeof(std::pair<int64_t, SignatureWithRelativeLevel>) * character_signature_item_map_size);
				}

				// Code 3
				// ParentDictionary::store_to_file(item.parentDictionary, os);
				FastParentDictionary::store_to_file(item.fastParentDictionary, os);

				// Code 5
				uint64_t unused_signatures_size = item.unused_signatures.size();
				os.write(reinterpret_cast<const char *>(&unused_signatures_size), sizeof(uint64_t));
				os.write(reinterpret_cast<const char *>(item.unused_signatures.data()), sizeof(SignatureWithRelativeLevel) * unused_signatures_size);

				// Code 8
				uint64_t character_id_map_size = item.character_id_map.size();
				os.write(reinterpret_cast<const char *>(&character_id_map_size), sizeof(uint64_t));
				for (const auto &pair : item.character_id_map)
				{
					os.write(reinterpret_cast<const char *>(&pair.first), sizeof(int64_t));
					os.write(reinterpret_cast<const char *>(&pair.second), sizeof(int64_t));
				}
			}

			static DynamicGrammarForLayeredRLSLP build_from_leveled_rlslp(std::ifstream &ifs)
			{
				DynamicGrammarForLayeredRLSLP r;

				GrammarForLayeredRLSLP grammar = GrammarForLayeredRLSLP::load_from_file(ifs);
				r.grammar.swap(grammar);

				const DictionaryForLayeredRLSLP &rlslp_dictionary = r.grammar.get_rlslp_dictionary();

				const std::vector<RLSLPRuleBody> &base_signature_rule_list = rlslp_dictionary.get_base_signature_rule_list();
				r.fastParentDictionary.set_pointer(&rlslp_dictionary.get_relative_max_level_list(), r.grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedBlockCompression);

				for (uint64_t i = 0; i < rlslp_dictionary.base_signature_count(); i++)
				{
					r.fastParentDictionary.add_new_element();
				}

				for (uint64_t i = 0; i < rlslp_dictionary.base_signature_count(); i++)
				{
					RLSLPRuleBody rule = RLSLPRuleBody::decodeRule(i, base_signature_rule_list);
					if (rule.get_type() == RLSLPRuleType::Character)
					{
						r.character_signature_item_map[rule.A] = i;
					}
					else if (rule.get_type() == RLSLPRuleType::Pair)
					{
						uint64_t base_A_sig = SignatureFunctions::get_base_signature(rule.A);
						uint64_t diff_level_A = SignatureFunctions::get_relative_level(rule.A);
						bool is_top_level_A = diff_level_A == rlslp_dictionary.get_relative_max_level_list()[base_A_sig];
						uint64_t base_B_sig = SignatureFunctions::get_base_signature(rule.B);
						uint64_t diff_level_B = SignatureFunctions::get_relative_level(rule.B);
						bool is_top_level_B = diff_level_B == rlslp_dictionary.get_relative_max_level_list()[base_B_sig];

						r.fastParentDictionary.insert(rule.A, i, is_top_level_A, base_signature_rule_list);
						r.fastParentDictionary.insert(rule.B, i, is_top_level_B, base_signature_rule_list);
					}
					else if (rule.get_type() == RLSLPRuleType::Power)
					{
						uint64_t base_A_sig = SignatureFunctions::get_base_signature(rule.A);
						uint64_t diff_level_A = SignatureFunctions::get_relative_level(rule.A);
						bool is_top_level_A = diff_level_A == rlslp_dictionary.get_relative_max_level_list()[base_A_sig];
						r.fastParentDictionary.insert(rule.A, i, is_top_level_A, base_signature_rule_list);
					}
					else if (rule.get_type() == RLSLPRuleType::Null)
					{
						r.unused_signatures.push_back(i);
					}
					else
					{
						throw std::runtime_error("build_from_leveled_rlslp: unknown rule type");
					}
				}

				std::vector<uint8_t> alphabet = r.get_alphabet();
				std::sort(alphabet.begin(), alphabet.end());
				for (uint64_t i = 0; i < alphabet.size(); i++)
				{
					r.character_id_map[alphabet[i]] = i;
				}

				return r;
			}

			//}@

		private:
			////////////////////////////////////////////////////////////////////////////////
			///   @name Private Functions for Basic Functions
			////////////////////////////////////////////////////////////////////////////////
			//@{

			int64_t get_new_number()
			{
				NonterminalLessComparer::base_signature_rule_list = &this->get_base_signature_rule_list();
				// const DictionaryForLayeredRLSLP &rlslp_dictionary = this->grammar.get_rlslp_dictionary();

				int64_t new_number = -1;
				uint64_t unused_signature_count = this->unused_signatures.size();
				if (unused_signature_count > 0)
				{
					new_number = this->unused_signatures[unused_signature_count - 1];
					this->unused_signatures.pop_back();
				}
				else
				{
					this->fastParentDictionary.add_new_element();
					new_number = this->grammar.add_new_base_signature();
				}
				return new_number;
			}
			void insert_new_item_into_list(SignatureWithRelativeLevel new_signature, RLSLPRuleBody new_item, uint16_t new_level)
			{

				uint64_t base_signature = SignatureFunctions::get_base_signature(new_signature);
				uint64_t diff = SignatureFunctions::get_relative_level(new_signature);
				const std::vector<RLSLPRuleBody> &base_signature_rule_list = this->get_base_signature_rule_list();

				if (diff == 0)
				{

					this->grammar.write_element(base_signature, new_item, new_level, 0, this->get_base_signature_length_list());

					if (new_item.get_type() == RLSLPRuleType::Pair)
					{

						uint64_t base_A_sig = SignatureFunctions::get_base_signature(new_item.A);
						uint64_t level_A = SignatureFunctions::get_relative_level(new_item.A);
						bool is_top_level_A = level_A == this->get_relative_max_level_list()[base_A_sig];
						uint64_t base_B_sig = SignatureFunctions::get_base_signature(new_item.B);
						uint64_t level_B = SignatureFunctions::get_relative_level(new_item.B);
						bool is_top_level_B = level_B == this->get_relative_max_level_list()[base_B_sig];

						this->fastParentDictionary.insert(new_item.A, new_signature, is_top_level_A, base_signature_rule_list);
						this->fastParentDictionary.insert(new_item.B, new_signature, is_top_level_B, base_signature_rule_list);
					}
					else if (new_item.get_type() == RLSLPRuleType::Power)
					{
						uint64_t base_A_sig = SignatureFunctions::get_base_signature(new_item.A);
						uint64_t level_A = SignatureFunctions::get_relative_level(new_item.A);
						bool is_top_level_A = level_A == this->get_relative_max_level_list()[base_A_sig];

						this->fastParentDictionary.insert(new_item.A, new_signature, is_top_level_A, base_signature_rule_list);
					}
					else if (new_item.get_type() == RLSLPRuleType::Character)
					{
						int64_t c = new_item.A;

#ifdef DEBUG
						if (this->character_signature_item_map.find(c) != this->character_signature_item_map.end())
						{
							std::cout << "Character already exists: " << c << " " << this->character_signature_item_map[c] << std::endl;
							assert(false);
						}
#endif
						// uint64_t current_bit_size = this->get_alphabet_bit_size();
						this->character_signature_item_map[c] = new_signature;
					}
					else
					{
						throw std::runtime_error("insert_new_item_into_list: unknown rule type");
					}
				}
				else
				{

					assert(new_level - this->get_base_signature_level_list()[base_signature] == diff);
					this->fastParentDictionary.insert_single_signature(base_signature, base_signature_rule_list);
					this->grammar.increase_relative_max_level(base_signature);
				}

				this->grammar.create_random_bit(base_signature);
			}

			//}@

		private:
			std::vector<std::pair<int64_t, SignatureWithRelativeLevel>> create_character_signatuere_item_vec() const
			{

				uint64_t character_signature_item_map_size = this->character_signature_item_map.size();
				std::vector<std::pair<int64_t, SignatureWithRelativeLevel>> character_signatuere_item_vec;
				character_signatuere_item_vec.resize(character_signature_item_map_size);

				uint64_t counter = 0;
				for (auto it : this->character_signature_item_map)
				{
					character_signatuere_item_vec[counter] = {it.first, it.second};
					counter++;
				}
				return character_signatuere_item_vec;
			}

			template <typename PREPROCESSOR = decltype(no_callback)>
			void __remove_document_sub(SignatureWithRelativeLevel removed_node_signature, [[maybe_unused]] uint64_t parent_signature, PREPROCESSOR &preprocessor = no_callback)
			{
				NonterminalLessComparer::base_signature_rule_list = &this->get_base_signature_rule_list();
				const std::vector<RLSLPRuleBody> &base_signature_rule_list = this->get_base_signature_rule_list();
				const std::unordered_map<SignatureWithRelativeLevel, uint64_t> &document_counter = this->grammar.get_document_counter();

				assert(RLSLPRuleBody::decodeRule(removed_node_signature, base_signature_rule_list).get_type() != RLSLPRuleType::Null);

				auto f = document_counter.find(removed_node_signature);

				bool has_parent = this->fastParentDictionary.has_parent(removed_node_signature);

				uint64_t base_signature = SignatureFunctions::get_base_signature(removed_node_signature);
				uint64_t level = SignatureFunctions::get_relative_level(removed_node_signature);

				if (!has_parent && f == document_counter.end())
				{
					preprocessor(removed_node_signature);
					this->grammar.erase_random_bit(removed_node_signature);

					assert(level == this->get_relative_max_level_list()[base_signature]);

					RLSLPRuleBody removed_item = RLSLPRuleBody::decodeRule(removed_node_signature, base_signature_rule_list);

					if (removed_item.get_type() == RLSLPRuleType::Pair)
					{

						assert(RLSLPRuleBody::decodeRule(removed_item.A, base_signature_rule_list).get_type() != RLSLPRuleType::Null);
						this->fastParentDictionary.erase_signature(removed_item.A, removed_node_signature, base_signature_rule_list);

						__remove_document_sub(removed_item.A, removed_node_signature, preprocessor);

						RLSLPRuleBody right_item = RLSLPRuleBody::decodeRule(removed_item.B, base_signature_rule_list);
						if (right_item.get_type() != RLSLPRuleType::Null)
						{
							this->fastParentDictionary.erase_signature(removed_item.B, removed_node_signature, base_signature_rule_list);
							__remove_document_sub(removed_item.B, removed_node_signature, preprocessor);
						}
					}
					else if (removed_item.get_type() == RLSLPRuleType::Power)
					{
						this->fastParentDictionary.erase_signature(removed_item.A, removed_node_signature, base_signature_rule_list);
						__remove_document_sub(removed_item.A, removed_node_signature, preprocessor);
					}
					else if (removed_item.get_type() == RLSLPRuleType::Signature)
					{
						this->fastParentDictionary.erase_signature(base_signature, base_signature_rule_list);
						this->grammar.decrease_relative_max_level(base_signature);
						__remove_document_sub(removed_item.A, removed_node_signature, preprocessor);
					}
					else if (removed_item.get_type() == RLSLPRuleType::Character)
					{
						this->character_signature_item_map.erase(removed_item.A);
					}
					else if (removed_item.get_type() == RLSLPRuleType::Null)
					{
						throw std::runtime_error("remove_document_sub: removed_item.get_type() == RLSLPRuleType::Null");
					}

					if (level == 0)
					{
						this->grammar.clear_element(base_signature);
						this->unused_signatures.push_back(base_signature);
					}
				}
			}

			int64_t get_signature_single(RLSLPRuleBody key) const
			{
				assert(key.get_type() == RLSLPRuleType::Signature);
				uint64_t base_sig = SignatureFunctions::get_base_signature(key.A);
				uint64_t child_level = SignatureFunctions::get_relative_level(key.A);
				if (child_level < this->get_relative_max_level_list()[base_sig])
				{
					return SignatureFunctions::get_signature(child_level + 1, base_sig);
				}
				else
				{
					return -1;
				}
			}
		};
	
}