#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "./components/random_bit_dictionary.hpp"
#include "./components/dictionary_for_layered_rlslp.hpp"
#include "../local_parsings/locally_consistent_parsing.hpp"
#include "./helper/nonterminal_less_comparer.hpp"
#include "./helper/derivation_tree_visualizer.hpp"
#include "./static_operations/all.hpp"

#include "./static_rlslp.hpp"

namespace dynRLSLP
{
	inline const auto no_callback = [](auto &&...) noexcept {};

	/**
	 * @brief Layered RLSLP grammar with dictionary and document set.
	 * @ingroup RLSLPClasses
	 */
	class GrammarForLayeredRLSLP
	{
	private:
		GrammarParsingType grammar_parsing_type = GrammarParsingType::SignatureEncoding;
		std::unordered_map<NonterminalWithRelativeLevel, uint64_t> document_counter; // M_{doc}
		RandomBitDictionary random_bit_dictionary;
		DictionaryForLayeredRLSLP rlslp_dictionary;

	public:
		inline static const NonterminalWithRelativeLevel DOCUMENT_MARK = INT64_MAX - 1;
		/**
		 * @brief Construct an empty layered RLSLP grammar.
		 */
		GrammarForLayeredRLSLP()
		{
		}
		/**
		 * @brief Destroy the grammar and release all stored data.
		 */
		~GrammarForLayeredRLSLP()
		{
			this->clear();
		}

		/** @brief Copy constructor (deleted). */
		GrammarForLayeredRLSLP(const GrammarForLayeredRLSLP &) = delete;
		/**
		 * @brief Move constructor; transfers dictionary, documents, and parsing type.
		 * @param other Source grammar to move from.
		 */
		GrammarForLayeredRLSLP(GrammarForLayeredRLSLP &&other) noexcept : grammar_parsing_type(std::move(other.grammar_parsing_type)),
																		  document_counter(std::move(other.document_counter)),
																		  random_bit_dictionary(std::move(other.random_bit_dictionary)),
																		  rlslp_dictionary(std::move(other.rlslp_dictionary))
		{
		}

		/** @brief Copy assignment (deleted). */
		GrammarForLayeredRLSLP &operator=(const GrammarForLayeredRLSLP &) = delete;
		/**
		 * @brief Move assignment; transfers contents from @p other when distinct.
		 * @param other Source grammar to move from.
		 * @return Reference to this grammar after assignment.
		 */
		GrammarForLayeredRLSLP &operator=(GrammarForLayeredRLSLP &&other) noexcept
		{
			if (this != &other)
			{
				this->grammar_parsing_type = std::move(other.grammar_parsing_type);
				this->document_counter = std::move(other.document_counter);
				this->random_bit_dictionary = std::move(other.random_bit_dictionary);
				this->rlslp_dictionary = std::move(other.rlslp_dictionary);
			}
			return *this;
		}

		/**
		 * @brief Return a const reference to the random-bit dictionary.
		 * @return Const reference to random bits used in restricted block compression.
		 */
		const RandomBitDictionary &get_random_bit_dictionary() const
		{
			return this->random_bit_dictionary;
		}
		/**
		 * @brief Return the layered RLSLP dictionary (D, H, L).
		 * @return Const reference to the internal dictionary.
		 */
		const DictionaryForLayeredRLSLP &get_rlslp_dictionary() const
		{
			return this->rlslp_dictionary;
		}
		/**
		 * @brief Return the document occurrence counter map.
		 * @return Map from document-root nonterminal to occurrence count.
		 */
		const std::unordered_map<NonterminalWithRelativeLevel, uint64_t> &get_document_counter() const
		{
			return this->document_counter;
		}
		/**
		 * @brief Return the grammar parsing algorithm type.
		 * @return Active grammar_parsing_type (nonterminal encoding or restricted block compression).
		 */
		GrammarParsingType get_grammar_parsing_type() const
		{
			return this->grammar_parsing_type;
		}
		/**
		 * @brief Return the number of distinct document roots.
		 * @return Size of the document counter map.
		 */
		uint64_t get_distinct_document_count() const
		{
			return this->document_counter.size();
		}
		/**
		 * @brief Return the total number of document occurrences.
		 * @return Sum of all values in the document counter map.
		 */
		uint64_t get_document_count() const
		{
			uint64_t sum = 0;
			for (auto it : this->document_counter)
			{
				sum += it.second;
			}
			return sum;
		}
		/**
		 * @brief Return the maximum derivation level.
		 * @return Absolute level of the document root, or 0 if no root exists.
		 */
		uint64_t get_max_level() const
		{
			if (this->has_root())
			{
				return this->rlslp_dictionary.get_explicit_nonterminal_level_list()[this->get_root()];
			}
			else
			{
				return 0;
			}
		}
		/**
		 * @brief Return whether the grammar has exactly one document root.
		 * @return True if the document counter contains exactly one entry.
		 */
		bool has_root() const
		{
			return this->document_counter.size() == 1;
		}
		/**
		 * @brief Return the base nonterminal index of the unique document root.
		 * @return Root base nonterminal when exactly one document exists.
		 * @throws std::runtime_error if there is no document or multiple documents.
		 */
		uint64_t get_root() const
		{
			if (this->document_counter.size() == 0)
			{
				throw std::runtime_error("No document");
			}
			else if (this->document_counter.size() > 1)
			{
				throw std::runtime_error("Multiple documents");
			}
			else
			{
				return this->document_counter.begin()->first;
			}
		}
		/**
		 * @brief Return the total count of single nonterminals.
		 * @return Sum of single-nonterminal counts from the internal dictionary.
		 */
		int64_t count_implicit_nonterminals() const
		{
			return this->rlslp_dictionary.count_valid_implicit_nonterminals();
		}

