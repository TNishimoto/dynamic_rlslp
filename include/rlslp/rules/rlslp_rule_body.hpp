#pragma once
// #include "cpplinq.hpp"
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <functional>
#include <list>
#include <set>
#include <cassert>
#include "../../types/types.hpp"
#include "./nonterminal_functions.hpp"
#include "../../json_helper.hpp"

namespace dynRLSLP
{
	/**
	 * @brief A representation of an RLSLP rule body.
	 * @ingroup RLSLPClasses
	 */
	class RLSLPRuleBody
	{

	public:
		/** @brief First operand of the rule body. */
		NonterminalWithRelativeLevel A;
		/** @brief Second operand of the rule body. */
		NonterminalWithRelativeLevel B;

		/** @brief Type of the rule body. */
		RLSLPRuleType type;
		/**
		 * @brief Default constructor.
		 */
		RLSLPRuleBody()
		{
		}
		/**
		 * @brief Construct a rule body from operands and type.
		 * @param a First operand (left child, character, or base nonterminal).
		 * @param b Second operand (right child, exponent, or sentinel).
		 * @param type_ Rule type tag.
		 */
		RLSLPRuleBody(NonterminalWithRelativeLevel a, NonterminalWithRelativeLevel b, RLSLPRuleType type_) : A(a), B(b), type(type_)
		{
		}

