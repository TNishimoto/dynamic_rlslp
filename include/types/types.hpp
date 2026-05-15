#pragma once
#include <cstdint>
#include <string>
#include <stack>
#include <vector>
namespace dynRLSLP
{

		using SignatureWithRelativeLevel = int64_t;
		using sig_char_type = char;
        using QuaternaryKey = uint64_t;
        using BaseSignature = int64_t;

		const inline int64_t BSignatureBottomLevel = 0;
		const inline int64_t LEVEL_LIMIT = 10000;

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
		 * @brief XXXXXXXX
		 * @ingroup EnumClasses
		 */
        enum class LocateQueryType { STANDARD = 0, CHAR = 1, RUN = 2, EMPTY = 3 };

		        /**
		 * @brief XXXXXXXX
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
		 * @brief XXXXXXXX
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
