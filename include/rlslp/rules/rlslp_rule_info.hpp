#pragma once
#include "./rlslp_rule_body.hpp"

namespace dynRLSLP
{
	/**
	  * @page terminology Terminology
	  *
	  * @section term_layered_rlslp Layered RLSLP
	  * The layered RLSLP is a variant of the RLSLP that represents its derivation tree by level-by-level sequences S_{0}, S_{1}, ..., S_{H} of nonterminals. 
	  * Here, H is the height of the derivation tree, and S_{i} is the sequence of nonterminals at level i.
	  * A nonterminal *X* is called implicit if X produces a single nonterminal. Otherwise, it is called explicit.
	  * 
	  * @section term_val val(X)
	  * val(X) is the string represented by a given RLSLP Rule $X$.
	  * 
	  * @section left_string Left string of RLSLPRuleBody
	  * The left string of a RLSLPRuleBody refers to the substring represented by its first (A) child:
	  * - For a Pair rule, it is the expansion (val(A)) of the left (first) child.
	  * - For a Power rule, it is also the expansion of its (single) child (the repeated substring).
	  * - For other rule types (such as Character or Nonterminal), the left string is typically defined as the string itself or may be empty depending on context.
	  *
	  * @section right_string Right string of RLSLPRuleBody
	  * The right string of a RLSLPRuleBody refers to the substring represented by its second (B) child or the repeated part excluding the first occurrence:
	  * - For a Pair rule, it is the expansion (val(B)) of the right (second) child.
	  * - For a Power rule, it consists of all repetitions of the child except the first (i.e., *val(A)* repeated *B-1* times).
	  * - For other rule types (such as Character or Nonterminal), the right string is typically defined as an empty string or the string itself, depending on context.
	  *
	  */



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
