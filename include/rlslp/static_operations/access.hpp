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
	 * @brief A class for accessing the string represented by RLSLP.
	 * @ingroup StaticOperationsClasses
	 */
	class Access
	{
		private:
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
				RLSLPRuleBody left = RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list);

				current_pos = Access::get_prefix(left, current_pos, len, base_signature_rule_list, output);
				if (current_pos == (int64_t)len)
				{
					return current_pos;
				}
				else
				{
					RLSLPRuleBody right = RLSLPRuleBody::decode_rule(item.B, base_signature_rule_list);
					current_pos = Access::get_prefix(right, current_pos, len, base_signature_rule_list, output);
					return current_pos;
				}
			}
			else if (item.get_type() == RLSLPRuleType::Power)
			{
				for (int64_t i = 0; i < item.B; i++)
				{
					current_pos = Access::get_prefix(RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list), current_pos, len, base_signature_rule_list, output);
					if (current_pos == (int64_t)len)
					{
						return current_pos;
					}
				}
				return current_pos;
			}
			else if (item.get_type() == RLSLPRuleType::Signature)
			{
				return Access::get_prefix(RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list), current_pos, len, base_signature_rule_list, output);
			}
			else
			{
				throw std::runtime_error("Invalid item type");
			}
		}

		template <typename ARRAY>
		static int64_t get_suffix(RLSLPRuleBody item, int64_t current_pos, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list, ARRAY &output)
		{
			if (item.get_type() == RLSLPRuleType::Character)
			{
				output[current_pos--] = item.A;
				return current_pos;
			}
			else if (item.get_type() == RLSLPRuleType::Pair)
			{
				RLSLPRuleBody right = RLSLPRuleBody::decode_rule(item.B, base_signature_rule_list);

				current_pos = Access::get_suffix(right, current_pos, len, base_signature_rule_list, output);
				if (current_pos == -1)
				{
					return current_pos;
				}
				else
				{
					RLSLPRuleBody left = RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list);
					current_pos = Access::get_suffix(left, current_pos, len, base_signature_rule_list, output);
					return current_pos;
				}
			}
			else if (item.get_type() == RLSLPRuleType::Power)
			{
				for (int64_t i = 0; i < item.B; i++)
				{
					current_pos = Access::get_suffix(RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list), current_pos, len, base_signature_rule_list, output);
					if (current_pos == -1)
					{
						return current_pos;
					}
				}
				return current_pos;
			}
			else if (item.get_type() == RLSLPRuleType::Signature)
			{
				return Access::get_suffix(RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list), current_pos, len, base_signature_rule_list, output);
			}
			else
			{
				throw std::runtime_error("Invalid item type");
			}
		}

	public:
		/**
		 * @brief Return @ref term_val "val(X)"[pos].
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @param base_signature_length_list The length list of DictionaryForLayeredRLSLP.
		 * @return *val(X)[pos]*.
		 */
		static uint64_t random_access(RLSLPRuleBody X, int64_t pos, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list)
		{
			uint64_t result = 0; // The character at pos.
			if (X.get_type() == RLSLPRuleType::Pair)
			{
				auto left = RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list);					 // The left child of X.
				auto leftLen = (int64_t)SignatureFunctions::get_length(X.A, base_signature_length_list); // The length of the left child of X.
				auto right = RLSLPRuleBody::decode_rule(X.B, base_signature_rule_list);					 // The right child of X.
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
				auto child = RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list);
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
				result = Access::random_access(RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list), pos, base_signature_rule_list, base_signature_length_list);
			}
			else
			{
				throw std::logic_error("Access::random_access: invalid item type");
			}
			// assert((int64_t)result.second != -1);
			return result;
		}

		/**
		 * @brief Return @ref term_val "val"(X_{1}, X_{2}, ..., X_{k}), where *X_{1}, X_{2}, ..., X_{k}* are the sequence of nonterminals repersented by *items*..
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return val(X_{1}, X_{2}, ..., X_{k}).
		 */
		static std::string get_string(const std::vector<RunRuleBody> &items, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			std::string r = "";
			for (auto item : items)
			{
				RLSLPRuleBody item2 = RLSLPRuleBody::decode_rule(item.number, base_signature_rule_list);
				for (uint64_t i = 0; i < item.power; i++)
				{
					r.append(Access::get_string(item2, base_signature_rule_list));
				}
			}
			return r;
		}

		/**
		 * @brief Return @ref term_val "val(X)".
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return *val(X)*.
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
				auto left = Access::get_string(RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list), base_signature_rule_list);
				auto right = Access::get_string(RLSLPRuleBody::decode_rule(X.B, base_signature_rule_list), base_signature_rule_list);
				left.append(right);
				return left;
			}
			else if (X.get_type() == RLSLPRuleType::Power)
			{
				auto raw = Access::get_string(RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list), base_signature_rule_list);
				std::string r;
				for (int64_t i = 0; i < X.B; i++)
				{
					r.append(raw);
				}
				return r;
			}
			else if (X.get_type() == RLSLPRuleType::Signature)
			{
				return Access::get_string(RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list), base_signature_rule_list);
			}
			else
			{
				throw std::logic_error("Access::get_string: invalid item type");
			}
		}

		/**
		 * @brief Return @ref term_val "val(X)[0..len-1]".
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return *val(X)[0..len-1]*.
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
		 * @brief Return @ref term_val "val(X)[|X|-len..|X|-1]", where |X| is the length of *val(X)*.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return *val(X)[|X|-len..|X|-1]*.
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
		 * @brief Return the @ref left_string "left string" of a given RLSLPRuleBody *X*.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return The left string of *X*.
		 */
		static std::string get_left_string(RLSLPRuleBody X, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			std::string r;
			if (X.get_type() == RLSLPRuleType::Pair)
			{
				r = Access::get_string(RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list), base_signature_rule_list);
				return r;
			}
			else if (X.get_type() == RLSLPRuleType::Power)
			{
				r = Access::get_string(RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list), base_signature_rule_list);
				return r;
			}
			else
			{
				return "";
			}
		}

		/**
		 * @brief Return the @ref right_string "right string" of a given RLSLPRuleBody *X*.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return The right string of *X*.
		 */
		static std::string get_right_string(RLSLPRuleBody X, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			std::string r = "";
			if (X.get_type() == RLSLPRuleType::Pair)
			{
				r = Access::get_string(RLSLPRuleBody::decode_rule(X.B, base_signature_rule_list), base_signature_rule_list);
				return r;
			}
			else if (X.get_type() == RLSLPRuleType::Power)
			{
				auto child_str = Access::get_string(RLSLPRuleBody::decode_rule(X.A, base_signature_rule_list), base_signature_rule_list);
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
		 * @brief Return @ref term_val "val(X)".
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return *val(X)*.
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
				auto left = Access::get_string_as_vector(RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list), base_signature_rule_list);
				auto right = Access::get_string_as_vector(RLSLPRuleBody::decode_rule(item.B, base_signature_rule_list), base_signature_rule_list);
				for (auto it : right)
				{
					left.push_back(it);
				}
				return left;
			}
			else if (item.get_type() == RLSLPRuleType::Power)
			{
				std::vector<sig_char_type> raw = Access::get_string_as_vector(RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list), base_signature_rule_list);
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
				return Access::get_string_as_vector(RLSLPRuleBody::decode_rule(item.A, base_signature_rule_list), base_signature_rule_list);
			}
			else
			{
				throw std::logic_error("Access::get_long_string: invalid item type");
			}
		}
	};

}
