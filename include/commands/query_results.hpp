#pragma once
#include "./line_query.hpp"
#include "stool/include/all.hpp"

namespace dynRLSLP
{
        /**
         * @brief An enum class for the types of vectors in QueryResults.
         * @ingroup EnumClasses
         */
        enum class VecType
        {
            PATTERN_LENGTH,
            PHASE1_TIME,
            PHASE2_TIME,
            PHASE3_TIME,
            PHASE4_TIME,
            PRIMARY_OCCURRENCE_COUNT,
            OCCURRENCE_COUNT,
            ELAPSED_TIME
        };

        /**
         * @brief A class for storing the results of queries.
         */
        struct QueryResults
        {
        public:
            std::vector<dynRLSLP::QueryType> query_types;
            std::vector<uint64_t> pattern_length_vector;
            std::vector<uint64_t> primary_occurrence_count_vector;
            std::vector<uint64_t> elapsed_time_vector;
            std::vector<uint64_t> phase1_time_vector;
            std::vector<uint64_t> phase2_time_vector;
            std::vector<uint64_t> phase3_time_vector;
            std::vector<uint64_t> phase4_time_vector;

            std::vector<uint64_t> occurrence_count_vector;

            uint64_t check_sum = 0;

            /**
             * @brief Constructs an empty query-results container.
             */
            QueryResults()
            {
                this->query_types.clear();
                this->pattern_length_vector.clear();
                this->primary_occurrence_count_vector.clear();
                this->elapsed_time_vector.clear();
                this->phase1_time_vector.clear();
                this->phase2_time_vector.clear();
                this->phase3_time_vector.clear();
                this->phase4_time_vector.clear();
                this->occurrence_count_vector.clear();
                this->check_sum = 0;
            }

            /**
             * @brief Swaps all stored vectors and the checksum with another instance.
             * @param item Other query-results object to swap with.
             */
            void swap(QueryResults &item)
            {
                this->query_types.swap(item.query_types);
                this->pattern_length_vector.swap(item.pattern_length_vector);
                this->primary_occurrence_count_vector.swap(item.primary_occurrence_count_vector);
                this->elapsed_time_vector.swap(item.elapsed_time_vector);
                this->phase1_time_vector.swap(item.phase1_time_vector);
                this->phase2_time_vector.swap(item.phase2_time_vector);
                this->phase3_time_vector.swap(item.phase3_time_vector);
                this->phase4_time_vector.swap(item.phase4_time_vector);

                this->occurrence_count_vector.swap(item.occurrence_count_vector);
                std::swap(this->check_sum, item.check_sum);
            }

