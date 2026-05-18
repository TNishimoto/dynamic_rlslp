#pragma once
#include "./rlslp_rule_body.hpp"

namespace dynRLSLP
{

        /**
         * @brief A representation of an RLSLP rule with its string-length and tree-height.
		 * @ingroup RLSLPClasses
         */
        class RLSLPRuleInfo
        {
        public:
            RLSLPRuleBody body;

            /** 
            * The length of the substring derived by the rule.
            */
            uint64_t length;

            /**
            * The height of the derivation tree corresponding to the rule.
            */
            uint16_t level;

            RLSLPRuleInfo(RLSLPRuleBody body, int64_t length, int16_t level) : body(body), length(length), level(level)
            {
            }


        };
}