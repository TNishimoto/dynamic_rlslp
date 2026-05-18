#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "stool/include/all.hpp"
#include "../rules/rlslp_rule_body.hpp"
#include "../rules/rlslp_rule_info.hpp"
#include "../rules/run_rule_body.hpp"

namespace dynRLSLP
{
	/**
	 * @brief Core dictionary (D, H, L) for a layered RLSLP.
	 * @ingroup RLSLPClasses
	 */
	class DictionaryForLayeredRLSLP
	{
	private:
		std::vector<RLSLPRuleBody> base_signature_rule_list_; // D
		std::vector<uint16_t> base_signature_level_list_;	  // H2
		std::vector<uint64_t> base_signature_length_list_;	  // L
		std::vector<uint16_t> relative_max_level_list_;

	public:
		/**
		 * @brief Construct an empty layered dictionary.
		 */
		DictionaryForLayeredRLSLP()
		{
		}

		/**
		 * @brief Return the base-signature rule list (D).
		 * @return Const reference to stored rule bodies.
		 */
		const std::vector<RLSLPRuleBody> &get_base_signature_rule_list() const
		{
			return this->base_signature_rule_list_;
		}
		/**
		 * @brief Return the base-signature level list (H).
		 * @return Const reference to absolute derivation levels per base signature.
		 */
		const std::vector<uint16_t> &get_base_signature_level_list() const
		{
			return this->base_signature_level_list_;
		}
		/**
		 * @brief Return the base-signature length list (L).
		 * @return Const reference to expanded string lengths per base signature.
		 */
		const std::vector<uint64_t> &get_base_signature_length_list() const
		{
			return this->base_signature_length_list_;
		}
		/**
		 * @brief Return the per-base-signature relative maximum level list.
		 * @return Const reference to highest relative level index per base signature.
		 */
		const std::vector<uint16_t> &get_relative_max_level_list() const
		{
			return this->relative_max_level_list_;
		}
		/**
		 * @brief Return the string length of a signature using the base length list.
		 * @param sig Encoded signature with relative level.
		 * @return Expanded string length of @p sig.
		 */
		uint64_t get_length(SignatureWithRelativeLevel sig) const
		{
			return SignatureFunctions::get_length(sig, this->base_signature_length_list_);
		}
		/**
		 * @brief Return the absolute derivation level of a signature.
		 * @param sig Encoded signature with relative level.
		 * @return Absolute level from the base level list (H).
		 */
		uint64_t get_level(SignatureWithRelativeLevel sig) const
		{
			return SignatureFunctions::get_level(sig, this->base_signature_level_list_);
		}
		/**
		 * @brief Return the decoded rule body for a signature.
		 * @param sig Encoded signature with relative level.
		 * @return Rule body obtained by decoding @p sig against D.
		 */
		RLSLPRuleBody get_rule_body(SignatureWithRelativeLevel sig) const
		{
			return RLSLPRuleBody::decode_rule(sig, this->base_signature_rule_list_);
		}