            /**
             * @brief Returns occurrence counts for each query type.
             * @return Vector indexed by QueryType with per-type counts.
             */
            std::vector<uint64_t> get_query_count_vector() const
            {
                std::vector<uint64_t> r;
                r.resize(8, 0);
                for (dynRLSLP::QueryType c : this->query_types)
                {
                    r[(int)c]++;
                }
                return r;
            }
            /**
             * @brief Extracts one metric vector filtered by query type.
             * @param type1 Query type to filter on.
             * @param type2 Which stored metric vector to return.
             * @return Values of @p type2 for all queries of type @p type1.
             */
            std::vector<uint64_t> get_vector(dynRLSLP::QueryType type1, VecType type2) const
            {
                const std::vector<uint64_t> *vec;

                if (type2 == VecType::PHASE1_TIME)
                {
                    vec = &this->phase1_time_vector;
                }
                else if (type2 == VecType::PHASE2_TIME)
                {
                    vec = &this->phase2_time_vector;
                }
                else if (type2 == VecType::PHASE3_TIME)
                {
                    vec = &this->phase3_time_vector;
                }
                else if (type2 == VecType::PHASE4_TIME)
                {
                    vec = &this->phase4_time_vector;
                }
                else if (type2 == VecType::ELAPSED_TIME)
                {
                    vec = &this->elapsed_time_vector;
                }
                else if (type2 == VecType::OCCURRENCE_COUNT)
                {
                    vec = &this->occurrence_count_vector;
                }
                else if (type2 == VecType::PATTERN_LENGTH)
                {
                    vec = &this->pattern_length_vector;
                }
                else if (type2 == VecType::PRIMARY_OCCURRENCE_COUNT)
                {
                    vec = &this->primary_occurrence_count_vector;
                }

                std::vector<uint64_t> r;
                for (uint64_t i = 0; i < this->query_types.size(); i++)
                {
                    if (this->query_types[i] == type1)
                    {
                        r.push_back(vec->at(i));
                    }
                }
                return r;
            }
            /**
             * @brief Records timing for an INSERT or DELETE query.
             * @param type Query type (INSERT or DELETE).
             * @param pattern_length Length of the inserted or deleted substring.
             * @param elapsed_time Elapsed time in microseconds.
             */
            void push_back_for_update(dynRLSLP::QueryType type, uint64_t pattern_length, uint64_t elapsed_time)
            {
                this->query_types.push_back(type);
                this->pattern_length_vector.push_back(pattern_length);
                this->elapsed_time_vector.push_back(elapsed_time);

                this->phase1_time_vector.push_back(0);
                this->phase2_time_vector.push_back(0);
                this->phase3_time_vector.push_back(0);
                this->phase4_time_vector.push_back(0);
                this->occurrence_count_vector.push_back(0);
                this->primary_occurrence_count_vector.push_back(0);
            }
            /**
             * @brief Appends a query entry with zeroed timing and occurrence fields.
             * @param type Query type to record.
             */
            void push_back_for_query(dynRLSLP::QueryType type)
            {
                this->query_types.push_back(type);
                this->pattern_length_vector.push_back(0);
                this->elapsed_time_vector.push_back(0);

                this->phase1_time_vector.push_back(0);
                this->phase2_time_vector.push_back(0);
                this->phase3_time_vector.push_back(0);
                this->phase4_time_vector.push_back(0);
                this->occurrence_count_vector.push_back(0);
                this->primary_occurrence_count_vector.push_back(0);
            }

            /**
             * @brief Records timing and occurrence data for a locate-style query.
             * @param type Query type (COUNT, LOCATE, LOCATE_DETAIL, or LOCATE_SUM).
             * @param pattern_length Length of the search pattern.
             * @param elapsed_time Total elapsed time in microseconds.
             * @param phase1_time Phase 1 time in microseconds.
             * @param phase2_time Phase 2 time in microseconds.
             * @param phase3_time Phase 3 time in microseconds.
             * @param phase4_time Phase 4 time in microseconds.
             * @param primary_occurrence_count Number of primary occurrences.
             * @param occurrence_count Total number of occurrences reported.
             */
            void push_back_for_locate(dynRLSLP::QueryType type, uint64_t pattern_length, uint64_t elapsed_time, uint64_t phase1_time, uint64_t phase2_time, uint64_t phase3_time, uint64_t phase4_time, uint64_t primary_occurrence_count, uint64_t occurrence_count)
            {
                this->query_types.push_back(type);
                this->pattern_length_vector.push_back(pattern_length);
                this->elapsed_time_vector.push_back(elapsed_time);
                this->phase1_time_vector.push_back(phase1_time);
                this->phase2_time_vector.push_back(phase2_time);
                this->phase3_time_vector.push_back(phase3_time);
                this->phase4_time_vector.push_back(phase4_time);
                this->occurrence_count_vector.push_back(occurrence_count);
                this->primary_occurrence_count_vector.push_back(primary_occurrence_count);
            }
        };

