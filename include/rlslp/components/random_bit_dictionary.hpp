#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "../rules/rlslp_rule_info.hpp"

namespace dynRLSLP
{
    /**
     * @brief Random-bit storage for restricted block recompression.
     * @ingroup RLSLPClasses
     */
    class RandomBitDictionary
    {
    private:
        uint64_t seed = 0;
        std::mt19937_64 *mt = nullptr;
        std::uniform_int_distribution<uint64_t> dist;

        std::vector<uint16_t> shortRandomBits;
        std::unordered_map<NonterminalWithRelativeLevel, uint64_t> middleRandomBits;
        std::unordered_map<NonterminalWithRelativeLevel, std::vector<uint64_t>> longRandomBits;

    public:
        /** @brief Deleted copy constructor. */
        RandomBitDictionary(const RandomBitDictionary &) = delete;
        /** @brief Deleted copy assignment operator. */
        RandomBitDictionary &operator=(const RandomBitDictionary &) = delete;
        /** @brief Move constructor. */
        RandomBitDictionary(RandomBitDictionary &&other) noexcept
            : seed(other.seed),
              mt(other.mt),
              dist(std::move(other.dist)),
              shortRandomBits(std::move(other.shortRandomBits)),
              middleRandomBits(std::move(other.middleRandomBits)),
              longRandomBits(std::move(other.longRandomBits))
        {
            other.mt = nullptr;
        }
        /**
         * @brief Move assignment operator.
         * @param other Source dictionary.
         * @return Reference to this dictionary.
         */
        RandomBitDictionary &operator=(RandomBitDictionary &&other) noexcept
        {
            if (this != &other)
            {
                // First, clean up current resources
                this->clear();
                if (this->mt != nullptr)
                {
                    delete this->mt;
                    this->mt = nullptr;
                }

                // Move data members
                seed = other.seed;
                mt = other.mt;
                dist = std::move(other.dist);
                shortRandomBits = std::move(other.shortRandomBits);
                middleRandomBits = std::move(other.middleRandomBits);
                longRandomBits = std::move(other.longRandomBits);

                other.mt = nullptr;
            }
            return *this;
        }

        /**
         * @brief Construct an empty random-bit dictionary.
         */
        RandomBitDictionary() {

        };
        /**
         * @brief Destructor; releases resources.
         */
        ~RandomBitDictionary()
        {
            this->clear();
            if (this->mt != nullptr)
            {
                delete this->mt;
                this->mt = nullptr;
            }
        }
        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Main queries
        ////////////////////////////////////////////////////////////////////////////////
        //@{

        /**
         * @brief Return the random bit associated with a nonterminal.
         * @param nonterminal Encoded nonterminal with relative level.
         * @return Random bit value at the nonterminal's level.
         */
        bool get_random_bit(NonterminalWithRelativeLevel nonterminal) const
        {
            int64_t base_sig = NonterminalFunctions::get_explicit_nonterminal(nonterminal);
            int64_t level = NonterminalFunctions::get_relative_level(nonterminal);
            if (level <= 33)
            {
                int64_t pos = (level / 2) % 16;
                uint64_t mask = 1 << pos;
                bool b = (this->shortRandomBits[base_sig] & mask) != 0;
                return b;
            }
            else
            {
                level -= 34;
                if (level <= 129)
                {
                    int64_t pos = (level / 2) % 64;
                    uint64_t mask = 1 << pos;
                    auto f = this->middleRandomBits.find(base_sig);
                    assert(f != this->middleRandomBits.end());
                    uint64_t bits = f->second;
                    bool b = (bits & mask) != 0;
                    return b;
                }
                else
                {
                    level -= 130;
                    int64_t pos = (level / 2) % 64;
                    int64_t idx = (level / 2) / 64;
                    uint64_t mask = 1 << pos;
                    auto f = this->longRandomBits.find(base_sig);
                    assert(f != this->longRandomBits.end());
                    uint64_t bits = f->second[idx];
                    bool b = (bits & mask) != 0;
                    return b;
                }
            }
        }
        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Update operations
        ////////////////////////////////////////////////////////////////////////////////
        //@{

