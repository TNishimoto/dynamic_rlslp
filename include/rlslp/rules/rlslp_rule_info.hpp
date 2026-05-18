#pragma once
#include "./rlslp_rule_body.hpp"

namespace dynRLSLP
{

        class RLSLPRuleInfo
        {
        public:
            RLSLPRuleBody body;

            uint64_t length;

            uint16_t level;

            /**
             * @brief Construct rule info from body, length, and level.
             * @param body Rule body or run rule body.
             * @param length Derived string length.
             * @param body Rule body or run rule body.
             * @param length Derived string length.
             * @param level Derivation-tree level.
             * @return Computed integer value.
             */
            RLSLPRuleInfo(RLSLPRuleBody body, int64_t length, int16_t level) : body(body), length(length), level(level)
            {
            }


        };
}