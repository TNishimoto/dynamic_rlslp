#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include "./run_rule_vector.hpp"


namespace dynRLSLP
{
        using ShortStringInfo = std::pair<uint64_t, uint8_t>;


        /**
         * @brief XXXXXXXX
         * @ingroup RLSLPClasses
         */
        class ShortString{
            public:
            
			static uint64_t create_left_short_string_for_pair(SignatureWithRelativeLevel left_signature, SignatureWithRelativeLevel right_signature, 
                uint8_t char_bit_size, 
                const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t>& leftShortStringList) {
                uint64_t left_length = SignatureFunctions::get_length(left_signature, base_signature_length_list);
                uint64_t short_string_length = 64 / char_bit_size;
                uint64_t base_left_signature = SignatureFunctions::get_base_signature(left_signature);

                if(left_length >= short_string_length){
                    return leftShortStringList[base_left_signature];
                }else{
                    uint64_t base_right_signature = SignatureFunctions::get_base_signature(right_signature);

                    uint64_t leftStr = leftShortStringList[base_left_signature];
                    uint64_t rightStr = leftShortStringList[base_right_signature];

                    uint64_t result = leftStr | (rightStr >> (left_length * char_bit_size));


                    return result;
                }
			}
			static uint64_t create_right_short_string_for_pair(SignatureWithRelativeLevel left_signature, SignatureWithRelativeLevel right_signature, 
                uint8_t char_bit_size, 
                const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t>& rightShortStringList) {
                return create_left_short_string_for_pair(right_signature, left_signature, char_bit_size, base_signature_length_list, rightShortStringList);
			}


			static uint64_t create_left_short_string_for_power(SignatureWithRelativeLevel child_signature, uint64_t power, 
                uint8_t char_bit_size, 
                const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t>& leftShortStringList) {
                uint64_t child_length = SignatureFunctions::get_length(child_signature, base_signature_length_list);
                uint64_t short_string_length = 64 / char_bit_size;
                uint64_t base_child_signature = SignatureFunctions::get_base_signature(child_signature);

                if(child_length >= short_string_length){
                    return leftShortStringList[base_child_signature];
                }else{
                    uint64_t childStr = leftShortStringList[base_child_signature];
                    uint64_t result = 0;
                    uint64_t childStrLen = 0;

                    for(uint64_t i = 0; i < power; i++){
                        result = result | (childStr >> (i * child_length * char_bit_size));
                        childStrLen += child_length;
                        if(childStrLen >= short_string_length){
                            break;
                        }
                    }

                    return result;
                }
			}
			static uint64_t create_right_short_string_for_power(SignatureWithRelativeLevel child_signature, uint64_t power, 
                uint8_t char_bit_size, 
                const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t>& rightShortStringList) {
                return create_left_short_string_for_power(child_signature, power, char_bit_size, base_signature_length_list, rightShortStringList);
			}

            

			static uint64_t create_left_short_string_for_character(uint64_t character, 
                uint8_t char_bit_size, const std::unordered_map<int64_t, uint64_t> &character_id_map) {
                    uint64_t character_id = character_id_map.at(character);
                    return character_id << (64 - char_bit_size);
			}
			static uint64_t create_right_short_string_for_character(uint64_t character, 
                uint8_t char_bit_size, const std::unordered_map<int64_t, uint64_t> &character_id_map) {
                    return create_left_short_string_for_character(character, char_bit_size, character_id_map);
			}

            static std::string get_short_string_as_string(uint64_t short_string, uint64_t length, uint8_t alphabet_bit_size, const std::unordered_map<int64_t, uint64_t> &character_id_map){
                std::unordered_map<uint64_t, uint8_t> id_to_character;
                for(const auto &pair : character_id_map){
                    id_to_character[pair.second] = pair.first;
                }


                std::string result = "";
                for(uint64_t i = 0; i < length; i++){
                    uint64_t id = (short_string << (i * alphabet_bit_size)) >> (64 - alphabet_bit_size);
                    result.append(1, id_to_character[id]);
                }
                assert(result.size() == length);


                return result;
            }

            static std::string get_left_short_string_as_string(VStack<RunRuleBody> &item1, uint8_t alphabet_bit_size, const std::vector<uint64_t> &base_signature_length_list, 
                const std::vector<uint64_t> &leftShortStringList, const std::unordered_map<int64_t, uint64_t> &character_id_map){
                std::pair<uint64_t, uint8_t> result = get_left_short_string(item1, alphabet_bit_size, base_signature_length_list, leftShortStringList);
                return get_short_string_as_string(result.first, result.second / alphabet_bit_size, alphabet_bit_size, character_id_map);
            }

            static std::string get_right_short_string_as_string(VStack<RunRuleBody> &item1, uint8_t alphabet_bit_size, const std::vector<uint64_t> &base_signature_length_list, 
                const std::vector<uint64_t> &rightShortStringList, const std::unordered_map<int64_t, uint64_t> &character_id_map){
                std::pair<uint64_t, uint8_t> result = get_right_short_string(item1, alphabet_bit_size, base_signature_length_list, rightShortStringList);
                return get_short_string_as_string(result.first, result.second / alphabet_bit_size, alphabet_bit_size, character_id_map);
            }