		StaticRLSLP convert_to_rlslp() const
		{
			if(this->document_counter.size() != 1){
				throw std::runtime_error("The size of document counter must be 1.");
			}
			const DictionaryForLayeredRLSLP &rlslp_dictionary = this->get_rlslp_dictionary();
			const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = rlslp_dictionary.get_explicit_nonterminal_rule_list();
			const std::vector<uint16_t> &relative_max_level_list = rlslp_dictionary.get_relative_max_level_list();


			StaticRLSLP r;
			r.root_id = this->get_root();

			for(uint64_t i = 0; i < explicit_nonterminal_rule_list.size(); i++){
				RLSLPRuleBody rule = explicit_nonterminal_rule_list[i];

				if(rule.get_type() != RLSLPRuleType::Null){
					uint16_t relative_level = relative_max_level_list[i];
					for(uint64_t j = 1; j <= relative_level; j++){
						NonterminalWithRelativeLevel nonterminal = NonterminalFunctions::get_nonterminal(j, i);
						NonterminalWithRelativeLevel child_nonterminal = NonterminalFunctions::get_nonterminal(j-1, i);
	
						RLSLPRuleBody new_rule = RLSLPRuleBody::create_nonterminal_item(child_nonterminal);
						r.nonterminal_list.push_back(nonterminal);
						r.rule_list.push_back(new_rule);
					}
					r.nonterminal_list.push_back(i);
					r.rule_list.push_back(rule);
				}
			}
			


			return r;

		}


		StaticRLSLP convert_to_canonized_rlslp() const
		{
			if(this->document_counter.size() != 1){
				throw std::runtime_error("The size of document counter must be 1.");
			}
			//NonterminalWithRelativeLevel root = this->get_root();
			const DictionaryForLayeredRLSLP &rlslp_dictionary = this->get_rlslp_dictionary();
			const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = rlslp_dictionary.get_explicit_nonterminal_rule_list();
			const std::vector<uint64_t> &explicit_nonterminal_length_list = rlslp_dictionary.get_explicit_nonterminal_length_list();
			uint64_t list_size = rlslp_dictionary.count_explicit_nonterminals();

			std::vector<uint64_t> id_list;
			for(uint64_t i = 0; i < list_size; i++){
				if(!rlslp_dictionary.check_null_item(i)){
					id_list.push_back(i);
				}
			}
			
			std::sort(id_list.begin(), id_list.end(), [&](uint64_t a, uint64_t b){ return explicit_nonterminal_length_list[a] < explicit_nonterminal_length_list[b]; });

			std::vector<uint64_t> id_mapper;
			id_mapper.resize(list_size, UINT64_MAX);
			for(uint64_t i = 0; i < id_list.size(); i++){
				id_mapper[id_list[i]] = i;
			}

			StaticRLSLP r;
			r.root_id = id_mapper[this->get_root()];

			for(uint64_t i = 0; i < id_list.size(); i++){
				RLSLPRuleBody rule = explicit_nonterminal_rule_list[id_list[i]];
				r.nonterminal_list.push_back(i);

				if(rule.get_type() == RLSLPRuleType::Character){
					r.rule_list.push_back(rule);
				}else if(rule.get_type() == RLSLPRuleType::Pair){
					NonterminalWithRelativeLevel left = NonterminalFunctions::get_explicit_nonterminal(rule.A);					
					NonterminalWithRelativeLevel right = NonterminalFunctions::get_explicit_nonterminal(rule.B);
					uint64_t left_sig = NonterminalFunctions::get_explicit_nonterminal(left);
					uint64_t right_sig = NonterminalFunctions::get_explicit_nonterminal(right);
					uint64_t left_id = id_mapper[left_sig];
					uint64_t right_id = id_mapper[right_sig];
					RLSLPRuleBody new_rule = RLSLPRuleBody::create_pair_item(left_id, right_id);
					r.rule_list.push_back(new_rule);
				}else if(rule.get_type() == RLSLPRuleType::Power){
					NonterminalWithRelativeLevel X = NonterminalFunctions::get_explicit_nonterminal(rule.A);
					uint64_t X_sig = NonterminalFunctions::get_explicit_nonterminal(X);
					uint64_t X_id = id_mapper[X_sig];
					RLSLPRuleBody new_rule = RLSLPRuleBody::create_run_rule_body(X_id, rule.B);
					r.rule_list.push_back(new_rule);
				}else{
					throw std::runtime_error("The rule type is not supported.");
				}
			}
			return r;
		}





