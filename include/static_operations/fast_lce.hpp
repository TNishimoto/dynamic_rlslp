#pragma once
#include <vector>
#include <iostream>
#include <list>
#include <stack>
#include <algorithm>
#include <limits>
#include "../rlslp/rlslp_rule_body.hpp"
#include "../rlslp/run_rule_vector.hpp"
#include "../rlslp/short_string.hpp"
#include "../types/types.hpp"

namespace dynRLSLP
{
		/**
		 * @brief XXXXXXXX
		 * @ingroup StaticOperationsClasses
		 */
        class FastLCE
        {

        private:
            static int64_t remove_common_power(VStack<RunRuleBody> &item1, VStack<RunRuleBody> &item2, const std::vector<uint64_t>& base_signature_length_list)
            {
                auto top1 = item1.top();
                auto top2 = item2.top();
                auto min = top1.power > top2.power ? top2.power : top1.power;
                auto matchedLen = SignatureFunctions::get_length(top1.number, base_signature_length_list) * min;

                if (min < top1.power)
                {
                    item1.pop();
                    item1.push(RunRuleBody(top1.number, top1.power - min));
                }
                else
                {
                    assert(min == top1.power);
                    item1.pop();
                }

                if (min < top2.power)
                {
                    item2.pop();
                    item2.push(RunRuleBody(top2.number, top2.power - min));
                }
                else
                {
                    assert(min == top2.power);
                    item2.pop();
                }
                return matchedLen;
            }
            static void break_front(VStack<RunRuleBody> &item, bool is_reverse, const std::vector<RLSLPRuleBody> &base_signature_rule_list)
            {
                RunRuleBody top = item.top();
                item.pop();

                if (top.power > 1)
                {
                    item.push(RunRuleBody(top.number, top.power - 1));
                }
                RLSLPRuleBody rule = RLSLPRuleBody::decodeRule(top.number, base_signature_rule_list);
                if (rule.get_type() == RLSLPRuleType::Pair)
                {
                    if (is_reverse)
                    {
                        item.push(RunRuleBody(rule.A, 1));
                        item.push(RunRuleBody(rule.B, 1));
                    }
                    else
                    {
                        item.push(RunRuleBody(rule.B, 1));
                        item.push(RunRuleBody(rule.A, 1));
                    }
                }
                else if (rule.get_type() == RLSLPRuleType::Power)
                {
                    item.push(RunRuleBody(rule.A, rule.B));
                }
                else if (rule.get_type() == RLSLPRuleType::Signature)
                {
                    SignatureWithRelativeLevel base_signature = SignatureFunctions::get_base_signature(top.number);
                    item.push(RunRuleBody(base_signature, 1));
                }
                else
                {
                    throw std::runtime_error("Error in break_front: rule.get_type() is not Pair, Power, or Signature");
                }
            }

        public:
            static inline int64_t total_lce_step_count = 0;
            static inline int64_t total_lce_count = 0;
            static inline int64_t total_only_short_lce_count = 0;
            static inline int64_t total_short_lce_count = 0;
            static inline int64_t total_lce_time = 0;
            static inline int64_t total_short_lce_time = 0;

            static inline int64_t lce_7_count = 0;
            static inline int64_t lce_15_count = 0;

