#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "./run_rule_vector.hpp"

namespace dynRLSLP
{
    class RandomBitDictionary
    {
    private:
        uint64_t seed = 0;
        std::mt19937_64 *mt = nullptr;
        std::uniform_int_distribution<uint64_t> dist;

        std::vector<uint16_t> shortRandomBits;
        std::unordered_map<SignatureWithRelativeLevel, uint64_t> middleRandomBits;
        std::unordered_map<SignatureWithRelativeLevel, std::vector<uint64_t>> longRandomBits;

    public:
        RandomBitDictionary(const RandomBitDictionary &) = delete;
        RandomBitDictionary &operator=(const RandomBitDictionary &) = delete;
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

        RandomBitDictionary() {

        };
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

        bool get_random_bit(SignatureWithRelativeLevel signature) const
        {
            int64_t base_sig = SignatureFunctions::get_base_signature(signature);
            int64_t level = SignatureFunctions::get_relative_level(signature);
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

        void swap(RandomBitDictionary &other)
        {
            this->shortRandomBits.swap(other.shortRandomBits);
            this->middleRandomBits.swap(other.middleRandomBits);
            this->longRandomBits.swap(other.longRandomBits);

            std::swap(this->mt, other.mt);
            std::swap(this->dist, other.dist);
            std::swap(this->seed, other.seed);
        }
        void clear()
        {
            this->shortRandomBits.clear();
            this->middleRandomBits.clear();
            this->longRandomBits.clear();
        }
        void clear(SignatureWithRelativeLevel base_signature)
        {
            this->shortRandomBits[base_signature] = 0;
            this->middleRandomBits.erase(base_signature);
            this->longRandomBits.erase(base_signature);
        }
        void add_new_element()
        {
            this->shortRandomBits.push_back(0);
        }
        void erase_random_bit(SignatureWithRelativeLevel signature)
        {
            uint64_t base_signature = SignatureFunctions::get_base_signature(signature);
            uint64_t level = SignatureFunctions::get_relative_level(signature);
            // assert(level == relative_max_level_list[base_signature]);
            if (level == 0)
            {
                this->shortRandomBits[base_signature] = 0;
            }
            else if (level >= 34)
            {
                level -= 34;
                if (level == 0)
                {
                    this->middleRandomBits.erase(base_signature);
                }
                else if (level >= 130)
                {
                    level -= 130;
                    int64_t pos = (level / 2) % 64;
                    bool b = (level % 2) == 0;
                    if (b && pos == 0)
                    {
                        this->longRandomBits[base_signature].pop_back();

                        if (this->longRandomBits[base_signature].size() == 0)
                        {
                            this->longRandomBits.erase(base_signature);
                        }
                    }
                }
            }
        }
        void create_random_bit(BaseSignature base_signature, uint64_t single_count)
        {
            assert(this->mt != nullptr);

            if (single_count == 0)
            {
                uint64_t rand_value = this->dist(*this->mt);
                this->shortRandomBits[base_signature] = rand_value;
            }
            else if (single_count >= 34)
            {
                single_count -= 34;
                if (single_count == 0)
                {
                    uint64_t rand_value = this->dist(*this->mt);
                    this->middleRandomBits[base_signature] = rand_value;
                }
                else if (single_count >= 130)
                {
                    single_count -= 130;
                    if (single_count == 0)
                    {
                        uint64_t rand_value = this->dist(*this->mt);
                        this->longRandomBits[base_signature] = std::vector<uint64_t>();
                        this->longRandomBits[base_signature].push_back(rand_value);
                    }
                    else
                    {
                        int64_t pos = (single_count / 2) % 64;
                        bool b = (single_count % 2) == 0;
                        if (b && pos == 0)
                        {
                            uint64_t rand_value = this->dist(*this->mt);
                            this->longRandomBits[base_signature].push_back(rand_value);
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
            std::vector<SignatureWithRelativeLevel> tmp_vec1;
            std::vector<uint64_t> tmp_vec2;
            tmp_vec1.resize(_middleRandomBits_count);
            tmp_vec2.resize(_middleRandomBits_count);
            ifs.read(reinterpret_cast<char *>(tmp_vec1.data()), sizeof(SignatureWithRelativeLevel) * _middleRandomBits_count);
            ifs.read(reinterpret_cast<char *>(tmp_vec2.data()), sizeof(uint64_t) * _middleRandomBits_count);
            for (uint64_t i = 0; i < _middleRandomBits_count; i++)
            {
                r.middleRandomBits[tmp_vec1[i]] = tmp_vec2[i];
            }

            uint64_t _longRandomBits_count;
            ifs.read(reinterpret_cast<char *>(&_longRandomBits_count), sizeof(_longRandomBits_count));
            for (uint64_t i = 0; i < _longRandomBits_count; i++)
            {
                SignatureWithRelativeLevel _sig;
                ifs.read(reinterpret_cast<char *>(&_sig), sizeof(SignatureWithRelativeLevel));
                uint64_t vec_size;
                ifs.read(reinterpret_cast<char *>(&vec_size), sizeof(uint64_t));
                std::vector<uint64_t> tmp_vec;
                tmp_vec.resize(vec_size);
                ifs.read(reinterpret_cast<char *>(tmp_vec.data()), sizeof(uint64_t) * vec_size);
                r.longRandomBits[_sig] = tmp_vec;
            }
            return r;
        }
        static void store_to_file(const RandomBitDictionary &item, std::ofstream &os)
        {
            uint64_t _shortRandomBits_count = item.shortRandomBits.size();
            os.write(reinterpret_cast<const char *>(&_shortRandomBits_count), sizeof(_shortRandomBits_count));
            os.write(reinterpret_cast<const char *>(item.shortRandomBits.data()), sizeof(uint16_t) * _shortRandomBits_count);

            uint64_t _middleRandomBits_count = item.middleRandomBits.size();
            os.write(reinterpret_cast<const char *>(&_middleRandomBits_count), sizeof(_middleRandomBits_count));

            std::vector<SignatureWithRelativeLevel> tmp_vec1;
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
            os.write(reinterpret_cast<const char *>(tmp_vec1.data()), sizeof(SignatureWithRelativeLevel) * _middleRandomBits_count);
            os.write(reinterpret_cast<const char *>(tmp_vec2.data()), sizeof(uint64_t) * _middleRandomBits_count);

            uint64_t _longRandomBits_count = item.longRandomBits.size();
            os.write(reinterpret_cast<const char *>(&_longRandomBits_count), sizeof(_longRandomBits_count));
            for (const auto &[key, value] : item.longRandomBits)
            {
                os.write(reinterpret_cast<const char *>(&key), sizeof(SignatureWithRelativeLevel));
                uint64_t vec_size = value.size();
                os.write(reinterpret_cast<const char *>(&vec_size), sizeof(uint64_t));
                os.write(reinterpret_cast<const char *>(value.data()), sizeof(uint64_t) * vec_size);
            }
        }
        //}@
    };

}
