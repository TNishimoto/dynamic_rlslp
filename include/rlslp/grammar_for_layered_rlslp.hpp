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
		GrammarParsingType grammarParsingType = GrammarParsingType::SignatureEncoding;
		std::unordered_map<SignatureWithRelativeLevel, uint64_t> documentCounter; // M_{doc}
		RandomBitDictionary random_bit_dictionary;
		DictionaryForLayeredRLSLP rlslp_dictionary;

	public:
		inline static const SignatureWithRelativeLevel DOCUMENT_MARK = INT64_MAX - 1;
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
		GrammarForLayeredRLSLP(GrammarForLayeredRLSLP &&other) noexcept : grammarParsingType(std::move(other.grammarParsingType)),
																		  documentCounter(std::move(other.documentCounter)),
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
				this->grammarParsingType = std::move(other.grammarParsingType);
				this->documentCounter = std::move(other.documentCounter);
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
		 * @return Map from document-root signature to occurrence count.
		 */
		const std::unordered_map<SignatureWithRelativeLevel, uint64_t> &get_document_counter() const
		{
			return this->documentCounter;
		}
		/**
		 * @brief Return the grammar parsing algorithm type.
		 * @return Active GrammarParsingType (signature encoding or restricted block compression).
		 */
		GrammarParsingType get_grammar_parsing_type() const
		{
			return this->grammarParsingType;
		}
		/**
		 * @brief Return the number of distinct document roots.
		 * @return Size of the document counter map.
		 */
		uint64_t get_distinct_document_count() const
		{
			return this->documentCounter.size();
		}
		/**
		 * @brief Return the total number of document occurrences.
		 * @return Sum of all values in the document counter map.
		 */
		uint64_t get_document_count() const
		{
			uint64_t sum = 0;
			for (auto it : this->documentCounter)
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
				return this->rlslp_dictionary.get_base_signature_level_list()[this->get_root()];
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
			return this->documentCounter.size() == 1;
		}
		/**
		 * @brief Return the base signature index of the unique document root.
		 * @return Root base signature when exactly one document exists.
		 * @throws std::runtime_error if there is no document or multiple documents.
		 */
		uint64_t get_root() const
		{
			if (this->documentCounter.size() == 0)
			{
				throw std::runtime_error("No document");
			}
			else if (this->documentCounter.size() > 1)
			{
				throw std::runtime_error("Multiple documents");
			}
			else
			{
				return this->documentCounter.begin()->first;
			}
		}
		/**
		 * @brief Return the total count of single signatures.
		 * @return Sum of single-signature counts from the internal dictionary.
		 */
		int64_t count_single_signatures() const
		{
			return this->rlslp_dictionary.count_single_signatures(this->grammarParsingType);
		}
		/**
		 * @brief Reset the grammar and select a parsing algorithm.
		 * @param restricted_recompression If true, use restricted block compression and initialize random bits.
		 * @param seed Random seed for the random-bit dictionary when restricted recompression is enabled.
		 */
		void initialize(bool restricted_recompression = false, uint64_t seed = 0)
		{
			this->clear();
			if (restricted_recompression)
			{
				this->grammarParsingType = GrammarParsingType::RestrictedBlockCompression;
				this->random_bit_dictionary.initialize(seed);
			}
			else
			{
				this->grammarParsingType = GrammarParsingType::SignatureEncoding;
			}
		}
		/**
		 * @brief Return the total number of signatures.
		 * @return Total signature count from the internal dictionary.
		 */
		uint64_t signature_count() const
		{
			return this->rlslp_dictionary.signature_count(this->grammarParsingType);
		}

		/**
		 * @brief Clear dictionary, random bits, and document counters.
		 */
		void clear()
		{
			this->random_bit_dictionary.clear();
			this->rlslp_dictionary.clear();
			this->documentCounter.clear();
		}

		/**
		 * @brief Swap contents with another instance.
		 * @param other Other instance to compare or swap with.
		 */
		void swap(GrammarForLayeredRLSLP &other)
		{
			std::swap(this->grammarParsingType, other.grammarParsingType);
			this->random_bit_dictionary.swap(other.random_bit_dictionary);
			this->rlslp_dictionary.swap(other.rlslp_dictionary);
			this->documentCounter.swap(other.documentCounter);
		}
		/**
		 * @brief Register a document rooted at the given signature.
		 * @param i Document-root signature (increments counter or inserts new entry).
		 */
		void add_document(SignatureWithRelativeLevel i)
		{
			NonterminalLessComparer::base_signature_rule_list = &this->rlslp_dictionary.get_base_signature_rule_list();

			auto f = this->documentCounter.find(i);
			if (f == this->documentCounter.end())
			{
				this->documentCounter[i] = 1;
				// this->parentList[signature].insert(DOCUMENT_MARK);
			}
			else
			{
				this->documentCounter[i]++;
			}
		}
		/**
		 * @brief Remove one occurrence of a document rooted at the given signature.
		 * @param i Signature or base-signature index.
		 * @return True after decrementing or removing the document entry.
		 * @throws std::runtime_error if the document is not found.
		 */
		bool remove_document(SignatureWithRelativeLevel i)
		{
			auto f = this->documentCounter.find(i);
			if (f == this->documentCounter.end())
			{
				throw std::runtime_error("Document not found");
			}
			else
			{
				assert(this->documentCounter[i] > 0);
				this->documentCounter[i]--;
				if (this->documentCounter[i] == 0)
				{
					this->documentCounter.erase(i);
				}
				return true;
			}
		}

		/**
		 * @brief Allocate a new base signature slot.
		 * @return Index of the new base signature (also extends random-bit storage when applicable).
		 */
		BaseSignature add_new_base_signature()
		{

			BaseSignature new_number = this->rlslp_dictionary.add_new_base_signature();

			if (this->grammarParsingType == GrammarParsingType::RestrictedBlockCompression)
			{
				this->random_bit_dictionary.add_new_element();
			}
			return new_number;
		}
		/**
		 * @brief Increment the relative maximum level of a base signature.
		 * @param base_signature Base signature index.
		 */
		void increase_relative_max_level(BaseSignature base_signature)
		{
			this->rlslp_dictionary.increase_relative_max_level(base_signature);
		}
		/**
		 * @brief Decrement the relative maximum level of a base signature.
		 * @param base_signature Base signature index.
		 */
		void decrease_relative_max_level(BaseSignature base_signature)
		{
			this->rlslp_dictionary.decrease_relative_max_level(base_signature);
		}
		/**
		 * @brief Reset a base signature slot to null in the dictionary.
		 * @param base_signature Base signature index.
		 */
		void clear_element(BaseSignature base_signature)
		{
			this->rlslp_dictionary.clear_element(base_signature);
		}
		/**
		 * @brief Write rule body and metadata for a base signature.
		 * @param base_signature Base signature index.
		 * @param rule_body RLSLP rule body to store.
		 * @param new_level Absolute level of the base signature.
		 * @param relative_max_level Maximum relative level for the base signature.
		 * @param base_signature_length_list Base-signature length list (L).
		 */
		void write_element(BaseSignature base_signature, const RLSLPRuleBody &rule_body, uint16_t new_level, uint16_t relative_max_level, const std::vector<uint64_t> &base_signature_length_list)
		{
			this->rlslp_dictionary.write_element(base_signature, rule_body, new_level, relative_max_level, base_signature_length_list);
		}

		/**
		 * @brief Create random bits for a base signature (restricted block compression only).
		 * @param base_signature Base signature index whose single-signature count defines bit width.
		 */
		void create_random_bit(BaseSignature base_signature)
		{
			if (this->grammarParsingType == GrammarParsingType::RestrictedBlockCompression)
			{
				uint64_t single_count = this->rlslp_dictionary.get_relative_max_level_list()[base_signature];
				this->random_bit_dictionary.create_random_bit(base_signature, single_count);
			}
		}

		/**
		 * @brief Remove random bits for a signature.
		 * @param signature Encoded signature with relative level.
		 */
		void erase_random_bit(SignatureWithRelativeLevel signature)
		{
			if (this->grammarParsingType == GrammarParsingType::RestrictedBlockCompression)
			{
#ifdef DEBUG
				BaseSignature base_signature = SignatureFunctions::get_base_signature(signature);
				assert(this->rlslp_dictionary.get_relative_max_level_list()[base_signature] == SignatureFunctions::get_relative_level(signature));
#endif
				this->random_bit_dictionary.erase_random_bit(signature);
			}
		}

		/**
		 * @brief Print document counts to standard output.
		 */
		void print_documents() const
		{
			std::cout << "Documents: " << std::endl;
			for (auto it : this->documentCounter)
			{
				std::cout << it.first << " " << it.second << std::endl;
			}
		}
		/**
		 * @brief Print all grammar rules to standard output.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_info(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{
			this->rlslp_dictionary.print_info(this->grammarParsingType, message_paragraph);
		}
		/**
		 * @brief Print summary statistics to standard output.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_statistics(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{

			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(Grammar):" << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Compression Algorithm:\t" << (this->get_grammar_parsing_type() == dynRLSLP::GrammarParsingType::RestrictedBlockCompression ? "Restricted Recompression" : "Signature Encoding") << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Documents:\t" << this->get_document_counter().size() << std::endl;

			const DictionaryForLayeredRLSLP &rlslp_dictionary = this->get_rlslp_dictionary();
			const std::vector<uint64_t> &base_signature_length_list = rlslp_dictionary.get_base_signature_length_list();
			if (this->has_root())
			{
				std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The level of the derivation tree: " << this->get_rlslp_dictionary().get_base_signature_level_list()[this->get_root()] << std::endl;
				std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The length of the derivation tree: " << SignatureFunctions::get_length(this->get_root(), base_signature_length_list) << std::endl;
			}
			else
			{
				std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The grammar does not have a root." << std::endl;
			}
			rlslp_dictionary.print_statistics(this->grammarParsingType, message_paragraph + 1);
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
			// Code 0: GrammarParsingType grammarParsingType = GrammarParsingType::SignatureEncoding;
			// Code 4: std::map<int64_t, SignatureWithRelativeLevel> character_signature_item_map;
			// Code 5: std::vector<std::vector<SignatureWithRelativeLevel>> parentVectorList;
			// Code 5: std::unordered_map<SignatureWithRelativeLevel, std::set<SignatureWithRelativeLevel, NonterminalLessComparer>> parentMap;
			// Code 6: std::unordered_map<SignatureWithRelativeLevel, uint64_t> documentCounter;
			// Code 7: std::vector<SignatureWithRelativeLevel> unused_signatures;
			// Code 8: std::vector<uint8_t> randomBitList;

			// Code 0

			if (this->grammarParsingType != other.grammarParsingType)
			{
				std::cout << "this->grammarParsingType = " << (int)this->grammarParsingType << ", other.grammarParsingType = " << (int)other.grammarParsingType << std::endl;
				throw std::runtime_error("Error in verify_nearly_equal: The grammar parsing type must be equal.");
			}

			this->rlslp_dictionary.verify_nearly_equal(other.rlslp_dictionary);

			// Code 6
			if (this->documentCounter.size() != other.documentCounter.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of document counter must be equal.");
			}

			// Code 8
			if (this->grammarParsingType == GrammarParsingType::RestrictedBlockCompression)
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
			r.grammarParsingType = static_cast<GrammarParsingType>(typeValue);

			DictionaryForLayeredRLSLP rlslp_dictionary = DictionaryForLayeredRLSLP::load_from_file(ifs);
			r.rlslp_dictionary.swap(rlslp_dictionary);

			// Code 4
			SignatureWithRelativeLevel _document_signature;
			uint64_t _document_counter_size;
			ifs.read(reinterpret_cast<char *>(&_document_signature), sizeof(_document_signature));
			ifs.read(reinterpret_cast<char *>(&_document_counter_size), sizeof(_document_counter_size));

			if (r.rlslp_dictionary.base_signature_count() > 0)
			{
				r.documentCounter[_document_signature] = _document_counter_size;
			}

			// Code 6
			if (r.grammarParsingType == GrammarParsingType::RestrictedBlockCompression)
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
			if (item.documentCounter.size() > 1)
			{
				throw std::runtime_error("The size of document counter must be 0 or 1.");
			}

			int typeValue = static_cast<int>(item.grammarParsingType);
			os.write(reinterpret_cast<const char *>(&typeValue), sizeof(int));

			DictionaryForLayeredRLSLP::store_to_file(item.rlslp_dictionary, os);

			// Code 4
			if (item.documentCounter.size() == 1)
			{
				std::pair<SignatureWithRelativeLevel, uint64_t> document_counter_pair = {item.documentCounter.begin()->first, item.documentCounter.begin()->second};
				SignatureWithRelativeLevel document_counter_signature = document_counter_pair.first;
				uint64_t document_counter_count = document_counter_pair.second;
				os.write(reinterpret_cast<const char *>(&document_counter_signature), sizeof(SignatureWithRelativeLevel));
				os.write(reinterpret_cast<const char *>(&document_counter_count), sizeof(uint64_t));
			}
			else
			{
				SignatureWithRelativeLevel document_counter_signature = 0;
				uint64_t document_counter_count = 0;
				os.write(reinterpret_cast<const char *>(&document_counter_signature), sizeof(SignatureWithRelativeLevel));
				os.write(reinterpret_cast<const char *>(&document_counter_count), sizeof(uint64_t));
			}

			// Code 6
			if (item.grammarParsingType == GrammarParsingType::RestrictedBlockCompression)
			{
				RandomBitDictionary::store_to_file(item.random_bit_dictionary, os);
			}
		}
	};

}
