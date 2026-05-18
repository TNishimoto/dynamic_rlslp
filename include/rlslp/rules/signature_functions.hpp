#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>
#include <stack>
#include <list>
#include "../../types/types.hpp"
#include "../../local_parsings/mu.hpp"

namespace dynRLSLP
{
    /**
     * @brief Encoding and decoding helpers for layered signatures.
     * @ingroup RLSLPClasses
     */
    class SignatureFunctions
    {
    public:
        /**
         * @brief Extract the base signature index from an encoded signature.
         * @param sig Encoded signature with relative level.
         * @return Base signature index.
         */
        static uint64_t get_base_signature(SignatureWithRelativeLevel sig)
        {
            return (((uint64_t)sig) << 16) >> 16;
        }
        /**
         * @brief Return the string length of a signature using the base length list.
         * @param sig Encoded signature with relative level.
         * @param base_signature_length_list Base-signature length list (L).
         * @return String length of \p sig.
         */
        static uint64_t get_length(SignatureWithRelativeLevel sig, const std::vector<uint64_t> &base_signature_length_list)
        {
            return base_signature_length_list[get_base_signature(sig)];
        }
        /**
         * @brief Extract the relative level from an encoded signature.
         * @param sig Encoded signature with relative level.
         * @return Level or height value.
         */
        static uint16_t get_relative_level(SignatureWithRelativeLevel sig)
        {
            return ((uint64_t)sig) >> 48;
        }
        /**
         * @brief Return whether the signature is a base signature (relative level zero).
         * @param sig Encoded signature with relative level.
         * @return Non-zero if \p sig is a base signature.
         */
        static uint64_t is_base_signature(SignatureWithRelativeLevel sig)
        {
            return sig == (SignatureWithRelativeLevel)get_base_signature(sig);
        }
        /**
         * @brief Encode a signature from a relative level and base signature index.
         * @param level_diff Relative level offset from the base signature.
         * @param base_signature Base signature index.
         * @return Encoded signature with relative level.
         */
        static SignatureWithRelativeLevel get_signature(uint16_t level_diff, BaseSignature base_signature)
        {
            return ((uint64_t)level_diff << 48) | base_signature;
        }
        /**
         * @brief Compute the mu-level for a string length.
         * @param baseLen Base string length.
         * @return Computed integer value.
         */
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
        /**
         * @brief Return a human-readable string representation.
         * @param sig Encoded signature with relative level.
         * @return Resulting string.
         */
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

        /**
         * @brief Compute the mu-level of a base signature from its length.
         * @param i Signature or base-signature index.
         * @param base_signature_length_list_ Base-signature length list.
         * @return Computed integer value.
         */
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

        /**
         * @brief Count uncountable single signatures for restricted recompression.
         * @param base_signature Base signature index.
         * @param base_signature_length_list_ Base-signature length list.
         * @param base_signature_level_list_ Base-signature level list.
         * @return Computed integer value.
         */
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
