#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <filesystem>
#include "dynamic_rlslp/dynamic_grammar_for_layered_rlslp.hpp"
#include "updates/all.hpp"

namespace dynRLSLP
{
    /**
     * @brief A class for representing a dynamic string \p T[0..n-1] compressed by an RLSLP grammar \p G.
     * @ingroup SearchClasses
     */
    class DynamicRLSLPString
    {
        DictionaryMode dictionaryMode = DictionaryMode::Standard;
        DynamicGrammarForLayeredRLSLP dynamic_grammar;

        // Data Structures for Fast Mode
        std::vector<uint64_t> left_short_string_list;
        std::vector<uint64_t> right_short_string_list;
        std::vector<TemporaryOccurrence> ancestor_cache_list;

        inline static const uint64_t ANCESTOR_CACHE_DEPTH = 10;

    public:
        inline static const uint64_t FINGERPRINT = 12737564839;

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Constructors and Destructor
        ////////////////////////////////////////////////////////////////////////////////
        //@{

        /**
         * @brief Default constructor with an empty dictionary.
         * @param use_restricted_block_compression If true, use restricted block compression during grammar updates.
         * @param seed Random seed for compression and hashing.
         */
        DynamicRLSLPString(GrammarParsingType parser = GrammarParsingType::RestrictedRecompression, int64_t seed = 0) : dynamic_grammar()
        {
            dynamic_grammar.initialize(parser, seed);
        }
        /**
         * @brief Construct an empty dynamic string with an explicit alphabet.
         * @param use_restricted_block_compression If true, use restricted block compression during grammar updates.
         * @param alphabet Explicit alphabet as byte values.
         * @param seed Random seed for compression and hashing.
         */
        DynamicRLSLPString(GrammarParsingType parser, const std::vector<uint8_t> &alphabet, int64_t seed) : dynamic_grammar()
        {
            dynamic_grammar.initialize(parser, alphabet, seed);
        }

        /**
         * @brief Deleted copy constructor.
         */
        DynamicRLSLPString(const DynamicRLSLPString &) = delete;
        /**
         * @brief Move constructor; transfers grammar and auxiliary caches from \p other.
         * @param other Source instance to move from.
         */
        DynamicRLSLPString(DynamicRLSLPString &&other) noexcept
            : dictionaryMode(std::move(other.dictionaryMode)),
              dynamic_grammar(std::move(other.dynamic_grammar)),
              left_short_string_list(std::move(other.left_short_string_list)),
              right_short_string_list(std::move(other.right_short_string_list)),
              ancestor_cache_list(std::move(other.ancestor_cache_list))

        {
        }
        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Operators
        ////////////////////////////////////////////////////////////////////////////////
        //@{
        /**
         * @brief Deleted copy assignment operator.
         */
        DynamicRLSLPString &operator=(const DynamicRLSLPString &) = delete;

        /**
         * @brief Move assignment operator; transfers grammar and auxiliary caches from \p other.
         * @param other Source instance to move from.
         * @return Reference to this instance after the move.
         */
        DynamicRLSLPString &operator=(DynamicRLSLPString &&other) noexcept
        {
            if (this != &other)
            {
                this->dynamic_grammar = std::move(other.dynamic_grammar);
                this->left_short_string_list = std::move(other.left_short_string_list);
                this->right_short_string_list = std::move(other.right_short_string_list);
                this->ancestor_cache_list = std::move(other.ancestor_cache_list);
                this->dictionaryMode = std::move(other.dictionaryMode);
            }
            return *this;
        }

        /**
         * @brief Random access to character \p T[pos]; alias for access().
         * @param pos Position in the string (0-based).
         * @return Character at position \p pos.
         */
        uint8_t operator[](size_t pos) const
        {
            if (this->dynamic_grammar.get_distinct_document_count() == 0)
            {
                throw std::runtime_error("DynamicRLSLPString::operator[]: No document");
            }
            else
            {
                const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();
                const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
                const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();
                const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();

                uint64_t nonterminal = grammar.get_root();
                RLSLPRuleBody item = small_dic.get_rule_body(nonterminal);
                auto c = Access::random_access(item, pos, explicit_nonterminal_rule_list, explicit_nonterminal_length_list);
                return c;
            }
        }

        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Lightweight functions for accessing to properties of this class
        ////////////////////////////////////////////////////////////////////////////////
        //@{
        /**
         * @brief Return the right short-string cache used in Fast dictionary mode.
         * @return Const reference to the right short-string list.
         */
        const std::vector<uint64_t> &get_right_short_string_list() const
        {
            return this->right_short_string_list;
        }
        /**
         * @brief Return the left short-string cache used in Fast dictionary mode.
         * @return Const reference to the left short-string list.
         */
        const std::vector<uint64_t> &get_left_short_string_list() const
        {
            return this->left_short_string_list;
        }

        /**
         * @brief Return the current dictionary operating mode.
         * @return Active \p DictionaryMode (Standard or Fast).
         */
        DictionaryMode get_dictionary_mode() const
        {
            return this->dictionaryMode;
        }
        /**
         * @brief Return a mutable reference to the layered RLSLP grammar dictionary \p G.
         * @return Reference to the internal \p DynamicGrammarForLayeredRLSLP.
         */
        DynamicGrammarForLayeredRLSLP &get_dictionary()
        {
            return this->dynamic_grammar;
        }
        /**
         * @brief Return a const reference to the layered RLSLP grammar dictionary \p G.
         * @return Const reference to the internal \p DynamicGrammarForLayeredRLSLP.
         */
        const DynamicGrammarForLayeredRLSLP &get_dictionary() const
        {
            return this->dynamic_grammar;
        }
        /**
         * @brief Return \p true if \p |T| = 0. Otherwise, return \p false.
         * @return Whether the compressed string is empty.
         */
        bool is_empty() const
        {
            return this->dynamic_grammar.get_distinct_document_count() == 0;
        }
        /**
         * @brief Return \p |T|.
         * @return Length of the represented string in characters.
         */
        uint64_t size() const
        {
            if (this->is_empty())
            {
                return 0;
            }
            else
            {
                const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
                NonterminalWithRelativeLevel nonterminal = grammar.get_root();
                const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();
                return NonterminalFunctions::get_length(nonterminal, explicit_nonterminal_length_list);
            }
        }
        /**
         * @brief Return the alphabet of \p T as a vector of characters.
         * @return Sorted distinct byte values occurring in the string.
         */
        std::vector<uint8_t> get_alphabet() const
        {
            return this->dynamic_grammar.get_alphabet();
        }
        /**
         * @brief Get the number of non-null rules in the base nonterminal rule list of \p G.
         * @return Count of grammar rules excluding null nonterminals.
         */
        uint64_t get_grammar_size() const
        {
            return this->dynamic_grammar.nonterminal_count_without_null_nonterminals();
        }

        /**
         * @brief Returns the total memory usage in bytes.
         * @param only_dynamic_memory If true, only the size of dynamically allocated memory is returned.
         * @return Total memory footprint in bytes.
         */
        uint64_t size_in_bytes(bool only_dynamic_memory = false) const
        {
            uint64_t total = 0;
            total += stool::Memory::estimate_memory_usage(this->left_short_string_list);
            total += stool::Memory::estimate_memory_usage(this->right_short_string_list);
            total += stool::Memory::estimate_memory_usage(this->ancestor_cache_list);
            total += this->dynamic_grammar.size_in_bytes(only_dynamic_memory);
            total += (sizeof(DynamicRLSLPString)) - (sizeof(DynamicGrammarForLayeredRLSLP));
            return total;
        }

        /**
         * @brief Return the length of the string derived by the first variable v_{1} in the body of the rule \p D[i] -> v_{1}, v_{2}, ..., v_{k}, which is stored in \p G.
         * @param i Signature (with relative level) of the rule.
         * @return Length of the left-derived substring for a Pair or Power rule.
         */
        uint64_t get_left_string_length(NonterminalWithRelativeLevel i) const
        {
            const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();
            RLSLPRuleBody item = small_dic.get_rule_body(i);
            const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();
            if (item.get_type() == RLSLPRuleType::Pair)
            {
                return NonterminalFunctions::get_length(item.A, explicit_nonterminal_length_list);
            }
            else if (item.get_type() == RLSLPRuleType::Power)
            {
                return NonterminalFunctions::get_length(item.A, explicit_nonterminal_length_list);
            }
            else
            {
                throw std::runtime_error("Not implemented");
            }
        }

        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Main queries
        ////////////////////////////////////////////////////////////////////////////////
        //@{
        /**
         * @brief Return T[i].
         * @param i Position in the string (0-based).
         * @return Character at position \p i.
         * @note O(log n) time.
         */
        uint8_t access_char(uint64_t i) const
        {
            return this->operator[](i);
        }