		/**
		 * @brief Return the number of base signatures.
		 * @return Size of the base-signature rule list D.
		 */
		uint64_t base_signature_count() const
		{
			return this->base_signature_rule_list_.size();
		}
		/**
		 * @brief Return the number of single (non-base) signatures for a base signature.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @param base_signature Base signature index.
		 * @return Count of relative-level signatures excluding the base slot; 0 if base is null.
		 */
		uint64_t get_single_signature_count(GrammarParsingType grammar_parsing_type, BaseSignature base_signature) const
		{
			RLSLPRuleBody rule_body = RLSLPRuleBody::decode_rule(base_signature, this->base_signature_rule_list_);
			if (rule_body.get_type() != RLSLPRuleType::Null)
			{
				if (grammar_parsing_type == GrammarParsingType::RestrictedBlockCompression)
				{
					uint64_t uncountable_signature_count = SignatureFunctions::count_uncountable_signatures(base_signature, this->base_signature_length_list_, this->base_signature_level_list_);
					return this->relative_max_level_list_[base_signature] - uncountable_signature_count;
				}
				else
				{
					return this->relative_max_level_list_[base_signature];
				}
			}
			else
			{
				return 0;
			}
		}
		/**
		 * @brief Return the total count of single signatures.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @return Sum of single-signature counts over all base signatures.
		 */
		int64_t count_single_signatures(GrammarParsingType grammar_parsing_type) const
		{
			uint64_t sz = 0;
			for (BaseSignature base_signature = 0; base_signature < (BaseSignature)this->base_signature_count(); base_signature++)
			{
				sz += this->get_single_signature_count(grammar_parsing_type, base_signature);
			}
			return sz;
		}
		/**
		 * @brief Return the total number of signatures (base plus single).
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @return Total signature count across all base signatures.
		 */
		uint64_t signature_count(GrammarParsingType grammar_parsing_type) const
		{
			uint64_t sz = 0;
			for (BaseSignature base_signature = 0; base_signature < (BaseSignature)this->base_signature_count(); base_signature++)
			{
				sz += 1 + this->get_single_signature_count(grammar_parsing_type, base_signature);
			}
			return sz;
		}
		/**
		 * @brief Return the number of null base signatures.
		 * @return Count of base signatures whose rule type is Null.
		 */
		uint64_t count_null_signatures() const
		{
			uint64_t counter = 0;
			for (uint64_t i = 0; i < this->base_signature_count(); i++)
			{
				if (this->base_signature_rule_list_[i].get_type() == RLSLPRuleType::Null)
				{
					counter++;
				}
			}
			return counter;
		}
		/**
		 * @brief Return whether the signature denotes a null rule.
		 * @param i Signature or base-signature index.
		 * @return True if the decoded rule at @p i is of type Null.
		 */
		bool check_empty_item(SignatureWithRelativeLevel i) const
		{
			return RLSLPRuleBody::decode_rule(i, this->base_signature_rule_list_).get_type() == RLSLPRuleType::Null;
		}
		/**
		 * @brief Return rule body, length, and level for a signature index.
		 * @param i Encoded signature or base-signature index.
		 * @return Aggregated RLSLPRuleInfo for @p i.
		 */
		RLSLPRuleInfo get_signature_info(int64_t i) const
		{
			uint64_t length = this->get_length(i);
			uint64_t level = this->get_level(i);
			return RLSLPRuleInfo(RLSLPRuleBody::decode_rule(i, this->base_signature_rule_list_), length, level);
		}

		/**
		 * @brief Verify structural near-equality with another dictionary.
		 * @param other Dictionary to compare against.
		 * @return True if rule lists, levels, lengths, and relative max levels match.
		 * @throws std::runtime_error on mismatch.
		 */
		bool verify_nearly_equal(const DictionaryForLayeredRLSLP &other) const
		{
			// Code 1
			if (this->base_signature_rule_list_.size() != other.base_signature_rule_list_.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of signature items must be equal.");
			}
			for (uint64_t i = 0; i < this->base_signature_rule_list_.size(); i++)
			{
				if (this->base_signature_rule_list_[i].get_type() != other.base_signature_rule_list_[i].get_type())
				{
					throw std::runtime_error("Error in verify_nearly_equal: The type of signature items must be equal.");
				}
			}

			// Code 2
			if (this->base_signature_level_list_.size() != other.base_signature_level_list_.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of level list must be equal.");
			}
			for (uint64_t i = 0; i < this->base_signature_level_list_.size(); i++)
			{
				if (this->base_signature_level_list_[i] != other.base_signature_level_list_[i])
				{
					throw std::runtime_error("Error in verify_nearly_equal: The level of signature items must be equal.");
				}
			}

			// Code 3
			if (this->base_signature_length_list_.size() != other.base_signature_length_list_.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of length list must be equal.");
			}
			for (uint64_t i = 0; i < this->base_signature_length_list_.size(); i++)
			{
				if (this->base_signature_length_list_[i] != other.base_signature_length_list_[i])
				{
					throw std::runtime_error("Error in verify_nearly_equal: The length of signature items must be equal.");
				}
			}

			// Code X
			if (this->relative_max_level_list_.size() != other.relative_max_level_list_.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of single signature count list must be equal.");
			}
			for (uint64_t i = 0; i < this->relative_max_level_list_.size(); i++)
			{
				if (this->relative_max_level_list_[i] != other.relative_max_level_list_[i])
				{
					throw std::runtime_error("Error in verify_nearly_equal: The single signature count must be equal.");
				}
			}
			return true;
		}