		/**
		 * @brief Reset the grammar and select a parsing algorithm.
		 * @param restricted_recompression If true, use restricted block compression and initialize random bits.
		 * @param seed Random seed for the random-bit dictionary when restricted recompression is enabled.
		 */
		void initialize(GrammarParsingType parser, uint64_t seed = 0)
		{
			this->clear();
			if (parser == GrammarParsingType::RestrictedRecompression)
			{
				this->grammar_parsing_type = GrammarParsingType::RestrictedRecompression;
				this->random_bit_dictionary.initialize(seed);
			}
			else
			{
				this->grammar_parsing_type = GrammarParsingType::SignatureEncoding;
			}
		}
		/**
		 * @brief Return the total number of nonterminals.
		 * @return Total nonterminal count from the internal dictionary.
		 */
		uint64_t nonterminal_count() const
		{
			return this->rlslp_dictionary.count_valid_nonterminals();
		}

		/**
		 * @brief Clear dictionary, random bits, and document counters.
		 */
		void clear()
		{
			this->random_bit_dictionary.clear();
			this->rlslp_dictionary.clear();
			this->document_counter.clear();
		}

		/**
		 * @brief Swap contents with another instance.
		 * @param other Other instance to compare or swap with.
		 */
		void swap(GrammarForLayeredRLSLP &other)
		{
			std::swap(this->grammar_parsing_type, other.grammar_parsing_type);
			this->random_bit_dictionary.swap(other.random_bit_dictionary);
			this->rlslp_dictionary.swap(other.rlslp_dictionary);
			this->document_counter.swap(other.document_counter);
		}
		/**
		 * @brief Register a document rooted at the given nonterminal.
		 * @param i Document-root nonterminal (increments counter or inserts new entry).
		 */
		void add_document(NonterminalWithRelativeLevel i)
		{
			NonterminalLessComparer::explicit_nonterminal_rule_list = &this->rlslp_dictionary.get_explicit_nonterminal_rule_list();

			auto f = this->document_counter.find(i);
			if (f == this->document_counter.end())
			{
				this->document_counter[i] = 1;
				// this->parentList[nonterminal].insert(DOCUMENT_MARK);
			}
			else
			{
				this->document_counter[i]++;
			}
		}
		/**
		 * @brief Remove one occurrence of a document rooted at the given nonterminal.
		 * @param i Signature or base-nonterminal index.
		 * @return True after decrementing or removing the document entry.
		 * @throws std::runtime_error if the document is not found.
		 */
		bool remove_document(NonterminalWithRelativeLevel i)
		{
			auto f = this->document_counter.find(i);
			if (f == this->document_counter.end())
			{
				throw std::runtime_error("Document not found");
			}
			else
			{
				assert(this->document_counter[i] > 0);
				this->document_counter[i]--;
				if (this->document_counter[i] == 0)
				{
					this->document_counter.erase(i);
				}
				return true;
			}
		}

