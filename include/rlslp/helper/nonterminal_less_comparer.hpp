#pragma once
#include "../rules/all.hpp"

namespace dynRLSLP
{

    /**
     * @brief Strict weak ordering of nonterminals by decoded rule bodies.
     * @ingroup RLSLPClasses
     */
    class NonterminalLessComparer
    {
    public:
        using is_transparent = void;
        static inline const std::vector<RLSLPRuleBody> *base_signature_rule_list = nullptr;
        /**
         * @brief Default constructor.
         */
        NonterminalLessComparer()
        {
        }
        /**
         * @brief Compare two signatures by their decoded rule bodies.
         * @param left Left signature index.
         * @param right Right signature index.
         * @return True if the left rule body is lexicographically smaller.
         */
        bool operator()(const int64_t left, const int64_t right) const
        {
            assert(base_signature_rule_list != nullptr);
            // assert(left < (int64_t)itemList->size());
            // assert(right < (int64_t)itemList->size());
            RLSLPRuleBody left_item = RLSLPRuleBody::decodeRule(left, *base_signature_rule_list);
            RLSLPRuleBody right_item = RLSLPRuleBody::decodeRule(right, *base_signature_rule_list);
            return left_item < right_item;
        }
        /**
         * @brief Compare a signature with a rule body.
         * @param left Left signature index.
         * @param right Rule body to compare against.
         * @return True if the decoded left rule is lexicographically smaller.
         */
        bool operator()(const int64_t left, const RLSLPRuleBody right) const
        {
            assert(base_signature_rule_list != nullptr);

            RLSLPRuleBody left_item = RLSLPRuleBody::decodeRule(left, *base_signature_rule_list);
            return left_item < right;
        }
        /**
         * @brief Compare a rule body with a signature.
         * @param left Rule body on the left.
         * @param right Right signature index.
         * @return True if \p left is lexicographically smaller than the decoded right rule.
         */
        bool operator()(const RLSLPRuleBody left, const int64_t right) const
        {
            assert(base_signature_rule_list != nullptr);
            RLSLPRuleBody right_item = RLSLPRuleBody::decodeRule(right, *base_signature_rule_list);
            return left < right_item;
        }
    };

}