            static std::pair<uint64_t, int8_t> lce(const RunRuleBody &body1, const RunRuleBody &body2, uint8_t alphabet_bit_size, const DictionaryForLayeredRLSLP &small_dic,
                                                   const std::vector<uint64_t> &leftShortStringList)
            {
                total_short_lce_count++;
                #ifdef TIME_DEBUG
                auto start = std::chrono::steady_clock::now();
                #endif

                const std::vector<uint64_t>& base_signature_length_list = small_dic.get_base_signature_length_list();
                ShortStringInfo short_string1 = ShortString::get_left_short_string(body1, alphabet_bit_size, base_signature_length_list, leftShortStringList);
                ShortStringInfo short_string2 = ShortString::get_left_short_string(body2, alphabet_bit_size, base_signature_length_list, leftShortStringList);
                std::pair<uint64_t, int8_t> result = ShortString::short_lce(short_string1, short_string2, alphabet_bit_size);

                #ifdef TIME_DEBUG
                auto end = std::chrono::steady_clock::now();
                total_short_lce_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                #endif

                if (result.second != 0)
                {
                    total_only_short_lce_count++;

                    if (result.first <= 7)
                    {
                        lce_7_count++;
                    }
                    if (result.first <= 15)
                    {
                        lce_15_count++;
                    }
                }
                else
                {
                    VStack<RunRuleBody> item1;
                    item1.push(body1);
                    VStack<RunRuleBody> item2;
                    item2.push(body2);
                    result = lce(item1, item2, small_dic);
                }

                #ifdef DEBUG
                VStack<RunRuleBody> item1;
                item1.push(body1);
                VStack<RunRuleBody> item2;
                item2.push(body2);

                std::pair<uint64_t, int8_t> result2 = lce(item1, item2, small_dic);
                if(result2.first != result.first || result2.second != result.second){
                    std::cout << "result: " << result.first << ", " << result.second << std::endl;
                    std::cout << "result2: " << result2.first << ", " << result2.second << std::endl;
                    throw std::runtime_error("Error in lce: result != result2");
                }
                #endif
                return result;
            }
            static std::pair<uint64_t, int8_t> lce(const RunRuleBody &body1, const std::vector<RunRuleBody> &body2, const ShortStringInfo &short_string2, uint8_t alphabet_bit_size, const DictionaryForLayeredRLSLP &small_dic,
                                                       const std::vector<uint64_t> &leftShortStringList)
            {
                total_short_lce_count++;
                #ifdef TIME_DEBUG
                auto start = std::chrono::steady_clock::now();
                #endif

                const std::vector<uint64_t>& base_signature_length_list = small_dic.get_base_signature_length_list();

                ShortStringInfo short_string1 = ShortString::get_left_short_string(body1, alphabet_bit_size, base_signature_length_list, leftShortStringList);
                std::pair<uint64_t, int8_t> result = ShortString::short_lce(short_string1, short_string2, alphabet_bit_size);
                #ifdef TIME_DEBUG
                auto end = std::chrono::steady_clock::now();
                total_short_lce_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                #endif

                if (result.second != 0)
                {
                    total_only_short_lce_count++;

                    if (result.first <= 7)
                    {
                        lce_7_count++;
                    }
                    if (result.first <= 15)
                    {
                        lce_15_count++;
                    }
                }
                else
                {
                    result = lce(body1, body2, small_dic);
                }

                #ifdef DEBUG
                std::pair<uint64_t, int8_t> result2 = lce(body1, body2, small_dic);
                if(result2.first != result.first || result2.second != result.second){
                    std::cout << "result: " << result.first << ", " << result.second << std::endl;
                    std::cout << "result2: " << result2.first << ", " << result2.second << std::endl;
                    throw std::runtime_error("Error in lce: result != result2");
                }
                #endif
                return result;
            }
            static std::pair<uint64_t, int8_t> lce(const RunRuleBody &body1, const std::vector<RunRuleBody> &body2, const DictionaryForLayeredRLSLP &small_dic)
            {
                VStack<RunRuleBody> item1;
                item1.push(body1);
                VStack<RunRuleBody> item2 = LevelSequenceFunction::create_stack(body2);
                return lce(item1, item2, small_dic);
            }

            static std::pair<uint64_t, int8_t> lce(const RunRuleBody &body1, const RunRuleBody &body2, const DictionaryForLayeredRLSLP &small_dic)
            {



                VStack<RunRuleBody> item1;
                item1.push(body1);
                VStack<RunRuleBody> item2;
                item2.push(body2);
                return lce(item1, item2, small_dic);
            }

