#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include "./tsv_parser.hpp"
namespace dynRLSLP
{
    enum class QueryType
    {
        NONE = 0,
        INSERT = 1,
        DELETE = 2,
        COUNT = 3,
        LOCATE = 4,
        LOCATE_DETAIL = 5,
        LOCATE_SUM = 6,
        PRINT = 7
    };

    /**
     * @brief Queries to be executed by a dynamic string index.
     */
    struct LineQuery
    {
    public:
        QueryType type = QueryType::NONE;
        uint64_t position = UINT64_MAX;
        uint64_t length = UINT64_MAX;
        std::vector<uint8_t> pattern;

        /**
         * @brief Converts escaped tab and newline codes in a string to real characters.
         * @param text Input string with escape codes (0 for newline, 1 for tab).
         * @return Sanitized byte vector suitable for pattern matching.
         */
        static std::vector<uint8_t> sanityze(const std::string &text)
        {
            std::vector<uint8_t> r;
            for (char c : text)
            {
                if (c == 0)
                {
                    r.push_back('\n');
                }
                else if (c == 1)
                {
                    r.push_back('\t');
                }
                else
                {
                    r.push_back(c);
                }
            }
            return r;
        }

        /**
         * @brief Creates a no-op query.
         * @return LineQuery with type NONE.
         */
        static LineQuery create_NONE_query()
        {
            LineQuery r;
            r.type = QueryType::NONE;
            return r;
        }
        /**
         * @brief Creates a PRINT query.
         * @return LineQuery with type PRINT.
         */
        static LineQuery create_PRINT_query()
        {
            LineQuery r;
            r.type = QueryType::PRINT;
            return r;
        }

        /**
         * @brief Creates an INSERT query.
         * @param insertion_position Position at which to insert.
         * @param text Text to insert (escape codes are sanitized).
         * @return LineQuery with type INSERT.
         */
        static LineQuery create_INSERT_query(uint64_t insertion_position, const std::string &text)
        {
            LineQuery r;
            r.type = QueryType::INSERT;
            r.position = insertion_position;
            r.pattern.clear();

            std::vector<uint8_t> sanitized_text = sanityze(text);
            r.pattern.swap(sanitized_text);

            return r;
        }
        /**
         * @brief Creates a DELETE query.
         * @param deletion_position Start position of the deletion range.
         * @param length Number of characters to delete.
         * @return LineQuery with type DELETE.
         */
        static LineQuery create_DELETE_query(uint64_t deletion_position, uint64_t length)
        {
            LineQuery r;
            r.type = QueryType::DELETE;
            r.position = deletion_position;
            r.length = length;
            r.pattern.clear();
            return r;
        }
        /**
         * @brief Creates a COUNT query.
         * @param text Pattern to count (escape codes are sanitized).
         * @return LineQuery with type COUNT.
         */
        static LineQuery create_COUNT_query(const std::string &text)
        {
            LineQuery r;
            r.type = QueryType::COUNT;
            r.pattern.clear();

            std::vector<uint8_t> sanitized_text = sanityze(text);
            r.pattern.swap(sanitized_text);

            return r;
        }
        /**
         * @brief Creates a LOCATE query.
         * @param text Pattern to locate (escape codes are sanitized).
         * @return LineQuery with type LOCATE.
         */
        static LineQuery create_LOCATE_query(const std::string &text)
        {
            LineQuery r;
            r.type = QueryType::LOCATE;
            r.pattern.clear();

            std::vector<uint8_t> sanitized_text = sanityze(text);
            r.pattern.swap(sanitized_text);

            return r;
        }
        /**
         * @brief Creates a LOCATE_SUM query.
         * @param text Pattern to locate (escape codes are sanitized).
         * @return LineQuery with type LOCATE_SUM.
         */
        static LineQuery create_LOCATE_SUM_query(const std::string &text)
        {
            LineQuery r;
            r.type = QueryType::LOCATE_SUM;
            r.pattern.clear();

            std::vector<uint8_t> sanitized_text = sanityze(text);
            r.pattern.swap(sanitized_text);

            return r;
        }
        /*
         * @brief Split a string into fields separated by tab characters.
         * @param input Input string.
         * @return Vector of tab-separated fields.
         */
        static std::vector<std::string> split_by_tab(const std::string &input)
        {
            std::vector<std::string> result;
            std::stringstream ss(input);
            std::string item;

            while (std::getline(ss, item, '\t'))
            {
                result.push_back(item);
            }

            return result;
        }
        */
        /**
         * @brief Parses a string as an unsigned 64-bit integer.
         * @param str Decimal string to parse.
         * @return Parsed value.
         * @throws std::invalid_argument if parsing fails.
         */
        static uint64_t string_to_uint64(const std::string &str)
        {
            try
            {
                size_t idx;
                uint64_t value = std::stoull(str, &idx);

                if (idx != str.length())
                {
                    throw std::invalid_argument("Invalid character in input string");
                }

                return value;
            }
            catch (const std::exception &e)
            {
                throw std::invalid_argument("Conversion to uint64_t failed: " + std::string(e.what()));
            }
        }
        /**
         * @brief Parses one TSV query line into a LineQuery.
         * @param line Raw query line from the input file.
         * @param alternative_tab_key Escape sequence representing a tab character.
         * @param alternative_line_break_key Escape sequence representing a newline character.
         * @return Parsed query, or NONE if the line is unrecognized.
         */
        static LineQuery load_line(const std::string &line, std::string alternative_tab_key, std::string alternative_line_break_key)
        {
            std::vector<std::string> result = dynRLSLP::TSVParser::line_parse(line, alternative_tab_key, alternative_line_break_key);
            if (result.size() > 0)
            {
                if (result[0] == "INSERT" && result.size() == 3)
                {
                    uint64_t insertion_pos = string_to_uint64(result[1]);
                    return LineQuery::create_INSERT_query(insertion_pos, result[2]);
                }
                else if (result[0] == "DELETE" && result.size() == 3)
                {
                    uint64_t deletion_pos = string_to_uint64(result[1]);
                    uint64_t length = string_to_uint64(result[2]);
                    return LineQuery::create_DELETE_query(deletion_pos, length);
                }
                else if (result[0] == "COUNT" && result.size() == 2)
                {
                    return LineQuery::create_COUNT_query(result[1]);
                }
                else if (result[0] == "LOCATE" && result.size() == 2)
                {
                    return LineQuery::create_LOCATE_query(result[1]);
                }
                else if (result[0] == "LOCATE_SUM" && result.size() == 2)
                {
                    return LineQuery::create_LOCATE_SUM_query(result[1]);
                }
                else if (result[0] == "PRINT" && result.size() == 1)
                {
                    return LineQuery::create_PRINT_query();
                }
                else
                {
                    std::cout << "Skipped: " << line << std::endl;
                    return LineQuery::create_NONE_query();
                }
            }
            else
            {
                return LineQuery::create_NONE_query();
            }
        }
    };

}