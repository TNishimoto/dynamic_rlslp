#pragma once
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include "stool/include/basic/byte.hpp"

namespace dynRLSLP
{

		/**
		 * @brief A naive algorithm for parsing a given sequence
         * @ingroup LocalParsingClasses
		 */
		class NaiveParsing
		{
		public:
			/**
			 * @brief Computes factor bits using a naive popcount-based rule.
			 * @param items Input nonterminal sequence.
			 * @param output Output vector of factor bits (true marks a block boundary).
			 */
			static void compute_factor_bits(const std::vector<int64_t> &items, std::vector<bool> &output)
			{
				output.clear();
				output.resize(items.size());

				output[0] = false;
				for (uint64_t i = 1; i < items.size() - 1; i++)
				{
					uint64_t p1 = stool::Byte::popcount(items[i - 1]), p2 = stool::Byte::popcount(items[i]), p3 = stool::Byte::popcount(items[i + 1]);
					output[i] = p1 <= p2 && p2 > p3;
				}
				output[items.size() - 1] = false;
			}
		};
	

}