		/**
		 * @brief Allocate a new base nonterminal slot.
		 * @return Index of the new base nonterminal (also extends random-bit storage when applicable).
		 */
		ExplicitNonterminal add_new_explicit_nonterminal()
		{

			ExplicitNonterminal new_number = this->rlslp_dictionary.add_new_explicit_nonterminal();

			if (this->grammar_parsing_type == GrammarParsingType::RestrictedRecompression)
			{
				this->random_bit_dictionary.add_new_element();
			}
			return new_number;
		}
		/**
		 * @brief Increment the relative maximum level of a base nonterminal.
		 * @param explicit_nonterminal Base nonterminal index.
		 */
		void increase_relative_max_level(ExplicitNonterminal explicit_nonterminal)
		{
			this->rlslp_dictionary.increase_relative_max_level(explicit_nonterminal);
		}
		/**
		 * @brief Decrement the relative maximum level of a base nonterminal.
		 * @param explicit_nonterminal Base nonterminal index.
		 */
		void decrease_relative_max_level(ExplicitNonterminal explicit_nonterminal)
		{
			this->rlslp_dictionary.decrease_relative_max_level(explicit_nonterminal);
		}
		/**
		 * @brief Reset a base nonterminal slot to null in the dictionary.
		 * @param explicit_nonterminal Base nonterminal index.
		 */
		void clear_element(ExplicitNonterminal explicit_nonterminal)
		{
			this->rlslp_dictionary.clear_element(explicit_nonterminal);
		}
		/**
		 * @brief Write rule body and metadata for a base nonterminal.
		 * @param explicit_nonterminal Base nonterminal index.
		 * @param rule_body RLSLP rule body to store.
		 * @param new_level Absolute level of the base nonterminal.
		 * @param relative_max_level Maximum relative level for the base nonterminal.
		 * @param explicit_nonterminal_length_list Base-nonterminal length list (L).
		 */
		void write_element(ExplicitNonterminal explicit_nonterminal, const RLSLPRuleBody &rule_body, uint16_t new_level, uint16_t relative_max_level, const std::vector<uint64_t> &explicit_nonterminal_length_list)
		{
			this->rlslp_dictionary.write_element(explicit_nonterminal, rule_body, new_level, relative_max_level, explicit_nonterminal_length_list);
		}

		/**
		 * @brief Create random bits for a base nonterminal (restricted block compression only).
		 * @param explicit_nonterminal Base nonterminal index whose single-nonterminal count defines bit width.
		 */
		void create_random_bit(ExplicitNonterminal explicit_nonterminal)
		{
			if (this->grammar_parsing_type == GrammarParsingType::RestrictedRecompression)
			{
				uint64_t single_count = this->rlslp_dictionary.get_relative_max_level_list()[explicit_nonterminal];
				this->random_bit_dictionary.create_random_bit(explicit_nonterminal, single_count);
			}
		}

		/**
		 * @brief Remove random bits for a nonterminal.
		 * @param nonterminal Encoded nonterminal with relative level.
		 */
		void erase_random_bit(NonterminalWithRelativeLevel nonterminal)
		{
			if (this->grammar_parsing_type == GrammarParsingType::RestrictedRecompression)
			{
#ifdef DEBUG
				ExplicitNonterminal explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(nonterminal);
				assert(this->rlslp_dictionary.get_relative_max_level_list()[explicit_nonterminal] == NonterminalFunctions::get_relative_level(nonterminal));
#endif
				this->random_bit_dictionary.erase_random_bit(nonterminal);
			}
		}

		/**
		 * @brief Print document counts to standard output.
		 */
		void print_documents() const
		{
			std::cout << "Documents: " << std::endl;
			for (auto it : this->document_counter)
			{
				std::cout << it.first << " " << it.second << std::endl;
			}
		}

