#pragma once
#include "./rlslp_rule_body.hpp"

namespace dynRLSLP
{

        /**
         * @brief A representation of an RLSLP rule with its string-length (The length of the substring derived by the rule) and tree-height (The height of the derivation tree corresponding to the rule).
		 * @ingroup RLSLPClasses
         */
        class RLSLPRuleInfo
        {
        public:
            RLSLPRuleBody item;
            uint64_t length;
            uint16_t level;

            RLSLPRuleInfo(RLSLPRuleBody item, int64_t length, int16_t level) : item(item), length(length), level(level)
            {
            }


        };
}