#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "../../../rlslp/static_operations/run_rule_vector.hpp"
#include "../../../json_helper.hpp"


namespace dynRLSLP
{
/**
         * @brief A nonterminal together with a position offset during occurrence enumeration.
         * @ingroup ParentClasses
         */
        struct TemporaryOccurrence
        {
            ExplicitNonterminal nonterminal;
            uint64_t position;
            /**
             * @brief Constructs a temporary occurrence for a nonterminal at a position offset.
             * @param nonterminal Base nonterminal of the occurrence.
             * @param position Position offset within the expanded string.
             */
            TemporaryOccurrence(ExplicitNonterminal nonterminal, uint64_t position) : nonterminal(nonterminal), position(position)
            {
            }
            /**
             * @brief Default constructor initializing fields to sentinel maximum values.
             */
            TemporaryOccurrence() : nonterminal(std::numeric_limits<ExplicitNonterminal>::max()), position(std::numeric_limits<uint64_t>::max())
            {
            }

            /**
             * @brief Creates a null sentinel occurrence.
             * @return Temporary occurrence with maximum nonterminal and position values.
             */
            static TemporaryOccurrence create_null_occurrence()
            {
                return TemporaryOccurrence(std::numeric_limits<ExplicitNonterminal>::max(), std::numeric_limits<uint64_t>::max());
            }

            bool equals(const TemporaryOccurrence &other) const
            {
                return this->nonterminal == other.nonterminal && this->position == other.position;
            }

            /**
             * @brief Tests whether this occurrence is the null sentinel.
             * @return True if both fields hold their maximum sentinel values.
             */
            bool is_null() const
            {
                return this->nonterminal == std::numeric_limits<ExplicitNonterminal>::max() && this->position == std::numeric_limits<uint64_t>::max();
            }

            std::string to_string() const{
                return "(" + NonterminalFunctions::to_string(this->nonterminal) + ", " + std::to_string(this->position) + ")";
            }
        };
}