            static ShortStringInfo get_left_short_string(VStack<RunRuleBody> &item1, uint8_t alphabet_bit_size, const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t> &leftShortStringList){
                VStack<RunRuleBody> tmp;
                uint64_t len = 0;
                uint64_t short_string = 0;
                //uint64_t short_string_length = 64 / alphabet_bit_size;
                while(len < 64 && item1.size() > 0){
                    RunRuleBody top = item1.top();
                    item1.pop();
                    tmp.push(top);
                    uint64_t base_child = SignatureFunctions::get_base_signature(top.number);
                    uint64_t base_child_len = SignatureFunctions::get_length(base_child, base_signature_length_list);
                    if(base_child_len * alphabet_bit_size >= (64 - len)){
                        short_string = short_string | (leftShortStringList[base_child] >> len);
                        len = 64;
                    }else{
                        uint64_t pow_len = base_child_len * top.power;
                        uint64_t pow_str = ShortString::create_left_short_string_for_power(base_child, top.power, alphabet_bit_size, base_signature_length_list, leftShortStringList);
                        if(pow_len * alphabet_bit_size >= (64 - len)){
                            short_string = short_string | (pow_str >> len);
                            len = 64;
                        }else{
                            short_string = short_string | (pow_str >> len);
                            len += pow_len * alphabet_bit_size;
                        }
                    }
                }

                while(tmp.size() > 0){
                    item1.push(tmp.top());
                    tmp.pop();
                }

                return std::make_pair(short_string, len);
            }
            static ShortStringInfo get_left_short_string(const std::vector<RunRuleBody> &item1, uint8_t alphabet_bit_size, const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t> &leftShortStringList){
                uint64_t len = 0;
                uint64_t short_string = 0;
                //uint64_t short_string_length = 64 / alphabet_bit_size;

                for(uint64_t i = 0; i < item1.size(); i++){
                    RunRuleBody top = item1[i];
                    uint64_t base_child = SignatureFunctions::get_base_signature(top.number);
                    uint64_t base_child_len = SignatureFunctions::get_length(base_child, base_signature_length_list);
                    if(base_child_len * alphabet_bit_size >= (64 - len)){
                        short_string = short_string | (leftShortStringList[base_child] >> len);
                        len = 64;
                        break;
                    }else{
                        uint64_t pow_len = base_child_len * top.power;
                        uint64_t pow_str = ShortString::create_left_short_string_for_power(base_child, top.power, alphabet_bit_size, base_signature_length_list, leftShortStringList);
                        if(pow_len * alphabet_bit_size >= (64 - len)){
                            short_string = short_string | (pow_str >> len);
                            len = 64;
                            break;
                        }else{
                            short_string = short_string | (pow_str >> len);
                            len += pow_len * alphabet_bit_size;
                        }
                    }
                }
                return std::make_pair(short_string, len);

            }


            static ShortStringInfo get_right_short_string(VStack<RunRuleBody> &item1, uint8_t alphabet_bit_size, const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t> &rightShortStringList){
                return get_left_short_string(item1, alphabet_bit_size, base_signature_length_list, rightShortStringList);
            }            
            static ShortStringInfo get_right_short_string(const std::vector<RunRuleBody> &item1, uint8_t alphabet_bit_size, const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t> &rightShortStringList){
                return get_left_short_string(item1, alphabet_bit_size, base_signature_length_list, rightShortStringList);
            }            

            static ShortStringInfo get_left_short_string(const RunRuleBody &body, 
                uint8_t char_bit_size, 
                const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t>& leftShortStringList) {
                    uint64_t len = SignatureFunctions::get_length(body.number, base_signature_length_list) * body.power;
                    uint64_t short_string = create_left_short_string_for_power(body.number, body.power, char_bit_size, base_signature_length_list, leftShortStringList);
                    return std::make_pair(short_string, std::min<uint64_t>(len, 64ULL / char_bit_size) * char_bit_size);

                }
            static ShortStringInfo get_right_short_string(const RunRuleBody &body, 
                uint8_t char_bit_size, 
                const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t>& rightShortStringList) {
                    uint64_t len = SignatureFunctions::get_length(body.number, base_signature_length_list) * body.power;
                    uint64_t short_string = create_left_short_string_for_power(body.number, body.power, char_bit_size, base_signature_length_list, rightShortStringList);
                    return std::make_pair(short_string, std::min<uint64_t>(len, 64ULL / char_bit_size) * char_bit_size);   
                }



