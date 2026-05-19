#pragma once
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include "stool/include/basic/lsb_byte.hpp"

namespace dynRLSLP
{



        /**
         * @brief The type of factor parsing.
         * @ingroup LocalParsingClasses
         */
        enum class FactorParsingType {
            AlphabetReduction = 0, 
            RandomBit = 1   
        };

        /**
         * @brief The type of grammar parsing.
         * @ingroup LocalParsingClasses
         */
        enum class GrammarParsingType {
            SignatureEncoding = 0, 
            RestrictedBlockCompression = 1
        };

        /**
         * @brief A class for locally consistent parsing.
         * @ingroup LocalParsingClasses
         */
        class LocallyConsistentParsing
        {
            const static int LogIterateN = 6;

        public:
            /**
             * @brief Returns the left delta parameter for locally consistent parsing.
             * @return Left delta value.
             */
            static int get_delta_L()
            {
                return LogIterateN + 6;
            }
            /**
             * @brief Returns the right delta parameter for locally consistent parsing.
             * @return Right delta value.
             */
            static int get_delta_R()
            {
                return 5;
            }
            /**
             * @brief Returns the left consistent-context length.
             * @return Left consistent-context length.
             */
            static int get_consistent_L()
            {
                return get_delta_L() + 3;
            }
            /**
             * @brief Returns the right consistent-context length.
             * @return Right consistent-context length.
             */
            static int get_consistent_R()
            {
                return get_delta_R() + 3;
            }


            /**
             * @brief Computes locally consistent factor bits for a nonterminal sequence.
             * @param items Input sequence of distinct nonterminals.
             * @param output Output vector of factor bits (true marks a block boundary).
             */
            static void compute_factor_bits(const std::vector<int64_t> &items, std::vector<bool> &output)
            {
                assert(items.size() > 1);
                output.clear();
                output.resize(items.size());

                std::vector<int64_t> threeSequecne = compute_three_colors(items);
                assert(threeSequecne.size() > 1);
                for (int64_t i = 0; i < (int64_t)items.size(); i++)
                {
                    if (i == 0)
                    {
                        output[i] = -1 < threeSequecne[i] && threeSequecne[i] > threeSequecne[i + 1];
                    }
                    else if (i == (int64_t)items.size() - 1)
                    {
                        output[i] = threeSequecne[i - 1] < threeSequecne[i] && threeSequecne[i] > -1;
                    }
                    else
                    {
                        output[i] = threeSequecne[i - 1] < threeSequecne[i] && threeSequecne[i] > threeSequecne[i + 1];
                    }
                }
                verify(output);
            }
            /**
             * @brief Verifies that no two consecutive nonterminals are equal.
             * @param items Input nonterminal sequence.
             * @return True if the sequence is valid.
             * @throws std::runtime_error if adjacent nonterminals are equal.
             */
            static bool verify_input(const std::vector<int64_t> &items){
                for(int64_t i = 1; i < (int64_t)items.size(); i++){
                    if(items[i] == items[i - 1]){
                        throw std::runtime_error("Error in input_check: items[i] == items[i - 1]");
                    }
                }
                return true;
            }

        private:
            