		/**
		 * @brief Return the decoded rule body for a signature index.
		 * @param i Signature or base-signature index.
		 * @return Decoded rule body at index @p i (alias for decode_rule).
		 */
		RLSLPRuleBody get_item(int64_t i) const
		{
			return RLSLPRuleBody::decode_rule(i, this->base_signature_rule_list_);
		}

		/**
		 * @brief Print summary dictionary statistics to standard output.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_statistics(GrammarParsingType grammar_parsing_type, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{
			uint64_t signature_count = this->signature_count(grammar_parsing_type);
			uint64_t base_signature_count = this->base_signature_count();
			uint64_t single_signature_count = this->count_single_signatures(grammar_parsing_type);
			uint64_t null_signature_count = this->count_null_signatures();
			uint64_t nonnull_signature_count = signature_count - null_signature_count;

			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(Dictionary):" << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Signatures:           \t" << signature_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Non-null Signatures:  \t" << nonnull_signature_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Base Signatures:      \t" << base_signature_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Single Signatures:    \t" << single_signature_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Null Signatures:      \t" << null_signature_count << std::endl;
		}
		/**
		 * @brief Print detailed dictionary statistics.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_detailed_statistics(GrammarParsingType grammar_parsing_type, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{

			uint64_t signature_count = this->signature_count(grammar_parsing_type);
			uint64_t base_signature_count = this->base_signature_count();
			uint64_t single_signature_count = this->count_single_signatures(grammar_parsing_type);
			uint64_t null_signature_count = this->count_null_signatures();
			uint64_t nonnull_signature_count = signature_count - null_signature_count;

			std::unordered_map<uint64_t, uint64_t> signature_length_map;

			for (uint64_t i = 0; i < base_signature_count; i++)
			{
				SignatureWithRelativeLevel sig = SignatureFunctions::get_signature(0, i);
				uint64_t length = this->get_length(sig);
				if (signature_length_map.find(length) == signature_length_map.end())
				{
					signature_length_map[length] = 0;
				}
				signature_length_map[length]++;
			}
			std::vector<std::pair<uint64_t, uint64_t>> signature_length_vector;
			for (auto &[length, count] : signature_length_map)
			{
				signature_length_vector.push_back({length, count});
			}
			std::sort(signature_length_vector.begin(), signature_length_vector.end(), [](const std::pair<uint64_t, uint64_t> &a, const std::pair<uint64_t, uint64_t> &b)
					  { return a.first < b.first; });

			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(Dictionary):" << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Signatures:           \t" << signature_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Non-null Signatures:  \t" << nonnull_signature_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Base Signatures:      \t" << base_signature_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Single Signatures:    \t" << single_signature_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Null Signatures:      \t" << null_signature_count << std::endl;

			for (auto &[length, count] : signature_length_vector)
			{
				if (count > 3)
				{
					std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Length: " << length << ", Count: " << count << std::endl;
				}
			}
		}

		/**
		 * @brief Print detailed information to standard output.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_info(GrammarParsingType grammar_parsing_type, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{

			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== Rules ====" << std::endl;
			uint64_t x = 0;
			for (int64_t i = 0; i < (int64_t)this->base_signature_count(); i++)
			{
				uint64_t uncountable_signature_count = SignatureFunctions::count_uncountable_signatures(i, this->base_signature_length_list_, this->base_signature_level_list_);

				for (uint64_t j = 0; j <= this->get_relative_max_level_list()[i]; j++)
				{
					if (grammar_parsing_type == GrammarParsingType::RestrictedBlockCompression && j > 0 && j <= uncountable_signature_count)
					{
						continue;
					}
					SignatureWithRelativeLevel sig = SignatureFunctions::get_signature(j, i);
					RLSLPRuleBody rule_body = RLSLPRuleBody::decode_rule(sig, this->base_signature_rule_list_);
					if (rule_body.get_type() != RLSLPRuleType::Null)
					{
						std::cout << stool::Message::get_paragraph_string(message_paragraph) << x << ": " << rule_body.get_detailed_info(i, j) << ",\t level = "
								  << (int)SignatureFunctions::get_level(sig, this->base_signature_level_list_) << ",\t string length = "
								  << SignatureFunctions::get_length(sig, this->base_signature_length_list_) << std::endl;
						x++;
					}
					else
					{
						std::cout << stool::Message::get_paragraph_string(message_paragraph) << x << ": " << rule_body.get_detailed_info(i, j) << std::endl;
						x++;
						break;
					}
				}
			}
			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "==== Rules[END] ====" << std::endl;
		}

		/**
		 * @brief Decrement the relative maximum level of a base signature.
		 * @param base_signature Base signature index.
		 */
		void decrease_relative_max_level(BaseSignature base_signature)
		{
			assert(this->relative_max_level_list_[base_signature] > 0);
			this->relative_max_level_list_[base_signature]--;
		}
		/**
		 * @brief Increment the relative maximum level of a base signature.
		 * @param base_signature Base signature index.
		 */
		void increase_relative_max_level(BaseSignature base_signature)
		{
			assert(this->relative_max_level_list_[base_signature] < UINT16_MAX);
			this->relative_max_level_list_[base_signature]++;
		}