        std::vector<uint8_t> access_substring(uint64_t i, uint64_t len) const
        {
            std::vector<uint8_t> result;
            const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();
            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
            NonterminalWithRelativeLevel root = grammar.get_root();
            RLSLPRuleBody item = small_dic.get_rule_body(root);
            const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();
            const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();

            Access::random_access(item, i, len, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, result);
            return result;
        }

        void decompress(std::ofstream &ofs, uint64_t buffer_size = 1024 * 1024 * 1024) const
        {
            uint64_t current_pos = 0;
            uint64_t length = this->size();

            while (current_pos < length)
            {
                if (current_pos + buffer_size > length)
                {
                    buffer_size = length - current_pos;
                }
                std::vector<uint8_t> buffer = this->access_substring(current_pos, buffer_size);
                ofs.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
                current_pos += buffer_size;
            }
        }

        /**
         * @brief Compute the longest common prefix length of suffixes starting at \p i and \p j.
         * @param i Starting position of the first suffix.
         * @param j Starting position of the second suffix.
         * @return Length of the longest common prefix of \p T[i..] and \p T[j..].
         */
        uint64_t lce(uint64_t i, uint64_t j) const
        {
            const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();

            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
            std::vector<RunRuleBody> item1 = LevelSequenceFunction::substring(grammar.get_root(), i, small_dic);
            std::vector<RunRuleBody> item2 = LevelSequenceFunction::substring(grammar.get_root(), j, small_dic);

            VStack<RunRuleBody> st1;
            for (int64_t k = item1.size() - 1; k >= 0; k--)
            {
                st1.push(item1[k]);
            }
            VStack<RunRuleBody> st2;
            for (int64_t k = item2.size() - 1; k >= 0; k--)
            {
                st2.push(item2[k]);
            }

            std::pair<uint64_t, int8_t> result = FastLCE::lce(st1, st2, small_dic);
            return result.first;
        }

        uint64_t lcp(ExplicitNonterminal X1, ExplicitNonterminal X2) const
        {
            const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();


            RunRuleBody rbody1 = RunRuleBody(X1, 1);
            RunRuleBody rbody2 = RunRuleBody(X2, 1);

            if (this->get_dictionary_mode() == DictionaryMode::Fast)
            {
                const std::vector<uint64_t> &leftShortStringList = this->get_left_short_string_list();
                auto result = FastLCE::lce(rbody1, rbody2, this->dynamic_grammar.get_alphabet_bit_size(), small_dic, leftShortStringList);
                return result.first;
            }
            else
            {
                auto result = FastLCE::lce(rbody1, rbody2, small_dic);
                return result.first;
            }
        }

        uint64_t reverse_lce(uint64_t i, uint64_t j) const
        {
            const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();

            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
            std::vector<RunRuleBody> item1 = LevelSequenceFunction::substring(grammar.get_root(), 0, i + 1, small_dic);
            std::vector<RunRuleBody> item2 = LevelSequenceFunction::substring(grammar.get_root(), 0, j + 1, small_dic);

            VStack<RunRuleBody> st1;
            for (auto &item : item1)
            {
                st1.push(item);
            }
            VStack<RunRuleBody> st2;
            for (auto &item : item2)
            {

                st2.push(item);
            }

            std::pair<uint64_t, int8_t> result = FastLCE::reverse_lce(st1, st2, small_dic);
            return result.first;
        }

        uint64_t lcs(ExplicitNonterminal X1, ExplicitNonterminal X2) const
        {
            const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();


            RunRuleBody rbody1 = RunRuleBody(X1, 1);
            RunRuleBody rbody2 = RunRuleBody(X2, 1);

            if(this->get_dictionary_mode() == DictionaryMode::Fast){
                const std::vector<uint64_t> &rightShortStringList = this->get_right_short_string_list();
                auto result = FastLCE::reverse_lce(rbody1, rbody2, this->dynamic_grammar.get_alphabet_bit_size(), small_dic, rightShortStringList);
                return result.first;
            }
            else{
                auto result = FastLCE::reverse_lce(rbody1, rbody2, small_dic);
                return result.first;
            }
        }


        /**
         * @brief Resolve temporary occurrences to absolute text positions.
         * @param input List of temporary occurrence descriptors to expand.
         * @return All occurrence positions in the current string.
         */
        std::vector<uint64_t> get_all_occurrences(const std::vector<TemporaryOccurrence> &input) const
        {
            if (this->dictionaryMode == DictionaryMode::Fast)
            {
                return this->dynamic_grammar.faster_get_all_occurrences_using_memory(input, &this->ancestor_cache_list);
            }
            else
            {
                return this->dynamic_grammar.faster_get_all_occurrences(input);
            }
        }

        std::vector<uint64_t> get_all_occurrences(ExplicitNonterminal explicit_nonterminal) const
        {
            std::vector<TemporaryOccurrence> input;
            input.push_back(TemporaryOccurrence(explicit_nonterminal, 0));
            return this->get_all_occurrences(input);
        }

        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Convert functions
        ////////////////////////////////////////////////////////////////////////////////
        //@{
        /**
         * @brief Return \p T as a string.
         * @return Decompressed text as a \p std::string.
         */
        std::string to_string() const
        {
            if (this->is_empty())
            {
                return "";
            }
            const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();
            const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();
            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
            NonterminalWithRelativeLevel root = grammar.get_root();
            RLSLPRuleBody item = small_dic.get_rule_body(root);
            std::string s = Access::get_string(item, explicit_nonterminal_rule_list);

            return s;
        }
        /**
         * @brief Alias of to_string().
         * @return Decompressed text as a \p std::string.
         */
        std::string get_text_str() const
        {
            return this->to_string();
        }

        /**
         * @brief Return \p T as a byte vector by decompressing the grammar root.
         * @return Decompressed text as \p std::vector<uint8_t>.
         */
        std::vector<uint8_t> to_vector() const
        {
            if (this->is_empty())
            {
                return std::vector<uint8_t>();
            }
            else
            {
                std::vector<uint8_t> text;
                const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();
                const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
                NonterminalWithRelativeLevel root = grammar.get_root();
                RLSLPRuleBody rootBody = RLSLPRuleBody::decode_rule(root, explicit_nonterminal_rule_list);
                rootBody.decompress(explicit_nonterminal_rule_list, text);
                return text;
            }
        }

        StaticRLSLP convert_to_rlslp() const
        {
            return this->dynamic_grammar.convert_to_rlslp();
        }

        StaticRLSLP convert_to_canonized_rlslp() const
        {
            return this->dynamic_grammar.convert_to_canonized_rlslp();
        }

