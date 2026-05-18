#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <stack>
#include <exception>
#include <set>
#include "../rules/all.hpp"
#include <cassert>
#include <random>
namespace dynRLSLP
{

	/**
	 * @brief Static string access (random access, prefix, suffix) on RLSLP rule bodies.
	 * @ingroup StaticOperationsClasses
	 */
	class Access
	{
		private:
		/**
		 * @brief Fill the first @p len characters of a rule body into an output array (recursive helper).
		 * @tparam ARRAY Output container type supporting indexed assignment.
		 * @param item Rule body whose string is read left-to-right.
		 * @param current_pos Next write index in @p output.
		 * @param len Number of characters to collect.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @param output Output array; prefix is written starting at index 0.
		 * @return Index after the last written character, or @p len when complete.
		 */
		template <typename ARRAY>
		static int64_t get_prefix(RLSLPRuleBody item, int64_t current_pos, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list, ARRAY &output)
		{
			if (item.get_type() == RLSLPRuleType::Character)
			{
				output[current_pos++] = item.A;
				return current_pos;
			}
			else if (item.get_type() == RLSLPRuleType::Pair)
			{
				RLSLPRuleBody left = RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list);

				current_pos = Access::get_prefix(left, current_pos, len, base_signature_rule_list, output);
				if (current_pos == (int64_t)len)
				{
					return current_pos;
				}
				else
				{
					RLSLPRuleBody right = RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list);
					current_pos = Access::get_prefix(right, current_pos, len, base_signature_rule_list, output);
					return current_pos;
				}
			}
			else if (item.get_type() == RLSLPRuleType::Power)
			{
				for (int64_t i = 0; i < item.B; i++)
				{
					current_pos = Access::get_prefix(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), current_pos, len, base_signature_rule_list, output);
					if (current_pos == (int64_t)len)
					{
						return current_pos;
					}
				}
				return current_pos;
			}
			else if (item.get_type() == RLSLPRuleType::Signature)
			{
				return Access::get_prefix(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), current_pos, len, base_signature_rule_list, output);
			}
			else
			{
				throw std::runtime_error("Invalid item type");
			}
		}

		template <typename ARRAY>
		/**
		 * @brief Fill the last @p len characters of a rule body into an output array (recursive helper).
		 * @tparam ARRAY Output container type supporting indexed assignment.
		 * @param item Rule body whose string is read right-to-left.
		 * @param current_pos Next write index in @p output (counts down from len-1).
		 * @param len Number of characters to collect.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @param output Output array; suffix is written ending at index len-1.
		 * @return Index after the last written character, or -1 when complete.
		 */
		static int64_t get_suffix(RLSLPRuleBody item, int64_t current_pos, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list, ARRAY &output)
		{
			if (item.get_type() == RLSLPRuleType::Character)
			{
				output[current_pos--] = item.A;
				return current_pos;
			}
			else if (item.get_type() == RLSLPRuleType::Pair)
			{
				RLSLPRuleBody right = RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list);

				current_pos = Access::get_suffix(right, current_pos, len, base_signature_rule_list, output);
				if (current_pos == -1)
				{
					return current_pos;
				}
				else
				{
					RLSLPRuleBody left = RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list);
					current_pos = Access::get_suffix(left, current_pos, len, base_signature_rule_list, output);
					return current_pos;
				}
			}
			else if (item.get_type() == RLSLPRuleType::Power)
			{
				for (int64_t i = 0; i < item.B; i++)
				{
					current_pos = Access::get_suffix(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), current_pos, len, base_signature_rule_list, output);
					if (current_pos == -1)
					{
						return current_pos;
					}
				}
				return current_pos;
			}
			else if (item.get_type() == RLSLPRuleType::Signature)
			{
				return Access::get_suffix(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), current_pos, len, base_signature_rule_list, output);
			}
			else
			{
				throw std::runtime_error("Invalid item type");
			}
		}

	public:
		/**
		 * @brief Return the character at a position in a rule body string.
		 * @param X RLSLP rule body.
		 * @param pos Start position in the represented string.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @param base_signature_length_list Base-signature length list (L).
		 * @return Character code at zero-based position @p pos in the expanded string.
		 */
		static uint64_t random_access(RLSLPRuleBody X, int64_t pos, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list)
		{
			uint64_t result = 0; // The character at pos.
			if (X.get_type() == RLSLPRuleType::Pair)
			{
				auto left = RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list);					 // The left child of X.
				auto leftLen = (int64_t)SignatureFunctions::get_length(X.A, base_signature_length_list); // The length of the left child of X.
				auto right = RLSLPRuleBody::decodeRule(X.B, base_signature_rule_list);					 // The right child of X.
				if (leftLen <= pos)
				{
					auto nextPos = pos - leftLen;
					result = Access::random_access(right, nextPos, base_signature_rule_list, base_signature_length_list);
				}
				else
				{
					result = Access::random_access(left, pos, base_signature_rule_list, base_signature_length_list);
				}
			}
			else if (X.get_type() == RLSLPRuleType::Power)
			{
				auto child = RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list);
				auto childLen = SignatureFunctions::get_length(X.A, base_signature_length_list);
				auto nextPos = pos % childLen;
				// auto nextPos = pos - (childLen * k);
				if (childLen == 1)
				{
					result = child.A;
				}
				else
				{
					result = Access::random_access(child, nextPos, base_signature_rule_list, base_signature_length_list);
				}
			}
			else if (X.get_type() == RLSLPRuleType::Character)
			{
				if (pos == 0)
				{
					result = X.A;
				}
				else
				{
					throw std::logic_error("Access::random_access: pos is not 0");
				}
			}
			else if (X.get_type() == RLSLPRuleType::Signature)
			{
				result = Access::random_access(RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list), pos, base_signature_rule_list, base_signature_length_list);
			}
			else
			{
				throw std::logic_error("Access::random_access: invalid item type");
			}
			// assert((int64_t)result.second != -1);
			return result;
		}

		/**
		 * @brief Return the string value of a rule body or run sequence.
		 * @param items Sequence of run rules or integers.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Concatenated expanded string of all run rules in @p items.
		 */
		static std::string get_string(const std::vector<RunRuleBody> &items, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			std::string r = "";
			for (auto item : items)
			{
				RLSLPRuleBody item2 = RLSLPRuleBody::decodeRule(item.number, base_signature_rule_list);
				for (uint64_t i = 0; i < item.power; i++)
				{
					r.append(Access::get_string(item2, base_signature_rule_list));
				}
			}
			return r;
		}

		/**
		 * @brief Return the string value of a rule body or run sequence.
		 * @param X RLSLP rule body.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Full expanded string represented by rule body @p X.
		 */
		static std::string get_string(RLSLPRuleBody X, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			if (X.get_type() == RLSLPRuleType::Character)
			{
				std::string r;
				r.append(1, X.A);
				return r;
			}
			else if (X.get_type() == RLSLPRuleType::Pair)
			{
				auto left = Access::get_string(RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list), base_signature_rule_list);
				auto right = Access::get_string(RLSLPRuleBody::decodeRule(X.B, base_signature_rule_list), base_signature_rule_list);
				left.append(right);
				return left;
			}
			else if (X.get_type() == RLSLPRuleType::Power)
			{
				auto raw = Access::get_string(RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list), base_signature_rule_list);
				std::string r;
				for (int64_t i = 0; i < X.B; i++)
				{
					r.append(raw);
				}
				return r;
			}
			else if (X.get_type() == RLSLPRuleType::Signature)
			{
				return Access::get_string(RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list), base_signature_rule_list);
			}
			else
			{
				throw std::logic_error("Access::get_string: invalid item type");
			}
		}

		/**
		 * @brief Return the length-prefix of a rule body string.
		 * @param X RLSLP rule body.
		 * @param len Length of the substring or prefix/suffix.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Prefix string of length @p len from the expanded string of @p X.
		 */
		static std::string get_prefix(RLSLPRuleBody X, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			std::string r;
			r.resize(len);
			int64_t current_pos = 0;
			current_pos = Access::get_prefix(X, current_pos, len, base_signature_rule_list, r);
			if (current_pos != (int64_t)len)
			{
				throw std::runtime_error("Invalid item type");
			}
			else
			{
				return r;
			}
		}

		/**
		 * @brief Return the length-suffix of a rule body string.
		 * @param X RLSLP rule body.
		 * @param len Length of the substring or prefix/suffix.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Suffix string of length @p len from the expanded string of @p X.
		 */
		static std::string get_suffix(RLSLPRuleBody X, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			std::string r;
			r.resize(len);
			int64_t current_pos = len - 1;
			current_pos = Access::get_suffix(X, current_pos, len, base_signature_rule_list, r);
			if (current_pos != -1)
			{
				throw std::runtime_error("Invalid item type");
			}
			else
			{
				return r;
			}
		}

		/**
		 * @brief Return the left substring of a rule body.
		 * @param X RLSLP rule body.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Left factor string (first child for Pair, single block for Power).
		 */
		static std::string get_left_string(RLSLPRuleBody X, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			std::string r;
			if (X.get_type() == RLSLPRuleType::Pair)
			{
				r = Access::get_string(RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list), base_signature_rule_list);
				return r;
			}
			else if (X.get_type() == RLSLPRuleType::Power)
			{
				r = Access::get_string(RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list), base_signature_rule_list);
				return r;
			}
			else
			{
				return "";
			}
		}

		/**
		 * @brief Return the right substring of a rule body.
		 * @param X RLSLP rule body.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Right factor string (second child for Pair, tail repeats for Power).
		 */
		static std::string get_right_string(RLSLPRuleBody X, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			std::string r = "";
			if (X.get_type() == RLSLPRuleType::Pair)
			{
				r = Access::get_string(RLSLPRuleBody::decodeRule(X.B, base_signature_rule_list), base_signature_rule_list);
				return r;
			}
			else if (X.get_type() == RLSLPRuleType::Power)
			{
				auto child_str = Access::get_string(RLSLPRuleBody::decodeRule(X.A, base_signature_rule_list), base_signature_rule_list);
				for (int64_t i = 0; i < X.B - 1; i++)
				{
					r.append(child_str);
				}
				return r;
			}
			else
			{
				return "";
			}
		}

		/**
		 * @brief Return the string as a vector of characters.
		 * @param item Rule body item.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Expanded string as a vector of character codes.
		 */
		static std::vector<sig_char_type> get_string_as_vector(RLSLPRuleBody item, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			if (item.get_type() == RLSLPRuleType::Character)
			{
				std::vector<sig_char_type> r;
				r.push_back(item.A);
				return r;
			}
			else if (item.get_type() == RLSLPRuleType::Pair)
			{
				auto left = Access::get_string_as_vector(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
				auto right = Access::get_string_as_vector(RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list), base_signature_rule_list);
				for (auto it : right)
				{
					left.push_back(it);
				}
				return left;
			}
			else if (item.get_type() == RLSLPRuleType::Power)
			{
				std::vector<sig_char_type> raw = Access::get_string_as_vector(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
				std::vector<sig_char_type> r;
				for (int64_t i = 0; i < item.B; i++)
				{
					for (auto it : raw)
					{
						r.push_back(it);
					}
				}
				return r;
			}
			else if (item.get_type() == RLSLPRuleType::Signature)
			{
				return Access::get_string_as_vector(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
			}
			else
			{
				throw std::logic_error("Access::get_long_string: invalid item type");
			}
		}
	};

}