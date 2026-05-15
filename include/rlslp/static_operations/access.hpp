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
		 * @brief A class for accessing the string represented by an RLSLP grammar.
		 * @ingroup StaticOperationsClasses
		 */
		class Access
		{
		public:
			static std::pair<uint64_t, uint64_t> random_access(RLSLPRuleBody item, int64_t pos, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t>& base_signature_length_list)
			{
				std::pair<uint64_t, uint64_t> result = std::pair<uint64_t, uint64_t>(0, 0);
				if (item.get_type() == RLSLPRuleType::Pair)
				{
					auto left = RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list);
					auto leftLen = (int64_t)SignatureFunctions::get_length(item.A, base_signature_length_list);
					auto right = RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list);
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
				else if (item.get_type() == RLSLPRuleType::Power)
				{
					auto child = RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list);
					auto childLen = SignatureFunctions::get_length(item.A, base_signature_length_list);
					auto nextPos = pos % childLen;
					// auto nextPos = pos - (childLen * k);
					if (childLen == 1)
					{
						result.first = child.A;
						result.second = item.B;
					}
					else
					{
						result = Access::random_access(child, nextPos, base_signature_rule_list, base_signature_length_list);
					}
				}
				else if (item.get_type() == RLSLPRuleType::Character)
				{
					if (pos == 0)
					{
						result.first = item.A;
						result.second = 1;
					}
					else
					{
						throw std::logic_error("Access::random_access: pos is not 0");
					}
				}
				else if(item.get_type() == RLSLPRuleType::Signature){
					result = Access::random_access(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), pos, base_signature_rule_list, base_signature_length_list);
				}
				else
				{
					throw std::logic_error("Access::random_access: invalid item type");
				}
				// MyFunction::Print64_t(*this->GetLongString(itemList));
				// assert(item.getLongString(itemList)->at(pos) == result.first);
				assert((int64_t)result.second != -1);
				return result;
			}
			static std::string get_string(const std::vector<RunRuleBody> &items, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
			{				
				std::string r = "";
				for(auto item : items){
					RLSLPRuleBody item2 = RLSLPRuleBody::decodeRule(item.number, base_signature_rule_list);
					for(uint64_t i = 0; i < item.power; i++){
						r.append(Access::get_string(item2, base_signature_rule_list));
					}
				}
				return r;
			}

			static std::string get_string(RLSLPRuleBody item, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
			{				

				if (item.get_type() == RLSLPRuleType::Character)
				{
					std::string r;
					r.append(1, item.A);
					return r;
				}
				else if (item.get_type() == RLSLPRuleType::Pair)
				{
					auto left = Access::get_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
					auto right = Access::get_string(RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list), base_signature_rule_list);
					left.append(right);
					return left;
				}
				else if (item.get_type() == RLSLPRuleType::Power)
				{
					auto raw = Access::get_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
					std::string r;
					for (int64_t i = 0; i < item.B; i++)
					{
						r.append(raw);
					}
					return r;
				}
				else if(item.get_type() == RLSLPRuleType::Signature){
					return Access::get_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
				}
				else
				{
					throw std::logic_error("Access::get_string: invalid item type");
				}
			}
			

			template<typename ARRAY>
			static int64_t get_prefix(RLSLPRuleBody item, int64_t current_pos, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list, ARRAY &output){
				if (item.get_type() == RLSLPRuleType::Character)
				{
					//uint64_t character = item.A;
					output[current_pos++] = item.A;

					return current_pos;
				}
				else if (item.get_type() == RLSLPRuleType::Pair)
				{
					RLSLPRuleBody left = RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list);

					current_pos = Access::get_prefix(left, current_pos ,len,base_signature_rule_list, output);
					if(current_pos == (int64_t)len){
						return current_pos;
					}else{
						RLSLPRuleBody right = RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list);
						current_pos = Access::get_prefix(right, current_pos, len,base_signature_rule_list, output);
						return current_pos;
					}
				}
				else if (item.get_type() == RLSLPRuleType::Power)
				{
					for (int64_t i = 0; i < item.B; i++)
					{
						current_pos = Access::get_prefix(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), current_pos, len,base_signature_rule_list, output);
						if(current_pos == (int64_t)len){
							return current_pos;
						}
					}
					return current_pos;
				}
				else if(item.get_type() == RLSLPRuleType::Signature){
					return Access::get_prefix(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), current_pos, len,base_signature_rule_list, output);
				}
				else
				{
					throw std::runtime_error("Invalid item type");
				}
			}

			template<typename ARRAY>
			static int64_t get_suffix(RLSLPRuleBody item, int64_t current_pos, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list, ARRAY &output){
				if (item.get_type() == RLSLPRuleType::Character)
				{
					//uint64_t character = item.A;
					output[current_pos--] = item.A;

					return current_pos;
				}
				else if (item.get_type() == RLSLPRuleType::Pair)
				{
					RLSLPRuleBody right = RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list);

					current_pos = Access::get_suffix(right, current_pos ,len,base_signature_rule_list, output);
					if(current_pos == -1){
						return current_pos;
					}else{
						RLSLPRuleBody left = RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list);
						current_pos = Access::get_suffix(left, current_pos, len,base_signature_rule_list, output);
						return current_pos;
					}
				}
				else if (item.get_type() == RLSLPRuleType::Power)
				{
					for (int64_t i = 0; i < item.B; i++)
					{
						current_pos = Access::get_suffix(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), current_pos, len,base_signature_rule_list, output);
						if(current_pos == -1){
							return current_pos;
						}
					}
					return current_pos;
				}
				else if(item.get_type() == RLSLPRuleType::Signature){
					return Access::get_suffix(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), current_pos, len,base_signature_rule_list, output);
				}
				else
				{
					throw std::runtime_error("Invalid item type");
				}
			}

			static std::string get_prefix(RLSLPRuleBody item, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list){
				std::string r;
				r.resize(len);
				int64_t current_pos = 0;
				current_pos = Access::get_prefix(item, current_pos, len, base_signature_rule_list, r);
				if(current_pos != (int64_t)len){
					throw std::runtime_error("Invalid item type");
				}else{
					return r;
				}
			}

			static std::string get_suffix(RLSLPRuleBody item, uint64_t len, const std::vector<RLSLPRuleBody> &base_signature_rule_list){
				std::string r;
				r.resize(len);
				int64_t current_pos = len - 1;
				current_pos = Access::get_suffix(item, current_pos, len, base_signature_rule_list, r);
				if(current_pos != -1){
					throw std::runtime_error("Invalid item type");
				}else{
					return r;
				}
			}

			static std::string get_left_string(RLSLPRuleBody item, const std::vector<RLSLPRuleBody> &base_signature_rule_list){
				std::string r;
				if(item.get_type() == RLSLPRuleType::Pair){
					r = Access::get_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
					return r;
				}else if(item.get_type() == RLSLPRuleType::Power){
					r = Access::get_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
					return r;
				}else{
					return "";
				}
			}
			static std::string get_right_string(RLSLPRuleBody item, const std::vector<RLSLPRuleBody> &base_signature_rule_list){
				std::string r = "";
				if(item.get_type() == RLSLPRuleType::Pair){
					r = Access::get_string(RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list), base_signature_rule_list);
					return r;
				}else if(item.get_type() == RLSLPRuleType::Power){
					auto child_str = Access::get_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
					for(int64_t i = 0; i < item.B-1; i++){
						r.append(child_str);
					}
					return r;
				}else{
					return "";
				}
			}


			static std::vector<sig_char_type> get_long_string(RLSLPRuleBody item, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
			{
				if (item.get_type() == RLSLPRuleType::Character)
				{
					std::vector<sig_char_type> r;
					r.push_back(item.A);
					return r;
				}
				else if (item.get_type() == RLSLPRuleType::Pair)
				{
					auto left = Access::get_long_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
					auto right = Access::get_long_string(RLSLPRuleBody::decodeRule(item.B, base_signature_rule_list), base_signature_rule_list);
					for (auto it : right)
					{
						left.push_back(it);
					}
					return left;
				}
				else if (item.get_type() == RLSLPRuleType::Power)
				{
					std::vector<sig_char_type> raw = Access::get_long_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
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
				else if(item.get_type() == RLSLPRuleType::Signature){
					return Access::get_long_string(RLSLPRuleBody::decodeRule(item.A, base_signature_rule_list), base_signature_rule_list);
				}
				else
				{
					throw std::logic_error("Access::get_long_string: invalid item type");
				}
			}

		};
	
}