        /**
         * @brief Reads and executes queries from a file against a dynamic index.
         * @tparam DYNINDEX Dynamic string index type supporting insert, delete, and locate.
         * @param dyn_index Dynamic index to query.
         * @param query_ifs Input stream of TSV query lines.
         * @param log_os Output stream for per-query log lines.
         * @param alternative_tab_key Escape sequence representing a tab character.
         * @param alternative_line_break_key Escape sequence representing a newline character.
         * @param replace_mode If true, LOCATE queries are executed as LOCATE_SUM.
         * @return Aggregated timing and occurrence results for all queries.
         */
        template <typename DYNINDEX>
        QueryResults process_query_file(DYNINDEX &dyn_index, std::ifstream &query_ifs, std::ostream &log_os, std::string alternative_tab_key, std::string alternative_line_break_key, bool replace_mode)
        {
            std::cout << "Processing query file... " << std::endl;
            std::string line;
            uint64_t query_number = 0;
            QueryResults query_results;

            while (std::getline(query_ifs, line))
            {
                std::chrono::steady_clock::time_point st1, st2;

                dynRLSLP::LineQuery q = dynRLSLP::LineQuery::load_line(line, alternative_tab_key, alternative_line_break_key);

                if (replace_mode && q.type == dynRLSLP::QueryType::LOCATE)
                {
                    q.type = dynRLSLP::QueryType::LOCATE_SUM;
                }

                if (q.type == dynRLSLP::QueryType::PRINT)
                {
                    // std::cout << "Text: " << dyn_index.to_string() << std::endl;
                    query_results.push_back_for_query(q.type);

                    log_os << query_number << "\t" << "PRINT" << "\t" << "Text:" << "\t" << dyn_index.get_text_str() << std::endl;
                }
                else if (q.type == dynRLSLP::QueryType::INSERT)
                {
                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }
                    st1 = std::chrono::steady_clock::now();

                    if (q.pattern.size() == 1)
                    {
                        dyn_index.insert_string(q.position, q.pattern[0]);
                    }
                    else
                    {
                        dyn_index.insert_string(q.position, q.pattern);
                    }
                    st2 = std::chrono::steady_clock::now();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();

                    query_results.push_back_for_update(q.type, q.pattern.size(), micro_time);

                    log_os << query_number << "\t" << "INSERT" << "\t" << "Time (microseconds): " << "\t" << micro_time << std::endl;
                }
                else if (q.type == dynRLSLP::QueryType::DELETE)
                {
                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }

                    st1 = std::chrono::steady_clock::now();

                    if (q.length == 1)
                    {
                        dyn_index.delete_substring(q.position);
                    }
                    else
                    {
                        dyn_index.delete_substring(q.position, q.length);
                    }

                    st2 = std::chrono::steady_clock::now();

                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    query_results.push_back_for_update(q.type, q.pattern.size(), micro_time);

                    log_os << query_number << "\t" << "DELETE" << "\t" << "Time (microseconds): " << "\t" << micro_time << std::endl;
                }
                else if (q.type == dynRLSLP::QueryType::COUNT)
                {
                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }
                    dynRLSLP::LocateQueryPerformanceInfo performance_info;


                    st1 = std::chrono::steady_clock::now();
                    uint64_t count_result = dyn_index.locate_query(q.pattern, &performance_info).size();
                    st2 = std::chrono::steady_clock::now();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();

                    query_results.push_back_for_locate(q.type, q.pattern.size(), micro_time, 
                    performance_info.phase1_time, performance_info.phase2_time, performance_info.phase3_time, performance_info.phase4_time, 
                    performance_info.primary_occurrence_count, count_result);

