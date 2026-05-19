#pragma once
#include "../rules/all.hpp"

namespace dynRLSLP
{

    /**
     * @brief Strict weak ordering of nonterminals by their rule bodies.
     * @ingroup RLSLPClasses
     */
    class NonterminalLessComparer
    {
    public:
        using is_transparent = void;
        static inline const std::vector<RLSLPRuleBody> *explicit_nonterminal_rule_list = nullptr;
        /**
         * @brief Default constructor.
         */
        NonterminalLessComparer()
        {
        }
        /**
         * @brief Compare two nonterminals by their rule bodies.
         * @param left Left nonterminal.
         * @param right Right nonterminal.
         * @return True if the left nonterminal is lexicographically smaller.
         */
        bool operator()(const int64_t left, const int64_t right) const
        {
            assert(explicit_nonterminal_rule_list != nullptr);
            // assert(left < (int64_t)itemList->size());
            // assert(right < (int64_t)itemList->size());
            RLSLPRuleBody left_item = RLSLPRuleBody::decode_rule(left, *explicit_nonterminal_rule_list);
            RLSLPRuleBody right_item = RLSLPRuleBody::decode_rule(right, *explicit_nonterminal_rule_list);
            return left_item < right_item;
        }
        /**
         * @brief Compare a nonterminal with a rule body.
         * @param left Left nonterminal.
         * @param right Right rule body.
         * @return True if the left nonterminal is lexicographically smaller.
         */
        bool operator()(const int64_t left, const RLSLPRuleBody right) const
        {
            assert(explicit_nonterminal_rule_list != nullptr);

            RLSLPRuleBody left_item = RLSLPRuleBody::decode_rule(left, *explicit_nonterminal_rule_list);
            return left_item < right;
        }
        /**
         * @brief Compare a rule body with a nonterminal.
         * @param left Left rule body.
         * @param right Right nonterminal.
         * @return True if the left rule body is lexicographically smaller.
         */
        bool operator()(const RLSLPRuleBody left, const int64_t right) const
        {
            assert(explicit_nonterminal_rule_list != nullptr);
            RLSLPRuleBody right_item = RLSLPRuleBody::decode_rule(right, *explicit_nonterminal_rule_list);
            return left < right_item;
        }
    };

}