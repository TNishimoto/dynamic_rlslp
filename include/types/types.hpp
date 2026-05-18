#pragma once
#include <cstdint>
#include <string>
#include <stack>
#include <vector>
namespace dynRLSLP
{

		/** @brief Encodes an RLSLP signature together with its relative level in a single integer. */
		using SignatureWithRelativeLevel = int64_t;
		/** @brief Character type used in signature-related string data. */
		using sig_char_type = char;
        /** @brief 64-bit key type for quaternary-tree indexing structures. */
        using QuaternaryKey = uint64_t;
        /** @brief Base signature identifier without relative level encoding. */
        using BaseSignature = int64_t;

		/** @brief Bottom level value for base signatures in layered RLSLP representations. */
		const inline int64_t BSignatureBottomLevel = 0;
		/** @brief Upper bound on relative level values used in signature encoding. */
		const inline int64_t LEVEL_LIMIT = 10000;

		/**
		 * @brief Stack adapter using `std::vector` as the underlying container.
		 * @tparam T Element type stored in the stack.
		 */
		template<typename T>
		using VStack = std::stack<T, std::vector<T>>;

		/**
		 * @brief The type of an RLSLP rule.
		 * @ingroup EnumClasses
		 */
		enum class RLSLPRuleType
		{
			Null = 0,
			Character = 1,
			Pair = 2,
			Power = 3,
			End = 4,
			Signature = 5			
		};
        /**
		 * @brief Kind of locate query used when searching pattern occurrences.
		 * @ingroup EnumClasses
		 */
        enum class LocateQueryType { STANDARD = 0, CHAR = 1, RUN = 2, EMPTY = 3 };

		        /**
		 * @brief Flags describing how parent/signature managers are organized.
		 * @ingroup EnumClasses
		 */
		enum class ManagerFlag : uint8_t
        {
            None = 0,
            Single = 1,
            FewParentsManager = 2,
			Vector = 3
        };

				        /**
		 * @brief Which child of a grammar rule is referenced (left, right, power, or signature).
		 * @ingroup EnumClasses
		 */
		enum class ChildType : uint8_t
        {
			None = 0, 
			LeftChild = 1,
			RightChild = 2,
			PowerChild = 3,
			SignatureChild = 4
		};

    
}
