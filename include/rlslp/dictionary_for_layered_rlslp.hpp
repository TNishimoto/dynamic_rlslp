#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "stool/include/all.hpp"
#include "./rlslp_rule_body.hpp"
#include "./rlslp_rule_info.hpp"
#include "./run_rule_body.hpp"

namespace dynRLSLP
{
	class DictionaryForLayeredRLSLP
	{
	private:
		std::vector<RLSLPRuleBody> base_signature_rule_list_; // D
		std::vector<uint16_t> base_signature_level_list_;	  // H2
		std::vector<uint64_t> base_signature_length_list_;	  // L
		std::vector<uint16_t> relative_max_level_list_;

	public:
		DictionaryForLayeredRLSLP()
		{
		}

		const std::vector<RLSLPRuleBody> &get_base_signature_rule_list() const
		{
			return this->base_signature_rule_list_;
		}
		const std::vector<uint16_t> &get_base_signature_level_list() const
		{
			return this->base_signature_level_list_;
		}
		const std::vector<uint64_t> &get_base_signature_length_list() const
		{
			return this->base_signature_length_list_;
		}
		const std::vector<uint16_t> &get_relative_max_level_list() const
		{
			return this->relative_max_level_list_;
		}
		uint64_t get_length(SignatureWithRelativeLevel sig) const
		{
			return SignatureFunctions::get_length(sig, this->base_signature_length_list_);
		}
		uint64_t get_level(SignatureWithRelativeLevel sig) const
		{
			return SignatureFunctions::get_level(sig, this->base_signature_level_list_);
		}
		RLSLPRuleBody get_rule_body(SignatureWithRelativeLevel sig) const
		{
			return RLSLPRuleBody::decodeRule(sig, this->base_signature_rule_list_);
		}

		uint64_t base_signature_count() const
		{
			return this->base_signature_rule_list_.size();
		}
		uint64_t get_single_signature_count(GrammarParsingType grammar_parsing_type, BaseSignature base_signature) const
		{
			RLSLPRuleBody rule_body = RLSLPRuleBody::decodeRule(base_signature, this->base_signature_rule_list_);
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
		 * @brief Return the size of the vector \p D
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
		 * @brief Return \p true if \p G contains the nonterminal \p v_{i}. Otherwise, return \p false.
		 */
		bool check_empty_item(SignatureWithRelativeLevel i) const
		{
			return RLSLPRuleBody::decodeRule(i, this->base_signature_rule_list_).get_type() == RLSLPRuleType::Null;
		}
		/**
		 * @brief Return the RLSLPRuleInfo of the nonterminal \p v_{i}
		 */
		RLSLPRuleInfo get_signature_info(int64_t i) const
		{
			uint64_t length = this->get_length(i);
			uint64_t level = this->get_level(i);
			return RLSLPRuleInfo(RLSLPRuleBody::decodeRule(i, this->base_signature_rule_list_), length, level);
		}

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
		 * @brief Return \p D[i]
		 */
		RLSLPRuleBody get_item(int64_t i) const
		{
			return RLSLPRuleBody::decodeRule(i, this->base_signature_rule_list_);
		}

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
		 * @brief Print the information of the grammar \p G
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
					RLSLPRuleBody rule_body = RLSLPRuleBody::decodeRule(sig, this->base_signature_rule_list_);
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

		void decrease_relative_max_level(BaseSignature base_signature)
		{
			assert(this->relative_max_level_list_[base_signature] > 0);
			this->relative_max_level_list_[base_signature]--;
		}
		void increase_relative_max_level(BaseSignature base_signature)
		{
			assert(this->relative_max_level_list_[base_signature] < UINT16_MAX);
			this->relative_max_level_list_[base_signature]++;
		}

		BaseSignature add_new_base_signature()
		{
			uint64_t new_number = this->base_signature_count();
			this->base_signature_rule_list_.push_back(RLSLPRuleBody::create_null_item());
			this->base_signature_length_list_.push_back(UINT64_MAX);
			this->base_signature_level_list_.push_back(UINT16_MAX);
			this->relative_max_level_list_.push_back(UINT16_MAX);
			return new_number;
		}
		void clear_element(BaseSignature base_signature)
		{
			assert(this->base_signature_rule_list_[base_signature].get_type() != RLSLPRuleType::Null);
			this->base_signature_rule_list_[base_signature] = RLSLPRuleBody::create_null_item();
			this->base_signature_length_list_[base_signature] = UINT64_MAX;
			this->base_signature_level_list_[base_signature] = UINT16_MAX;
			this->relative_max_level_list_[base_signature] = UINT16_MAX;
		}
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

		void clear()
		{
			this->base_signature_rule_list_.clear();
			this->base_signature_level_list_.clear();
			this->base_signature_length_list_.clear();
			this->relative_max_level_list_.clear();
		}

		void swap(DictionaryForLayeredRLSLP &other)
		{
			this->base_signature_rule_list_.swap(other.base_signature_rule_list_);
			this->base_signature_level_list_.swap(other.base_signature_level_list_);
			this->base_signature_length_list_.swap(other.base_signature_length_list_);
			this->relative_max_level_list_.swap(other.relative_max_level_list_);
		}

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
