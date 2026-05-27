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
		 * @li \p std::vector<NonterminalWithRelativeLevel> \p Z[0..g-m-1]: This vector stores unused nonterminals (identifiers) in any order.
		 * @li \p std::vector<bool> \p B[0..m-1]: \p B[i] stores a random bit assigned to the nonterminal \p v_{i} if it exists.
		 * @li \p std::vector<std::vector<NonterminalWithRelativeLevel> \p F: \p F[i] stores the list \p P_{i} of parents of the nonterminal \p v_{i} in the DAG correspnding to the grammar \p G if \p v_{i} exists and the length of the list is at most \p PARENT_NUMBER_THRESHOLD.
		 * @li \p std::map<int64_t, \p NonterminalWithRelativeLevel> \p M_{char}: \p M_{char}[c] stores the nonterminal deriving the character \p c.
		 * @li \p std::unordered_map<NonterminalWithRelativeLevel, \p std::set<NonterminalWithRelativeLevel, \p NonterminalLessComparer>> \p M_{pair}: \p M_{pair}[i] stores the list \p P_{i} of parents of the nonterminal \p v_{i} if the list is longer than \p PARENT_NUMBER_THRESHOLD.
		 * @li \p std::unordered_map<NonterminalWithRelativeLevel, \p uint64_t> \p M_{doc}: \p M_{doc}[i] stores the number of derivation trees rooted at the nonterminal \p v_{i}
		 * @ingroup DynamicDictionaryClasses
		 * @note @li \p v_{i} is the nonterminal with the nonterminal (identifier) \p i in {0, 1, ..., m-1}. \p v_{i} does not exist if \p G does not contain the nonterminal.
		 * @note @li \p m >= g and m = Ω(g)
		 */
		class DynamicGrammarForLayeredRLSLP
		{

		private:
			GrammarForLayeredRLSLP grammar;
			FastParentDictionary fastParentDictionary;
			std::vector<NonterminalWithRelativeLevel> unused_nonterminals;					// Z
			std::map<int64_t, NonterminalWithRelativeLevel> character_nonterminal_item_map; // M_{char}
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
				NonterminalLessComparer::explicit_nonterminal_rule_list = &this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_rule_list();

				this->clear();
			}
			/**
			 * @brief Default destructor.
			 */
			~DynamicGrammarForLayeredRLSLP()
			{
				NonterminalLessComparer::explicit_nonterminal_rule_list = nullptr;
			}
			/**
			 * @brief Deleted copy constructor.
			 */
			DynamicGrammarForLayeredRLSLP(const DynamicGrammarForLayeredRLSLP &) = delete;
			/**
			 * @brief Move constructor.
			 * @param other Source grammar to move from.
			 */
			DynamicGrammarForLayeredRLSLP(DynamicGrammarForLayeredRLSLP &&other) noexcept : grammar(std::move(other.grammar)),
																							fastParentDictionary(std::move(other.fastParentDictionary)),
																							unused_nonterminals(std::move(other.unused_nonterminals)),
																							character_nonterminal_item_map(std::move(other.character_nonterminal_item_map)),
																							character_id_map(std::move(other.character_id_map))
			{
				// this->parentDictionary.set_pointer(&this->relative_max_level_list_);
				this->fastParentDictionary.set_pointer(&this->grammar.get_rlslp_dictionary().get_relative_max_level_list(), this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression);
				NonterminalLessComparer::explicit_nonterminal_rule_list = &this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_rule_list();
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
			 * @param other Source grammar to move from.
			 * @return Reference to this grammar after the move.
			 */
			DynamicGrammarForLayeredRLSLP &operator=(DynamicGrammarForLayeredRLSLP &&other) noexcept
			{
				if (this != &other)
				{
					this->grammar = std::move(other.grammar);
					this->unused_nonterminals = std::move(other.unused_nonterminals);
					this->character_nonterminal_item_map = std::move(other.character_nonterminal_item_map);
					this->fastParentDictionary = std::move(other.fastParentDictionary);
					this->character_id_map = std::move(other.character_id_map);

					this->fastParentDictionary.set_pointer(&this->grammar.get_rlslp_dictionary().get_relative_max_level_list(), this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression);
					NonterminalLessComparer::explicit_nonterminal_rule_list = &this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_rule_list();
				}
				return *this;
			}

			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Lightweight functions for accessing to properties of this class
			////////////////////////////////////////////////////////////////////////////////
			//@{
		public:
			/**
			 * @brief Returns the underlying static layered RLSLP grammar.
			 * @return Const reference to the grammar object.
			 */
			const GrammarForLayeredRLSLP &get_grammar() const
			{
				return this->grammar;
			}
			/**
			 * @brief Returns the fast parent dictionary of this dynamic grammar.
			 * @return Const reference to the parent dictionary.
			 */
			const FastParentDictionary &get_parent_dictionary() const
			{
				return this->fastParentDictionary;
			}
			/**
			 * @brief Returns the random-bit dictionary used for restricted recompression.
			 * @return Const reference to the random bit dictionary.
			 */
			const RandomBitDictionary &get_random_bit_dictionary() const
			{
				return this->grammar.get_random_bit_dictionary();
			}
			/**
			 * @brief Returns the list of rule bodies indexed by base nonterminal.
			 * @return Const reference to the base nonterminal rule list.
			 */
			const std::vector<RLSLPRuleBody> &get_explicit_nonterminal_rule_list() const
			{
				return this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_rule_list();
			}
			/**
			 * @brief Returns derived string lengths indexed by base nonterminal.
			 * @return Const reference to the length list.
			 */
			const std::vector<uint64_t> &get_explicit_nonterminal_length_list() const
			{
				return this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_length_list();
			}
			/**
			 * @brief Returns height levels indexed by base nonterminal.
			 * @return Const reference to the level list.
			 */
			const std::vector<uint16_t> &get_explicit_nonterminal_level_list() const
			{
				return this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_level_list();
			}
			/**
			 * @brief Returns the maximum relative level per base nonterminal.
			 * @return Const reference to the relative max-level list.
			 */
			const std::vector<uint16_t> &get_relative_max_level_list() const
			{
				return this->grammar.get_rlslp_dictionary().get_relative_max_level_list();
			}
			/**
			 * @brief Returns the layered RLSLP dictionary embedded in the grammar.
			 * @return Const reference to the dictionary.
			 */
			const DictionaryForLayeredRLSLP &get_dictionary() const
			{
				return this->grammar.get_rlslp_dictionary();
			}
			/**
			 * @brief Returns the grammar parsing mode.
			 * @return Grammar parsing type flag.
			 */
			GrammarParsingType get_grammar_parsing_type() const
			{
				return this->grammar.get_grammar_parsing_type();
			}
			/**
			 * @brief Returns the number of distinct derivation trees (documents) stored.
			 * @return Distinct document count.
			 */
			uint64_t get_distinct_document_count() const
			{
				return this->grammar.get_distinct_document_count();
			}
			/**
			 * @brief Returns the total number of document references including duplicates.
			 * @return Total document count.
			 */
			uint64_t get_document_count() const
			{
				return this->grammar.get_document_count();
			}

			/**
			 * @brief Returns the number of base nonterminal slots allocated in the dictionary.
			 * @return Base nonterminal count including unused slots.
			 */
			uint64_t count_explicit_nonterminals() const
			{
				return this->grammar.get_rlslp_dictionary().count_explicit_nonterminals();
			}

			/**
			 * @brief Return the number of distinct nonterminals in \p G
			 */
			uint64_t nonterminal_count_without_null_nonterminals() const
			{
				return this->grammar.nonterminal_count() - this->unused_nonterminals.size();
			}


			/**
			 * @brief Return \p true if |D| = 0. Otherwise, return \p false.
			 */
			bool is_empty() const
			{
				return this->count_explicit_nonterminals() == 0;
			}


			/**
			 * @brief Returns the total memory usage in bytes
			 * @param only_dynamic_memory If true, only the size of the dynamic memory is returned
			 */
			uint64_t size_in_bytes([[maybe_unused]] bool only_dynamic_memory = false) const
			{
				/*
				GrammarForLayeredRLSLP grammar;
				FastParentDictionary fastParentDictionary;
				std::vector<NonterminalWithRelativeLevel> unused_nonterminals;					// Z
				std::map<int64_t, NonterminalWithRelativeLevel> character_nonterminal_item_map; // M_{char}
				std::unordered_map<int64_t, uint64_t> character_id_map;
				*/

				uint64_t total_size = 0;
				total_size += this->grammar.size_in_bytes();
				total_size += this->fastParentDictionary.size_in_bytes();
				total_size += stool::Memory::estimate_memory_usage(this->unused_nonterminals);
				total_size += stool::Memory::estimate_memory_usage(this->character_nonterminal_item_map);
				total_size += stool::Memory::estimate_memory_usage(this->character_id_map);
				return total_size;
	
			}

			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Main queries
			////////////////////////////////////////////////////////////////////////////////
			//@{

			/**
			 * @brief Enumerates all leaf occurrence positions for the given seed occurrences.
			 * @param input Seed temporary occurrences to start enumeration from.
			 * @return All absolute occurrence positions.
			 */
			std::vector<uint64_t> faster_get_all_occurrences(const std::vector<TemporaryOccurrence> &input) const
			{
				return NodeOccurrenceQuery::faster_get_all_occurrences(input, this->fastParentDictionary, this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_rule_list(), this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_length_list());
			}
			/**
			 * @brief Enumerates occurrences using the low-memory implicit-tree algorithm.
			 * @param input Seed temporary occurrences to start enumeration from.
			 * @param occCacheList Optional per-base-nonterminal occurrence cache; nullptr disables the cache.
			 * @return All absolute occurrence positions.
			 */
			std::vector<uint64_t> faster_get_all_occurrences_using_memory(const std::vector<TemporaryOccurrence> &input, const std::vector<TemporaryOccurrence> *occCacheList) const
			{
				return NodeOccurrenceQuery::faster_get_all_occurrences_using_low_memory(input, this->fastParentDictionary,
																						this->grammar.get_rlslp_dictionary(), this->grammar.get_max_level(), occCacheList);
			}
			/**
			 * @brief Return the nonterminal \p i of v_{i} -> c if such a nonterminal exists. Otherwise, return -1.
			 */
			int64_t get_character_signatuere(uint8_t c) const
			{
				auto f = this->character_nonterminal_item_map.find(c);
				if (f == this->character_nonterminal_item_map.end())
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
					for (auto &pair : this->character_nonterminal_item_map)
					{
						r.push_back(pair.first);
					}
				}
				return r;
			}
			/**
			 * @brief Tests whether an explicit alphabet with numeric character IDs is configured.
			 * @return True if character_id_map is non-empty.
			 */
			bool has_explicit_alphabet() const
			{
				return this->character_id_map.size() > 0;
			}
			/**
			 * @brief Returns the bit width needed to encode alphabet indices.
			 * @return 1, 2, 4, 8, or 64 depending on alphabet size.
			 */
			uint64_t get_alphabet_bit_size() const
			{

				uint64_t alphabet_size = 0;
				if (this->has_explicit_alphabet())
				{
					alphabet_size = this->character_id_map.size();
				}
				else
				{
					alphabet_size = this->character_nonterminal_item_map.size();
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
			/**
			 * @brief Returns the explicit character-to-index map when configured.
			 * @return Const reference to the character ID map.
			 */
			const std::unordered_map<int64_t, uint64_t> &get_character_id_map() const
			{
				return this->character_id_map;
			}

			/**
			 * @brief Return the nonterminal of the nonterminal \p v_{i} -> \p body if such a nonterminal exists. Otherwise, return -1.
			 */
			int64_t get_nonterminal(const RLSLPRuleBody body) const
			{
				const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->get_explicit_nonterminal_rule_list();
				if (body.get_type() == RLSLPRuleType::Character)
				{
					int64_t c = body.A;
					auto f = this->character_nonterminal_item_map.find(c);
					if (f == this->character_nonterminal_item_map.end())
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
					int64_t searchResult = this->fastParentDictionary.get_pair_nonterminal(body.A, body.B, explicit_nonterminal_rule_list);
					return searchResult;
				}
				else if (body.get_type() == RLSLPRuleType::Power)
				{
					int64_t searchResult = this->fastParentDictionary.get_power_nonterminal(body.A, body.B, explicit_nonterminal_rule_list);
					return searchResult;
				}
				else if (body.get_type() == RLSLPRuleType::Nonterminal)
				{
					return this->get_nonterminal_single(body);
				}
				else
				{
					throw std::runtime_error("Error in get_nonterminal: Unexpected rule type");
				}
			}

			StaticRLSLP convert_to_rlslp() const
			{
				return this->grammar.convert_to_rlslp();
			}

			StaticRLSLP convert_to_canonized_rlslp() const
			{
				return this->grammar.convert_to_canonized_rlslp();
			}

			//}@

			////////////////////////////////////////////////////////////////////////////////
			///   @name Update operations
			////////////////////////////////////////////////////////////////////////////////
			//@{
		public:
			/**
			 * @brief Initializes an empty dynamic grammar with optional restricted recompression.
			 * @param restricted_recompression True to enable restricted block compression mode.
			 * @param seed Random seed for internal randomized structures.
			 */
			void initialize(GrammarParsingType parser, uint64_t seed = 0)
			{
				this->clear();
				this->grammar.initialize(parser, seed);
				this->fastParentDictionary.set_pointer(&this->grammar.get_rlslp_dictionary().get_relative_max_level_list(), this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression);
			}
			/**
			 * @brief Initializes an empty grammar and registers an explicit alphabet with numeric IDs.
			 * @param restricted_recompression True to enable restricted block compression mode.
			 * @param alphabet Sorted list of characters defining the alphabet.
			 * @param seed Random seed for internal randomized structures.
			 */
			void initialize(GrammarParsingType parser, const std::vector<uint8_t> &alphabet, uint64_t seed)
			{
				this->initialize(parser, seed);
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
				this->unused_nonterminals.clear();
				this->character_nonterminal_item_map.clear();
				this->character_id_map.clear();
			}
			/**
			 * @brief Swap operation
			 * @param other Grammar to swap contents with.
			 */
			void swap(DynamicGrammarForLayeredRLSLP &other)
			{
				this->grammar.swap(other.grammar);
				this->unused_nonterminals.swap(other.unused_nonterminals);
				this->character_nonterminal_item_map.swap(other.character_nonterminal_item_map);
				this->fastParentDictionary.swap(other.fastParentDictionary);
				this->character_id_map.swap(other.character_id_map);

				this->fastParentDictionary.set_pointer(&this->grammar.get_rlslp_dictionary().get_relative_max_level_list(), this->grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression);
				other.fastParentDictionary.set_pointer(&other.grammar.get_rlslp_dictionary().get_relative_max_level_list(), other.grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression);
			}

			/**
			 * @brief Looks up a nonterminal without creating a new nonterminal.
			 * @param body Rule body to look up.
			 * @return Existing nonterminal, or -1 if the rule is absent or a new nonterminal level would be required.
			 */
			int64_t try_get_nonterminal(RLSLPRuleBody body) const
			{
				NonterminalLessComparer::explicit_nonterminal_rule_list = &this->grammar.get_rlslp_dictionary().get_explicit_nonterminal_rule_list();

				if (body.get_type() == RLSLPRuleType::Power && body.B == 1)
				{
					throw std::runtime_error("get_or_add_nonterminal: body.get_type() == RLSLPRuleType::Power && body.B == 1");
				}

				if (body.get_type() == RLSLPRuleType::Nonterminal)
				{
					int64_t child_sig = body.A;
					int64_t base_sig = NonterminalFunctions::get_explicit_nonterminal(child_sig);
					int64_t child_level = NonterminalFunctions::get_relative_level(child_sig);
					int64_t body_nonterminal = NonterminalFunctions::get_nonterminal(child_level + 1, base_sig);
					bool is_new_nonterminal = child_level >= this->grammar.get_rlslp_dictionary().get_relative_max_level_list()[base_sig];
					if (is_new_nonterminal)
					{
						return -1;
					}
					else
					{
						return body_nonterminal;
					}
				}
				else
				{
					auto index = this->get_nonterminal(body);
					return index;
				}
			}

			/**
			 * @brief Return the nonterminal \p i of the rule v_{i} -> \p body. If such a nonterminal does not exist, add it to the grammar \p G and execute the callback function \p callback_for_added_nonterminal.
			 * @tparam CALLBACK Callable type invoked with the new nonterminal when a rule is created.
			 * @param body Rule body to look up or insert.
			 * @param new_level Height level assigned when a new nonterminal is created.
			 * @param callback_for_added_nonterminal Callback invoked with the nonterminal of each newly added nonterminal.
			 * @return Nonterminal of the existing or newly created nonterminal.
			 */
			template <typename CALLBACK>
			NonterminalWithRelativeLevel get_or_add_nonterminal(RLSLPRuleBody body, uint16_t new_level, CALLBACK &callback_for_added_nonterminal)
			{
				NonterminalLessComparer::explicit_nonterminal_rule_list = &this->get_explicit_nonterminal_rule_list();

				NonterminalWithRelativeLevel registedNonterminal = -1;

				if (body.get_type() == RLSLPRuleType::Power && body.B == 1)
				{
					throw std::runtime_error("get_or_add_nonterminal: body.get_type() == RLSLPRuleType::Power && body.B == 1");
				}

				if (body.get_type() == RLSLPRuleType::Nonterminal)
				{
					int64_t child_sig = body.A;
					int64_t base_sig = NonterminalFunctions::get_explicit_nonterminal(child_sig);
					int64_t child_level = NonterminalFunctions::get_relative_level(child_sig);
					int64_t body_nonterminal = NonterminalFunctions::get_nonterminal(child_level + 1, base_sig);
					bool is_new_nonterminal = child_level >= this->get_relative_max_level_list()[base_sig];
					if (is_new_nonterminal)
					{
						this->insert_new_item_into_list(body_nonterminal, body, new_level);
					}
					registedNonterminal = body_nonterminal;
				}
				else
				{
					auto index = this->get_nonterminal(body);
					if (index == -1)
					{
						auto newNumber = this->get_new_number();
						this->insert_new_item_into_list(newNumber, body, new_level);
						registedNonterminal = newNumber;
						callback_for_added_nonterminal(newNumber);
					}
					else
					{
						registedNonterminal = index;
					}
				}

				return registedNonterminal;
			}

			/**
			 * @brief Add a new document to the grammar \p G, where the derivation tree of the document is rooted at the nonterminal \p v_{i}
			 */
			void add_document(NonterminalWithRelativeLevel i)
			{
				this->grammar.add_document(i);
			}
			/**
			 * @brief If \p G contains a document rooted at the nonterminal \p v_{i}, remove it from \p G and execute the callback function \p preprocessor. Otherwise, throw an error.
			 * @tparam PREPROCESSOR Callable type invoked for each removed nonterminal during cleanup.
			 * @param i Nonterminal of the document root to remove.
			 * @param preprocessor Callback executed before each unused nonterminal is erased.
			 * @return True if the document was found and removed.
			 */
			template <typename PREPROCESSOR = decltype(no_callback)>
			bool remove_document(NonterminalWithRelativeLevel i, PREPROCESSOR &preprocessor)
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



			/**
			 * @brief Prints storage and structure statistics for this grammar.
			 * @param message_paragraph Indentation depth for log output.
			 */
			void print_statistics(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
			{
				/*
				GrammarForLayeredRLSLP grammar;
				FastParentDictionary fastParentDictionary;
				std::vector<NonterminalWithRelativeLevel> unused_nonterminals;					// Z
				std::map<int64_t, NonterminalWithRelativeLevel> character_nonterminal_item_map; // M_{char}
				std::unordered_map<int64_t, uint64_t> character_id_map;
				*/
	


				std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics (DynamicGrammarForLayeredRLSLP): " << std::endl;
				this->grammar.print_statistics(message_paragraph + 1);
				this->fastParentDictionary.print_statistics(message_paragraph + 1);
				std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
			}

			void print_memory_breakdown(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
			{
				std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Memory Breakdown (DynamicGrammarForLayeredRLSLP): " << this->size_in_bytes() << " bytes" << std::endl;
				this->grammar.print_memory_breakdown(message_paragraph + 1);
				std::cout << stool::Message::get_paragraph_string(message_paragraph+1) << "unused_nonterminals: " << stool::Memory::estimate_memory_usage(this->unused_nonterminals) << " bytes" << std::endl;
				std::cout << stool::Message::get_paragraph_string(message_paragraph+1) << "character_nonterminal_item_map: " << stool::Memory::estimate_memory_usage(this->character_nonterminal_item_map) << " bytes" << std::endl;
				std::cout << stool::Message::get_paragraph_string(message_paragraph+1) << "character_id_map: " << stool::Memory::estimate_memory_usage(this->character_id_map) << " bytes" << std::endl;
				this->fastParentDictionary.print_memory_breakdown(message_paragraph + 1);
				std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;

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
			/**
			 * @brief Prints the derivation tree rooted at the given nonterminal.
			 * @param sig Root nonterminal of the derivation tree to print.
			 * @param message_paragraph Indentation depth for log output.
			 */
			void print_derivation_tree(NonterminalWithRelativeLevel sig, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
			{
				const DictionaryForLayeredRLSLP &rlslp_dictionary = this->grammar.get_rlslp_dictionary();
				RunRuleVector item1(sig, rlslp_dictionary);
				item1.print_derivation_tree(3, message_paragraph);
			}


			void write_content_as_json_format(std::ofstream &ofs, std::string name, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const{
				ofs << stool::Message::get_paragraph_string(message_paragraph) << "\"" << name << "\": " << "{" << std::endl;
				this->fastParentDictionary.write_content_as_json_format(ofs, "fastParentDictionary", message_paragraph+1);
				ofs << ", " << std::endl;
				this->grammar.write_content_as_json_format(ofs, "grammar", message_paragraph+1);
				ofs << ", " << std::endl;

				JsonHelper::write_content_as_json_format<NonterminalWithRelativeLevel>(
					"unused_nonterminals(std::vector<NonterminalWithRelativeLevel>)",
					this->unused_nonterminals,
					[](const NonterminalWithRelativeLevel &value){ return NonterminalFunctions::to_string(value); },
					false,
					ofs,
					false,
					message_paragraph+1
				);
				ofs << ", " << std::endl;

				JsonHelper::write_content_as_json_format<int64_t, NonterminalWithRelativeLevel>(
					"character_nonterminal_item_map(std::map<int64_t, NonterminalWithRelativeLevel>)",
					this->character_nonterminal_item_map,
					[](const int64_t &key){ return std::to_string((uint8_t)key); },
					[](const NonterminalWithRelativeLevel &value){ return NonterminalFunctions::to_string(value); },
					false,
					ofs,
					message_paragraph+1
				);
				ofs << ", " << std::endl;

				JsonHelper::write_content_as_json_format<int64_t, uint64_t>(
					"character_id_map(std::unordered_map<int64_t, uint64_t>)",
					this->character_id_map,
					[](const int64_t &key){ return std::to_string((uint8_t)key); },
					[](const uint64_t &value){ return std::to_string(value); },
					false,
					ofs,
					message_paragraph+1
				);
				ofs << "}";
            
			}
	

			/**
			 * @brief Return the memory usage information of this grammar as a vector of strings
			 * @param message_paragraph The paragraph depth of message logs
			 */
			std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
			{

				std::vector<std::string> r;
				uint64_t size_in_bytes = this->size_in_bytes();
				uint64_t nonterminal_count = this->nonterminal_count_without_null_nonterminals();
				uint64_t unused_nonterminal_count = this->unused_nonterminals.size();

				double bits_per_element = nonterminal_count > 0 ? ((double)size_in_bytes / (double)nonterminal_count) : 0;

				r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicGrammarForLayeredRLSLP: " + std::to_string(this->size_in_bytes()) + " bytes, " + std::to_string(nonterminal_count) + " nonterminals, " + std::to_string(unused_nonterminal_count) + " unused nonterminals, " + std::to_string(bits_per_element) + " bytes per nonterminal =");

				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "explicit_nonterminal_rule_list: \t\t\t" + std::to_string(this->get_explicit_nonterminal_rule_list().size() * sizeof(RLSLPRuleBody)) + " bytes" + " (" + std::to_string(sizeof(RLSLPRuleBody)) + " bytes per nonterminal)");
				// r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "heightList: \t\t\t" + std::to_string(this->heightList.size() * sizeof(unsigned char)) + " bytes" + " (" + std::to_string(sizeof(unsigned char)) + " bytes per nonterminal)");
				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "explicit_nonterminal_length_list_: \t\t\t" + std::to_string(this->get_explicit_nonterminal_length_list().size() * sizeof(uint64_t)) + " bytes" + " (" + std::to_string(sizeof(uint64_t)) + " bytes per nonterminal)");

				uint64_t character_nonterminal_item_map_byte_size = ((this->character_nonterminal_item_map.size() + 1) * (sizeof(std::pair<int64_t, NonterminalWithRelativeLevel>) + 3 * sizeof(void *))) + sizeof(void *);
				uint64_t bits_per_character_nonterminal_item_map = this->character_nonterminal_item_map.size() > 0 ? ((double)character_nonterminal_item_map_byte_size / (double)this->character_nonterminal_item_map.size()) : 0;
				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "character_nonterminal_item_map: \t" + std::to_string(character_nonterminal_item_map_byte_size) + " bytes" + " (" + std::to_string(bits_per_character_nonterminal_item_map) + " bytes per character)");

				auto r2 = this->fastParentDictionary.get_memory_usage_info(nonterminal_count, message_paragraph + 1);
				for (auto s : r2)
				{
					r.push_back(s);
				}

				uint64_t documentCounter_byte_size = 0;
				documentCounter_byte_size += ((this->grammar.get_document_counter().size() + 1) * (sizeof(std::pair<NonterminalWithRelativeLevel, uint64_t>) + 3 * sizeof(void *))) + sizeof(void *);
				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "documentCounter: \t\t\t" + std::to_string(documentCounter_byte_size) + " bytes");

				uint64_t unused_nonterminals_byte_size = 0;
				unused_nonterminals_byte_size += sizeof(std::vector<NonterminalWithRelativeLevel>) + (sizeof(NonterminalWithRelativeLevel) * this->unused_nonterminals.size());
				uint64_t bits_per_unused_nonterminals = this->unused_nonterminals.size() > 0 ? ((double)unused_nonterminals_byte_size / (double)this->unused_nonterminals.size()) : 0;
				r.push_back(stool::Message::get_paragraph_string(message_paragraph + 1) + "unused_nonterminals: \t\t" + std::to_string(unused_nonterminals_byte_size) + " bytes" + " (" + std::to_string(bits_per_unused_nonterminals) + " bytes per unused nonterminal)");

				r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");

				return r;
			}
			/**
			 * @brief Verifies internal consistency of the parent dictionary.
			 * @return True if verification succeeds.
			 * @throws std::runtime_error on validation failure.
			 */
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
				// Code 4: std::map<int64_t, NonterminalWithRelativeLevel> character_nonterminal_item_map;
				// Code 5: std::vector<std::vector<NonterminalWithRelativeLevel>> parentVectorList;
				// Code 5: std::unordered_map<NonterminalWithRelativeLevel, std::set<NonterminalWithRelativeLevel, NonterminalLessComparer>> parentMap;
				// Code 6: std::unordered_map<NonterminalWithRelativeLevel, uint64_t> documentCounter;
				// Code 7: std::vector<NonterminalWithRelativeLevel> unused_nonterminals;
				// Code 8: std::vector<uint8_t> randomBitList;

				// Code 0

				if (!this->grammar.verify_nearly_equal(other.grammar))

					// Code 4
					if (this->character_nonterminal_item_map.size() != other.character_nonterminal_item_map.size())
					{
						throw std::runtime_error("Error in verify_nearly_equal: The size of character nonterminal item map must be equal.");
					}
				std::vector<std::pair<int64_t, NonterminalWithRelativeLevel>> character_signatuere_item_vec1 = this->create_character_signatuere_item_vec();
				std::vector<std::pair<int64_t, NonterminalWithRelativeLevel>> character_signatuere_item_vec2 = other.create_character_signatuere_item_vec();

				for (uint64_t i = 0; i < character_signatuere_item_vec1.size(); i++)
				{
					if (character_signatuere_item_vec1[i].first != character_signatuere_item_vec2[i].first || character_signatuere_item_vec1[i].second != character_signatuere_item_vec2[i].second)
					{
						throw std::runtime_error("Error in verify_nearly_equal: The character nonterminal item map must be equal.");
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
				if (this->unused_nonterminals.size() != other.unused_nonterminals.size())
				{
					throw std::runtime_error("Error in verify_nearly_equal: The size of unused nonterminals must be equal.");
				}
				for (uint64_t i = 0; i < this->unused_nonterminals.size(); i++)
				{
					if (this->unused_nonterminals[i] != other.unused_nonterminals[i])
					{
						throw std::runtime_error("Error in verify_nearly_equal: The unused nonterminals must be equal.");
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
				// Code 2: std::map<int64_t, NonterminalWithRelativeLevel> character_nonterminal_item_map;
				// Code 3: std::vector<std::vector<NonterminalWithRelativeLevel>> parentVectorList;
				// Code 3: std::unordered_map<NonterminalWithRelativeLevel, std::set<NonterminalWithRelativeLevel, NonterminalLessComparer>> parentMap;
				// Code 4: std::unordered_map<NonterminalWithRelativeLevel, uint64_t> documentCounter;
				// Code 5: std::vector<NonterminalWithRelativeLevel> unused_nonterminals;
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
				uint64_t _character_nonterminal_item_map_size;
				ifs.read(reinterpret_cast<char *>(&_character_nonterminal_item_map_size), sizeof(_character_nonterminal_item_map_size));
				{
					std::vector<std::pair<int64_t, NonterminalWithRelativeLevel>> character_signatuere_item_vec;
					character_signatuere_item_vec.resize(_character_nonterminal_item_map_size);
					ifs.read(reinterpret_cast<char *>(character_signatuere_item_vec.data()), sizeof(std::pair<int64_t, NonterminalWithRelativeLevel>) * _character_nonterminal_item_map_size);

					std::map<int64_t, NonterminalWithRelativeLevel> _character_nonterminal_item_map(character_signatuere_item_vec.begin(), character_signatuere_item_vec.end());
					r.character_nonterminal_item_map.swap(_character_nonterminal_item_map);
				}

				// Code 3
				// r.parentDictionary = ParentDictionary::load_from_file(&r.get_relative_max_level_list(), ifs);
				r.fastParentDictionary = FastParentDictionary::load_from_file(&r.grammar.get_rlslp_dictionary().get_relative_max_level_list(), ifs);

				// Code 5
				uint64_t _unused_nonterminals_size;
				ifs.read(reinterpret_cast<char *>(&_unused_nonterminals_size), sizeof(_unused_nonterminals_size));
				r.unused_nonterminals.resize(_unused_nonterminals_size);
				ifs.read(reinterpret_cast<char *>(r.unused_nonterminals.data()), sizeof(NonterminalWithRelativeLevel) * _unused_nonterminals_size);

				// Initialize decoders and parentDictionary
				// r.levelDecoder = LevelDecoder(&r.small_dic.get_explicit_nonterminal_level_list());
				const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = r.grammar.get_rlslp_dictionary().get_explicit_nonterminal_rule_list();
				NonterminalLessComparer::explicit_nonterminal_rule_list = &explicit_nonterminal_rule_list;

				// Code 8
				uint64_t character_id_map_size;
				ifs.read(reinterpret_cast<char *>(&character_id_map_size), sizeof(uint64_t));
				for (uint64_t i = 0; i < character_id_map_size; i++)
				{
					int64_t character_id;
					ifs.read(reinterpret_cast<char *>(&character_id), sizeof(int64_t));
					int64_t nonterminal_id;
					ifs.read(reinterpret_cast<char *>(&nonterminal_id), sizeof(int64_t));
					r.character_id_map[character_id] = nonterminal_id;
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
				// Code 2: std::map<int64_t, NonterminalWithRelativeLevel> character_nonterminal_item_map;
				// Code 3: std::vector<std::vector<NonterminalWithRelativeLevel>> parentVectorList;
				// Code 3: std::unordered_map<NonterminalWithRelativeLevel, std::set<NonterminalWithRelativeLevel, NonterminalLessComparer>> parentMap;
				// Code 4: std::unordered_map<NonterminalWithRelativeLevel, uint64_t> documentCounter;
				// Code 5: std::vector<NonterminalWithRelativeLevel> unused_nonterminals;
				// Code 6: std::vector<uint8_t> randomBitList;

				os.write(reinterpret_cast<const char *>(&FINGERPRINT), sizeof(uint64_t));
				GrammarForLayeredRLSLP::store_to_file(item.grammar, os);

				// Code 2
				uint64_t character_nonterminal_item_map_size = item.character_nonterminal_item_map.size();
				os.write(reinterpret_cast<const char *>(&character_nonterminal_item_map_size), sizeof(uint64_t));
				{
					std::vector<std::pair<int64_t, NonterminalWithRelativeLevel>> character_signatuere_item_vec = item.create_character_signatuere_item_vec();
					os.write(reinterpret_cast<const char *>(character_signatuere_item_vec.data()), sizeof(std::pair<int64_t, NonterminalWithRelativeLevel>) * character_nonterminal_item_map_size);
				}

				// Code 3
				// ParentDictionary::store_to_file(item.parentDictionary, os);
				FastParentDictionary::store_to_file(item.fastParentDictionary, os);

				// Code 5
				uint64_t unused_nonterminals_size = item.unused_nonterminals.size();
				os.write(reinterpret_cast<const char *>(&unused_nonterminals_size), sizeof(uint64_t));
				os.write(reinterpret_cast<const char *>(item.unused_nonterminals.data()), sizeof(NonterminalWithRelativeLevel) * unused_nonterminals_size);

				// Code 8
				uint64_t character_id_map_size = item.character_id_map.size();
				os.write(reinterpret_cast<const char *>(&character_id_map_size), sizeof(uint64_t));
				for (const auto &pair : item.character_id_map)
				{
					os.write(reinterpret_cast<const char *>(&pair.first), sizeof(int64_t));
					os.write(reinterpret_cast<const char *>(&pair.second), sizeof(int64_t));
				}
			}

			/**
			 * @brief Builds a dynamic grammar from a serialized layered RLSLP grammar stream.
			 * @param ifs Input stream containing a GrammarForLayeredRLSLP snapshot.
			 * @return Populated DynamicGrammarForLayeredRLSLP with parent links reconstructed.
			 */
			static DynamicGrammarForLayeredRLSLP build_from_leveled_rlslp(std::ifstream &ifs)
			{
				DynamicGrammarForLayeredRLSLP r;

				GrammarForLayeredRLSLP grammar = GrammarForLayeredRLSLP::load_from_file(ifs);
				r.grammar.swap(grammar);

				const DictionaryForLayeredRLSLP &rlslp_dictionary = r.grammar.get_rlslp_dictionary();

				const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = rlslp_dictionary.get_explicit_nonterminal_rule_list();
				r.fastParentDictionary.set_pointer(&rlslp_dictionary.get_relative_max_level_list(), r.grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression);

				for (uint64_t i = 0; i < rlslp_dictionary.count_explicit_nonterminals(); i++)
				{
					r.fastParentDictionary.add_new_element();
				}

				for (uint64_t i = 0; i < rlslp_dictionary.count_explicit_nonterminals(); i++)
				{
					RLSLPRuleBody rule = RLSLPRuleBody::decode_rule(i, explicit_nonterminal_rule_list);
					if (rule.get_type() == RLSLPRuleType::Character)
					{
						r.character_nonterminal_item_map[rule.A] = i;
					}
					else if (rule.get_type() == RLSLPRuleType::Pair)
					{
						uint64_t base_A_sig = NonterminalFunctions::get_explicit_nonterminal(rule.A);
						uint64_t diff_level_A = NonterminalFunctions::get_relative_level(rule.A);
						bool is_top_level_A = diff_level_A == rlslp_dictionary.get_relative_max_level_list()[base_A_sig];
						uint64_t base_B_sig = NonterminalFunctions::get_explicit_nonterminal(rule.B);
						uint64_t diff_level_B = NonterminalFunctions::get_relative_level(rule.B);
						bool is_top_level_B = diff_level_B == rlslp_dictionary.get_relative_max_level_list()[base_B_sig];

						r.fastParentDictionary.insert(rule.A, i, is_top_level_A, explicit_nonterminal_rule_list);
						r.fastParentDictionary.insert(rule.B, i, is_top_level_B, explicit_nonterminal_rule_list);
					}
					else if (rule.get_type() == RLSLPRuleType::Power)
					{
						uint64_t base_A_sig = NonterminalFunctions::get_explicit_nonterminal(rule.A);
						uint64_t diff_level_A = NonterminalFunctions::get_relative_level(rule.A);
						bool is_top_level_A = diff_level_A == rlslp_dictionary.get_relative_max_level_list()[base_A_sig];
						r.fastParentDictionary.insert(rule.A, i, is_top_level_A, explicit_nonterminal_rule_list);
					}
					else if (rule.get_type() == RLSLPRuleType::Null)
					{
						r.unused_nonterminals.push_back(i);
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

			/**
			 * @brief Allocates a new base nonterminal, reusing an unused slot when available.
			 * @return New base nonterminal identifier.
			 */
			int64_t get_new_number()
			{
				NonterminalLessComparer::explicit_nonterminal_rule_list = &this->get_explicit_nonterminal_rule_list();
				// const DictionaryForLayeredRLSLP &rlslp_dictionary = this->grammar.get_rlslp_dictionary();

				int64_t new_number = -1;
				uint64_t unused_nonterminal_count = this->unused_nonterminals.size();
				if (unused_nonterminal_count > 0)
				{
					new_number = this->unused_nonterminals[unused_nonterminal_count - 1];
					this->unused_nonterminals.pop_back();
				}
				else
				{
					this->fastParentDictionary.add_new_element();
					new_number = this->grammar.add_new_explicit_nonterminal();
				}
				return new_number;
			}
			/**
			 * @brief Registers a new rule body and updates parent links and auxiliary tables.
			 * @param new_nonterminal Signature assigned to the new or promoted nonterminal.
			 * @param new_item Rule body being inserted.
			 * @param new_level Height level of the new nonterminal.
			 */
			void insert_new_item_into_list(NonterminalWithRelativeLevel new_nonterminal, RLSLPRuleBody new_item, uint16_t new_level)
			{

				uint64_t explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(new_nonterminal);
				uint64_t diff = NonterminalFunctions::get_relative_level(new_nonterminal);
				const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->get_explicit_nonterminal_rule_list();

				if (diff == 0)
				{

					this->grammar.write_element(explicit_nonterminal, new_item, new_level, 0, this->get_explicit_nonterminal_length_list());

					if (new_item.get_type() == RLSLPRuleType::Pair)
					{

						uint64_t base_A_sig = NonterminalFunctions::get_explicit_nonterminal(new_item.A);
						uint64_t level_A = NonterminalFunctions::get_relative_level(new_item.A);
						bool is_top_level_A = level_A == this->get_relative_max_level_list()[base_A_sig];
						uint64_t base_B_sig = NonterminalFunctions::get_explicit_nonterminal(new_item.B);
						uint64_t level_B = NonterminalFunctions::get_relative_level(new_item.B);
						bool is_top_level_B = level_B == this->get_relative_max_level_list()[base_B_sig];

						this->fastParentDictionary.insert(new_item.A, new_nonterminal, is_top_level_A, explicit_nonterminal_rule_list);
						this->fastParentDictionary.insert(new_item.B, new_nonterminal, is_top_level_B, explicit_nonterminal_rule_list);
					}
					else if (new_item.get_type() == RLSLPRuleType::Power)
					{
						uint64_t base_A_sig = NonterminalFunctions::get_explicit_nonterminal(new_item.A);
						uint64_t level_A = NonterminalFunctions::get_relative_level(new_item.A);
						bool is_top_level_A = level_A == this->get_relative_max_level_list()[base_A_sig];

						this->fastParentDictionary.insert(new_item.A, new_nonterminal, is_top_level_A, explicit_nonterminal_rule_list);
					}
					else if (new_item.get_type() == RLSLPRuleType::Character)
					{
						int64_t c = new_item.A;

#ifdef DEBUG
						if (this->character_nonterminal_item_map.find(c) != this->character_nonterminal_item_map.end())
						{
							std::cout << "Character already exists: " << c << " " << this->character_nonterminal_item_map[c] << std::endl;
							assert(false);
						}
#endif
						// uint64_t current_bit_size = this->get_alphabet_bit_size();
						this->character_nonterminal_item_map[c] = new_nonterminal;
					}
					else
					{
						throw std::runtime_error("insert_new_item_into_list: unknown rule type");
					}
				}
				else
				{

					assert(new_level - this->get_explicit_nonterminal_level_list()[explicit_nonterminal] == diff);
					this->fastParentDictionary.insert_implicit_nonterminal(explicit_nonterminal, explicit_nonterminal_rule_list);
					this->grammar.increase_relative_max_level(explicit_nonterminal);
				}

				this->grammar.create_random_bit(explicit_nonterminal);
			}

			//}@

		private:
			/**
			 * @brief Serializes the character-to-nonterminal map as a vector of pairs.
			 * @return Vector of (character, nonterminal) pairs for I/O and verification.
			 */
			std::vector<std::pair<int64_t, NonterminalWithRelativeLevel>> create_character_signatuere_item_vec() const
			{

				uint64_t character_nonterminal_item_map_size = this->character_nonterminal_item_map.size();
				std::vector<std::pair<int64_t, NonterminalWithRelativeLevel>> character_signatuere_item_vec;
				character_signatuere_item_vec.resize(character_nonterminal_item_map_size);

				uint64_t counter = 0;
				for (auto it : this->character_nonterminal_item_map)
				{
					character_signatuere_item_vec[counter] = {it.first, it.second};
					counter++;
				}
				return character_signatuere_item_vec;
			}

			/**
			 * @brief Recursively removes an unused nonterminal and its descendants from the grammar.
			 * @tparam PREPROCESSOR Callback invoked for each removed nonterminal.
			 * @param removed_node_nonterminal Signature of the nonterminal being removed.
			 * @param parent_nonterminal Signature of the parent nonterminal (unused, reserved).
			 * @param preprocessor Callback executed before each node is erased.
			 */
			template <typename PREPROCESSOR = decltype(no_callback)>
			void __remove_document_sub(NonterminalWithRelativeLevel removed_node_nonterminal, [[maybe_unused]] uint64_t parent_nonterminal, PREPROCESSOR &preprocessor = no_callback)
			{
				NonterminalLessComparer::explicit_nonterminal_rule_list = &this->get_explicit_nonterminal_rule_list();
				const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->get_explicit_nonterminal_rule_list();
				const std::unordered_map<NonterminalWithRelativeLevel, uint64_t> &document_counter = this->grammar.get_document_counter();

				assert(RLSLPRuleBody::decode_rule(removed_node_nonterminal, explicit_nonterminal_rule_list).get_type() != RLSLPRuleType::Null);

				auto f = document_counter.find(removed_node_nonterminal);

				bool has_parent = this->fastParentDictionary.has_parent(removed_node_nonterminal);

				uint64_t explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(removed_node_nonterminal);
				uint64_t level = NonterminalFunctions::get_relative_level(removed_node_nonterminal);

				if (!has_parent && f == document_counter.end())
				{
					preprocessor(removed_node_nonterminal);
					this->grammar.erase_random_bit(removed_node_nonterminal);

					assert(level == this->get_relative_max_level_list()[explicit_nonterminal]);

					RLSLPRuleBody removed_item = RLSLPRuleBody::decode_rule(removed_node_nonterminal, explicit_nonterminal_rule_list);

					if (removed_item.get_type() == RLSLPRuleType::Pair)
					{

						assert(RLSLPRuleBody::decode_rule(removed_item.A, explicit_nonterminal_rule_list).get_type() != RLSLPRuleType::Null);
						this->fastParentDictionary.erase_nonterminal(removed_item.A, removed_node_nonterminal, explicit_nonterminal_rule_list);

						__remove_document_sub(removed_item.A, removed_node_nonterminal, preprocessor);

						RLSLPRuleBody right_item = RLSLPRuleBody::decode_rule(removed_item.B, explicit_nonterminal_rule_list);
						if (right_item.get_type() != RLSLPRuleType::Null)
						{
							this->fastParentDictionary.erase_nonterminal(removed_item.B, removed_node_nonterminal, explicit_nonterminal_rule_list);
							__remove_document_sub(removed_item.B, removed_node_nonterminal, preprocessor);
						}
					}
					else if (removed_item.get_type() == RLSLPRuleType::Power)
					{
						this->fastParentDictionary.erase_nonterminal(removed_item.A, removed_node_nonterminal, explicit_nonterminal_rule_list);
						__remove_document_sub(removed_item.A, removed_node_nonterminal, preprocessor);
					}
					else if (removed_item.get_type() == RLSLPRuleType::Nonterminal)
					{
						this->fastParentDictionary.erase_nonterminal(explicit_nonterminal, explicit_nonterminal_rule_list);
						this->grammar.decrease_relative_max_level(explicit_nonterminal);
						__remove_document_sub(removed_item.A, removed_node_nonterminal, preprocessor);
					}
					else if (removed_item.get_type() == RLSLPRuleType::Character)
					{
						this->character_nonterminal_item_map.erase(removed_item.A);
					}
					else if (removed_item.get_type() == RLSLPRuleType::Null)
					{
						throw std::runtime_error("remove_document_sub: removed_item.get_type() == RLSLPRuleType::Null");
					}

					if (level == 0)
					{
						this->grammar.clear_element(explicit_nonterminal);
						this->unused_nonterminals.push_back(explicit_nonterminal);
					}
				}
			}

			/**
			 * @brief Looks up the nonterminal of a Signature rule body without creating a new level.
			 * @param key Rule body of type Signature whose child nonterminal is encoded in A.
			 * @return Existing promoted nonterminal, or -1 if the next relative level does not yet exist.
			 */
			int64_t get_nonterminal_single(RLSLPRuleBody key) const
			{
				assert(key.get_type() == RLSLPRuleType::Nonterminal);
				uint64_t base_sig = NonterminalFunctions::get_explicit_nonterminal(key.A);
				uint64_t child_level = NonterminalFunctions::get_relative_level(key.A);
				if (child_level < this->get_relative_max_level_list()[base_sig])
				{
					return NonterminalFunctions::get_nonterminal(child_level + 1, base_sig);
				}
				else
				{
					return -1;
				}
			}
		};
	
}