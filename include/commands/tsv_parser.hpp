#pragma once
#include "./line_query.hpp"
#include "stool/include/all.hpp"

namespace dynRLSLP
{
        /**
         * @brief A parser for TSV files.
         */
        class TSVParser
        {
        private:
            /**
             * @brief Replaces escape sequences in a line with tab and newline characters.
             * @param line Raw input line.
             * @param replacement_code_to_tab Escape sequence representing a tab.
             * @param replacement_code_to_line_break Escape sequence representing a newline.
             * @return Sanitized line with real tab and newline characters.
             */
            static std::string get_sanitized_line(const std::string line, const std::string replacement_code_to_tab, const std::string replacement_code_to_line_break)
            {
                std::string r;
                uint64_t i = 0;
                while (i < line.size())
                {
                    uint64_t len = line.size() - i;
                    std::string cand1 = "";
                    if (len <= replacement_code_to_tab.size())
                    {
                        cand1 = line.substr(i, len);
                    }
                    std::string cand2 = "";
                    if (len <= replacement_code_to_line_break.size())
                    {
                        cand2 = line.substr(i, len);
                    }

                    if (cand1.size() > 0 && cand1 == replacement_code_to_tab)
                    {
                        r.push_back('\t');
                        i += replacement_code_to_tab.size();
                    }
                    else if (cand2.size() > 0 && cand2 == replacement_code_to_line_break)
                    {
                        r.push_back('\n');
                        i += replacement_code_to_line_break.size();
                    }
                    else
                    {
                        r += line[i];
                        i++;
                    }
                }
                return r;
            }

        public:
            /**
             * @brief Splits a TSV line into fields after sanitizing escape sequences.
             * @param line Raw input line.
             * @param replacement_code_to_tab Escape sequence representing a tab.
             * @param replacement_code_to_line_break Escape sequence representing a newline.
             * @return Vector of tab-separated field strings.
             */
            static std::vector<std::string> line_parse(const std::string line, const std::string replacement_code_to_tab, const std::string replacement_code_to_line_break)
            {
                std::string sanitized_line = get_sanitized_line(line, replacement_code_to_tab, replacement_code_to_line_break);
                std::vector<std::string> result;
                std::stringstream ss(sanitized_line);
                std::string item;

                while (std::getline(ss, item, '\t'))
                {
                    result.push_back(item);
                }

                return result;
            }
        };
    
}