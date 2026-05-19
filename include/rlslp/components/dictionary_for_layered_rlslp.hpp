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
		std::vector<RLSLPRuleBody> explicit_nonterminal_rule_list_; // D
		std::vector<uint16_t> explicit_nonterminal_level_list_;	  // H2
		std::vector<uint64_t> explicit_nonterminal_length_list_;	  // L
		std::vector<uint16_t> relative_max_level_list_;

	public:
		/**
		 * @brief Construct an empty layered dictionary.
		 */
		DictionaryForLayeredRLSLP()
		{
		}

		/**
		 * @brief Return the base-nonterminal rule list (D).
		 * @return Const reference to stored rule bodies.
		 */
		const std::vector<RLSLPRuleBody> &get_explicit_nonterminal_rule_list() const
		{
			return this->explicit_nonterminal_rule_list_;
		}
		/**
		 * @brief Return the base-nonterminal level list (H).
		 * @return Const reference to absolute derivation levels per base nonterminal.
		 */
		const std::vector<uint16_t> &get_explicit_nonterminal_level_list() const
		{
			return this->explicit_nonterminal_level_list_;
		}
		/**
		 * @brief Return the base-nonterminal length list (L).
		 * @return Const reference to expanded string lengths per base nonterminal.
		 */
		const std::vector<uint64_t> &get_explicit_nonterminal_length_list() const
		{
			return this->explicit_nonterminal_length_list_;
		}
		/**
		 * @brief Return the per-base-nonterminal relative maximum level list.
		 * @return Const reference to highest relative level index per base nonterminal.
		 */
		const std::vector<uint16_t> &get_relative_max_level_list() const
		{
			return this->relative_max_level_list_;
		}
		/**
		 * @brief Return the string length of a nonterminal using the base length list.
		 * @param sig Encoded nonterminal with relative level.
		 * @return Expanded string length of @p sig.
		 */
		uint64_t get_length(NonterminalWithRelativeLevel sig) const
		{
			return NonterminalFunctions::get_length(sig, this->explicit_nonterminal_length_list_);
		}
		/**
		 * @brief Return the absolute derivation level of a nonterminal.
		 * @param sig Encoded nonterminal with relative level.
		 * @return Absolute level from the base level list (H).
		 */
		uint64_t get_level(NonterminalWithRelativeLevel sig) const
		{
			return NonterminalFunctions::get_level(sig, this->explicit_nonterminal_level_list_);
		}
		/**
		 * @brief Return the decoded rule body for a nonterminal.
		 * @param sig Encoded nonterminal with relative level.
		 * @return Rule body obtained by decoding @p sig against D.
		 */
		RLSLPRuleBody get_rule_body(NonterminalWithRelativeLevel sig) const
		{
			return RLSLPRuleBody::decode_rule(sig, this->explicit_nonterminal_rule_list_);
		}

		/**
		 * @brief Return the number of base nonterminals.
		 * @return Size of the base-nonterminal rule list D.
		 */
		uint64_t explicit_nonterminal_count() const
		{
			return this->explicit_nonterminal_rule_list_.size();
		}
		/**
		 * @brief Return the number of single (non-base) nonterminals for a base nonterminal.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @param explicit_nonterminal Base nonterminal index.
		 * @return Count of relative-level nonterminals excluding the base slot; 0 if base is null.
		 */
		uint64_t get_single_nonterminal_count(GrammarParsingType grammar_parsing_type, ExplicitNonterminal explicit_nonterminal) const
		{
			RLSLPRuleBody rule_body = RLSLPRuleBody::decode_rule(explicit_nonterminal, this->explicit_nonterminal_rule_list_);
			if (rule_body.get_type() != RLSLPRuleType::Null)
			{
				if (grammar_parsing_type == GrammarParsingType::RestrictedBlockCompression)
				{
					uint64_t uncountable_nonterminal_count = NonterminalFunctions::count_uncountable_nonterminals(explicit_nonterminal, this->explicit_nonterminal_length_list_, this->explicit_nonterminal_level_list_);
					return this->relative_max_level_list_[explicit_nonterminal] - uncountable_nonterminal_count;
				}
				else
				{
					return this->relative_max_level_list_[explicit_nonterminal];
				}
			}
			else
			{
				return 0;
			}
		}
		/**
		 * @brief Return the total count of single nonterminals.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @return Sum of single-nonterminal counts over all base nonterminals.
		 */
		int64_t count_single_nonterminals(GrammarParsingType grammar_parsing_type) const
		{
			uint64_t sz = 0;
			for (ExplicitNonterminal explicit_nonterminal = 0; explicit_nonterminal < (ExplicitNonterminal)this->explicit_nonterminal_count(); explicit_nonterminal++)
			{
				sz += this->get_single_nonterminal_count(grammar_parsing_type, explicit_nonterminal);
			}
			return sz;
		}
		/**
		 * @brief Return the total number of nonterminals (base plus single).
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @return Total nonterminal count across all base nonterminals.
		 */
		uint64_t nonterminal_count(GrammarParsingType grammar_parsing_type) const
		{
			uint64_t sz = 0;
			for (ExplicitNonterminal explicit_nonterminal = 0; explicit_nonterminal < (ExplicitNonterminal)this->explicit_nonterminal_count(); explicit_nonterminal++)
			{
				sz += 1 + this->get_single_nonterminal_count(grammar_parsing_type, explicit_nonterminal);
			}
			return sz;
		}
		/**
		 * @brief Return the number of null base nonterminals.
		 * @return Count of base nonterminals whose rule type is Null.
		 */
		uint64_t count_null_nonterminals() const
		{
			uint64_t counter = 0;
			for (uint64_t i = 0; i < this->explicit_nonterminal_count(); i++)
			{
				if (this->explicit_nonterminal_rule_list_[i].get_type() == RLSLPRuleType::Null)
				{
					counter++;
				}
			}
			return counter;
		}
		/**
		 * @brief Return whether the nonterminal denotes a null rule.
		 * @param i Nonterminal or base-nonterminal index.
		 * @return True if the decoded rule at @p i is of type Null.
		 */
		bool check_empty_item(NonterminalWithRelativeLevel i) const
		{
			return RLSLPRuleBody::decode_rule(i, this->explicit_nonterminal_rule_list_).get_type() == RLSLPRuleType::Null;
		}
		/**
		 * @brief Return rule body, length, and level for a nonterminal index.
		 * @param i Encoded nonterminal or base-nonterminal index.
		 * @return Aggregated RLSLPRuleInfo for @p i.
		 */
		RLSLPRuleInfo get_nonterminal_info(int64_t i) const
		{
			uint64_t length = this->get_length(i);
			uint64_t level = this->get_level(i);
			return RLSLPRuleInfo(RLSLPRuleBody::decode_rule(i, this->explicit_nonterminal_rule_list_), length, level);
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
			if (this->explicit_nonterminal_rule_list_.size() != other.explicit_nonterminal_rule_list_.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of nonterminal items must be equal.");
			}
			for (uint64_t i = 0; i < this->explicit_nonterminal_rule_list_.size(); i++)
			{
				if (this->explicit_nonterminal_rule_list_[i].get_type() != other.explicit_nonterminal_rule_list_[i].get_type())
				{
					throw std::runtime_error("Error in verify_nearly_equal: The type of nonterminal items must be equal.");
				}
			}

			// Code 2
			if (this->explicit_nonterminal_level_list_.size() != other.explicit_nonterminal_level_list_.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of level list must be equal.");
			}
			for (uint64_t i = 0; i < this->explicit_nonterminal_level_list_.size(); i++)
			{
				if (this->explicit_nonterminal_level_list_[i] != other.explicit_nonterminal_level_list_[i])
				{
					throw std::runtime_error("Error in verify_nearly_equal: The level of nonterminal items must be equal.");
				}
			}

			// Code 3
			if (this->explicit_nonterminal_length_list_.size() != other.explicit_nonterminal_length_list_.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of length list must be equal.");
			}
			for (uint64_t i = 0; i < this->explicit_nonterminal_length_list_.size(); i++)
			{
				if (this->explicit_nonterminal_length_list_[i] != other.explicit_nonterminal_length_list_[i])
				{
					throw std::runtime_error("Error in verify_nearly_equal: The length of nonterminal items must be equal.");
				}
			}

			// Code X
			if (this->relative_max_level_list_.size() != other.relative_max_level_list_.size())
			{
				throw std::runtime_error("Error in verify_nearly_equal: The size of single nonterminal count list must be equal.");
			}
			for (uint64_t i = 0; i < this->relative_max_level_list_.size(); i++)
			{
				if (this->relative_max_level_list_[i] != other.relative_max_level_list_[i])
				{
					throw std::runtime_error("Error in verify_nearly_equal: The single nonterminal count must be equal.");
				}
			}
			return true;
		}

		/**
		 * @brief Return the decoded rule body for a nonterminal index.
		 * @param i Nonterminal or base-nonterminal index.
		 * @return Decoded rule body at index @p i (alias for decode_rule).
		 */
		RLSLPRuleBody get_item(int64_t i) const
		{
			return RLSLPRuleBody::decode_rule(i, this->explicit_nonterminal_rule_list_);
		}

		/**
		 * @brief Print summary dictionary statistics to standard output.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_statistics(GrammarParsingType grammar_parsing_type, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{
			uint64_t nonterminal_count = this->nonterminal_count(grammar_parsing_type);
			uint64_t explicit_nonterminal_count = this->explicit_nonterminal_count();
			uint64_t single_nonterminal_count = this->count_single_nonterminals(grammar_parsing_type);
			uint64_t null_nonterminal_count = this->count_null_nonterminals();
			uint64_t nonnull_nonterminal_count = nonterminal_count - null_nonterminal_count;

			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(Dictionary):" << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Nonterminals:           \t" << nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Non-null Nonterminals:  \t" << nonnull_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Base Nonterminals:      \t" << explicit_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Single Nonterminals:    \t" << single_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Null Nonterminals:      \t" << null_nonterminal_count << std::endl;
		}
		/**
		 * @brief Print detailed dictionary statistics.
		 * @param grammar_parsing_type Grammar parsing algorithm type.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_detailed_statistics(GrammarParsingType grammar_parsing_type, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{

			uint64_t nonterminal_count = this->nonterminal_count(grammar_parsing_type);
			uint64_t explicit_nonterminal_count = this->explicit_nonterminal_count();
			uint64_t single_nonterminal_count = this->count_single_nonterminals(grammar_parsing_type);
			uint64_t null_nonterminal_count = this->count_null_nonterminals();
			uint64_t nonnull_nonterminal_count = nonterminal_count - null_nonterminal_count;

			std::unordered_map<uint64_t, uint64_t> nonterminal_length_map;

			for (uint64_t i = 0; i < explicit_nonterminal_count; i++)
			{
				NonterminalWithRelativeLevel sig = NonterminalFunctions::get_nonterminal(0, i);
				uint64_t length = this->get_length(sig);
				if (nonterminal_length_map.find(length) == nonterminal_length_map.end())
				{
					nonterminal_length_map[length] = 0;
				}
				nonterminal_length_map[length]++;
			}
			std::vector<std::pair<uint64_t, uint64_t>> nonterminal_length_vector;
			for (auto &[length, count] : nonterminal_length_map)
			{
				nonterminal_length_vector.push_back({length, count});
			}
			std::sort(nonterminal_length_vector.begin(), nonterminal_length_vector.end(), [](const std::pair<uint64_t, uint64_t> &a, const std::pair<uint64_t, uint64_t> &b)
					  { return a.first < b.first; });

			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(Dictionary):" << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Nonterminals:           \t" << nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Non-null Nonterminals:  \t" << nonnull_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Base Nonterminals:      \t" << explicit_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Single Nonterminals:    \t" << single_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Null Nonterminals:      \t" << null_nonterminal_count << std::endl;

			for (auto &[length, count] : nonterminal_length_vector)
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
			for (int64_t i = 0; i < (int64_t)this->explicit_nonterminal_count(); i++)
			{
				uint64_t uncountable_nonterminal_count = NonterminalFunctions::count_uncountable_nonterminals(i, this->explicit_nonterminal_length_list_, this->explicit_nonterminal_level_list_);

				for (uint64_t j = 0; j <= this->get_relative_max_level_list()[i]; j++)
				{
					if (grammar_parsing_type == GrammarParsingType::RestrictedBlockCompression && j > 0 && j <= uncountable_nonterminal_count)
					{
						continue;
					}
					NonterminalWithRelativeLevel sig = NonterminalFunctions::get_nonterminal(j, i);
					RLSLPRuleBody rule_body = RLSLPRuleBody::decode_rule(sig, this->explicit_nonterminal_rule_list_);
					if (rule_body.get_type() != RLSLPRuleType::Null)
					{
						std::cout << stool::Message::get_paragraph_string(message_paragraph) << x << ": " << rule_body.get_detailed_info(i, j) << ",\t level = "
								  << (int)NonterminalFunctions::get_level(sig, this->explicit_nonterminal_level_list_) << ",\t string length = "
								  << NonterminalFunctions::get_length(sig, this->explicit_nonterminal_length_list_) << std::endl;
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
		 * @brief Decrement the relative maximum level of a base nonterminal.
		 * @param explicit_nonterminal Base nonterminal index.
		 */
		void decrease_relative_max_level(ExplicitNonterminal explicit_nonterminal)
		{
			assert(this->relative_max_level_list_[explicit_nonterminal] > 0);
			this->relative_max_level_list_[explicit_nonterminal]--;
		}
		/**
		 * @brief Increment the relative maximum level of a base nonterminal.
		 * @param explicit_nonterminal Base nonterminal index.
		 */
		void increase_relative_max_level(ExplicitNonterminal explicit_nonterminal)
		{
			assert(this->relative_max_level_list_[explicit_nonterminal] < UINT16_MAX);
			this->relative_max_level_list_[explicit_nonterminal]++;
		}

		/**
		 * @brief Allocate a new base nonterminal slot initialized to null.
		 * @return Index of the newly added base nonterminal.
		 */
		ExplicitNonterminal add_new_explicit_nonterminal()
		{
			uint64_t new_number = this->explicit_nonterminal_count();
			this->explicit_nonterminal_rule_list_.push_back(RLSLPRuleBody::create_null_item());
			this->explicit_nonterminal_length_list_.push_back(UINT64_MAX);
			this->explicit_nonterminal_level_list_.push_back(UINT16_MAX);
			this->relative_max_level_list_.push_back(UINT16_MAX);
			return new_number;
		}
		/**
		 * @brief Reset a base nonterminal slot to null.
		 * @param explicit_nonterminal Base nonterminal index.
		 */
		void clear_element(ExplicitNonterminal explicit_nonterminal)
		{
			assert(this->explicit_nonterminal_rule_list_[explicit_nonterminal].get_type() != RLSLPRuleType::Null);
			this->explicit_nonterminal_rule_list_[explicit_nonterminal] = RLSLPRuleBody::create_null_item();
			this->explicit_nonterminal_length_list_[explicit_nonterminal] = UINT64_MAX;
			this->explicit_nonterminal_level_list_[explicit_nonterminal] = UINT16_MAX;
			this->relative_max_level_list_[explicit_nonterminal] = UINT16_MAX;
		}
		/**
		 * @brief Write rule body and metadata into a previously null base nonterminal slot.
		 * @param explicit_nonterminal Base nonterminal index (must currently be null).
		 * @param rule_body RLSLP rule body to store.
		 * @param new_level Absolute level of the base nonterminal.
		 * @param relative_max_level Maximum relative level for the base nonterminal.
		 * @param explicit_nonterminal_length_list Length list used to compute stored length.
		 */
		void write_element(ExplicitNonterminal explicit_nonterminal, const RLSLPRuleBody &rule_body, uint16_t new_level, uint16_t relative_max_level, const std::vector<uint64_t> &explicit_nonterminal_length_list)
		{
			assert(this->explicit_nonterminal_rule_list_[explicit_nonterminal].get_type() == RLSLPRuleType::Null);
			assert(this->explicit_nonterminal_length_list_[explicit_nonterminal] == UINT64_MAX);
			assert(this->explicit_nonterminal_level_list_[explicit_nonterminal] == UINT16_MAX);
			assert(this->relative_max_level_list_[explicit_nonterminal] == UINT16_MAX);

			this->explicit_nonterminal_rule_list_[explicit_nonterminal] = rule_body;
			this->explicit_nonterminal_level_list_[explicit_nonterminal] = new_level;
			this->explicit_nonterminal_length_list_[explicit_nonterminal] = rule_body.get_length(explicit_nonterminal_length_list);
			this->relative_max_level_list_[explicit_nonterminal] = relative_max_level;

#ifdef DEBUG
			if (rule_body.get_type() == RLSLPRuleType::Pair)
			{
				assert(NonterminalFunctions::get_explicit_nonterminal(rule_body.A) < this->explicit_nonterminal_rule_list_.size());
				assert(NonterminalFunctions::get_explicit_nonterminal(rule_body.B) < this->explicit_nonterminal_rule_list_.size());
			}
			else if (rule_body.get_type() == RLSLPRuleType::Power)
			{
				assert(NonterminalFunctions::get_explicit_nonterminal(rule_body.A) < this->explicit_nonterminal_rule_list_.size());
			}
			else if (rule_body.get_type() == RLSLPRuleType::Nonterminal)
			{
				assert(NonterminalFunctions::get_explicit_nonterminal(rule_body.A) < this->explicit_nonterminal_rule_list_.size());
			}
#endif
		}

		/**
		 * @brief Clear all stored data.
		 */
		void clear()
		{
			this->explicit_nonterminal_rule_list_.clear();
			this->explicit_nonterminal_level_list_.clear();
			this->explicit_nonterminal_length_list_.clear();
			this->relative_max_level_list_.clear();
		}

		/**
		 * @brief Swap contents with another dictionary.
		 * @param other Dictionary to swap with.
		 */
		void swap(DictionaryForLayeredRLSLP &other)
		{
			this->explicit_nonterminal_rule_list_.swap(other.explicit_nonterminal_rule_list_);
			this->explicit_nonterminal_level_list_.swap(other.explicit_nonterminal_level_list_);
			this->explicit_nonterminal_length_list_.swap(other.explicit_nonterminal_length_list_);
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
			uint64_t _nonterminal_vec_size;
			ifs.read(reinterpret_cast<char *>(&_nonterminal_vec_size), sizeof(_nonterminal_vec_size));
			r.explicit_nonterminal_rule_list_.resize(_nonterminal_vec_size);
			ifs.read(reinterpret_cast<char *>(r.explicit_nonterminal_rule_list_.data()), sizeof(RLSLPRuleBody) * _nonterminal_vec_size);
			r.explicit_nonterminal_level_list_.resize(_nonterminal_vec_size);
			ifs.read(reinterpret_cast<char *>(r.explicit_nonterminal_level_list_.data()), sizeof(uint16_t) * _nonterminal_vec_size);
			r.explicit_nonterminal_length_list_.resize(_nonterminal_vec_size);
			ifs.read(reinterpret_cast<char *>(r.explicit_nonterminal_length_list_.data()), sizeof(uint64_t) * _nonterminal_vec_size);
			r.relative_max_level_list_.resize(_nonterminal_vec_size);
			ifs.read(reinterpret_cast<char *>(r.relative_max_level_list_.data()), sizeof(uint16_t) * _nonterminal_vec_size);

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
			uint64_t nonterminal_vec_size = item.explicit_nonterminal_rule_list_.size();
			os.write(reinterpret_cast<const char *>(&nonterminal_vec_size), sizeof(uint64_t));
			os.write(reinterpret_cast<const char *>(item.explicit_nonterminal_rule_list_.data()), sizeof(RLSLPRuleBody) * nonterminal_vec_size);
			os.write(reinterpret_cast<const char *>(item.explicit_nonterminal_level_list_.data()), sizeof(uint16_t) * nonterminal_vec_size);
			os.write(reinterpret_cast<const char *>(item.explicit_nonterminal_length_list_.data()), sizeof(uint64_t) * nonterminal_vec_size);
			os.write(reinterpret_cast<const char *>(item.relative_max_level_list_.data()), sizeof(uint16_t) * nonterminal_vec_size);
		}
	};

}
