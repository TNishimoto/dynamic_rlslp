#pragma once
#include <limits>
#include "../../../types/types.hpp"

namespace dynRLSLP
{
                /**
         * @brief XXXXXXXX
         * @ingroup ParentClasses
         */
        struct OccurrencePointer
        {
            SignatureWithRelativeLevel signature;
            uint64_t pointer;
            uint64_t positionOffset;
            OccurrencePointer(SignatureWithRelativeLevel signature, uint64_t pointer, uint64_t positionOffset) : signature(signature), pointer(pointer), positionOffset(positionOffset)
            {
            }
            OccurrencePointer() : signature(std::numeric_limits<SignatureWithRelativeLevel>::max()), pointer(std::numeric_limits<uint64_t>::max()), positionOffset(std::numeric_limits<uint64_t>::max()){

            }
        };
}