            static std::pair<uint64_t, int8_t> lce(VStack<RunRuleBody> &item1, VStack<RunRuleBody> &item2, uint8_t alphabet_bit_size, const DictionaryForLayeredRLSLP &small_dic,
                                                   const std::vector<uint64_t> &leftShortStringList)
            {
                total_short_lce_count++;

                #ifdef TIME_DEBUG
                auto start = std::chrono::steady_clock::now();
                #endif
                const std::vector<uint64_t>& base_signature_length_list = small_dic.get_base_signature_length_list();
                ShortStringInfo short_string1 = ShortString::get_left_short_string(item1, alphabet_bit_size, base_signature_length_list, leftShortStringList);
                ShortStringInfo short_string2 = ShortString::get_left_short_string(item2, alphabet_bit_size, base_signature_length_list, leftShortStringList);
                std::pair<uint64_t, int8_t> result = ShortString::short_lce(short_string1, short_string2, alphabet_bit_size);

                #ifdef TIME_DEBUG
                auto end = std::chrono::steady_clock::now();
                total_short_lce_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                #endif

                if (result.second != 0)
                {
                    total_only_short_lce_count++;
                    if (result.first <= 7)
                    {
                        lce_7_count++;
                    }
                    if (result.first <= 15)
                    {
                        lce_15_count++;
                    }
                }
                else
                {
                    result = lce(item1, item2, small_dic);
                }
                return result;
            }
            static std::pair<uint64_t, int8_t> reverse_lce(const RunRuleBody &body1, const std::vector<RunRuleBody> &body2, ShortStringInfo short_string2, uint8_t alphabet_bit_size, const DictionaryForLayeredRLSLP &small_dic,
                                                           const std::vector<uint64_t> &rightShortStringList)
            {
                total_short_lce_count++;

                #ifdef TIME_DEBUG
                auto start = std::chrono::steady_clock::now();
                #endif
                const std::vector<uint64_t>& base_signature_length_list = small_dic.get_base_signature_length_list();


                ShortStringInfo short_string1 = ShortString::get_right_short_string(body1, alphabet_bit_size, base_signature_length_list, rightShortStringList);
                std::pair<uint64_t, int8_t> result = ShortString::short_reverse_lce(short_string1, short_string2, alphabet_bit_size);

                #ifdef TIME_DEBUG
                auto end = std::chrono::steady_clock::now();
                total_short_lce_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                #endif

                if (result.second != 0)
                {
                    total_only_short_lce_count++;
                    if (result.first <= 7)
                    {
                        lce_7_count++;
                    }
                    if (result.first <= 15)
                    {
                        lce_15_count++;
                    }
                }
                else
                {
                    VStack<RunRuleBody> item1;
                    item1.push(body1);
                    VStack<RunRuleBody> item2 = LevelSequenceFunction::create_stack(body2);
                    result = reverse_lce(item1, item2, small_dic);
                }

                #ifdef DEBUG
                std::pair<uint64_t, int8_t> result2 = reverse_lce(body1, body2, small_dic);
                if(result2.first != result.first || result2.second != result.second){
                    VStack<RunRuleBody> item1;
                    item1.push(body1);
                    VStack<RunRuleBody> item2 = LevelSequenceFunction::create_stack(body2);

                    /*
                    std::string x_short_string1 = ShortString::get_right_short_string_as_string(item1, alphabet_bit_size, base_signature_length_list, rightShortStringList);
                    std::string x_short_string2 = ShortString::get_right_short_string_as_string(item2, alphabet_bit_size, base_signature_length_list, rightShortStringList);
                    std::string x_short_string3 = ShortString::get_short_string_as_string(short_string2.first, short_string2.second / alphabet_bit_size, alphabet_bit_size);
                    std::cout << "x_short_string1: " << x_short_string1 << std::endl;
                    std::cout << "x_short_string2: " << x_short_string2 << std::endl;
                    std::cout << "x_short_string3: " << x_short_string3 << std::endl;
                    */


                    std::cout << "result: " << result.first << ", " << (int)result.second << std::endl;
                    std::cout << "result2: " << result2.first << ", " << (int)result2.second << std::endl;
                    throw std::runtime_error("Error in reverse_lce: result != result2");
                }
                #endif

                return result;
            }
            static std::pair<uint64_t, int8_t> reverse_lce(const RunRuleBody &body1, const std::vector<RunRuleBody> &body2, const DictionaryForLayeredRLSLP &small_dic)
            {
                VStack<RunRuleBody> item1;
                item1.push(body1);
                VStack<RunRuleBody> item2 = LevelSequenceFunction::create_stack(body2);
                return reverse_lce(item1, item2, small_dic);
            }

