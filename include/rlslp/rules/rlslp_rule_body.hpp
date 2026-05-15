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

	/**
	 * @brief A representation of an RLSLP rule body.
	 * @ingroup RLSLPClasses
	 */
	class RLSLPRuleBody
	{

	public:
		SignatureWithRelativeLevel A;
		SignatureWithRelativeLevel B;
		uint8_t type;
		RLSLPRuleBody()
		{
		}
		RLSLPRuleBody(SignatureWithRelativeLevel a, SignatureWithRelativeLevel b, RLSLPRuleType type_) : A(a), B(b), type((uint8_t)type_)
		{
		}
		// RLSLPRuleBody &operator=(const RLSLPRuleBody &rhs) = default;

		static RLSLPRuleBody create_char_item(SignatureWithRelativeLevel c)
		{
			return RLSLPRuleBody(c, -1, RLSLPRuleType::Character);
		}
		static RLSLPRuleBody create_pair_item(SignatureWithRelativeLevel left, SignatureWithRelativeLevel right)
		{
			return RLSLPRuleBody(left, right, RLSLPRuleType::Pair);
		}
		static RLSLPRuleBody create_signature_item(SignatureWithRelativeLevel single)
		{
			return RLSLPRuleBody(single, INT64_MAX, RLSLPRuleType::Signature);
		}
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
		static const RLSLPRuleBody &refer_body_of_base_signature(BaseSignature sig, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
		{
			return base_signature_rule_list[sig];
		}

		/*
		static RLSLPRuleBody create_pair_item(SignatureWithRelativeLevel single)
		{
			return RLSLPRuleBody(single, INT64_MAX, RLSLPRuleType::Pair);
		}
		*/

		static RLSLPRuleBody create_run_rule_body(SignatureWithRelativeLevel number, SignatureWithRelativeLevel power)
		{
			return RLSLPRuleBody(number, power, RLSLPRuleType::Power);
		}
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
		static RLSLPRuleBody create_null_item()
		{
			return RLSLPRuleBody(0, 0, RLSLPRuleType::Null);
		}

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
		uint64_t get_hash() const
		{
			int64_t c = this->A < this->B ? 3 : 4;
			return (this->type + 1) * (this->A * this->B + this->A + this->B) * c;
		}
		RLSLPRuleType get_type() const
		{
			return (RLSLPRuleType)this->type;
		}

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
		bool operator==(const RLSLPRuleBody &item) const
		{
			return (this->type == item.type) && (this->A == item.A) && (this->B == item.B);
		}

		static uint64_t get_byte()
		{
			return (2 * sizeof(SignatureWithRelativeLevel)) + sizeof(unsigned char);
		}
		std::string to_original_text_str(const std::vector<RLSLPRuleBody> &base_signature_rule_list) const
		{
			std::string r;
			this->decompress(base_signature_rule_list, r);
			return r;
		}
	};

}