            /**
             * @brief Returns the index of the least significant differing bit between two values.
             * @param a First value.
             * @param b Second value.
             * @return Bit position of the first difference.
             */
            static int64_t get_first_different_position(int64_t a, int64_t b)
            {

                int64_t p2 = a ^ b;
                int64_t s4 = stool::Byte::count_trailing_zeros(p2);
                return s4;
            }
            /**
             * @brief Computes the three-color label at position @p i from a six-color sequence.
             * @param seq Six-color sequence.
             * @param i Index of the position to label.
             * @return Three-color label in {0, 1, 2}.
             */
            static int compute_three_color_sub(std::vector<int64_t> &seq, int i)
            {
                int64_t left = i == 0 ? std::numeric_limits<int64_t>::max() : seq[i - 1];
                int64_t right = i == (int64_t)seq.size() - 1 ? std::numeric_limits<int64_t>::max() : seq[i + 1];

                if (left == 0)
                {
                    if (right == 0)
                    {
                        return 1;
                    }
                    else if (right == 1)
                    {
                        return 2;
                    }
                    else
                    {
                        return 1;
                    }
                }
                else if (left == 1)
                {
                    if (right == 0)
                    {
                        return 2;
                    }
                    else if (right == 1)
                    {
                        return 0;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    if (right == 0)
                    {
                        return 1;
                    }
                    else if (right == 1)
                    {
                        return 0;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }

            /**
             * @brief Verifies and repairs factor bits at both ends of the sequence.
             * @param bools Factor-bit vector to verify and adjust in place.
             */
            static void verify(std::vector<bool> &bools)
            {
                verify_prefix(bools);
                verify_suffix(bools);
                if (!bools[0] || bools[bools.size() - 1])
                    throw std::logic_error("LocallyConsistentParsing::verify: !bools[0] || bools[bools.size() - 1]");
            }
            /**
             * @brief Verifies and repairs factor bits at the prefix of the sequence.
             * @param bools Factor-bit vector to verify and adjust in place.
             */
            static void verify_prefix(std::vector<bool> &bools)
            {

                if (bools.size() == 2)
                {
                    bools[0] = true;
                    bools[1] = false;
                }
                else if (bools.size() == 3)
                {
                    bools[0] = true;
                    bools[1] = false;
                    bools[2] = false;
                }
                else if (!bools[0])
                {
                    if (bools[1] && !bools[2] && !bools[3])
                    {
                        bools[0] = true;
                        bools[1] = false;
                        bools[2] = true;
                    }
                    else if (bools[1] && !bools[2] && bools[3])
                    {
                        bools[0] = true;
                        bools[1] = false;
                    }
                    else
                    {
                        bools[0] = true;
                        bools[1] = false;
                    }
                }
            }
            /**
             * @brief Verifies and repairs factor bits at the suffix of the sequence.
             * @param bools Factor-bit vector to verify and adjust in place.
             */
            static void verify_suffix(std::vector<bool> &bools)
            {
                int end = (int)bools.size() - 1;
                if (bools[end])
                {
                    if (bools[end - 2])
                    {
                        bools[end] = false;
                    }
                    else
                    {
                        bools[end - 1] = true;
                        bools[end] = false;
                    }
                }
            }
            /**
             * @brief Reduces a six-color sequence to a three-color sequence.
             * @param seq Input six-color sequence.
             * @return Three-color sequence of the same length.
             */
            static std::vector<int64_t> compute_three_colors(const std::vector<int64_t> &seq)
            {
                int n = (int)seq.size();
                auto cSequence = compute_six_colors(seq);
                for (int c = 3; c <= 5; c++)
                {
                    for (int i = 0; i < n; i++)
                    {
                        if (cSequence[i] == c)
                        {
                            (cSequence)[i] = compute_three_color_sub(cSequence, i);
                        }
                    }
                }
                return cSequence;
            }
            /**
             * @brief Returns the number of six-color refinement iterations.
             * @return Iteration count derived from the delta-L parameter.
             */
            static int six_colors_loop_in_delta_L()
            {
                return LocallyConsistentParsing::LogIterateN + 2;
            }
            /**
             * @brief Computes a six-color labeling of the input sequence.
             * @param seq Input nonterminal sequence.
             * @return Six-color sequence of the same length.
             */
            static std::vector<int64_t> compute_six_colors(const std::vector<int64_t> &seq)
            {
                int64_t N = *std::max_element(seq.begin(), seq.end());
                std::vector<int64_t> cSequence(seq);

                // auto cSequence = unique_ptr<vector<int>>(new vector<int>(seq));
                //  if (!IsCreated) CreateArray();
                for (int i = 0; i < six_colors_loop_in_delta_L(); i++)
                {
                    if (i == 0)
                    {
                        cSequence = six_colors_sub(cSequence);
                    }
                    else
                    {
                        cSequence = six_colors_sub(cSequence);
                        // cSequence = FastSixColorsLoop(cSequence);
                    }
                }
                N = *std::max_element(cSequence.begin(), cSequence.end()) + 1;
                if (N > 6)
                    throw std::logic_error("LocallyConsistentParsing::verify: N > 6");
                return cSequence;
            }
            /**
             * @brief Performs one six-color refinement step on the sequence.
             * @param seq Input sequence for this refinement step.
             * @return Refined six-color sequence.
             */
            static std::vector<int64_t> six_colors_sub(const std::vector<int64_t> &seq)
            {
                auto r = std::vector<int64_t>(seq.size());
                r[0] = stool::LSBByte::get_bit(seq[0], 0) ? 1 : 0;
                // r[0] = 2;
                for (int i = 1; i < (int)seq.size(); i++)
                {
                    assert(seq[i] != seq[i - 1]);
                    int j = get_first_different_position(seq[i], seq[i - 1]);
                    if (j == -1){
                        throw std::runtime_error("Error in six_colors_sub: j == -1");
                    }
                    int b = stool::LSBByte::get_bit(seq[i], j) ? 1 : 0;
                    r[i] = 2 * j + b;
                    if (r[i] == r[i - 1])
                    {
                        throw std::logic_error("LocallyConsistentParsing::verify: r[i] == r[i - 1]");
                    }
                }
                return r;
            }
            
        };
    

}