            static std::pair<uint64_t, int8_t> reverse_lce(const RunRuleBody &body1, const RunRuleBody &body2, uint8_t alphabet_bit_size, const DictionaryForLayeredRLSLP &small_dic,
                                                           const std::vector<uint64_t> &rightShortStringList)
            {
                total_short_lce_count++;

                #ifdef TIME_DEBUG
                auto start = std::chrono::steady_clock::now();
                #endif
                const std::vector<uint64_t>& base_signature_length_list = small_dic.get_base_signature_length_list();

                ShortStringInfo short_string1 = ShortString::get_right_short_string(body1, alphabet_bit_size, base_signature_length_list, rightShortStringList);
                ShortStringInfo short_string2 = ShortString::get_right_short_string(body2, alphabet_bit_size, base_signature_length_list, rightShortStringList);
                std::pair<uint64_t, int8_t> result = ShortString::short_reverse_lce(short_string1, short_string2, alphabet_bit_size);

                #ifdef TIME_DEBUG
                auto end = std::chrono::steady_clock::now();
                total_short_lce_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                #endif

                if (result.second != 0)
                {
                    total_only_short_lce_count++;
                    if (result.first <= 7)
                    {
                        lce_7_count++;
                    }
                    if (result.first <= 15)
                    {
                        lce_15_count++;
                    }
                }
                else
                {
                    VStack<RunRuleBody> item1;
                    item1.push(body1);
                    VStack<RunRuleBody> item2;
                    item2.push(body2);
                    result = reverse_lce(item1, item2, small_dic);
                }

                #ifdef DEBUG
                VStack<RunRuleBody> item1;
                item1.push(body1);
                VStack<RunRuleBody> item2;
                item2.push(body2);
                std::pair<uint64_t, int8_t> result2 = reverse_lce(item1, item2, small_dic);
                if(result2.first != result.first || result2.second != result.second){
                    std::cout << "result: " << result.first << ", " << result.second << std::endl;
                    std::cout << "result2: " << result2.first << ", " << result2.second << std::endl;
                    throw std::runtime_error("Error in reverse_lce: result != result2");
                }
                #endif

                return result;
            }

            static std::pair<uint64_t, int8_t> reverse_lce(const RunRuleBody &body1, const RunRuleBody &body2, const DictionaryForLayeredRLSLP &small_dic)
            {
                VStack<RunRuleBody> item1;
                item1.push(body1);
                VStack<RunRuleBody> item2;
                item2.push(body2);
                return reverse_lce(item1, item2, small_dic);
            }

            static std::pair<uint64_t, int8_t> reverse_lce(VStack<RunRuleBody> &item1, VStack<RunRuleBody> &item2, uint8_t alphabet_bit_size, const DictionaryForLayeredRLSLP &small_dic,
                                                           const std::vector<uint64_t> &rightShortStringList)
            {
                total_short_lce_count++;

                #ifdef TIME_DEBUG
                auto start = std::chrono::steady_clock::now();
                #endif
                const std::vector<uint64_t>& base_signature_length_list = small_dic.get_base_signature_length_list();


                std::pair<uint64_t, uint8_t> str1 = ShortString::get_right_short_string(item1, alphabet_bit_size, base_signature_length_list, rightShortStringList);
                std::pair<uint64_t, uint8_t> str2 = ShortString::get_right_short_string(item2, alphabet_bit_size, base_signature_length_list, rightShortStringList);
                std::pair<uint64_t, int8_t> result = ShortString::short_reverse_lce(str1, str2, alphabet_bit_size);

                #ifdef TIME_DEBUG
                auto end = std::chrono::steady_clock::now();
                total_short_lce_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                #endif
                
                if (result.second != 0)
                {
                    total_only_short_lce_count++;
                    if (result.first <= 7)
                    {
                        lce_7_count++;
                    }
                    if (result.first <= 15)
                    {
                        lce_15_count++;
                    }
                }
                else
                {
                    result = reverse_lce(item1, item2, small_dic);
                }
                return result;
            }