		uint64_t size_in_bytes() const
		{
			uint64_t total_size = 0;
			total_size += this->rlslp_dictionary.size_in_bytes();
			total_size += this->random_bit_dictionary.size_in_bytes();
			total_size += stool::Memory::estimate_memory_usage(this->document_counter);
			total_size += sizeof(grammar_parsing_type);
			return total_size;
		}
		/**
		 * @brief Print all grammar rules to standard output.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_info(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{
			this->rlslp_dictionary.print_info(this->grammar_parsing_type, message_paragraph);
		}
		/**
		 * @brief Print summary statistics to standard output.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_statistics(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{
			//std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Documents:\t" << this->get_document_counter().size() << std::endl;

			std::string copmression_algorithm = this->get_grammar_parsing_type() == dynRLSLP::GrammarParsingType::RestrictedRecompression ? "Restricted Recompression" : "Signature Encoding";

			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics (GrammarForLayeredRLSLP): " << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Compression Algorithm:\t" << copmression_algorithm << std::endl;

			const DictionaryForLayeredRLSLP &rlslp_dictionary = this->get_rlslp_dictionary();
			const std::vector<uint64_t> &explicit_nonterminal_length_list = rlslp_dictionary.get_explicit_nonterminal_length_list();
			if (this->has_root())
			{
				int64_t root = this->get_root();
				uint64_t text_length = NonterminalFunctions::get_length(root, explicit_nonterminal_length_list);
	
				std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text Length:\t" << text_length << std::endl;
				std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The level of the derivation tree: " << this->get_rlslp_dictionary().get_explicit_nonterminal_level_list()[this->get_root()] << std::endl;
			}
			else
			{
				std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The grammar does not have a root." << std::endl;
			}
			rlslp_dictionary.print_statistics(message_paragraph + 1);
			random_bit_dictionary.print_statistics(message_paragraph + 1);
			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;

		}

		void print_memory_breakdown(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{
			//std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Documents:\t" << this->get_document_counter().size() << std::endl;


			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Memory Breakdown (GrammarForLayeredRLSLP): " << this->size_in_bytes() << " bytes" << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph+1) << "document_counter: " << stool::Memory::estimate_memory_usage(this->document_counter) << " bytes" << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph+1) << "grammar_parsing_type: " << sizeof(grammar_parsing_type) << " bytes" << std::endl;

			const DictionaryForLayeredRLSLP &rlslp_dictionary = this->get_rlslp_dictionary();

			rlslp_dictionary.print_memory_breakdown(message_paragraph + 1);
			random_bit_dictionary.print_memory_breakdown(message_paragraph + 1);
			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;

		}

		void write_content_as_json_format(std::ofstream &ofs, std::string name, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const{
			ofs << stool::Message::get_paragraph_string(message_paragraph) << "\"" << name << "\": " << "{" << std::endl;

			std::stringstream grammarParsingType_ss;
			if(this->grammar_parsing_type == GrammarParsingType::RestrictedRecompression)
			{
				grammarParsingType_ss << "\"Restricted Recompression\"";
			}
			else
			{
				grammarParsingType_ss << "\"Signature Encoding\"";
			}

			ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "\"grammar_parsing_type\": " << grammarParsingType_ss.str() << ", " << std::endl;

			JsonHelper::write_content_as_json_format<NonterminalWithRelativeLevel, uint64_t>(
                "document_counter(std::unordered_map<int64_t, uint64_t>)",
                this->document_counter,
                [](const NonterminalWithRelativeLevel &key){ return NonterminalFunctions::to_string(key); },
                [](const uint64_t &value){ return std::to_string(value); },
                true,
                ofs,
                message_paragraph+1
            );
			ofs << ", " << std::endl;
			this->random_bit_dictionary.write_content_as_json_format(ofs, "random_bit_dictionary", message_paragraph+1);
			ofs << ", " << std::endl;
			this->rlslp_dictionary.write_content_as_json_format(ofs, "rlslp_dictionary", message_paragraph+1);			
			ofs << "}";
        }


		/**
		 * @brief Verify structural near-equality with another instance.
		 * @param other Other instance to compare or swap with.
		 * @return True if parsing type, dictionary, documents, and random bits match.
		 * @throws std::runtime_error on mismatch.
		 */
		bool verify_nearly_equal(const GrammarForLayeredRLSLP &other) const
		{
			/*
			Checklist
			*/
			// Code 0: grammar_parsing_type grammar_parsing_type = grammar_parsing_type::SignatureEncoding;
			// Code 4: std::map<int64_t, NonterminalWithRelativeLevel> character_nonterminal_item_map;
			// Code 5: std::vector<std::vector<NonterminalWithRelativeLevel>> parentVectorList;
			// Code 5: std::unordered_map<NonterminalWithRelativeLevel, std::set<NonterminalWithRelativeLevel, NonterminalLessComparer>> parentMap;
			// Code 6: std::unordered_map<NonterminalWithRelativeLevel, uint64_t> document_counter;
			// Code 7: std::vector<NonterminalWithRelativeLevel> unused_nonterminals;
			// Code 8: std::vector<uint8_t> randomBitList;

			// Code 0

			if (this->grammar_parsing_type != other.grammar_parsing_type)
			{
				std::cout << "this->grammar_parsing_type = " << (int)this->grammar_parsing_type << ", other.grammar_parsing_type = " << (int)other.grammar_parsing_type << std::endl;
				throw std::runtime_error("Error in verify_nearly_equal: The grammar parsing type must be equal.");
			}

			this->rlslp_dictionary.verify_nearly_equal(other.rlslp_dictionary);

			// Code 6
			if (this->document_counter.size() != other.document_counter.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of document counter must be equal.");
			}

			// Code 8
			if (this->grammar_parsing_type == GrammarParsingType::RestrictedRecompression)
			{
				if (!this->random_bit_dictionary.verify_equal(other.random_bit_dictionary))
				{
					throw std::runtime_error("Error in verify_nearly_equal: The random bit dictionary must be equal.");
				}
			}

			return true;
		}

