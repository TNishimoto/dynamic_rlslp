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
#include "./signature_functions.hpp"

namespace dynRLSLP
{

	class RLSLPRuleBody
	{

	public:
		SignatureWithRelativeLevel A;
		SignatureWithRelativeLevel B;
		uint8_t type;
		/**
		 * @brief Default constructor.
		 */
		RLSLPRuleBody()
		{
		}
		/**
		 * @brief Construct a rule body from operands and type.
		 * @param a First operand (left child, character, or base signature).
		 * @param b Second operand (right child, exponent, or sentinel).
		 * @param type_ Rule type tag.
		 */
		RLSLPRuleBody(SignatureWithRelativeLevel a, SignatureWithRelativeLevel b, RLSLPRuleType type_) : A(a), B(b), type((uint8_t)type_)
		{
		}
		// RLSLPRuleBody &operator=(const RLSLPRuleBody &rhs) = default;

		/**
		 * @brief Create a terminal (character) rule body.
		 * @param c Character code stored as a signature-with-level value.
		 * @return Rule body of type Character.
		 */
		static RLSLPRuleBody create_char_item(SignatureWithRelativeLevel c)
		{
			return RLSLPRuleBody(c, -1, RLSLPRuleType::Character);
		}
		/**
		 * @brief Create a pair rule body.
		 * @param left Left nonterminal or operand.
		 * @param right Right nonterminal or operand.
		 * @return Rule body of type Pair.
		 */
		static RLSLPRuleBody create_pair_item(SignatureWithRelativeLevel left, SignatureWithRelativeLevel right)
		{
			return RLSLPRuleBody(left, right, RLSLPRuleType::Pair);
		}
		/**
		 * @brief Create a unary signature (alias) rule body.
		 * @param single Child signature referenced at a higher relative level.
		 * @return Rule body of type Signature.
		 */
		static RLSLPRuleBody create_signature_item(SignatureWithRelativeLevel single)
		{
			return RLSLPRuleBody(single, INT64_MAX, RLSLPRuleType::Signature);
		}
		/**
		 * @brief Decode an encoded signature to its rule body.
		 * @param sig Encoded signature with relative level.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Decoded rule body at the given level, or the base rule when level is zero.
		 */
		static RLSLPRuleBody decodeRule(SignatureWithRelativeLevel sig, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			int64_t base_sig = SignatureFunctions::get_base_signature(sig);
			int64_t level = SignatureFunctions::get_relative_level(sig);
			if (level > 0)
			{
				return RLSLPRuleBody::create_signature_item(SignatureFunctions::get_signature(level - 1, base_sig));
			}
			else
			{
				return base_signature_rule_list[base_sig];
			}
		}
		/**
		 * @brief Return a const reference to the rule body of a base signature.
		 * @param sig Base signature index.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Const reference to the rule body at @p sig.
		 */
		static const RLSLPRuleBody &refer_body_of_base_signature(BaseSignature sig, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			return base_signature_rule_list[sig];
		}

		
		/**
		 * @brief Create a pair rule body from a single operand (legacy overload).
		 * @param single Child signature for a signature rule.
		 * @return Rule body of type Pair with unary operand.
		 */
		static RLSLPRuleBody create_pair_item(SignatureWithRelativeLevel single)
		{
			return RLSLPRuleBody(single, INT64_MAX, RLSLPRuleType::Pair);
		}
		

		/**
		 * @brief Create a power (run) rule body X^e.
		 * @param number Encoded signature of the repeated substring.
		 * @param power Exponent (repeat count).
		 * @return Rule body of type Power.
		 */
		static RLSLPRuleBody create_run_rule_body(SignatureWithRelativeLevel number, SignatureWithRelativeLevel power)
		{
			return RLSLPRuleBody(number, power, RLSLPRuleType::Power);
		}
		/**
		 * @brief Reconstruct a rule body from serialized binary operands.
		 * @param a First operand (left child, character, or base signature).
		 * @param b Second operand; negative denotes power, INT64_MAX denotes character.
		 * @return Character, pair, or power rule body inferred from @p b.
		 */
		static RLSLPRuleBody create_item_from_binary(SignatureWithRelativeLevel a, SignatureWithRelativeLevel b)
		{
			if (b < 0)
			{
				return RLSLPRuleBody::create_run_rule_body(a, -b);
			}
			else if (b == std::numeric_limits<int64_t>::max())
			{
				return RLSLPRuleBody::create_char_item(a);
			}
			else
			{
				return RLSLPRuleBody::create_pair_item(a, b);
			}
		}
		/**
		 * @brief Create a null (empty) rule body.
		 * @return Rule body of type Null.
		 */
		static RLSLPRuleBody create_null_item()
		{
			return RLSLPRuleBody(0, 0, RLSLPRuleType::Null);
		}