            static std::pair<uint64_t, int8_t> lce(VStack<RunRuleBody> &item1, VStack<RunRuleBody> &item2, const DictionaryForLayeredRLSLP &small_dic)
            {
                bool is_reverse = false;
                total_lce_count++;

                uint64_t matchedLength = 0;
                int8_t comp = 0;

                const std::vector<uint64_t>& base_signature_length_list = small_dic.get_base_signature_length_list();
                const std::vector<uint16_t>& base_signature_level_list = small_dic.get_base_signature_level_list();
                const std::vector<RLSLPRuleBody> &base_signature_rule_list = small_dic.get_base_signature_rule_list();

                #ifdef TIME_DEBUG
                auto start = std::chrono::steady_clock::now();
                #endif
                while (item1.size() > 0 && item2.size() > 0)
                {
                    total_lce_step_count++;
                    RunRuleBody top1 = item1.top();
                    RunRuleBody top2 = item2.top();

                    auto h1 = SignatureFunctions::get_level(top1.number, base_signature_level_list);
                    auto h2 = SignatureFunctions::get_level(top2.number, base_signature_level_list);

                    if (h1 == h2)
                    {
                        if (top1.number == top2.number)
                        {
                            matchedLength += remove_common_power(item1, item2, base_signature_length_list);
                        }
                        else
                        {
                            if (h1 == BSignatureBottomLevel && h2 == BSignatureBottomLevel)
                            {
                                RLSLPRuleBody front_item1 = RLSLPRuleBody::decodeRule(top1.number, base_signature_rule_list);
                                RLSLPRuleBody front_item2 = RLSLPRuleBody::decodeRule(top2.number, base_signature_rule_list);
                                assert(front_item1.get_type() == RLSLPRuleType::Character);
                                assert(front_item2.get_type() == RLSLPRuleType::Character);
                                auto c1 = front_item1.A;
                                auto c2 = front_item2.A;
                                assert(c1 != c2);

                                comp = c1 - c2 > 0 ? 1 : -1;
                                break;
                            }
                            else
                            {
                                break_front(item1, is_reverse, base_signature_rule_list);
                            }
                        }
                    }
                    else if (h1 < h2)
                    {
                        if (h2 == BSignatureBottomLevel)
                        {
                            break;
                        }
                        else
                        {
                            break_front(item2, is_reverse, base_signature_rule_list);
                        }
                    }
                    else
                    {
                        if (h1 == BSignatureBottomLevel)
                        {
                        }
                        else
                        {
                            break_front(item1, is_reverse, base_signature_rule_list);
                        }
                    }
                }

                #ifdef TIME_DEBUG
                auto end = std::chrono::steady_clock::now();
                total_lce_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                #endif

                if (matchedLength <= 7)
                {
                    lce_7_count++;
                }
                if (matchedLength <= 15)
                {
                    lce_15_count++;
                }

                if (item1.size() == 0 && item2.size() == 0)
                {
                    return std::pair<int64_t, int64_t>(matchedLength, 0);
                }
                else if (item1.size() == 0 && item2.size() > 0)
                {
                    return std::pair<int64_t, int64_t>(matchedLength, -1);
                }
                else if (item1.size() > 0 && item2.size() == 0)
                {
                    return std::pair<int64_t, int64_t>(matchedLength, 1);
                }
                else
                {
                    return std::pair<int64_t, int64_t>(matchedLength, comp);
                }
            }