		/**
		 * @brief Deserialize from a binary input stream.
		 * @param ifs Input file stream.
		 * @return Grammar loaded from the binary stream.
		 */
		static GrammarForLayeredRLSLP load_from_file(std::ifstream &ifs)
		{
			/*
			Checklist
			*/

			GrammarForLayeredRLSLP r;

			int typeValue;
			ifs.read(reinterpret_cast<char *>(&typeValue), sizeof(int));
			r.grammar_parsing_type = static_cast<GrammarParsingType>(typeValue);

			DictionaryForLayeredRLSLP rlslp_dictionary = DictionaryForLayeredRLSLP::load_from_file(ifs);
			r.rlslp_dictionary.swap(rlslp_dictionary);

			// Code 4
			NonterminalWithRelativeLevel _document_nonterminal;
			uint64_t _document_counter_size;
			ifs.read(reinterpret_cast<char *>(&_document_nonterminal), sizeof(_document_nonterminal));
			ifs.read(reinterpret_cast<char *>(&_document_counter_size), sizeof(_document_counter_size));

			if (r.rlslp_dictionary.count_explicit_nonterminals() > 0)
			{
				r.document_counter[_document_nonterminal] = _document_counter_size;
			}

			// Code 6
			if (r.grammar_parsing_type == GrammarParsingType::RestrictedRecompression)
			{
				uint64_t seed;
				std::random_device rd;
				seed = (static_cast<uint64_t>(rd()) << 32) | rd();

				r.random_bit_dictionary = RandomBitDictionary::load_from_file(seed, ifs);
			}

			return r;
		}

		/**
		 * @brief Serialize to a binary output stream.
		 * @param item Grammar to write.
		 * @param os Output file stream.
		 */
		static void store_to_file(const GrammarForLayeredRLSLP &item, std::ofstream &os)
		{
			if (item.document_counter.size() > 1)
			{
				throw std::runtime_error("The size of document counter must be 0 or 1.");
			}

			int typeValue = static_cast<int>(item.grammar_parsing_type);
			os.write(reinterpret_cast<const char *>(&typeValue), sizeof(int));

			DictionaryForLayeredRLSLP::store_to_file(item.rlslp_dictionary, os);

			// Code 4
			if (item.document_counter.size() == 1)
			{
				std::pair<NonterminalWithRelativeLevel, uint64_t> document_counter_pair = {item.document_counter.begin()->first, item.document_counter.begin()->second};
				NonterminalWithRelativeLevel document_counter_nonterminal = document_counter_pair.first;
				uint64_t document_counter_count = document_counter_pair.second;
				os.write(reinterpret_cast<const char *>(&document_counter_nonterminal), sizeof(NonterminalWithRelativeLevel));
				os.write(reinterpret_cast<const char *>(&document_counter_count), sizeof(uint64_t));
			}
			else
			{
				NonterminalWithRelativeLevel document_counter_nonterminal = 0;
				uint64_t document_counter_count = 0;
				os.write(reinterpret_cast<const char *>(&document_counter_nonterminal), sizeof(NonterminalWithRelativeLevel));
				os.write(reinterpret_cast<const char *>(&document_counter_count), sizeof(uint64_t));
			}

			// Code 6
			if (item.grammar_parsing_type == GrammarParsingType::RestrictedRecompression)
			{
				RandomBitDictionary::store_to_file(item.random_bit_dictionary, os);
			}
		}
	};

}