		/**
		 * @brief Allocate a new base signature slot initialized to null.
		 * @return Index of the newly added base signature.
		 */
		BaseSignature add_new_base_signature()
		{
			uint64_t new_number = this->base_signature_count();
			this->base_signature_rule_list_.push_back(RLSLPRuleBody::create_null_item());
			this->base_signature_length_list_.push_back(UINT64_MAX);
			this->base_signature_level_list_.push_back(UINT16_MAX);
			this->relative_max_level_list_.push_back(UINT16_MAX);
			return new_number;
		}
		/**
		 * @brief Reset a base signature slot to null.
		 * @param base_signature Base signature index.
		 */
		void clear_element(BaseSignature base_signature)
		{
			assert(this->base_signature_rule_list_[base_signature].get_type() != RLSLPRuleType::Null);
			this->base_signature_rule_list_[base_signature] = RLSLPRuleBody::create_null_item();
			this->base_signature_length_list_[base_signature] = UINT64_MAX;
			this->base_signature_level_list_[base_signature] = UINT16_MAX;
			this->relative_max_level_list_[base_signature] = UINT16_MAX;
		}
		/**
		 * @brief Write rule body and metadata into a previously null base signature slot.
		 * @param base_signature Base signature index (must currently be null).
		 * @param rule_body RLSLP rule body to store.
		 * @param new_level Absolute level of the base signature.
		 * @param relative_max_level Maximum relative level for the base signature.
		 * @param base_signature_length_list Length list used to compute stored length.
		 */
		void write_element(BaseSignature base_signature, const RLSLPRuleBody &rule_body, uint16_t new_level, uint16_t relative_max_level, const std::vector<uint64_t> &base_signature_length_list)
		{
			assert(this->base_signature_rule_list_[base_signature].get_type() == RLSLPRuleType::Null);
			assert(this->base_signature_length_list_[base_signature] == UINT64_MAX);
			assert(this->base_signature_level_list_[base_signature] == UINT16_MAX);
			assert(this->relative_max_level_list_[base_signature] == UINT16_MAX);

			this->base_signature_rule_list_[base_signature] = rule_body;
			this->base_signature_level_list_[base_signature] = new_level;
			this->base_signature_length_list_[base_signature] = rule_body.get_length(base_signature_length_list);
			this->relative_max_level_list_[base_signature] = relative_max_level;

#ifdef DEBUG
			if (rule_body.get_type() == RLSLPRuleType::Pair)
			{
				assert(SignatureFunctions::get_base_signature(rule_body.A) < this->base_signature_rule_list_.size());
				assert(SignatureFunctions::get_base_signature(rule_body.B) < this->base_signature_rule_list_.size());
			}
			else if (rule_body.get_type() == RLSLPRuleType::Power)
			{
				assert(SignatureFunctions::get_base_signature(rule_body.A) < this->base_signature_rule_list_.size());
			}
			else if (rule_body.get_type() == RLSLPRuleType::Signature)
			{
				assert(SignatureFunctions::get_base_signature(rule_body.A) < this->base_signature_rule_list_.size());
			}
#endif
		}

