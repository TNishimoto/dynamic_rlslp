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
            SignatureWithRelativeLevel signature;
            uint64_t pointer;
            uint64_t positionOffset;
            /**
             * @brief Constructs an occurrence pointer for a signature at a node with a position offset.
             * @param signature The signature of the occurrence.
             * @param pointer Index of the occurrence node, or UINT64_MAX for a leaf occurrence.
             * @param positionOffset Position offset relative to the parent occurrence.
             */
            OccurrencePointer(SignatureWithRelativeLevel signature, uint64_t pointer, uint64_t positionOffset) : signature(signature), pointer(pointer), positionOffset(positionOffset)
            {
            }
            /**
             * @brief Default constructor initializing all fields to sentinel maximum values.
             */
            OccurrencePointer() : signature(std::numeric_limits<SignatureWithRelativeLevel>::max()), pointer(std::numeric_limits<uint64_t>::max()), positionOffset(std::numeric_limits<uint64_t>::max()){

            }
        };
}