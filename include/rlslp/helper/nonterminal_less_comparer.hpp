#pragma once
#include "../rules/all.hpp"

namespace dynRLSLP
{

    /**
     * @brief Comparator for nonterminal less than.
     * @ingroup RLSLPClasses
     */
    class NonterminalLessComparer
    {
    public:
        using is_transparent = void;
        static inline const std::vector<RLSLPRuleBody> *base_signature_rule_list = nullptr;
        NonterminalLessComparer()
        {
        }
        bool operator()(const int64_t left, const int64_t right) const
        {
            assert(base_signature_rule_list != nullptr);
            // assert(left < (int64_t)itemList->size());
            // assert(right < (int64_t)itemList->size());
            RLSLPRuleBody left_item = RLSLPRuleBody::decodeRule(left, *base_signature_rule_list);
            RLSLPRuleBody right_item = RLSLPRuleBody::decodeRule(right, *base_signature_rule_list);
            return left_item < right_item;
        }
        bool operator()(const int64_t left, const RLSLPRuleBody right) const
        {
            assert(base_signature_rule_list != nullptr);

            RLSLPRuleBody left_item = RLSLPRuleBody::decodeRule(left, *base_signature_rule_list);
            return left_item < right;
        }
        bool operator()(const RLSLPRuleBody left, const int64_t right) const
        {
            assert(base_signature_rule_list != nullptr);
            RLSLPRuleBody right_item = RLSLPRuleBody::decodeRule(right, *base_signature_rule_list);
            return left < right_item;
        }
    };

}