		/**
		 * @brief Clear all stored data.
		 */
		void clear()
		{
			this->base_signature_rule_list_.clear();
			this->base_signature_level_list_.clear();
			this->base_signature_length_list_.clear();
			this->relative_max_level_list_.clear();
		}

		/**
		 * @brief Swap contents with another dictionary.
		 * @param other Dictionary to swap with.
		 */
		void swap(DictionaryForLayeredRLSLP &other)
		{
			this->base_signature_rule_list_.swap(other.base_signature_rule_list_);
			this->base_signature_level_list_.swap(other.base_signature_level_list_);
			this->base_signature_length_list_.swap(other.base_signature_length_list_);
			this->relative_max_level_list_.swap(other.relative_max_level_list_);
		}

		/**
		 * @brief Deserialize from a binary input stream.
		 * @param ifs Input file stream.
		 * @return Dictionary loaded from the binary stream.
		 */
		static DictionaryForLayeredRLSLP load_from_file(std::ifstream &ifs)
		{
			/*
			Checklist
			*/

			DictionaryForLayeredRLSLP r;

			// Code 1
			uint64_t _signature_vec_size;
			ifs.read(reinterpret_cast<char *>(&_signature_vec_size), sizeof(_signature_vec_size));
			r.base_signature_rule_list_.resize(_signature_vec_size);
			ifs.read(reinterpret_cast<char *>(r.base_signature_rule_list_.data()), sizeof(RLSLPRuleBody) * _signature_vec_size);
			r.base_signature_level_list_.resize(_signature_vec_size);
			ifs.read(reinterpret_cast<char *>(r.base_signature_level_list_.data()), sizeof(uint16_t) * _signature_vec_size);
			r.base_signature_length_list_.resize(_signature_vec_size);
			ifs.read(reinterpret_cast<char *>(r.base_signature_length_list_.data()), sizeof(uint64_t) * _signature_vec_size);
			r.relative_max_level_list_.resize(_signature_vec_size);
			ifs.read(reinterpret_cast<char *>(r.relative_max_level_list_.data()), sizeof(uint16_t) * _signature_vec_size);

			return r;
		}

		/**
		 * @brief Serialize to a binary output stream.
		 * @param item Dictionary to write.
		 * @param os Output file stream.
		 */
		static void store_to_file(const DictionaryForLayeredRLSLP &item, std::ofstream &os)
		{
			// Code 1
			uint64_t signature_vec_size = item.base_signature_rule_list_.size();
			os.write(reinterpret_cast<const char *>(&signature_vec_size), sizeof(uint64_t));
			os.write(reinterpret_cast<const char *>(item.base_signature_rule_list_.data()), sizeof(RLSLPRuleBody) * signature_vec_size);
			os.write(reinterpret_cast<const char *>(item.base_signature_level_list_.data()), sizeof(uint16_t) * signature_vec_size);
			os.write(reinterpret_cast<const char *>(item.base_signature_length_list_.data()), sizeof(uint64_t) * signature_vec_size);
			os.write(reinterpret_cast<const char *>(item.relative_max_level_list_.data()), sizeof(uint16_t) * signature_vec_size);
		}
	};

}