		/**
		 * @brief Return the string length represented by this rule body.
		 * @param explicit_nonterminal_length_list Base-nonterminal length list (L).
		 */
		uint64_t get_length(const std::vector<uint64_t> &explicit_nonterminal_length_list) const
		{
			if (this->get_type() == RLSLPRuleType::Pair)
			{
				return explicit_nonterminal_length_list[NonterminalFunctions::get_explicit_nonterminal(this->A)] + explicit_nonterminal_length_list[NonterminalFunctions::get_explicit_nonterminal(this->B)];
			}
			else if (this->get_type() == RLSLPRuleType::Power)
			{
				return explicit_nonterminal_length_list[NonterminalFunctions::get_explicit_nonterminal(this->A)] * this->B;
			}
			else if (this->get_type() == RLSLPRuleType::Nonterminal)
			{
				return explicit_nonterminal_length_list[NonterminalFunctions::get_explicit_nonterminal(this->A)];
			}
			else if (this->get_type() == RLSLPRuleType::Character)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		/**
		 * @brief Return a hash value for the rule body.
		 */
		uint64_t get_hash() const
		{
			int64_t c = this->A < this->B ? 3 : 4;
			return ((int64_t)this->type + 1) * (this->A * this->B + this->A + this->B) * c;
		}

		/**
		 * @brief Return the rule type tag.
		 */
		RLSLPRuleType get_type() const
		{
			return (RLSLPRuleType)this->type;
		}

		/**
		 * @brief Split a power rule X^e into left factor X and right factor X^(e-1).
		 * @param itemList Base-nonterminal rule list (D).
		 */
		std::pair<const RLSLPRuleBody, const RLSLPRuleBody> break_power(const std::vector<RLSLPRuleBody> &itemList) const
		{
			RLSLPRuleBody front = itemList[this->A];
			if (this->B > 2)
			{
				RLSLPRuleBody back = RLSLPRuleBody::create_run_rule_body(this->A, this->B - 1);
				return std::pair<const RLSLPRuleBody, const RLSLPRuleBody>(front, back);
			}
			else if (this->B == 2)
			{
				return std::pair<const RLSLPRuleBody, const RLSLPRuleBody>(front, front);
			}
			else
			{
				throw std::logic_error("RLSLPRuleBody::break_power: this->B is not 2");
			}
		}

		std::string to_string(NonterminalWithRelativeLevel sig) const
		{
			std::stringstream ss;
			ss << NonterminalFunctions::to_string(sig) << " -> " << this->to_string();
			return ss.str();
		}


		std::string to_string() const
		{
			std::stringstream ss;
			if (this->get_type() == RLSLPRuleType::Power)
			{
				ss << "(" << NonterminalFunctions::to_string(this->A) << ")^" << this->B;
			}
			else if (this->get_type() == RLSLPRuleType::Pair)
			{
				ss << "(" << NonterminalFunctions::to_string(this->A) << ", " << NonterminalFunctions::to_string(this->B) <<  ")";
			}
			else if (this->get_type() == RLSLPRuleType::Character)
			{
				ss << JsonHelper::escapeJsonChar((char)this->A);
			}
			else if (this->get_type() == RLSLPRuleType::Nonterminal)
			{
				ss << NonterminalFunctions::to_string(this->A);
			}
			else if (this->get_type() == RLSLPRuleType::Null)
			{
				ss << "[NULL]";
			}
			else
			{
				ss << "[" << this->A << "/" << this->B << "/" << (int)this->type << "]";
			}
			return ss.str();
		}


		/**
		 * @brief Return a compact string description of the rule.
		 */
		std::string get_info() const
		{
			std::stringstream ss;
			if (this->get_type() == RLSLPRuleType::Power)
			{
				uint64_t explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(this->A);
				uint64_t level = NonterminalFunctions::get_relative_level(this->A);
				ss << "[" << explicit_nonterminal << "(H" << level << ")" << "^" << this->B << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Pair)
			{
				uint64_t explicit_nonterminalA = NonterminalFunctions::get_explicit_nonterminal(this->A);
				uint64_t levelA = NonterminalFunctions::get_relative_level(this->A);
				uint64_t explicit_nonterminalB = NonterminalFunctions::get_explicit_nonterminal(this->B);
				uint64_t levelB = NonterminalFunctions::get_relative_level(this->B);
				ss << "[" << explicit_nonterminalA << "(H" << levelA << ")" << ", " << explicit_nonterminalB << "(H" << levelB << ")" << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Character)
			{
				ss << "[" << (char)this->A << "(" << (int64_t)this->A << ")" << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Nonterminal)
			{
				uint64_t explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(this->A);
				uint64_t level = NonterminalFunctions::get_relative_level(this->A);
				ss << "[" << explicit_nonterminal << "(H" << level << ")" << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Null)
			{
				ss << "[NULL]";
			}
			else
			{
				ss << "[" << this->A << "/" << this->B << "/" << (int)this->type << "]";
			}
			return ss.str();
		}

		/**
		 * @brief Return a detailed string description with base nonterminal and level.
		 * @param explicit_nonterminal Base nonterminal index.
		 * @param level Derivation-tree level.
		 * @return Detailed rule string including base nonterminal and level.
		 */
		std::string get_detailed_info(ExplicitNonterminal explicit_nonterminal, uint64_t level) const
		{
			std::stringstream ss;
			ss << "[" << explicit_nonterminal << "_" << level << " -> ";

			if (this->get_type() == RLSLPRuleType::Power)
			{
				uint64_t explicit_nonterminalA = NonterminalFunctions::get_explicit_nonterminal(this->A);
				uint64_t levelA = NonterminalFunctions::get_relative_level(this->A);
				ss << explicit_nonterminalA << "_" << levelA << "^" << this->B << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Pair)
			{
				uint64_t explicit_nonterminalA = NonterminalFunctions::get_explicit_nonterminal(this->A);
				uint64_t levelA = NonterminalFunctions::get_relative_level(this->A);
				uint64_t explicit_nonterminalB = NonterminalFunctions::get_explicit_nonterminal(this->B);
				uint64_t levelB = NonterminalFunctions::get_relative_level(this->B);
				ss << explicit_nonterminalA << "_" << levelA << ", " << explicit_nonterminalB << "_" << levelB << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Character)
			{
				ss << (char)this->A << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Nonterminal)
			{
				uint64_t explicit_nonterminalA = NonterminalFunctions::get_explicit_nonterminal(this->A);
				uint64_t levelA = NonterminalFunctions::get_relative_level(this->A);
				ss << explicit_nonterminalA << "_" << levelA << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Null)
			{
				ss << "NULL]";
			}
			else
			{
				ss << "[" << this->A << "/" << this->B << "/" << (int)this->type << "]";
			}
			return ss.str();
		}

		/**
		 * @brief Return the level of this rule body.
		 * @param heightList Base-nonterminal level list (H).
		 */
		uint64_t get_height(const std::vector<uint16_t> &heightList) const
		{
			if (this->get_type() == RLSLPRuleType::Pair)
			{
				return heightList[this->B] + 1;
			}
			else if (this->get_type() == RLSLPRuleType::Power)
			{
				return heightList[this->A];
			}
			else if (this->get_type() == RLSLPRuleType::Nonterminal)
			{
				return heightList[this->A] + 1;
			}
			else if (this->get_type() == RLSLPRuleType::Character)
			{
				return NonterminalBottomLevel;
			}
			else
			{
				return 0;
			}
		}

		/**
		 * @brief Return the string derived by this rule body.
		 * @tparam OUTPUT_VEC_TYPE Container type supporting push_back (default std::vector<uint8_t>).
		 * @param itemList Base-nonterminal rule list (D).
		 * @param output Output container; expanded bytes are appended in place.
		 */
		template <typename OUTPUT_VEC_TYPE = std::vector<uint8_t>>
		void decompress(const std::vector<RLSLPRuleBody> &itemList, OUTPUT_VEC_TYPE &output) const
		{
			if (this->get_type() == RLSLPRuleType::Character)
			{
				output.push_back(this->A);
			}
			else if (this->get_type() == RLSLPRuleType::Pair)
			{
				assert(NonterminalFunctions::get_explicit_nonterminal(this->A) < itemList.size());
				assert(NonterminalFunctions::get_explicit_nonterminal(this->B) < itemList.size());
				itemList[NonterminalFunctions::get_explicit_nonterminal(this->A)].decompress(itemList, output);
				itemList[NonterminalFunctions::get_explicit_nonterminal(this->B)].decompress(itemList, output);
			}
			else if (this->get_type() == RLSLPRuleType::Nonterminal)
			{
				assert(NonterminalFunctions::get_explicit_nonterminal(this->A) < itemList.size());
				itemList[NonterminalFunctions::get_explicit_nonterminal(this->A)].decompress(itemList, output);
			}
			else if (this->get_type() == RLSLPRuleType::Power)
			{
				std::vector<uint8_t> tmp;
				assert(NonterminalFunctions::get_explicit_nonterminal(this->A) < itemList.size());
				itemList[NonterminalFunctions::get_explicit_nonterminal(this->A)].decompress(itemList, tmp);
				for (int64_t i = 0; i < this->B; i++)
				{
					for (auto it : tmp)
					{
						output.push_back(it);
					}
				}
			}
			else
			{
				throw std::runtime_error("Error in decompress: unknown rule type");
			}
		}

		/**
		 * @brief Return the string derived by this rule body.
		 * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
		 */
		std::string decompress2(const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list) const
		{
			std::string r;
			this->decompress(explicit_nonterminal_rule_list, r);
			return r;
		}

		/**
		 * @brief Lexicographic comparison for ordering rule bodies.
		 * @param item Other rule body.
		 * @return True if this body is less than @p item (type, then A, then B).
		 */
		bool operator<(const RLSLPRuleBody &item) const
		{
			if (this->type == item.type)
			{
				if (this->A == item.A)
				{
					return this->B < item.B;
				}
				else
				{
					return this->A < item.A;
				}
			}
			else
			{
				return this->type < item.type;
			}
		}
		/**
		 * @brief Equality comparison of type and operands.
		 * @param item Other rule body.
		 * @return True if type, A, and B all match.
		 */
		bool operator==(const RLSLPRuleBody &item) const
		{
			return (this->type == item.type) && (this->A == item.A) && (this->B == item.B);
		}

		/**
		 * @brief Create a character rule body representing a given character.
		 */
		static RLSLPRuleBody create_char_item(int64_t c)
		{
			return RLSLPRuleBody(c, -1, RLSLPRuleType::Character);
		}
		/**
		 * @brief Create a pair rule body representing a given pair of nonterminals.
		 */
		static RLSLPRuleBody create_pair_item(NonterminalWithRelativeLevel left, NonterminalWithRelativeLevel right)
		{
			return RLSLPRuleBody(left, right, RLSLPRuleType::Pair);
		}
		/**
		 * @brief Create a run rule body X^k.
		 */
		static RLSLPRuleBody create_run_rule_body(NonterminalWithRelativeLevel X, NonterminalWithRelativeLevel k)
		{
			return RLSLPRuleBody(X, k, RLSLPRuleType::Power);
		}

		/**
		 * @brief Create a unary nonterminal representing a given nonterminal.
		 */
		static RLSLPRuleBody create_nonterminal_item(NonterminalWithRelativeLevel single)
		{
			return RLSLPRuleBody(single, INT64_MAX, RLSLPRuleType::Nonterminal);
		}

		/**
		 * @brief Create a null (empty) rule body.
		 */
		static RLSLPRuleBody create_null_item()
		{
			return RLSLPRuleBody(0, 0, RLSLPRuleType::Null);
		}

		/**
		 * @brief Return the RLSLP rule body representing a given nonterminal.
		 * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
		 */
		static RLSLPRuleBody decode_rule(NonterminalWithRelativeLevel sig, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
		{
			int64_t base_sig = NonterminalFunctions::get_explicit_nonterminal(sig);
			int64_t level = NonterminalFunctions::get_relative_level(sig);
			if (level > 0)
			{
				return RLSLPRuleBody::create_nonterminal_item(NonterminalFunctions::get_nonterminal(level - 1, base_sig));
			}
			else
			{
				return explicit_nonterminal_rule_list[base_sig];
			}
		}

		/**
		 * @brief Return a const reference to the rule body of a given base nonterminal.
		 * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
		 */
		static const RLSLPRuleBody &refer_body_of_explicit_nonterminal(ExplicitNonterminal sig, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list)
		{
			return explicit_nonterminal_rule_list[sig];
		}

		/**
		 * @brief Return the serialized size in bytes of a rule body.
		 * @return Size in bytes of the on-disk/binary representation.
		 */
		static uint64_t get_byte()
		{
			return (2 * sizeof(NonterminalWithRelativeLevel)) + sizeof(unsigned char);
		}
	};

}