		/**
		 * @brief Return the string length represented by this rule body.
		 * @param base_signature_length_list Base-signature length list (L).
		 * @return Expanded string length of this rule.
		 */
		uint64_t get_length(const std::vector<uint64_t> &base_signature_length_list) const
		{
			if (this->get_type() == RLSLPRuleType::Pair)
			{
				return base_signature_length_list[SignatureFunctions::get_base_signature(this->A)] + base_signature_length_list[SignatureFunctions::get_base_signature(this->B)];
			}
			else if (this->get_type() == RLSLPRuleType::Power)
			{
				return base_signature_length_list[SignatureFunctions::get_base_signature(this->A)] * this->B;
			}
			else if (this->get_type() == RLSLPRuleType::Signature)
			{
				return base_signature_length_list[SignatureFunctions::get_base_signature(this->A)];
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
		 * @return Hash combining type and operands for use in containers.
		 */
		uint64_t get_hash() const
		{
			int64_t c = this->A < this->B ? 3 : 4;
			return (this->type + 1) * (this->A * this->B + this->A + this->B) * c;
		}
		/**
		 * @brief Return the rule type tag.
		 * @return Rule type as RLSLPRuleType.
		 */
		RLSLPRuleType get_type() const
		{
			return (RLSLPRuleType)this->type;
		}

		/**
		 * @brief Split a power rule X^e into left factor X and right factor X^(e-1).
		 * @param itemList Base-signature rule list (D).
		 * @return Pair (X, X^(e-1)) when e > 2, or (X, X) when e == 2.
		 * @throws std::logic_error if exponent is less than 2.
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

		/**
		 * @brief Return a compact string description of the rule.
		 * @return Human-readable summary of operands and type.
		 */
		std::string get_info() const
		{
			std::stringstream ss;
			if (this->get_type() == RLSLPRuleType::Power)
			{
				uint64_t base_signature = SignatureFunctions::get_base_signature(this->A);
				uint64_t level = SignatureFunctions::get_relative_level(this->A);
				ss << "[" << base_signature << "(H" << level << ")" << "^" << this->B << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Pair)
			{
				uint64_t base_signatureA = SignatureFunctions::get_base_signature(this->A);
				uint64_t levelA = SignatureFunctions::get_relative_level(this->A);
				uint64_t base_signatureB = SignatureFunctions::get_base_signature(this->B);
				uint64_t levelB = SignatureFunctions::get_relative_level(this->B);
				ss << "[" << base_signatureA << "(H" << levelA << ")" << ", " << base_signatureB << "(H" << levelB << ")" << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Character)
			{
				ss << "[" << (char)this->A << "(" << (int64_t)this->A << ")" << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Signature)
			{
				uint64_t base_signature = SignatureFunctions::get_base_signature(this->A);
				uint64_t level = SignatureFunctions::get_relative_level(this->A);
				ss << "[" << base_signature << "(H" << level << ")" << "]";
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
		 * @brief Return a detailed string description with base signature and level.
		 * @param base_signature Base signature index.
		 * @param level Derivation-tree level.
		 * @return Detailed rule string including base signature and level.
		 */
		std::string get_detailed_info(BaseSignature base_signature, uint64_t level) const
		{
			std::stringstream ss;
			ss << "[" << base_signature << "_" << level << " -> ";

			if (this->get_type() == RLSLPRuleType::Power)
			{
				uint64_t base_signatureA = SignatureFunctions::get_base_signature(this->A);
				uint64_t levelA = SignatureFunctions::get_relative_level(this->A);
				ss << base_signatureA << "_" << levelA << "^" << this->B << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Pair)
			{
				uint64_t base_signatureA = SignatureFunctions::get_base_signature(this->A);
				uint64_t levelA = SignatureFunctions::get_relative_level(this->A);
				uint64_t base_signatureB = SignatureFunctions::get_base_signature(this->B);
				uint64_t levelB = SignatureFunctions::get_relative_level(this->B);
				ss << base_signatureA << "_" << levelA << ", " << base_signatureB << "_" << levelB << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Character)
			{
				ss << (char)this->A << "]";
			}
			else if (this->get_type() == RLSLPRuleType::Signature)
			{
				uint64_t base_signatureA = SignatureFunctions::get_base_signature(this->A);
				uint64_t levelA = SignatureFunctions::get_relative_level(this->A);
				ss << base_signatureA << "_" << levelA << "]";
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
		 * @brief Return the derivation-tree height using a height list.
		 * @param heightList Per-signature height list.
		 * @return Derivation-tree height of the string represented by this rule.
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
			else if (this->get_type() == RLSLPRuleType::Signature)
			{
				return heightList[this->A] + 1;
			}
			else if (this->get_type() == RLSLPRuleType::Character)
			{
				return BSignatureBottomLevel;
			}
			else
			{
				return 0;
			}
		}
		/**
		 * @brief Append the expanded string of this rule to an output container.
		 * @tparam OUTPUT_VEC_TYPE Container type supporting push_back (default std::vector<uint8_t>).
		 * @param itemList Base-signature rule list (D).
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
				assert(SignatureFunctions::get_base_signature(this->A) < itemList.size());
				assert(SignatureFunctions::get_base_signature(this->B) < itemList.size());
				itemList[SignatureFunctions::get_base_signature(this->A)].decompress(itemList, output);
				itemList[SignatureFunctions::get_base_signature(this->B)].decompress(itemList, output);
			}
			else if (this->get_type() == RLSLPRuleType::Signature)
			{
				assert(SignatureFunctions::get_base_signature(this->A) < itemList.size());
				itemList[SignatureFunctions::get_base_signature(this->A)].decompress(itemList, output);
			}
			else if (this->get_type() == RLSLPRuleType::Power)
			{
				std::vector<uint8_t> tmp;
				assert(SignatureFunctions::get_base_signature(this->A) < itemList.size());
				itemList[SignatureFunctions::get_base_signature(this->A)].decompress(itemList, tmp);
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
		 * @brief Return the serialized size in bytes of a rule body.
		 * @return Size in bytes of the on-disk/binary representation.
		 */
		static uint64_t get_byte()
		{
			return (2 * sizeof(SignatureWithRelativeLevel)) + sizeof(unsigned char);
		}
		/**
		 * @brief Expand this rule to the original plaintext string.
		 * @param base_signature_rule_list Base-signature rule list (D).
		 * @return Decompressed string represented by this rule.
		 */
		std::string to_original_text_str(const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
		{
			std::string r;
			this->decompress(base_signature_rule_list, r);
			return r;
		}
	};

}