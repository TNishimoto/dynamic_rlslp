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
     * @brief Helper functions for NonterminalWithRelativeLevel.
     * @ingroup RLSLPClasses
     */
    class NonterminalFunctions
    {
    public:
        /**
         * @brief Return the base nonterminal of a nonterminal with relative level.
         */
        static uint64_t get_explicit_nonterminal(NonterminalWithRelativeLevel sig)
        {
            return (((uint64_t)sig) << 16) >> 16;
        }
        /**
         * @brief Return the length of the string derived by a given nonterminal with relative level.
         * @param explicit_nonterminal_length_list Base-nonterminal length list (L).
         */
        static uint64_t get_length(NonterminalWithRelativeLevel sig, const std::vector<uint64_t> &explicit_nonterminal_length_list)
        {
            return explicit_nonterminal_length_list[get_explicit_nonterminal(sig)];
        }
        /**
         * @brief Return the relative level of a nonterminal with relative level.
         */
        static uint16_t get_relative_level(NonterminalWithRelativeLevel sig)
        {
            return ((uint64_t)sig) >> 48;
        }
        /**
         * @brief Return whether the given nonterminal is a base nonterminal (relative level zero).
         */
        static bool is_explicit_nonterminal(NonterminalWithRelativeLevel sig)
        {
            return sig == (NonterminalWithRelativeLevel)get_explicit_nonterminal(sig);
        }
        /**
         * @brief Return a nonterminal with relative level from a given relative level and base nonterminal.
         */
        static NonterminalWithRelativeLevel get_nonterminal(uint16_t relative_level, ExplicitNonterminal explicit_nonterminal)
        {
            return ((uint64_t)relative_level << 48) | explicit_nonterminal;
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
         * @param sig Encoded nonterminal with relative level.
         * @return Resulting string.
         */
        static std::string to_string(NonterminalWithRelativeLevel sig)
        {
            uint64_t base_sig = NonterminalFunctions::get_explicit_nonterminal(sig);
            uint64_t level = NonterminalFunctions::get_relative_level(sig);
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
         * @brief Compute the mu-level of a base nonterminal from its length.
         * @param i Nonterminal or base-nonterminal index.
         * @param explicit_nonterminal_length_list_ Base-nonterminal length list.
         * @return Computed integer value.
         */
        static uint64_t get_mu_level_of_nonterminal(ExplicitNonterminal i, const std::vector<uint64_t> &explicit_nonterminal_length_list_)
        {
            uint64_t baseLen = explicit_nonterminal_length_list_[i];
            return get_mu_level(baseLen);
        }
        static uint16_t get_level(NonterminalWithRelativeLevel sig, const std::vector<uint16_t> &explicit_nonterminal_level_list_)
        {
            int64_t base_sig = NonterminalFunctions::get_explicit_nonterminal(sig);
            int64_t level = NonterminalFunctions::get_relative_level(sig);
            return (explicit_nonterminal_level_list_)[base_sig] + level;
        }

        /**
         * @brief Count uncountable single nonterminals for restricted recompression.
         * @param explicit_nonterminal Base nonterminal index.
         * @param explicit_nonterminal_length_list_ Base-nonterminal length list (D).
         * @param explicit_nonterminal_level_list_ Base-nonterminal level list (H).
         * @return Computed integer value.
         */
        static uint64_t count_uncountable_nonterminals(ExplicitNonterminal explicit_nonterminal, const std::vector<uint64_t> &explicit_nonterminal_length_list_, const std::vector<uint16_t> &explicit_nonterminal_level_list_)
        {
            uint64_t mu_level = NonterminalFunctions::get_mu_level_of_nonterminal(explicit_nonterminal, explicit_nonterminal_length_list_);
            uint64_t level = NonterminalFunctions::get_level(explicit_nonterminal, explicit_nonterminal_level_list_);
            if (mu_level >= level)
            {
                return mu_level - level + 1;
            }
            else
            {
                return 0;
            }
        }
    };

}