                    log_os << query_number << "\t" << "COUNT" << "\t" << "The number of occurrencces of the given pattern: " << "\t" << count_result << "\t" << "Time (microseconds): " << "\t" << micro_time << std::endl;
                }
                else if (q.type == dynRLSLP::QueryType::LOCATE)
                {
                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }
                    dynRLSLP::LocateQueryPerformanceInfo performance_info;

                    st1 = std::chrono::steady_clock::now();
                    std::vector<uint64_t> sa_values = dyn_index.locate_query(q.pattern, &performance_info);
                    st2 = std::chrono::steady_clock::now();

                    uint64_t total_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();

                    std::string sa_value_str = "";
                    sa_value_str += "[";
                    for (uint64_t i = 0; i < sa_values.size(); i++)
                    {
                        sa_value_str += std::to_string(sa_values[i]);
                        if (i < sa_values.size() - 1)
                        {
                            sa_value_str += ", ";
                        }
                    }
                    sa_value_str += "]";

                    query_results.push_back_for_locate(q.type, q.pattern.size(), total_time, 
                    performance_info.phase1_time, performance_info.phase2_time, performance_info.phase3_time, performance_info.phase4_time, 
                    performance_info.primary_occurrence_count, sa_values.size());

                    log_os << query_number << "\t" << "LOCATE" << "\t" << "The occurrences of the given pattern: " << "\t" << sa_value_str << "\t" << "Time (microseconds): " << "\t" << total_time << std::endl;
                }
                else if (q.type == dynRLSLP::QueryType::LOCATE_DETAIL)
                {

                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }

                    dynRLSLP::LocateQueryPerformanceInfo performance_info;
                    st1 = std::chrono::steady_clock::now();
                    std::vector<uint64_t> sa_values = dyn_index.locate_query(q.pattern, &performance_info);
                    st2 = std::chrono::steady_clock::now();

                    uint64_t total_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();

                    uint64_t locate_sum = 0;
                    for (uint64_t v : sa_values)
                    {
                        locate_sum += v;
                    }

                    query_results.push_back_for_locate(q.type, q.pattern.size(), total_time, 
                    performance_info.phase1_time, performance_info.phase2_time, performance_info.phase3_time, performance_info.phase4_time, 
                    performance_info.primary_occurrence_count, sa_values.size());

                    log_os << query_number << ", " << "LOCATE, " << "Total time (microseconds)," << total_time << ", The number of occurrencces, " << sa_values.size() << ", Checksum, " << locate_sum << std::endl;
                }
                else if (q.type == dynRLSLP::QueryType::LOCATE_SUM)
                {
                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }

                    dynRLSLP::LocateQueryPerformanceInfo performance_info;

                    st1 = std::chrono::steady_clock::now();
                    std::vector<uint64_t> sa_values = dyn_index.locate_query(q.pattern, &performance_info);
                    st2 = std::chrono::steady_clock::now();

                    uint64_t total_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();

                    uint64_t locate_sum = 0;

                    std::string occ_str = "";
                    for (uint64_t v : sa_values)
                    {
                        locate_sum += v;
                    }
                    query_results.push_back_for_locate(q.type, q.pattern.size(), total_time, 
                    performance_info.phase1_time, performance_info.phase2_time, performance_info.phase3_time, performance_info.phase4_time, 
                    performance_info.primary_occurrence_count, sa_values.size());



                    

                    log_os << query_number << "\t" << "LOCATE_SUM" << "\t" 
                    << "The sum of occurrence positions of the given pattern: " << "\t" << locate_sum 
                    << "\t" << "The number of occurences: " << "\t" << sa_values.size() << "\t" 
                    << "Time (microseconds): " << "\t" << total_time 
                    << "\t" << "Type: " << "\t" << performance_info.get_type_string()
                    << "\t" << "Candidate position count: " << "\t" << performance_info.candidate_position_count
                    << "\t" << "Primary occurrence count: " << "\t" << performance_info.primary_occurrence_count
                    << "\t" << "Phase 1 time (microseconds): " << "\t" << performance_info.phase1_time
                    << "\t" << "Phase 2 time (microseconds): " << "\t" << performance_info.phase2_time
                    << "\t" << "Phase 3 time (microseconds): " << "\t" << performance_info.phase3_time
                    << "\t" << "Phase 4 time (microseconds): " << "\t" << performance_info.phase4_time
                    << std::endl;
                }
                else
                {
                    std::cout << "N" << std::flush;

                    query_results.push_back_for_query(q.type);

                    log_os << query_number << ", " << "NONE" << std::endl;
                }
                query_number++;
            }

            return query_results;
        }
        /**
         * @brief Prints total, average, min, and max for one metric vector.
         * @param result Query results to summarize.
         * @param type1 Query type to filter on.
         * @param type2 Which stored metric vector to summarize.
         * @param message_name Label printed before the statistics.
         * @param unit_name Unit suffix appended to numeric values.
         * @param message_paragraph Indentation level for output messages.
         */
        void print_sub(const QueryResults &result, dynRLSLP::QueryType type1, VecType type2, std::string message_name, std::string unit_name, int message_paragraph = stool::Message::SHOW_MESSAGE)
        {
            std::vector<uint64_t> vec = result.get_vector(type1, type2);
            uint64_t _total = std::reduce(std::begin(vec), std::end(vec));
            uint64_t _average = _total / vec.size();
            uint64_t _min = *min_element(std::begin(vec), std::end(vec));
            uint64_t _max = *max_element(std::begin(vec), std::end(vec));
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << message_name << ": \t\t\t\t\t" << _total << " " << unit_name << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Average: \t\t\t\t\t" << _average << " " << unit_name << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Min: \t\t\t\t\t" << _min << " " << unit_name << std::endl;
            std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Max: \t\t\t\t\t" << _max << " " << unit_name << std::endl;
        }

        /**
         * @brief Prints a summary of results for one query type.
         * @param result Query results to summarize.
         * @param type Query type whose statistics are printed.
         * @param message_paragraph Indentation level for output messages.
         */
        void print_query_result(const QueryResults &result, dynRLSLP::QueryType type, int message_paragraph = stool::Message::SHOW_MESSAGE)
        {
            std::vector<uint64_t> query_count_vector = result.get_query_count_vector();
            uint64_t counter = query_count_vector[(int)type];

            if (type == dynRLSLP::QueryType::PRINT)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of PRINT queries: \t\t\t\t\t" << counter << std::endl;
            }
            else if (type == dynRLSLP::QueryType::INSERT)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of INSERT queries: \t\t\t\t\t" << counter << std::endl;
                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    //print_sub(result, type, VecType::REORDER_COUNT, "Total reorder count", "", message_paragraph);
                }
            }
            else if (type == dynRLSLP::QueryType::DELETE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of DELETE queries: \t\t\t\t\t" << counter << std::endl;

                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    //print_sub(result, type, VecType::REORDER_COUNT, "Total reorder count", "", message_paragraph);
                }
            }
            else if (type == dynRLSLP::QueryType::COUNT)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of COUNT queries: \t\t\t\t\t" << counter << std::endl;
                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    print_sub(result, type, VecType::OCCURRENCE_COUNT, "Total occurrences", "", message_paragraph);
                }
            }
            else if (type == dynRLSLP::QueryType::LOCATE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of LOCATE queries: \t\t\t\t\t" << counter << std::endl;
                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PHASE1_TIME, "Total phase 1 time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PHASE2_TIME, "Total phase 2 time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PHASE3_TIME, "Total phase 3 time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PHASE4_TIME, "Total phase 4 time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    print_sub(result, type, VecType::PRIMARY_OCCURRENCE_COUNT, "Total primary occurrences", "", message_paragraph);
                    print_sub(result, type, VecType::OCCURRENCE_COUNT, "Total occurrences", "", message_paragraph);
                }
            }
            else if (type == dynRLSLP::QueryType::LOCATE_SUM)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of LOCATE_SUM queries: \t\t\t\t\t" << counter << std::endl;
                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PHASE1_TIME, "Total phase 1 time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PHASE2_TIME, "Total phase 2 time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PHASE3_TIME, "Total phase 3 time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PHASE4_TIME, "Total phase 4 time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    print_sub(result, type, VecType::PRIMARY_OCCURRENCE_COUNT, "Total primary occurrences", "", message_paragraph);
                    print_sub(result, type, VecType::OCCURRENCE_COUNT, "Total occurrences", "", message_paragraph);
                }
            }
            else
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of NONE queries: \t\t\t\t\t" << counter << std::endl;
            }
        }
    

}