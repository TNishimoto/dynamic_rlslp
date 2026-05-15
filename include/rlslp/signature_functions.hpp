#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include <stack>
#include <list>
#include "../types/types.hpp"
#include "../local_parsings/mu.hpp"
#include "../types/types.hpp"

namespace dynRLSLP
{
    class SignatureFunctions
    {
    public:
        static uint64_t get_base_signature(SignatureWithRelativeLevel sig)
        {
            return (((uint64_t)sig) << 16) >> 16;
        }
        static uint64_t get_length(SignatureWithRelativeLevel sig, const std::vector<uint64_t> &base_signature_length_list)
        {
            return base_signature_length_list[get_base_signature(sig)];
        }
        static uint16_t get_relative_level(SignatureWithRelativeLevel sig)
        {
            return ((uint64_t)sig) >> 48;
        }
        static uint64_t is_base_signature(SignatureWithRelativeLevel sig)
        {
            return sig == (SignatureWithRelativeLevel)get_base_signature(sig);
        }
        static SignatureWithRelativeLevel get_signature(uint16_t level_diff, BaseSignature base_signature)
        {
            return ((uint64_t)level_diff << 48) | base_signature;
        }
        static uint64_t get_mu_level(uint64_t baseLen)
        {
            uint64_t h = 0;
            uint64_t mu_floor = dynRLSLP::Mu::mu_floor(h + 1);
            while (baseLen > mu_floor)
            {
                h++;
                mu_floor = dynRLSLP::Mu::mu_floor(h + 1);
            }
            return h;
        }
        static std::string to_string(SignatureWithRelativeLevel sig)
        {
            uint64_t base_sig = SignatureFunctions::get_base_signature(sig);
            uint64_t level = SignatureFunctions::get_relative_level(sig);
            if (level == 0)
            {
                return std::to_string(base_sig);
            }
            else
            {
                return std::to_string(base_sig) + "(D" + std::to_string(level) + ")";
            }
        }

        static uint64_t get_mu_level_of_signature(BaseSignature i, const std::vector<uint64_t> &base_signature_length_list_)
        {
            uint64_t baseLen = base_signature_length_list_[i];
            return get_mu_level(baseLen);
        }
        static uint16_t get_level(SignatureWithRelativeLevel sig, const std::vector<uint16_t> &base_signature_level_list_)
        {
            int64_t base_sig = SignatureFunctions::get_base_signature(sig);
            int64_t level = SignatureFunctions::get_relative_level(sig);
            return (base_signature_level_list_)[base_sig] + level;
        }

        static uint64_t count_uncountable_signatures(BaseSignature base_signature, const std::vector<uint64_t> &base_signature_length_list_, const std::vector<uint16_t> &base_signature_level_list_)
        {
            uint64_t mu_level = SignatureFunctions::get_mu_level_of_signature(base_signature, base_signature_length_list_);
            uint64_t level = SignatureFunctions::get_level(base_signature, base_signature_level_list_);
            if (mu_level >= level)
            {
                return mu_level - level + 1;
            }
            else
            {
                return 0;
            }
        }
        /*
        template <typename OUTPUT_VEC_TYPE = std::vector<uint8_t>>
        static void decompress(SignatureWithRelativeLevel sig, const std::vector<RLSLPRuleBody> &base_signature_rule_list, OUTPUT_VEC_TYPE &output){
            RLSLPRuleBody item = RLSLPRuleBody::decodeRule(sig, base_signature_rule_list);
            RLSLPRuleBody::decompress(item, base_signature_rule_list, output);
        }
        */
    };

}