        /**
         * @brief Initialize the random-bit generator with a seed.
         * @param seed Random seed value.
         */
        void initialize(uint64_t seed)
        {
            this->clear();
            this->seed = seed;
            if (this->mt != nullptr)
            {
                delete this->mt;
            }
            this->mt = new std::mt19937_64(this->seed);
            this->dist = std::uniform_int_distribution<uint64_t>(0, std::numeric_limits<uint64_t>::max());
        }

        /**
         * @brief Swap contents with another instance.
         * @param other Other random-bit dictionary.
         */
        void swap(RandomBitDictionary &other)
        {
            this->shortRandomBits.swap(other.shortRandomBits);
            this->middleRandomBits.swap(other.middleRandomBits);
            this->longRandomBits.swap(other.longRandomBits);

            std::swap(this->mt, other.mt);
            std::swap(this->dist, other.dist);
            std::swap(this->seed, other.seed);
        }
        /** @brief Clear all random-bit storage. */
        void clear()
        {
            this->shortRandomBits.clear();
            this->middleRandomBits.clear();
            this->longRandomBits.clear();
        }
        /**
         * @brief Clear random bits for one base nonterminal.
         * @param explicit_nonterminal Base nonterminal index.
         */
        void clear(NonterminalWithRelativeLevel explicit_nonterminal)
        {
            this->shortRandomBits[explicit_nonterminal] = 0;
            this->middleRandomBits.erase(explicit_nonterminal);
            this->longRandomBits.erase(explicit_nonterminal);
        }
        /** @brief Append a new short random-bit slot. */
        void add_new_element()
        {
            this->shortRandomBits.push_back(0);
        }
        /**
         * @brief Remove random bits for a nonterminal.
         * @param nonterminal Encoded nonterminal with relative level.
         */
        void erase_random_bit(NonterminalWithRelativeLevel nonterminal)
        {
            uint64_t explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(nonterminal);
            uint64_t level = NonterminalFunctions::get_relative_level(nonterminal);
            // assert(level == relative_max_level_list[explicit_nonterminal]);
            if (level == 0)
            {
                this->shortRandomBits[explicit_nonterminal] = 0;
            }
            else if (level >= 34)
            {
                level -= 34;
                if (level == 0)
                {
                    this->middleRandomBits.erase(explicit_nonterminal);
                }
                else if (level >= 130)
                {
                    level -= 130;
                    int64_t pos = (level / 2) % 64;
                    bool b = (level % 2) == 0;
                    if (b && pos == 0)
                    {
                        this->longRandomBits[explicit_nonterminal].pop_back();

                        if (this->longRandomBits[explicit_nonterminal].size() == 0)
                        {
                            this->longRandomBits.erase(explicit_nonterminal);
                        }
                    }
                }
            }
        }
        /**
         * @brief Create random bits for a base nonterminal at a given single count.
         * @param explicit_nonterminal Base nonterminal index.
         * @param single_count Relative maximum level (single-nonterminal count).
         */
        void create_random_bit(ExplicitNonterminal explicit_nonterminal, uint64_t single_count)
        {
            assert(this->mt != nullptr);

            if (single_count == 0)
            {
                uint64_t rand_value = this->dist(*this->mt);
                this->shortRandomBits[explicit_nonterminal] = rand_value;
            }
            else if (single_count >= 34)
            {
                single_count -= 34;
                if (single_count == 0)
                {
                    uint64_t rand_value = this->dist(*this->mt);
                    this->middleRandomBits[explicit_nonterminal] = rand_value;
                }
                else if (single_count >= 130)
                {
                    single_count -= 130;
                    if (single_count == 0)
                    {
                        uint64_t rand_value = this->dist(*this->mt);
                        this->longRandomBits[explicit_nonterminal] = std::vector<uint64_t>();
                        this->longRandomBits[explicit_nonterminal].push_back(rand_value);
                    }
                    else
                    {
                        int64_t pos = (single_count / 2) % 64;
                        bool b = (single_count % 2) == 0;
                        if (b && pos == 0)
                        {
                            uint64_t rand_value = this->dist(*this->mt);
                            this->longRandomBits[explicit_nonterminal].push_back(rand_value);
                        }
                    }
                }
            }
        }
        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Print and verification functions
        ////////////////////////////////////////////////////////////////////////////////
        //@{

