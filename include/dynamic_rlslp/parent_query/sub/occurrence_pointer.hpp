#pragma once
#include <limits>
#include "../../../types/types.hpp"

namespace dynRLSLP
{
        /**
         * @brief Pointer to an occurrence node in the low-memory occurrence enumeration tree.
         * @ingroup ParentClasses
         */
        struct OccurrencePointer
        {
            NonterminalWithRelativeLevel nonterminal;
            uint64_t pointer;
            uint64_t positionOffset;
            /**
             * @brief Constructs an occurrence pointer for a nonterminal at a node with a position offset.
             * @param nonterminal The nonterminal of the occurrence.
             * @param pointer Index of the occurrence node, or UINT64_MAX for a leaf occurrence.
             * @param positionOffset Position offset relative to the parent occurrence.
             */
            OccurrencePointer(NonterminalWithRelativeLevel nonterminal, uint64_t pointer, uint64_t positionOffset) : nonterminal(nonterminal), pointer(pointer), positionOffset(positionOffset)
            {
            }
            /**
             * @brief Default constructor initializing all fields to sentinel maximum values.
             */
            OccurrencePointer() : nonterminal(std::numeric_limits<NonterminalWithRelativeLevel>::max()), pointer(std::numeric_limits<uint64_t>::max()), positionOffset(std::numeric_limits<uint64_t>::max()){

            }
        };
}