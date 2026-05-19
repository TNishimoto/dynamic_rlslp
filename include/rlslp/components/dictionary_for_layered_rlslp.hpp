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
	 * @brief Core dictionary (D, Y, L, R) for @ref term_layered_rlslp "Layered RLSLP".
	 * This class stores four vector arrays D, Y, L, R.
	 * - std::vector<RLSLPRuleBody> D[i] stores the rule body of the explicit nonterminal *i*.
	 * - std::vector<uint16_t> Y[i] stores the level of the explicit nonterminal *i*.
	 * - std::vector<uint64_t> L[i] stores the length of the string derived from the explicit nonterminal *i*.
	 * - std::vector<uint16_t> R[i] stores the largest integer *d* such that NonterminalWithRelativeLevel (i, d).
	 * @ingroup RLSLPClasses
	 */
	class DictionaryForLayeredRLSLP
	{
	private:
		std::vector<RLSLPRuleBody> explicit_nonterminal_rule_list_; // D
		std::vector<uint16_t> explicit_nonterminal_level_list_;		// Y
		std::vector<uint64_t> explicit_nonterminal_length_list_;	// L
		std::vector<uint16_t> relative_max_level_list_;				// R

	public:
		/**
		 * @brief Construct an empty layered dictionary.
		 */
		DictionaryForLayeredRLSLP()
		{
		}

		////////////////////////////////////////////////////////////////////////////////
		///   @name Basic functions for accessing to properties of this class
		////////////////////////////////////////////////////////////////////////////////
		//@{

		/**
		 * @brief Return const reference to *D*.
		 */
		const std::vector<RLSLPRuleBody> &get_explicit_nonterminal_rule_list() const
		{
			return this->explicit_nonterminal_rule_list_;
		}
		/**
		 * @brief Return const reference to *Y*.
		 */
		const std::vector<uint16_t> &get_explicit_nonterminal_level_list() const
		{
			return this->explicit_nonterminal_level_list_;
		}
		/**
		 * @brief Return const reference to *L*.
		 */
		const std::vector<uint64_t> &get_explicit_nonterminal_length_list() const
		{
			return this->explicit_nonterminal_length_list_;
		}
		/**
		 * @brief Return const reference to *R*.
		 */
		const std::vector<uint16_t> &get_relative_max_level_list() const
		{
			return this->relative_max_level_list_;
		}
		/**
		 * @brief Return the string length of a given nonterminal X.
		 */
		uint64_t get_length(NonterminalWithRelativeLevel X) const
		{
			return NonterminalFunctions::get_length(X, this->explicit_nonterminal_length_list_);
		}
		/**
		 * @brief Return the level of a given nonterminal X.
		 */
		uint64_t get_level(NonterminalWithRelativeLevel X) const
		{
			return NonterminalFunctions::get_level(X, this->explicit_nonterminal_level_list_);
		}
		/**
		 * @brief Return the rule body for a given nonterminal X. (O(1) time)
		 */
		RLSLPRuleBody get_rule_body(NonterminalWithRelativeLevel X) const
		{
			return RLSLPRuleBody::decode_rule(X, this->explicit_nonterminal_rule_list_);
		}

		/**
		 * @brief Return the number of explicit nonterminals, which includes unused explicit nonterminals (O(1) time).
		 */
		uint64_t count_explicit_nonterminals() const
		{
			return this->explicit_nonterminal_rule_list_.size();
		}
		/**
		 * @brief Return the total number of implicit nonterminals.
		 */
		uint64_t count_valid_implicit_nonterminals() const
		{
			const auto &relative_max_level_list = this->get_relative_max_level_list();
			uint64_t sz = 0;
			for (ExplicitNonterminal X = 0; X < (ExplicitNonterminal)this->count_explicit_nonterminals(); X++)
			{
				RLSLPRuleBody body = RLSLPRuleBody::decode_rule(X, this->explicit_nonterminal_rule_list_);
				if (body.get_type() != RLSLPRuleType::Null)
				{
					sz += relative_max_level_list[X];
				}
			}
			return sz;
		}
		/**
		 * @brief Return the total number of explicit nonterminals that are not null.
		 */
		uint64_t count_valid_explicit_nonterminals() const
		{
			uint64_t sz = 0;
			for (ExplicitNonterminal X = 0; X < (ExplicitNonterminal)this->count_explicit_nonterminals(); X++)
			{
				RLSLPRuleBody body = RLSLPRuleBody::decode_rule(X, this->explicit_nonterminal_rule_list_);
				if (body.get_type() != RLSLPRuleType::Null)
				{
					sz += 1;
				}
			}
			return sz;
		}
		/**
		 * @brief Return the total number of nonterminals that are not null.
		 */
		uint64_t count_valid_nonterminals() const
		{
			return this->count_valid_implicit_nonterminals() + this->count_valid_explicit_nonterminals();
		}

		/**
		 * @brief Return the number of null explicit nonterminals.
		 */
		uint64_t count_null_nonterminals() const
		{
			uint64_t counter = 0;
			for (uint64_t i = 0; i < this->count_explicit_nonterminals(); i++)
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
		bool check_null_item(NonterminalWithRelativeLevel i) const
		{
			return RLSLPRuleBody::decode_rule(i, this->explicit_nonterminal_rule_list_).get_type() == RLSLPRuleType::Null;
		}
		/**
		 * @brief Return the RLSLPRuleInfo for a given explicit nonterminal *i*.
		 */
		RLSLPRuleInfo get_nonterminal_info(ExplicitNonterminal i) const
		{
			uint64_t length = this->get_length(i);
			uint64_t level = this->get_level(i);
			return RLSLPRuleInfo(RLSLPRuleBody::decode_rule(i, this->explicit_nonterminal_rule_list_), length, level);
		}

		//}@

		////////////////////////////////////////////////////////////////////////////////
		///   @name Update operations
		////////////////////////////////////////////////////////////////////////////////
		//@{
		/**
		 * @brief Reset an explicit nonterminal X slot to null.
		 */
		void clear_element(ExplicitNonterminal X)
		{
			assert(this->explicit_nonterminal_rule_list_[X].get_type() != RLSLPRuleType::Null);
			this->explicit_nonterminal_rule_list_[X] = RLSLPRuleBody::create_null_item();
			this->explicit_nonterminal_length_list_[X] = UINT64_MAX;
			this->explicit_nonterminal_level_list_[X] = UINT16_MAX;
			this->relative_max_level_list_[X] = UINT16_MAX;
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
		 * @brief Decrement the relative maximum level of a given explicit nonterminal X.
		 */
		void decrease_relative_max_level(ExplicitNonterminal X)
		{
			assert(this->relative_max_level_list_[X] > 0);
			this->relative_max_level_list_[X]--;
		}
		/**
		 * @brief Increment the relative maximum level of a given explicit nonterminal X.
		 */
		void increase_relative_max_level(ExplicitNonterminal X)
		{
			assert(this->relative_max_level_list_[X] < UINT16_MAX);
			this->relative_max_level_list_[X]++;
		}

		/**
		 * @brief Allocate a new explicit nonterminal slot initialized to null.
		 * @return Index of the newly added base nonterminal.
		 */
		ExplicitNonterminal add_new_explicit_nonterminal()
		{
			uint64_t new_number = this->count_explicit_nonterminals();
			this->explicit_nonterminal_rule_list_.push_back(RLSLPRuleBody::create_null_item());
			this->explicit_nonterminal_length_list_.push_back(UINT64_MAX);
			this->explicit_nonterminal_level_list_.push_back(UINT16_MAX);
			this->relative_max_level_list_.push_back(UINT16_MAX);
			return new_number;
		}
		/**
		 * @brief Write rule body and metadata into the slot of a given explicit nonterminal X.
		 * @param rule_body RLSLP rule body to store.
		 * @param new_level Absolute level of the explicit nonterminal X.
		 * @param relative_max_level Maximum relative level for the explicit nonterminal X.
		 * @param explicit_nonterminal_length_list Length list used to compute stored length.
		 */
		void write_element(ExplicitNonterminal X, const RLSLPRuleBody &rule_body, uint16_t new_level, uint16_t relative_max_level, const std::vector<uint64_t> &explicit_nonterminal_length_list)
		{
			assert(this->explicit_nonterminal_rule_list_[X].get_type() == RLSLPRuleType::Null);
			assert(this->explicit_nonterminal_length_list_[X] == UINT64_MAX);
			assert(this->explicit_nonterminal_level_list_[X] == UINT16_MAX);
			assert(this->relative_max_level_list_[X] == UINT16_MAX);

			this->explicit_nonterminal_rule_list_[X] = rule_body;
			this->explicit_nonterminal_level_list_[X] = new_level;
			this->explicit_nonterminal_length_list_[X] = rule_body.get_length(explicit_nonterminal_length_list);
			this->relative_max_level_list_[X] = relative_max_level;

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
		//}@

		////////////////////////////////////////////////////////////////////////////////
		///   @name Print and verification functions
		////////////////////////////////////////////////////////////////////////////////
		//@{

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
		 * @brief Print summary dictionary statistics to standard output.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_statistics(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{
			uint64_t nonterminal_count = this->count_valid_nonterminals();
			uint64_t explicit_nonterminal_count = this->count_valid_explicit_nonterminals();
			uint64_t implicit_nonterminal_count = this->count_valid_implicit_nonterminals();
			uint64_t null_nonterminal_count = this->count_null_nonterminals();

			std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(Dictionary):" << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Nonterminals:           \t" << nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Explicit Nonterminals:      \t" << explicit_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Implicit Nonterminals:    \t" << implicit_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Null Nonterminals:      \t" << null_nonterminal_count << std::endl;
		}
		/**
		 * @brief Print detailed dictionary statistics.
		 * @param message_paragraph Indentation level for formatted output.
		 */
		void print_detailed_statistics(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
		{

			uint64_t nonterminal_count = this->count_valid_nonterminals();
			uint64_t explicit_nonterminal_count = this->count_valid_explicit_nonterminals();
			uint64_t implicit_nonterminal_count = this->count_valid_implicit_nonterminals();
			uint64_t null_nonterminal_count = this->count_null_nonterminals();
			uint64_t list_size = this->explicit_nonterminal_rule_list_.size();

			std::unordered_map<uint64_t, uint64_t> nonterminal_length_map;

			for (uint64_t i = 0; i < list_size; i++)
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
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Explicit Nonterminals:      \t" << explicit_nonterminal_count << std::endl;
			std::cout << stool::Message::get_paragraph_string(message_paragraph + 3) << "Implicit Nonterminals:    \t" << implicit_nonterminal_count << std::endl;
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
			for (int64_t i = 0; i < (int64_t)this->count_explicit_nonterminals(); i++)
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
		//}@

		////////////////////////////////////////////////////////////////////////////////
		///   @name Load, save, and builder functions
		////////////////////////////////////////////////////////////////////////////////
		//@{

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
		//}@
	};

}