            static std::pair<uint64_t, int8_t> reverse_lce(VStack<RunRuleBody> &item1, VStack<RunRuleBody> &item2, const DictionaryForLayeredRLSLP &small_dic)
            {
                total_lce_count++;
                bool is_reverse = true;

                const std::vector<uint64_t>& base_signature_length_list = small_dic.get_base_signature_length_list();
                const std::vector<uint16_t>& base_signature_level_list = small_dic.get_base_signature_level_list();
                const std::vector<RLSLPRuleBody> &base_signature_rule_list = small_dic.get_base_signature_rule_list();

                uint64_t matchedLength = 0;
                int8_t comp = 0;

                #ifdef TIME_DEBUG
                auto start = std::chrono::steady_clock::now();
                #endif

                while (item1.size() > 0 && item2.size() > 0)
                {
                    RunRuleBody top1 = item1.top();
                    RunRuleBody top2 = item2.top();

                    auto h1 = SignatureFunctions::get_level(top1.number, base_signature_level_list);
                    auto h2 = SignatureFunctions::get_level(top2.number, base_signature_level_list);

                    if (h1 == h2)
                    {
                        if (top1.number == top2.number)
                        {
                            matchedLength += remove_common_power(item1, item2, base_signature_length_list);
                        }
                        else
                        {
                            if (h1 == BSignatureBottomLevel && h2 == BSignatureBottomLevel)
                            {
                                RLSLPRuleBody front_item1 = RLSLPRuleBody::decodeRule(top1.number, base_signature_rule_list);
                                RLSLPRuleBody front_item2 = RLSLPRuleBody::decodeRule(top2.number, base_signature_rule_list);
                                assert(front_item1.get_type() == RLSLPRuleType::Character);
                                assert(front_item2.get_type() == RLSLPRuleType::Character);
                                auto c1 = front_item1.A;
                                auto c2 = front_item2.A;
                                #ifdef DEBUG
                                if(c1 == c2){
                                    std::cout << front_item1.get_info() << std::endl;
                                    std::cout << front_item2.get_info() << std::endl;
                                    std::cout << "c1: " << (char)c1 << ", c2: " << (char)c2 << std::endl;    
                                    std::cout << top1.number << " " << top2.number << std::endl;
                                }
                                #endif
                                assert(c1 != c2);

                                comp = c1 - c2 > 0 ? 1 : -1;
                                break;
                            }
                            else
                            {
                                break_front(item1, is_reverse, base_signature_rule_list);
                            }
                        }
                    }
                    else if (h1 < h2)
                    {
                        if (h2 == BSignatureBottomLevel)
                        {
                            break;
                        }
                        else
                        {
                            break_front(item2, is_reverse, base_signature_rule_list);
                        }
                    }
                    else
                    {
                        if (h1 == BSignatureBottomLevel)
                        {
                        }
                        else
                        {
                            break_front(item1, is_reverse, base_signature_rule_list);
                        }
                    }
                }

                #ifdef TIME_DEBUG
                auto end = std::chrono::steady_clock::now();
                total_lce_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                #endif

                if (matchedLength <= 7)
                {
                    lce_7_count++;
                }
                if (matchedLength <= 15)
                {
                    lce_15_count++;
                }

                if (item1.size() == 0 && item2.size() == 0)
                {
                    return std::pair<int64_t, int64_t>(matchedLength, 0);
                }
                else if (item1.size() == 0 && item2.size() > 0)
                {
                    return std::pair<int64_t, int64_t>(matchedLength, -1);
                }
                else if (item1.size() > 0 && item2.size() == 0)
                {
                    return std::pair<int64_t, int64_t>(matchedLength, 1);
                }
                else
                {
                    return std::pair<int64_t, int64_t>(matchedLength, comp);
                }
            }
        };
    
}