        /**
         * @brief Verify bitwise equality with another instance.
         * @param other Other random-bit dictionary.
         * @return True if equal; otherwise throws.
         */
        bool verify_equal(const RandomBitDictionary &other) const
        {
            if (this->shortRandomBits.size() != other.shortRandomBits.size())
            {
                throw std::runtime_error("Error in verify_equal(1): The size of short random bits must be equal.");
            }
            for (size_t i = 0; i < this->shortRandomBits.size(); ++i)
            {
                if (this->shortRandomBits[i] != other.shortRandomBits[i])
                {
                    throw std::runtime_error("Error in verify_equal(2): The short random bits must be equal.");
                }
            }

            if (this->middleRandomBits.size() != other.middleRandomBits.size())
            {
                throw std::runtime_error("Error in verify_equal(3): The size of middle random bits must be equal.");
            }
            for (const auto &pair : this->middleRandomBits)
            {
                auto it = other.middleRandomBits.find(pair.first);
                if (it == other.middleRandomBits.end())
                {
                    throw std::runtime_error("Error in verify_equal(4): The middle random bits must be equal.");
                }
                if (it->second != pair.second)
                {
                    throw std::runtime_error("Error in verify_equal(5): The middle random bits must be equal.");
                }
            }

            if (this->longRandomBits.size() != other.longRandomBits.size())
            {
                throw std::runtime_error("Error in verify_equal(6): The size of long random bits must be equal.");
            }
            for (const auto &pair : this->longRandomBits)
            {
                auto it = other.longRandomBits.find(pair.first);
                if (it == other.longRandomBits.end())
                {
                    throw std::runtime_error("Error in verify_equal(7): The long random bits must be equal.");
                }
                if (pair.second != it->second)
                {
                    throw std::runtime_error("Error in verify_equal(8): The long random bits must be equal.");
                }
            }

            return true;
        }
        /**
         * @brief Print summary statistics to standard output.
         * @param message_paragraph Indentation level for formatted output.
         */
        void print_statistics(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
        {
            uint64_t long_value_count = 0;
            for (const auto &[key, value] : this->longRandomBits)
            {
                long_value_count += value.size();
            }
            std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(RandomBitDictionary):" << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Short random bits: " << this->shortRandomBits.size() << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Middle random bits: " << this->middleRandomBits.size() << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Long random bits: " << this->longRandomBits.size() << ", counts: " << long_value_count << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
        }
        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Load, save, and builder functions
        ////////////////////////////////////////////////////////////////////////////////
        //@{

        /**
         * @brief Deserialize from a binary input stream.
         * @param seed Random seed used to initialize the generator after load.
         * @param ifs Input file stream.
         * @return Loaded random-bit dictionary.
         */
        static RandomBitDictionary load_from_file(uint64_t seed, std::ifstream &ifs)
        {
            RandomBitDictionary r;
            r.initialize(seed);

            uint64_t _shortRandomBits_count;
            ifs.read(reinterpret_cast<char *>(&_shortRandomBits_count), sizeof(_shortRandomBits_count));
            r.shortRandomBits.resize(_shortRandomBits_count);
            ifs.read(reinterpret_cast<char *>(r.shortRandomBits.data()), sizeof(uint16_t) * _shortRandomBits_count);

            uint64_t _middleRandomBits_count;
            ifs.read(reinterpret_cast<char *>(&_middleRandomBits_count), sizeof(_middleRandomBits_count));
            std::vector<NonterminalWithRelativeLevel> tmp_vec1;
            std::vector<uint64_t> tmp_vec2;
            tmp_vec1.resize(_middleRandomBits_count);
            tmp_vec2.resize(_middleRandomBits_count);
            ifs.read(reinterpret_cast<char *>(tmp_vec1.data()), sizeof(NonterminalWithRelativeLevel) * _middleRandomBits_count);
            ifs.read(reinterpret_cast<char *>(tmp_vec2.data()), sizeof(uint64_t) * _middleRandomBits_count);
            for (uint64_t i = 0; i < _middleRandomBits_count; i++)
            {
                r.middleRandomBits[tmp_vec1[i]] = tmp_vec2[i];
            }

            uint64_t _longRandomBits_count;
            ifs.read(reinterpret_cast<char *>(&_longRandomBits_count), sizeof(_longRandomBits_count));
            for (uint64_t i = 0; i < _longRandomBits_count; i++)
            {
                NonterminalWithRelativeLevel _sig;
                ifs.read(reinterpret_cast<char *>(&_sig), sizeof(NonterminalWithRelativeLevel));
                uint64_t vec_size;
                ifs.read(reinterpret_cast<char *>(&vec_size), sizeof(uint64_t));
                std::vector<uint64_t> tmp_vec;
                tmp_vec.resize(vec_size);
                ifs.read(reinterpret_cast<char *>(tmp_vec.data()), sizeof(uint64_t) * vec_size);
                r.longRandomBits[_sig] = tmp_vec;
            }
            return r;
        }
        /**
         * @brief Serialize to a binary output stream.
         * @param item Random-bit dictionary to store.
         * @param os Output file stream.
         */
        static void store_to_file(const RandomBitDictionary &item, std::ofstream &os)
        {
            uint64_t _shortRandomBits_count = item.shortRandomBits.size();
            os.write(reinterpret_cast<const char *>(&_shortRandomBits_count), sizeof(_shortRandomBits_count));
            os.write(reinterpret_cast<const char *>(item.shortRandomBits.data()), sizeof(uint16_t) * _shortRandomBits_count);

            uint64_t _middleRandomBits_count = item.middleRandomBits.size();
            os.write(reinterpret_cast<const char *>(&_middleRandomBits_count), sizeof(_middleRandomBits_count));

            std::vector<NonterminalWithRelativeLevel> tmp_vec1;
            std::vector<uint64_t> tmp_vec2;
            uint64_t k = 0;
            tmp_vec1.resize(_middleRandomBits_count);
            tmp_vec2.resize(_middleRandomBits_count);
            for (const auto &[key, value] : item.middleRandomBits)
            {
                tmp_vec1[k] = key;
                tmp_vec2[k] = value;
                k++;
            }
            os.write(reinterpret_cast<const char *>(tmp_vec1.data()), sizeof(NonterminalWithRelativeLevel) * _middleRandomBits_count);
            os.write(reinterpret_cast<const char *>(tmp_vec2.data()), sizeof(uint64_t) * _middleRandomBits_count);

            uint64_t _longRandomBits_count = item.longRandomBits.size();
            os.write(reinterpret_cast<const char *>(&_longRandomBits_count), sizeof(_longRandomBits_count));
            for (const auto &[key, value] : item.longRandomBits)
            {
                os.write(reinterpret_cast<const char *>(&key), sizeof(NonterminalWithRelativeLevel));
                uint64_t vec_size = value.size();
                os.write(reinterpret_cast<const char *>(&vec_size), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(value.data()), sizeof(uint64_t) * vec_size);
            }
        }
        //}@
    };

}