            static std::pair<uint64_t, int8_t> short_lce(const std::pair<uint64_t, uint8_t> &str1, const std::pair<uint64_t, uint8_t> &str2, uint8_t char_bit_size){
                //uint64_t short_string_length = 64 / char_bit_size;


                //uint64_t base_str1 = SignatureFunctions::get_base_signature(str1.first);
                //uint64_t base_str2 = SignatureFunctions::get_base_signature(str2.first);                
                uint64_t len1 = str1.second;
                uint64_t len2 = str2.second;
                uint64_t minLen = std::min<uint64_t>(len1, len2);

                if(str1.first == str2.first){
                    if(len1 < len2){
                        return std::make_pair(len1 / char_bit_size, -1);
                    }else if(len1 > len2){
                        return std::make_pair(len2 / char_bit_size, 1);
                    }else{
                        return std::make_pair(minLen / char_bit_size, 0);
                    }
                }else{
                    uint64_t xor_val = str1.first ^ str2.first;
                    uint64_t diff = stool::Byte::number_of_leading_zero(xor_val);

                    if(diff <= minLen){
                        uint64_t lce = diff / char_bit_size;
                        uint64_t char1 = (str1.first << diff) >> (64 - char_bit_size);
                        uint64_t char2 = (str2.first << diff) >> (64 - char_bit_size);
                        assert(char1 != char2);
                        if(char1 < char2){
                            return std::make_pair(lce, -1);
                        }else{
                            return std::make_pair(lce, 1);
                        }
                    }else{
                        if(len1 < len2){
                            return std::make_pair(len1 / char_bit_size, -1);
                        }else if(len1 > len2){
                            return std::make_pair(len2 / char_bit_size, 1);
                        }else{
                            return std::make_pair(minLen / char_bit_size, 0);
                        }
                    }
                }
            }

            static std::pair<uint64_t, int8_t> short_reverse_lce(const std::pair<uint64_t, uint8_t> &str1, const std::pair<uint64_t, uint8_t> &str2, uint8_t char_bit_size){
                return short_lce(str1, str2, char_bit_size);
            }

            static bool verify_left_short_string(SignatureWithRelativeLevel signature, uint8_t alphabet_bit_size, const std::vector<RLSLPRuleBody> &base_signature_rule_list, const std::vector<uint64_t> &base_signature_length_list, const std::vector<uint64_t> &leftShortStringList, 
                const std::unordered_map<int64_t, uint64_t> &character_id_map){
                RLSLPRuleBody item = RLSLPRuleBody::decodeRule(signature, base_signature_rule_list);
                std::string original_string = item.to_original_text_str(base_signature_rule_list);
                uint64_t length = SignatureFunctions::get_length(signature, base_signature_length_list);
                uint64_t minLen = std::min<uint64_t>(length, 64ULL / alphabet_bit_size);

                uint64_t short_string = leftShortStringList[SignatureFunctions::get_base_signature(signature)];
                std::string short_string_str = get_short_string_as_string(short_string, minLen, alphabet_bit_size, character_id_map);
                assert(short_string_str.size() == minLen);

                std::string prefix = original_string.substr(0, minLen);
                
                if(short_string_str != prefix){
                    std::cout << std::endl;
                    RLSLPRuleBody item = RLSLPRuleBody::decodeRule(signature, base_signature_rule_list);
                    if(item.get_type() == RLSLPRuleType::Pair){
                        SignatureWithRelativeLevel left_signature = item.A;
                        SignatureWithRelativeLevel right_signature = item.B;
                        BaseSignature left_base_signature = SignatureFunctions::get_base_signature(left_signature);
                        BaseSignature right_base_signature = SignatureFunctions::get_base_signature(right_signature);
                        uint64_t left_short_string = leftShortStringList[left_base_signature];
                        uint64_t right_short_string = leftShortStringList[right_base_signature];
                        uint64_t left_short_string_len = SignatureFunctions::get_length(left_signature, base_signature_length_list);
                        uint64_t right_short_string_len = SignatureFunctions::get_length(right_signature, base_signature_length_list);
                        uint64_t left_min_length = std::min<uint64_t>(left_short_string_len, 64ULL / alphabet_bit_size);
                        uint64_t right_min_length = std::min<uint64_t>(right_short_string_len, 64ULL / alphabet_bit_size);
                        std::string left_short_string_str = get_short_string_as_string(left_short_string, left_min_length, alphabet_bit_size, character_id_map);
                        std::string right_short_string_str = get_short_string_as_string(right_short_string, right_min_length, alphabet_bit_size, character_id_map);
                        std::cout << "left_short_string_str: " << left_short_string_str << std::endl;
                        std::cout << "right_short_string_str: " << right_short_string_str << std::endl;
                    }
                    std::cout << "signature: " << signature << std::endl;
                    std::cout << "item: " << item.get_info() << std::endl;
                    std::cout << "original_string: " << original_string << std::endl;
                    std::cout << "short_string_str: " << short_string_str << std::endl;
                    std::cout << "prefix: " << prefix << std::endl;
                    std::cout << "minLen: " << minLen << std::endl;
                    std::cout << "short_string: " << std::bitset<64>(short_string) << std::endl;

                    throw std::runtime_error("verify_left_short_string: short_string_str != original_string");
                }
                return true;
            }


        };
    
}