        void write_content_as_json_format(std::ofstream &ofs, std::string name, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
        {
            ofs << stool::Message::get_paragraph_string(message_paragraph) << "\"" << name << "\": " << "{" << std::endl;

            std::string dictionaryMode_str;
            if (this->dictionaryMode == DictionaryMode::Standard)
            {
                dictionaryMode_str = "\"Standard\"";
            }
            else
            {
                dictionaryMode_str = "\"Fast\"";
            }

            ofs << stool::Message::get_paragraph_string(message_paragraph + 2) << "\"dictionaryMode\": " << dictionaryMode_str << ", " << std::endl;
            this->dynamic_grammar.write_content_as_json_format(ofs, "dynamic_grammar", message_paragraph + 2);
            ofs << ", " << std::endl;

            JsonHelper::write_content_as_json_format<uint64_t>(
                "left_short_string_list(std::vector<uint64_t>)",
                this->left_short_string_list,
                [](const uint64_t &value)
                { return std::to_string(value); },
                true,
                ofs,
                false,
                message_paragraph + 2);
            ofs << ", " << std::endl;

            JsonHelper::write_content_as_json_format<uint64_t>(
                "right_short_string_list(std::vector<uint64_t>)",
                this->right_short_string_list,
                [](const uint64_t &value)
                { return std::to_string(value); },
                true,
                ofs,
                false,
                message_paragraph + 2);
            ofs << ", " << std::endl;

            JsonHelper::write_content_as_json_format<TemporaryOccurrence>(
                "ancestor_cache_list(std::vector<TemporaryOccurrence>)",
                this->ancestor_cache_list,
                [](const TemporaryOccurrence &value)
                { return value.to_string(); },
                true,
                ofs,
                false,
                message_paragraph + 2);
            ofs << std::endl;

            // ofs << stool::Message::get_paragraph_string(message_paragraph+1) << "}" << std::endl;
            ofs << stool::Message::get_paragraph_string(message_paragraph) << "}" << std::endl;
        }

        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Update operations
        ////////////////////////////////////////////////////////////////////////////////
        //@{
        /**
         * @brief Swap the contents of this instance with \p other.
         * @param other Instance to exchange data with.
         */
        void swap(DynamicRLSLPString &other)
        {
            this->dynamic_grammar.swap(other.dynamic_grammar);
            this->ancestor_cache_list.swap(other.ancestor_cache_list);
            std::swap(this->dictionaryMode, other.dictionaryMode);
            std::swap(this->left_short_string_list, other.left_short_string_list);
            std::swap(this->right_short_string_list, other.right_short_string_list);
        }
        /**
         * @brief Set the dictionary operating mode and rebuild Fast-mode auxiliary structures when needed.
         * @param mode Target \p DictionaryMode (Standard or Fast).
         */
        void set_mode(DictionaryMode mode)
        {
            if (this->dictionaryMode != mode)
            {
                this->dictionaryMode = mode;
                this->left_short_string_list.clear();
                this->right_short_string_list.clear();
                this->ancestor_cache_list.clear();

                if (mode == DictionaryMode::Fast)
                {
                    if (!this->dynamic_grammar.has_explicit_alphabet())
                    {
                        throw std::runtime_error("set_mode: DictionaryMode::Fast is not supported when the alphabet is not explicit");
                    }

                    uint64_t ruleSize = this->dynamic_grammar.count_explicit_nonterminals();
                    this->left_short_string_list.resize(ruleSize, UINT64_MAX);
                    this->right_short_string_list.resize(ruleSize, UINT64_MAX);

                    this->rebuild_short_string_list();
                    this->rebuild_ancestor_cache_list();
                }
            }
        }
        /**
         * @brief Clear the elements in \p T and \p G.
         */
        void clear()
        {
            this->dynamic_grammar.clear();
            this->ancestor_cache_list.clear();
            this->left_short_string_list.clear();
            this->right_short_string_list.clear();
        }
        /**
         * @brief Set the alphabet of \p T to the given alphabet \p alphabet (not implemented yet).
         * @param alphabet New alphabet as byte values.
         */
        void set_alphabet([[maybe_unused]] const std::vector<uint8_t> &alphabet)
        {
        }

        /**
         * @brief Insert a byte pattern into \p T at position \p i, invoking callbacks on grammar changes.
         * @tparam CALLBACK1 Callable invoked before a nonterminal is removed (\p NonterminalWithRelativeLevel).
         * @tparam CALLBACK2 Callable invoked after a nonterminal is inserted (\p NonterminalWithRelativeLevel).
         * @param i Insert position (0 .. |T|).
         * @param pattern Byte sequence to insert.
         * @param preprocessor_for_removed_nonterminal Callback run before nonterminal removal.
         * @param postprocessor_for_inserted_nonterminal Callback run after nonterminal insertion.
         */
        template <typename CALLBACK1 = decltype(no_callback), typename CALLBACK2 = decltype(no_callback)>
        void insert_string_with_callback(int64_t i, const std::vector<uint8_t> &pattern, const CALLBACK1 &preprocessor_for_removed_nonterminal = no_callback, const CALLBACK2 &postprocessor_for_inserted_nonterminal = no_callback)
        {
            std::unordered_set<NonterminalWithRelativeLevel> changed_nonterminals;
            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();

            #ifdef DEBUG
            std::unordered_set<NonterminalWithRelativeLevel> removed_nonterminals;
            std::unordered_set<NonterminalWithRelativeLevel> added_nonterminals;
            #endif



            auto wrapped_preprocessor_for_removed_nonterminal = [&](NonterminalWithRelativeLevel sig)
            {
                this->callback_for_removed_nonterminal(sig, changed_nonterminals);
                preprocessor_for_removed_nonterminal(sig);

            #ifdef DEBUG
            removed_nonterminals.insert(sig);
            #endif
            };
            auto wrapped_postprocessor_for_inserted_nonterminal = [&](NonterminalWithRelativeLevel sig)
            {
                this->callback_for_added_nonterminal(sig, changed_nonterminals);
                postprocessor_for_inserted_nonterminal(sig);

            #ifdef DEBUG
            added_nonterminals.insert(sig);
            #endif
            };

            uint64_t size = this->size();

            if (pattern.size() == 0)
            {
                return;
            }
            else if (size == 0)
            {

                Compress::compress(this->dynamic_grammar, pattern, wrapped_postprocessor_for_inserted_nonterminal, stool::Message::NO_MESSAGE);
                assert(this->size() == size + pattern.size());
                assert(this->dynamic_grammar.get_document_count() == 1);
            }
            else
            {

                if (i > 0 && i < (int64_t)size)
                {

                    uint64_t leftLen = i;

                    NonterminalWithRelativeLevel root = grammar.get_root();
                    auto pair = SplitAndConcatenation::split(root, leftLen, true, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);

                    NonterminalWithRelativeLevel pattern_sig = Compress::compress(this->dynamic_grammar, pattern, wrapped_postprocessor_for_inserted_nonterminal, stool::Message::NO_MESSAGE);
                    NonterminalWithRelativeLevel LC = SplitAndConcatenation::concatenate(pair.first, pattern_sig, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);

                    [[maybe_unused]] NonterminalWithRelativeLevel LCR = SplitAndConcatenation::concatenate(LC, pair.second, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);

                    assert(this->size() == size + pattern.size());
                    assert(this->dynamic_grammar.get_document_count() == 1);
                }
                else
                {
                    if (i == 0)
                    {

                        NonterminalWithRelativeLevel root = grammar.get_root();

                        NonterminalWithRelativeLevel pattern_sig = Compress::compress(this->dynamic_grammar, pattern, wrapped_postprocessor_for_inserted_nonterminal, stool::Message::NO_MESSAGE);

                        SplitAndConcatenation::concatenate(pattern_sig, root, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);

                        assert(this->size() == size + pattern.size());
                        assert(this->dynamic_grammar.get_document_count() == 1);
                    }
                    else if (i == (int64_t)size)
                    {

                        NonterminalWithRelativeLevel root = grammar.get_root();
                        NonterminalWithRelativeLevel pattern_sig = Compress::compress(this->dynamic_grammar, pattern, wrapped_postprocessor_for_inserted_nonterminal, stool::Message::NO_MESSAGE);
                        SplitAndConcatenation::concatenate(root, pattern_sig, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);
                        assert(this->size() == size + pattern.size());
                        assert(this->dynamic_grammar.get_document_count() == 1);
                    }
                    else
                    {
                        std::cout << i << " " << size << std::endl;
                        throw std::runtime_error("DynamicRLSLPString::insert: Not implemented");
                    }
                }
            }


            /*
            #ifdef DEBUG
            std::vector<NonterminalWithRelativeLevel> removed_nonterminals_list(removed_nonterminals.begin(), removed_nonterminals.end());
            std::vector<NonterminalWithRelativeLevel> added_nonterminals_list(added_nonterminals.begin(), added_nonterminals.end());
            std::sort(removed_nonterminals_list.begin(), removed_nonterminals_list.end());
            std::sort(added_nonterminals_list.begin(), added_nonterminals_list.end());

            std::cout << "Removed nonterminals: " << std::endl;
            for(auto sig : removed_nonterminals_list){
                std::cout  << NonterminalFunctions::to_string(sig) << ", ";
            }
            std::cout << std::endl;
            std::cout << "Added nonterminals: " << std::endl;
            for(auto sig : added_nonterminals_list){
                std::cout << NonterminalFunctions::to_string(sig) << ", ";
            }
            std::cout << std::endl;
            #endif
            */

            this->callback_for_finished_update(changed_nonterminals);

        }

        /**
         * @brief Insert a character \p c at the position \p i in \p T.
         * @param i Insert position (0 .. |T|).
         * @param c Character to insert.
         */
        void insert_string(int64_t i, uint8_t c)
        {
            this->insert_string_with_callback(i, std::vector<uint8_t>(1, c), dynRLSLP::no_callback, dynRLSLP::no_callback);
        }

        /**
         * @brief Insert a byte pattern into \p T at position \p i.
         * @param i Insert position (0 .. |T|).
         * @param pattern Byte sequence to insert.
         */
        void insert_string(int64_t i, const std::vector<uint8_t> &pattern)
        {
            this->insert_string_with_callback(i, pattern, dynRLSLP::no_callback, dynRLSLP::no_callback);
        }

        /**
         * @brief Delete \p T[i] from \p T.
         * @param i Position of the character to delete.
         */
        void delete_substring(uint64_t i)
        {
            this->delete_substring_with_callback(i, 1, dynRLSLP::no_callback, dynRLSLP::no_callback);
        }

        /**
         * @brief Delete \p T[i..i+len-1] from \p T.
         * @param i Starting position of the range to delete.
         * @param len Number of characters to delete.
         */
        void delete_substring(uint64_t i, uint64_t len)
        {
            this->delete_substring_with_callback(i, len, dynRLSLP::no_callback, dynRLSLP::no_callback);
        }

        /**
         * @brief Delete a substring from \p T, invoking callbacks on grammar changes.
         * @tparam CALLBACK1 Callable invoked before a nonterminal is removed (\p NonterminalWithRelativeLevel).
         * @tparam CALLBACK2 Callable invoked after a nonterminal is inserted (\p NonterminalWithRelativeLevel).
         * @param i Starting position of the range to delete.
         * @param len Number of characters to delete.
         * @param preprocessor_for_removed_nonterminal Callback run before nonterminal removal.
         * @param postprocessor_for_inserted_nonterminal Callback run after nonterminal insertion.
         */
        template <typename CALLBACK1 = decltype(no_callback), typename CALLBACK2 = decltype(no_callback)>
        void delete_substring_with_callback(uint64_t i, uint64_t len, const CALLBACK1 &preprocessor_for_removed_nonterminal = no_callback, const CALLBACK2 &postprocessor_for_inserted_nonterminal = no_callback)
        {
            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
            std::unordered_set<NonterminalWithRelativeLevel> changed_nonterminals;

            #ifdef DEBUG
            std::unordered_set<NonterminalWithRelativeLevel> removed_nonterminals;
            std::unordered_set<NonterminalWithRelativeLevel> added_nonterminals;
            #endif


            auto wrapped_preprocessor_for_removed_nonterminal = [&](NonterminalWithRelativeLevel sig)
            {
                this->callback_for_removed_nonterminal(sig, changed_nonterminals);                
                preprocessor_for_removed_nonterminal(sig);

                #ifdef DEBUG
                removed_nonterminals.insert(sig);
                #endif
            };
            auto wrapped_postprocessor_for_inserted_nonterminal = [&](NonterminalWithRelativeLevel sig)
            {
                this->callback_for_added_nonterminal(sig, changed_nonterminals);
                postprocessor_for_inserted_nonterminal(sig);

                #ifdef DEBUG
                added_nonterminals.insert(sig);
                #endif
            };

            if (len == 0)
            {
                return;
            }
            uint64_t end_pos = i + len - 1;
            if (end_pos >= this->size())
            {
                throw std::runtime_error("DynamicRLSLPString::delete_substring_with_callback: end_pos >= this->size()");
            }

            uint64_t size = this->size();
            if (i > 0 && end_pos + 1 < size)
            {
                uint64_t leftLen1 = end_pos + 1;
                uint64_t leftLen2 = i;
                auto pair1 = SplitAndConcatenation::split(grammar.get_root(), leftLen1, true, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);
                auto pair2 = SplitAndConcatenation::split(pair1.first, leftLen2, true, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);
                NonterminalWithRelativeLevel L = pair2.first;
                NonterminalWithRelativeLevel C = pair2.second;
                NonterminalWithRelativeLevel R = pair1.second;
                this->dynamic_grammar.remove_document(C, wrapped_preprocessor_for_removed_nonterminal);
                SplitAndConcatenation::concatenate(L, R, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);
            }
            else
            {
                if (i == 0)
                {
                    if (end_pos + 1 == size)
                    {
                        this->dynamic_grammar.remove_document(grammar.get_root(), wrapped_preprocessor_for_removed_nonterminal);
                    }
                    else
                    {
                        uint64_t leftLen1 = end_pos + 1;
                        auto pair1 = SplitAndConcatenation::split(grammar.get_root(), leftLen1, true, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);
                        NonterminalWithRelativeLevel L = pair1.first;
                        // NonterminalWithRelativeLevel R = pair1.second;
                        this->dynamic_grammar.remove_document(L, wrapped_preprocessor_for_removed_nonterminal);
                    }
                }
                else
                {
                    uint64_t leftLen1 = i;
                    auto pair1 = SplitAndConcatenation::split(grammar.get_root(), leftLen1, true, true, this->dynamic_grammar, wrapped_preprocessor_for_removed_nonterminal, wrapped_postprocessor_for_inserted_nonterminal);
                    // NonterminalWithRelativeLevel L = pair1.first;
                    NonterminalWithRelativeLevel R = pair1.second;
                    this->dynamic_grammar.remove_document(R, wrapped_preprocessor_for_removed_nonterminal);
                }
            }

            /*
            #ifdef DEBUG
            std::vector<NonterminalWithRelativeLevel> removed_nonterminals_list(removed_nonterminals.begin(), removed_nonterminals.end());
            std::vector<NonterminalWithRelativeLevel> added_nonterminals_list(added_nonterminals.begin(), added_nonterminals.end());
            std::sort(removed_nonterminals_list.begin(), removed_nonterminals_list.end());
            std::sort(added_nonterminals_list.begin(), added_nonterminals_list.end());
            stool::DebugPrinter::print_integers(removed_nonterminals_list, "removed_nonterminals_list");
            stool::DebugPrinter::print_integers(added_nonterminals_list, "added_nonterminals_list");
            #endif
            */
            this->callback_for_finished_update(changed_nonterminals);
        }

        /**
         * @brief Rebuild the ancestor occurrence cache for all base nonterminals (Fast dictionary mode).
         */
        void rebuild_ancestor_cache_list()
        {
            this->ancestor_cache_list.clear();
            const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();
            const FastParentDictionary &fast_parent_dictionary = this->dynamic_grammar.get_parent_dictionary();
            const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();
            this->ancestor_cache_list.resize(this->dynamic_grammar.count_explicit_nonterminals(), TemporaryOccurrence::create_null_occurrence());

            for (ExplicitNonterminal i = 0; i < (int64_t)this->dynamic_grammar.count_explicit_nonterminals(); i++)
            {
                RLSLPRuleBody item = RLSLPRuleBody::decode_rule(i, explicit_nonterminal_rule_list);
                if (item.get_type() != RLSLPRuleType::Null)
                {
                    TemporaryOccurrence occurrence = NodeOccurrenceQuery::find_type_2_primary_occurrence_of_nonterminal_using_limited_depth(i, fast_parent_dictionary, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, DynamicRLSLPString::ANCESTOR_CACHE_DEPTH, 0);
                    this->ancestor_cache_list[i] = occurrence;
                }
            }
        }

        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Print and verification functions
        ////////////////////////////////////////////////////////////////////////////////
        //@{

        /**
         * @brief Verify internal consistency of the layered RLSLP grammar.
         * @return \p true if the grammar structure is valid.
         */
        bool verify() const
        {
            bool b = this->dynamic_grammar.verify();
            if(!b){
                throw std::runtime_error("DynamicRLSLPString::verify: The grammar is not valid.");
            }

            if(this->dictionaryMode == DictionaryMode::Fast){
                auto dic = this->dynamic_grammar.get_dictionary();
                auto explicit_nonterminal_rule_list = dic.get_explicit_nonterminal_rule_list();
                auto parent_dictionary = this->dynamic_grammar.get_parent_dictionary();

                if(this->left_short_string_list.size() != explicit_nonterminal_rule_list.size()){
                    throw std::runtime_error("DynamicRLSLPString::verify: The size of left short string list is not equal to the size of explicit nonterminal rule list.");
                }
                if(this->right_short_string_list.size() != explicit_nonterminal_rule_list.size()){
                    throw std::runtime_error("DynamicRLSLPString::verify: The size of right short string list is not equal to the size of explicit nonterminal rule list.");
                }
                if(this->ancestor_cache_list.size() != explicit_nonterminal_rule_list.size()){
                    throw std::runtime_error("DynamicRLSLPString::verify: The size of ancestor cache list is not equal to the size of explicit nonterminal rule list.");
                }


                for(uint64_t i = 0; i < explicit_nonterminal_rule_list.size(); i++){
                    
                    if(explicit_nonterminal_rule_list[i].get_type() != RLSLPRuleType::Null){
                        auto correct_occurrence = NodeOccurrenceQuery::find_type_2_primary_occurrence_of_nonterminal_using_limited_depth(i, parent_dictionary, explicit_nonterminal_rule_list, dic.get_explicit_nonterminal_length_list(), DynamicRLSLPString::ANCESTOR_CACHE_DEPTH, 0);
                        if(!this->ancestor_cache_list[i].equals(correct_occurrence)){
                            std::cout << "Explicit nonterminal: " << i << ", Correct occurrence: " << correct_occurrence.to_string() << ", Cache occurrence: " << this->ancestor_cache_list[i].to_string() << std::endl;
                            throw std::runtime_error("DynamicRLSLPString::verify: The ancestor cache is not equal to the correct occurrence.");
                        }

                        ShortString::verify_left_short_string(i, this->dynamic_grammar.get_alphabet_bit_size(), explicit_nonterminal_rule_list, dic.get_explicit_nonterminal_length_list(), this->left_short_string_list, this->dynamic_grammar.get_character_id_map());
                    }else{
                        if(!this->ancestor_cache_list[i].is_null()){
                            throw std::runtime_error("DynamicRLSLPString::verify: The ancestor cache is not null.");
                        }

                        if(this->left_short_string_list[i] != UINT64_MAX){
                            throw std::runtime_error("DynamicRLSLPString::verify: The left short string is not UINT64_MAX.");
                        }

                        if(this->right_short_string_list[i] != UINT64_MAX){
                            throw std::runtime_error("DynamicRLSLPString::verify: The right short string is not UINT64_MAX.");
                        }
                    }


                }
            }


            return true;
        }

        /**
         * @brief Verify that the grammar derives the same text as a full decompression.
         * @return \p true if decompressed text matches a recompilation of the grammar root.
         */
        bool verify_string() const
        {
            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
            if (!grammar.has_root())
            {
                return true;
            }
            std::vector<uint8_t> text = this->to_vector();
            NonterminalWithRelativeLevel root = grammar.get_root();
            const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();
            RunRuleVector output = RunRuleVector::create_empty_vector(small_dic);

            bool b1 = TryCommonSequenceCompiler::try_build_common_sequence_from_text(this->dynamic_grammar, text, output, stool::Message::NO_MESSAGE);
            if (!b1)
            {
                throw std::runtime_error("DynamicRLSLPString::verify_string: Failed to build common sequence from text");
            }
            int64_t root_test = TryCommonSequenceCompiler::try_single_compile(output, this->dynamic_grammar, stool::Message::NO_MESSAGE);
            if (root_test == -1)
            {
                throw std::runtime_error("DynamicRLSLPString::verify_string: Failed to compile common sequence");
            }

            if (root_test != root)
            {
                throw std::runtime_error("DynamicRLSLPString::verify_string: The root of the grammar is not equal to the root of the text");
            }
            return true;
        }

        /**
         * @brief Verify if the grammar in this instance is nearly equal to the grammar in the other instance.
         * @param other Instance to compare against.
         * @return \p true if grammars and Fast-mode caches are structurally equivalent.
         */
        bool verify_nearly_equal(const DynamicRLSLPString &other) const
        {
            if (this->dictionaryMode != other.dictionaryMode)
            {
                throw std::runtime_error("DynamicRLSLPString::verify_nearly_equal: The dictionary mode must be equal.");
            }

            if (this->dictionaryMode == DictionaryMode::Fast)
            {
                if (this->left_short_string_list.size() != other.left_short_string_list.size())
                {
                    throw std::runtime_error("DynamicRLSLPString::verify_nearly_equal: The size of left short string list must be equal.");
                }
                if (this->right_short_string_list.size() != other.right_short_string_list.size())
                {
                    throw std::runtime_error("DynamicRLSLPString::verify_nearly_equal: The size of right short string list must be equal.");
                }
                if (this->ancestor_cache_list.size() != other.ancestor_cache_list.size())
                {
                    throw std::runtime_error("DynamicRLSLPString::verify_nearly_equal: The size of ancestor cache list must be equal.");
                }
                for (uint64_t i = 0; i < this->left_short_string_list.size(); i++)
                {
                    if (this->left_short_string_list[i] != other.left_short_string_list[i])
                    {
                        throw std::runtime_error("DynamicRLSLPString::verify_nearly_equal: The left short string must be equal.");
                    }
                    if (this->right_short_string_list[i] != other.right_short_string_list[i])
                    {
                        throw std::runtime_error("DynamicRLSLPString::verify_nearly_equal: The right short string must be equal.");
                    }
                    if (this->ancestor_cache_list[i].nonterminal != other.ancestor_cache_list[i].nonterminal || this->ancestor_cache_list[i].position != other.ancestor_cache_list[i].position)
                    {
                        throw std::runtime_error("DynamicRLSLPString::verify_nearly_equal: The nonterminal and position of ancestor cache must be equal.");
                    }
                }
            }

            this->dynamic_grammar.verify_nearly_equal(other.dynamic_grammar);
            return true;
        }
        /**
         * @brief Return the memory usage information of this instance as a vector of strings.
         * @param message_paragraph The paragraph depth of message logs.
         * @return Human-readable memory breakdown lines.
         */
        std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
        {

            std::vector<std::string> r;

            r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicRLSLPString: ");

            auto log1 = this->dynamic_grammar.get_memory_usage_info(message_paragraph + 1);
            for (auto s : log1)
            {
                r.push_back(s);
            }

            r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");
            return r;
        }

        /**
         * @brief Print the derivation tree of the grammar root to standard output.
         */
        std::vector<std::string> get_derivation_tree_as_plain_text() const
        {
            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
            std::vector<NonterminalWithRelativeLevel> items;
            items.push_back(grammar.get_root());

            const std::vector<uint16_t> &explicit_nonterminal_level_list = this->dynamic_grammar.get_explicit_nonterminal_level_list();
            const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();
            const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();

            std::vector<std::string> r = DerivationTreeVisualizer::compute_derivation_tree(items, explicit_nonterminal_rule_list, explicit_nonterminal_level_list, explicit_nonterminal_length_list);
            return r;
        }

        /**
         * @brief Print statistics about this instance and its grammar to standard output.
         * @param message_paragraph The paragraph depth of message logs.
         */
        void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
        {
            std::string mode = this->dictionaryMode == DictionaryMode::Fast ? "Fast" : (this->dictionaryMode == DictionaryMode::Standard ? "Standard" : "Lightweight");
            std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics (DynamicRLSLPString)" << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Mode: " << mode << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Left Short Strings (Cache): " << this->left_short_string_list.size() << " * 8 bytes" << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Right Short Strings (Cache): " << this->right_short_string_list.size() << " * 8 bytes" << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Ancestor Cache List (Cache): " << this->ancestor_cache_list.size() << " * " << sizeof(TemporaryOccurrence) << " bytes" << std::endl;

            this->dynamic_grammar.print_statistics(message_paragraph + 1);
            std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
        }

        void print_memory_breakdown(int message_paragraph = stool::Message::SHOW_MESSAGE) const
        {
            std::string mode = this->dictionaryMode == DictionaryMode::Fast ? "Fast" : "Standard";
            std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Memory Breakdown (DynamicRLSLPString): " << this->size_in_bytes() << " bytes" << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "left_short_string_list: " << stool::Memory::estimate_memory_usage(this->left_short_string_list) << " bytes" << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "right_short_string_list: " << stool::Memory::estimate_memory_usage(this->right_short_string_list) << " bytes" << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "ancestor_cache_list: " << stool::Memory::estimate_memory_usage(this->ancestor_cache_list) << " bytes" << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "dictionary_mode: " << sizeof(this->dictionaryMode) << " bytes" << std::endl;
            this->dynamic_grammar.print_memory_breakdown(message_paragraph + 1);

            std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
        }

        /*
        void print_summary(int64_t message_paragraph = stool::Message::SHOW_MESSAGE) const
        {
            std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Summary" << std::endl;
            uint64_t text_length = this->size();
            auto& grammar = this->dynamic_grammar.get_grammar();
            auto& dictionary = this->dynamic_grammar.get_dictionary();
            auto& random_bit_dictionary = this->dynamic_grammar.get_random_bit_dictionary();
            std::string mode = this->dictionaryMode == DictionaryMode::Fast ? "Fast" : (this->dictionaryMode == DictionaryMode::Standard ? "Standard" : "Lightweight");


            std::string parser = this->dynamic_grammar.get_grammar_parsing_type() == GrammarParsingType::RestrictedRecompression ? "Restricted Recompression" : "Signature Encoding";

            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Parser: " << parser << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Mode: " << mode << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Seed: " << random_bit_dictionary.get_seed() << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: " << text_length << std::endl;
        }
        */

        /**
         * @brief Print the information about the locate query.
         */
        void print_infomation_about_locate_query() const
        {
            /*
            const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = dic.get_explicit_nonterminal_rule_list();
            uint64_t size = explicit_nonterminal_rule_list.size();
            for (uint64_t i = 0; i < size; i++)
            {
                if (!this->dynamic_grammar.check_null_item(i))
                {
                    RLSLPRuleBody item = this->dynamic_grammar.get_rule_body(i);
                    auto left_str = Access::get_left_string(item, explicit_nonterminal_rule_list);
                    auto right_str = Access::get_right_string(item, explicit_nonterminal_rule_list);
                    std::cout << "i = " << i << "\t" << left_str << "|" << right_str << std::endl;
                }
            }
            */
        }

        /**
         * @brief Print debug information about locate queries for the pattern \p pattern.
         * @param pattern Pattern bytes used for the locate query analysis.
         */
        void print_infomation_about_locate_query2(const std::vector<uint8_t> &pattern)
        {
            this->dynamic_grammar.print_info();
            if (pattern.size() == 1)
            {
                std::cout << "pattern: " << (char)pattern[0] << std::endl;
            }
            else
            {

                const DictionaryForLayeredRLSLP &small_dic = this->dynamic_grammar.get_dictionary();

                NonterminalWithRelativeLevel pattern_sig = Compress::compress(this->dynamic_grammar, pattern, dynRLSLP::no_callback, stool::Message::NO_MESSAGE);
                RunRuleVector pattern_link = FastCommonSequenceBuilder::build(pattern_sig, small_dic);
                std::vector<uint64_t> common_positions = pattern_link.to_starting_position_vector();

                std::vector<bool> checker;
                checker.resize(pattern.size(), false);
                for (auto it : common_positions)
                {
                    checker[it] = true;
                }
                // pattern_link.print_trees();
                stool::DebugPrinter::print_integers(common_positions, "Common Positions");
                std::cout << "pattern_with_partition: " << std::flush;
                for (uint64_t i = 0; i < pattern.size(); i++)
                {
                    if (checker[i])
                    {
                        std::cout << "|" << std::flush;
                    }
                    std::cout << (char)pattern[i] << std::flush;
                }
                std::cout << std::endl;

                this->dynamic_grammar.remove_document(pattern_sig, no_callback);
            }
        }

        //}@

        ////////////////////////////////////////////////////////////////////////////////
        ///   @name Load, save, and builder functions
        ////////////////////////////////////////////////////////////////////////////////
        //@{

        /**
         * @brief Return a new DynamicRLSLPString built from a given text \p text (alphabet inferred from \p text).
         * @param text Input byte sequence to compress.
         * @param use_restricted_block_compression If true, use restricted block compression.
         * @param mode Dictionary operating mode after build.
         * @param seed Random seed for compression.
         * @param message_paragraph The paragraph depth of message logs.
         * @return Compressed \p DynamicRLSLPString representing \p text.
         */
        static DynamicRLSLPString offline_build_from_text(const std::vector<uint8_t> &text, GrammarParsingType parser, DictionaryMode mode, int64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
        {
            std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(text);

            return DynamicRLSLPString::offline_build_from_text(text, parser, alphabet, mode, seed, message_paragraph);
        }
        /**
         * @brief Return a new DynamicRLSLPString built from a given text \p text with an explicit alphabet.
         * @param text Input byte sequence to compress.
         * @param use_restricted_block_compression If true, use restricted block compression.
         * @param alphabet Explicit alphabet as byte values.
         * @param mode Dictionary operating mode after build.
         * @param seed Random seed for compression.
         * @param message_paragraph The paragraph depth of message logs.
         * @return Compressed \p DynamicRLSLPString representing \p text.
         */
        static DynamicRLSLPString offline_build_from_text(const std::vector<uint8_t> &text, GrammarParsingType parser, const std::vector<uint8_t> &alphabet, DictionaryMode mode, int64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
        {
            if (message_paragraph != stool::Message::NO_MESSAGE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Building DynamicRLSLPString from text in offline mode... " << std::endl;
            }
            DynamicRLSLPString r(parser, alphabet, seed);
            Compress::compress(r.dynamic_grammar, text, dynRLSLP::no_callback, message_paragraph);

            r.set_mode(mode);

            return r;
        }
        /**
         * @brief Return a new DynamicRLSLPString built from a given text \p text (debug helper using \p std::string).
         * @param text Input string to compress.
         * @param use_restricted_block_compression If true, use restricted block compression.
         * @param mode Dictionary operating mode after build.
         * @param seed Random seed for compression.
         * @param message_paragraph The paragraph depth of message logs.
         * @return Compressed \p DynamicRLSLPString representing \p text.
         */
        static DynamicRLSLPString build_from_text_for_debug(const std::string &text, GrammarParsingType parser, DictionaryMode mode, int64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
        {
            std::vector<uint8_t> text_vector;
            for (auto c : text)
            {
                text_vector.push_back(c);
            }
            return DynamicRLSLPString::offline_build_from_text(text_vector, parser, mode, seed, message_paragraph);
        }

        /**
         * @brief Return a new DynamicRLSLPString built from a given file \p file_path using a buffer of size \p buffer_size.
         * @param file_path Path to the input text file.
         * @param use_restricted_block_compression If true, use restricted block compression.
         * @param mode Dictionary operating mode after build.
         * @param seed Random seed for compression.
         * @param buffer_size Read buffer size in bytes for streaming compression.
         * @param message_paragraph The paragraph depth of message logs.
         * @return Compressed \p DynamicRLSLPString built incrementally from the file.
         */
        static DynamicRLSLPString online_build_from_text_file(std::string file_path, GrammarParsingType parser, DictionaryMode mode = DictionaryMode::Standard, int64_t seed = 0, uint64_t buffer_size = 100000, int message_paragraph = stool::Message::SHOW_MESSAGE)
        {
            if (!std::filesystem::exists(file_path))
            {
                std::cerr << "Error: The specified input file does not exist: " << file_path << std::endl;
                throw std::runtime_error("File open error");
            }
            if (buffer_size == 0)
            {
                throw std::runtime_error("DynamicRLSLPString::build_from_text_file: buffer_size == 0");
            }

            if (message_paragraph != stool::Message::NO_MESSAGE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Building DynamicRLSLPString from text file in online mode... " << std::endl;
            }

            std::chrono::steady_clock::time_point st1, st2;
            st1 = std::chrono::steady_clock::now();

            std::vector<uint8_t> alphabet = stool::OnlineFileReader::get_alphabet(file_path);

            DynamicRLSLPString r(parser, alphabet, seed);

            std::ifstream stream;
            stream.open(file_path, std::ios::binary);
            std::vector<uint8_t> buffer_text;
            uint64_t text_size = stool::OnlineFileReader::get_text_size(stream);

            NonterminalWithRelativeLevel text_nonterminal = INT64_MAX;
            uint64_t current_length = 0;

            uint64_t text_size_in_mega_bytes = (text_size / (1000 * 1000)) + 1;
            std::vector<bool> progress_checker;
            progress_checker.resize(text_size_in_mega_bytes, false);

            if (text_size > 0)
            {
                stool::OnlineFileReader::read(stream, buffer_text, buffer_size, text_size);
                assert(buffer_text.size() > 0);
                text_nonterminal = Compress::compress(r.dynamic_grammar, buffer_text, dynRLSLP::no_callback, stool::Message::NO_MESSAGE);
                current_length += buffer_text.size();

                uint64_t current_length_in_mega_bytes = (current_length / (1000 * 1000));
                if (!progress_checker[current_length_in_mega_bytes])
                {
                    if (message_paragraph != stool::Message::NO_MESSAGE)
                    {
                        std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Processing... [" << current_length_in_mega_bytes << "/" << text_size_in_mega_bytes << "MB]" << std::endl;
                    }
                    progress_checker[current_length_in_mega_bytes] = true;
                }
            }

            while (stool::OnlineFileReader::read(stream, buffer_text, buffer_size, text_size))
            {
                NonterminalWithRelativeLevel new_string_nonterminal = Compress::compress(r.dynamic_grammar, buffer_text, dynRLSLP::no_callback, stool::Message::NO_MESSAGE);
                assert(buffer_text.size() > 0);
                text_nonterminal = SplitAndConcatenation::concatenate(text_nonterminal, new_string_nonterminal, true, r.dynamic_grammar, dynRLSLP::no_callback, dynRLSLP::no_callback);

                current_length += buffer_text.size();

                uint64_t current_length_in_mega_bytes = (current_length / (1000 * 1000));
                if (!progress_checker[current_length_in_mega_bytes])
                {
                    if (current_length_in_mega_bytes % 100 == 0 && message_paragraph != stool::Message::NO_MESSAGE)
                    {
                        std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Processing... [" << current_length_in_mega_bytes << "/" << text_size_in_mega_bytes << "MB]" << std::endl;
                    }

                    progress_checker[current_length_in_mega_bytes] = true;
                }
            }

            r.set_mode(mode);
            st2 = std::chrono::steady_clock::now();
            uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
            if (message_paragraph != stool::Message::NO_MESSAGE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[DONE] Elapsed Time: " << sec_time << " sec" << std::endl;
            }

            return r;
        }
        /**
         * @brief Load a DynamicRLSLPString instance from a binary file stream.
         * @param os Input stream positioned at a serialized \p DynamicRLSLPString.
         * @return Deserialized \p DynamicRLSLPString instance.
         */
        static DynamicRLSLPString load_from_file(std::ifstream &os)
        {
            DynamicRLSLPString r(GrammarParsingType::RestrictedRecompression);

            uint64_t fingerprint;
            os.read(reinterpret_cast<char *>(&fingerprint), sizeof(uint64_t));
            if (fingerprint != FINGERPRINT)
            {
                throw std::runtime_error("DynamicRLSLPString::load_from_file: Fingerprint mismatch");
            }

            int modeValue;
            os.read(reinterpret_cast<char *>(&modeValue), sizeof(int));
            r.dictionaryMode = static_cast<DictionaryMode>(modeValue);

            // Code 7
            uint64_t _left_short_string_list_size;
            os.read(reinterpret_cast<char *>(&_left_short_string_list_size), sizeof(_left_short_string_list_size));
            r.left_short_string_list.resize(_left_short_string_list_size);
            os.read(reinterpret_cast<char *>(r.left_short_string_list.data()), sizeof(uint64_t) * _left_short_string_list_size);

            uint64_t _right_short_string_list_size;
            os.read(reinterpret_cast<char *>(&_right_short_string_list_size), sizeof(_right_short_string_list_size));
            r.right_short_string_list.resize(_right_short_string_list_size);
            os.read(reinterpret_cast<char *>(r.right_short_string_list.data()), sizeof(uint64_t) * _right_short_string_list_size);

            uint64_t size = 0;
            os.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
            r.ancestor_cache_list.resize(size);
            if (size > 0)
            {
                os.read(reinterpret_cast<char *>(r.ancestor_cache_list.data()), sizeof(TemporaryOccurrence) * size);
            }

            auto tmp = DynamicGrammarForLayeredRLSLP::load_from_file(os);
            r.dynamic_grammar.swap(tmp);

            return r;
        }
        /**
         * @brief Build a DynamicRLSLPString from a serialized leveled RLSLP grammar stream.
         * @param os Input stream containing leveled RLSLP data.
         * @return \p DynamicRLSLPString wrapping the loaded grammar.
         */
        static DynamicRLSLPString build_from_leveled_rlslp(std::ifstream &os)
        {
            DynamicRLSLPString r(GrammarParsingType::RestrictedRecompression);
            auto tmp = DynamicGrammarForLayeredRLSLP::build_from_leveled_rlslp(os);
            r.dynamic_grammar.swap(tmp);
            r.rebuild_ancestor_cache_list();
            return r;
        }
        /**
         * @brief Serialize the given instance \p item to a binary file stream.
         * @param item Instance to write.
         * @param os Output stream for binary serialization.
         */
        static void store_to_file(const DynamicRLSLPString &item, std::ofstream &os)
        {
            os.write(reinterpret_cast<const char *>(&FINGERPRINT), sizeof(uint64_t));

            int modeValue = static_cast<int>(item.dictionaryMode);
            os.write(reinterpret_cast<const char *>(&modeValue), sizeof(int));

            // Code 7
            uint64_t left_short_string_list_size = item.left_short_string_list.size();
            os.write(reinterpret_cast<const char *>(&left_short_string_list_size), sizeof(uint64_t));
            os.write(reinterpret_cast<const char *>(item.left_short_string_list.data()), sizeof(uint64_t) * left_short_string_list_size);

            uint64_t right_short_string_list_size = item.right_short_string_list.size();
            os.write(reinterpret_cast<const char *>(&right_short_string_list_size), sizeof(uint64_t));
            os.write(reinterpret_cast<const char *>(item.right_short_string_list.data()), sizeof(uint64_t) * right_short_string_list_size);

            uint64_t size = item.ancestor_cache_list.size();
            os.write(reinterpret_cast<const char *>(&size), sizeof(uint64_t));
            os.write(reinterpret_cast<const char *>(item.ancestor_cache_list.data()), sizeof(TemporaryOccurrence) * size);

            DynamicGrammarForLayeredRLSLP::store_to_file(item.dynamic_grammar, os);
        }

        //}@

    private:
        /**
         * @brief Fast-mode callback when a nonterminal is removed during an update.
         * @param sig Nonterminal that was removed.
         * @param changed_nonterminals Set collecting base nonterminals whose caches must be refreshed.
         */
        void callback_for_removed_nonterminal(NonterminalWithRelativeLevel sig, std::unordered_set<NonterminalWithRelativeLevel> &changed_nonterminals)
        {

            if (this->dictionaryMode == DictionaryMode::Fast)
            {

                if (NonterminalFunctions::is_explicit_nonterminal(sig))
                {
                    this->left_short_string_list[sig] = UINT64_MAX;
                    this->right_short_string_list[sig] = UINT64_MAX;

                    this->add_descendants(sig, changed_nonterminals, DynamicRLSLPString::ANCESTOR_CACHE_DEPTH + 1, 0);
                }
            }
        }

        /**
         * @brief Fast-mode callback when a nonterminal is added during an update.
         * @param sig Nonterminal that was added.
         * @param changed_nonterminals Set collecting base nonterminals whose caches must be refreshed.
         */
        void callback_for_added_nonterminal(NonterminalWithRelativeLevel sig, std::unordered_set<NonterminalWithRelativeLevel> &changed_nonterminals)
        {
            if (this->dictionaryMode == DictionaryMode::Fast)
            {
                const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();
                const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();

                ExplicitNonterminal explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(sig);
                if (sig == explicit_nonterminal)
                {
                    RLSLPRuleBody item = RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list);

                    while ((uint64_t)this->left_short_string_list.size() <= (uint64_t)sig)
                    {
                        this->left_short_string_list.push_back(UINT64_MAX);
                        this->right_short_string_list.push_back(UINT64_MAX);
                        this->ancestor_cache_list.push_back(TemporaryOccurrence::create_null_occurrence());
                    }
                    uint64_t alphabet_bit_size = this->dynamic_grammar.get_alphabet_bit_size();

                    if (item.get_type() == RLSLPRuleType::Pair)
                    {
                        uint64_t left_short_string = ShortString::create_left_short_string_for_pair(item.A, item.B, alphabet_bit_size, explicit_nonterminal_length_list, this->left_short_string_list);
                        uint64_t right_short_string = ShortString::create_right_short_string_for_pair(item.A, item.B, alphabet_bit_size, explicit_nonterminal_length_list, this->right_short_string_list);
                        this->left_short_string_list[sig] = left_short_string;
                        this->right_short_string_list[sig] = right_short_string;
                    }
                    else if (item.get_type() == RLSLPRuleType::Power)
                    {
                        uint64_t left_short_string = ShortString::create_left_short_string_for_power(item.A, item.B, alphabet_bit_size, explicit_nonterminal_length_list, this->left_short_string_list);
                        uint64_t right_short_string = ShortString::create_right_short_string_for_power(item.A, item.B, alphabet_bit_size, explicit_nonterminal_length_list, this->right_short_string_list);
                        this->left_short_string_list[sig] = left_short_string;
                        this->right_short_string_list[sig] = right_short_string;
                    }
                    else if (item.get_type() == RLSLPRuleType::Character)
                    {
                        const std::unordered_map<int64_t, uint64_t> &character_id_map = this->dynamic_grammar.get_character_id_map();
                        uint64_t left_short_string = ShortString::create_left_short_string_for_character(item.A, alphabet_bit_size, character_id_map);
                        uint64_t right_short_string = ShortString::create_right_short_string_for_character(item.A, alphabet_bit_size, character_id_map);
                        this->left_short_string_list[sig] = left_short_string;
                        this->right_short_string_list[sig] = right_short_string;
                    }


                    this->add_descendants(sig, changed_nonterminals, DynamicRLSLPString::ANCESTOR_CACHE_DEPTH + 1, 0);
                }
            }
        }
        /**
         * @brief Refresh ancestor caches for all nonterminals changed during an update (Fast mode).
         * @param changed_nonterminals Base nonterminals whose ancestor occurrences were invalidated.
         */
        void callback_for_finished_update(const std::unordered_set<NonterminalWithRelativeLevel> &changed_nonterminals)
        {
            if (this->dictionaryMode == DictionaryMode::Fast)
            {
                std::vector<uint64_t> changed_nonterminals_list;
                changed_nonterminals_list.resize(changed_nonterminals.size());
                uint64_t index = 0;
                for (auto sig : changed_nonterminals)
                {
                    changed_nonterminals_list[index] = sig;
                    index++;
                }

                //std::sort(changed_nonterminals_list.begin(), changed_nonterminals_list.end());
                //stool::DebugPrinter::print_integers(changed_nonterminals_list, "changed_nonterminals_list");

                const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();
                const FastParentDictionary &fast_parent_dictionary = this->dynamic_grammar.get_parent_dictionary();
                const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();

                for (uint64_t i = 0; i < changed_nonterminals_list.size(); i++)
                {
                    ExplicitNonterminal sig = changed_nonterminals_list[i];
                    RLSLPRuleBody item = RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list);
                    if (item.get_type() != RLSLPRuleType::Null)
                    {
                        if ((uint64_t)this->ancestor_cache_list.size() <= (uint64_t)sig)
                        {
                            while ((uint64_t)this->ancestor_cache_list.size() <= (uint64_t)sig)
                            {
                                this->ancestor_cache_list.push_back(TemporaryOccurrence::create_null_occurrence());
                            }
                        }
                        TemporaryOccurrence occurrence = NodeOccurrenceQuery::find_type_2_primary_occurrence_of_nonterminal_using_limited_depth(sig, fast_parent_dictionary, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, DynamicRLSLPString::ANCESTOR_CACHE_DEPTH, 0);


                        this->ancestor_cache_list[sig] = occurrence;

                    }
                    else
                    {
                        if ((uint64_t)sig < (uint64_t)this->ancestor_cache_list.size())
                        {
                            this->ancestor_cache_list[sig] = TemporaryOccurrence::create_null_occurrence();
                        }
                    }
                }
            }

        }

        /**
         * @brief Recursively mark descendant base nonterminals up to \p max_depth for cache invalidation.
         * @param sig Root nonterminal whose descendants are collected.
         * @param changed_nonterminals Output set of affected base nonterminals.
         * @param max_depth Maximum recursion depth.
         * @param current_depth Current recursion depth.
         */
        void add_descendants(NonterminalWithRelativeLevel sig, std::unordered_set<NonterminalWithRelativeLevel> &changed_nonterminals, uint64_t max_depth, uint64_t current_depth) const
        {
            ExplicitNonterminal explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(sig);
            const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();
            auto f = changed_nonterminals.find(explicit_nonterminal);
            if (f == changed_nonterminals.end())
            {
                changed_nonterminals.insert(explicit_nonterminal);

                RLSLPRuleBody item = RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list);
                if (item.get_type() == RLSLPRuleType::Pair)
                {
                    ExplicitNonterminal left_explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(item.A);
                    ExplicitNonterminal right_explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(item.B);
                    if (current_depth < max_depth)
                    {
                        add_descendants(left_explicit_nonterminal, changed_nonterminals, max_depth, current_depth + 1);
                        add_descendants(right_explicit_nonterminal, changed_nonterminals, max_depth, current_depth + 1);
                    }
                }
                else if (item.get_type() == RLSLPRuleType::Power)
                {
                    ExplicitNonterminal base_child_nonterminal = NonterminalFunctions::get_explicit_nonterminal(item.A);
                    if (current_depth < max_depth)
                    {
                        add_descendants(base_child_nonterminal, changed_nonterminals, max_depth, current_depth + 1);
                    }
                }
            }
        }
        /**
         * @brief Rebuild left and right short-string caches for all rules reachable from the grammar root (Fast mode).
         */
        void rebuild_short_string_list()
        {
            const std::vector<uint64_t> &explicit_nonterminal_length_list = this->dynamic_grammar.get_explicit_nonterminal_length_list();
            const GrammarForLayeredRLSLP &grammar = this->dynamic_grammar.get_grammar();
            uint64_t alphabet_bit_size = this->dynamic_grammar.get_alphabet_bit_size();
            uint64_t ruleSize = this->dynamic_grammar.count_explicit_nonterminals();
            assert(this->left_short_string_list.size() == ruleSize);

            for (uint64_t i = 0; i < ruleSize; i++)
            {
                this->left_short_string_list[i] = UINT64_MAX;
                this->right_short_string_list[i] = UINT64_MAX;
            }

            if (grammar.has_root())
            {
                VStack<NonterminalWithRelativeLevel> nonterminalStack;
                NonterminalWithRelativeLevel root = grammar.get_root();
                const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = this->dynamic_grammar.get_explicit_nonterminal_rule_list();
                nonterminalStack.push(root);

                std::vector<bool> flags;
                flags.resize(ruleSize, false);

                while (nonterminalStack.size() > 0)
                {
                    NonterminalWithRelativeLevel sig = nonterminalStack.top();
                    nonterminalStack.pop();
                    RLSLPRuleBody item = RLSLPRuleBody::decode_rule(sig, explicit_nonterminal_rule_list);
                    if (!flags[sig])
                    {
                        if (item.get_type() == RLSLPRuleType::Pair)
                        {
                            NonterminalWithRelativeLevel left_sig = item.A;
                            NonterminalWithRelativeLevel right_sig = item.B;
                            ExplicitNonterminal left_base_sig = NonterminalFunctions::get_explicit_nonterminal(left_sig);
                            ExplicitNonterminal right_base_sig = NonterminalFunctions::get_explicit_nonterminal(right_sig);

                            if (flags[left_base_sig] && flags[right_base_sig])
                            {
                                this->left_short_string_list[sig] = ShortString::create_left_short_string_for_pair(left_base_sig, right_base_sig, alphabet_bit_size, explicit_nonterminal_length_list, this->left_short_string_list);
                                this->right_short_string_list[sig] = ShortString::create_right_short_string_for_pair(left_base_sig, right_base_sig, alphabet_bit_size, explicit_nonterminal_length_list, this->right_short_string_list);

                                flags[sig] = true;
                            }
                            else
                            {
                                nonterminalStack.push(sig);
                                if (!flags[left_base_sig])
                                {
                                    nonterminalStack.push(left_base_sig);
                                }

                                if (left_base_sig != right_base_sig && !flags[right_base_sig])
                                {
                                    nonterminalStack.push(right_base_sig);
                                }
                            }
                        }
                        else if (item.get_type() == RLSLPRuleType::Power)
                        {
                            NonterminalWithRelativeLevel child_sig = item.A;
                            ExplicitNonterminal child_base_sig = NonterminalFunctions::get_explicit_nonterminal(child_sig);
                            if (flags[child_base_sig])
                            {
                                this->left_short_string_list[sig] = ShortString::create_left_short_string_for_power(child_base_sig, item.B, alphabet_bit_size, explicit_nonterminal_length_list, this->left_short_string_list);
                                this->right_short_string_list[sig] = ShortString::create_right_short_string_for_power(child_base_sig, item.B, alphabet_bit_size, explicit_nonterminal_length_list, this->right_short_string_list);

                                flags[sig] = true;
                            }
                            else
                            {
                                nonterminalStack.push(sig);
                                nonterminalStack.push(child_base_sig);
                            }
                        }
                        else if (item.get_type() == RLSLPRuleType::Character)
                        {
                            const std::unordered_map<int64_t, uint64_t> &character_id_map = this->dynamic_grammar.get_character_id_map();
                            this->left_short_string_list[sig] = ShortString::create_left_short_string_for_character(item.A, alphabet_bit_size, character_id_map);
                            this->right_short_string_list[sig] = ShortString::create_right_short_string_for_character(item.A, alphabet_bit_size, character_id_map);

                            flags[sig] = true;
                        }
                        else
                        {
                            throw std::runtime_error("rebuild_short_string_list: unknown rule type");
                        }
                    }
                }
            }
        }
    };

}