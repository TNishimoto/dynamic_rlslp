

#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>

#include "stool/include/all.hpp"
#include "../include/all.hpp"
#include "cmdline/cmdline.h"




void process_query_file(dynRLSLP::DynamicRLSLPString &dyn_index, std::ifstream &query_ifs, std::ostream &query_result_os, 
    const std::string replacement_code_to_tab = "", const std::string replacement_code_to_line_break = "")
{

    std::string line;
    uint64_t query_number = 0;

    query_result_os << "Query Number\tQuery Name\tTime(ns)\tResult" << std::endl;

    while (std::getline(query_ifs, line))
    {
        std::chrono::system_clock::time_point st1, st2;

        std::vector<std::string> fields = dynRLSLP::TSVParser::line_parse(line, replacement_code_to_tab, replacement_code_to_line_break);

        if(fields.size() == 0){
            std::cout << query_number << " " << "Skipped: " << line << std::endl;
            continue;
        }

        std::string query_name = fields[0];

        if(query_name == "INSERT"){
            uint64_t insertion_pos = stoull(fields[1]);
            std::vector<uint8_t> insertion_text = dynRLSLP::TSVParser::to_uint8_t_vector(fields[2]);
            st1 = std::chrono::system_clock::now();
            dyn_index.insert_string(insertion_pos, insertion_text);
            st2 = std::chrono::system_clock::now();
            uint64_t nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

            query_result_os << query_number << "\tINSERT\t" << nano_time << std::endl;
        }else if(query_name == "DELETE"){
            uint64_t deletion_pos = stoull(fields[1]);
            uint64_t deletion_length = stoull(fields[2]);
            st1 = std::chrono::system_clock::now();
            dyn_index.delete_substring(deletion_pos, deletion_length);
            st2 = std::chrono::system_clock::now();
            uint64_t nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

            query_result_os << query_number << "\tDELETE\t" << nano_time << std::endl;
        }else if(query_name == "ACCESS"){
            uint64_t access_pos = stoull(fields[1]);
            uint64_t access_length = stoull(fields[2]);

            st1 = std::chrono::system_clock::now();
            std::vector<uint8_t> access_text = dyn_index.access_substring(access_pos, access_length);
            st2 = std::chrono::system_clock::now();
            uint64_t nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

            std::string access_text_str = std::string(access_text.begin(), access_text.end());

            query_result_os << query_number << "\tACCESS\t" << nano_time << "\t" << access_text_str << std::endl;
        }
        else if(query_name == "DECOMPRESS"){
            std::string output_path = fields[1];
            std::ofstream output_ofs(output_path);
            st1 = std::chrono::system_clock::now();
            dyn_index.decompress(output_ofs);
            st2 = std::chrono::system_clock::now();
            uint64_t nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();
            output_ofs.close();
            
            query_result_os << query_number << "\tDECOMPRESS\t" << nano_time << "\t" << fields[1] << std::endl;
        }
        else if(query_name == "LCE"){
            uint64_t lce_pos1 = stoull(fields[1]);
            uint64_t lce_pos2 = stoull(fields[2]);
            st1 = std::chrono::system_clock::now();
            uint64_t lce_result = dyn_index.lce(lce_pos1, lce_pos2);
            st2 = std::chrono::system_clock::now();
            uint64_t nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();

            query_result_os << query_number << "\tLCE\t" << nano_time << "\t" << lce_result << std::endl;

        }
        else
        {
            std::cout << "N" << std::flush;
            query_result_os << query_number << "\t" << "UNKNOWN" << std::endl;
        }
        query_number++;
    }
}


/**
 * @brief Main function to execute queries on a dynamic index
 * @param argc Number of command-line arguments
 * @param argv Command-line arguments
 * @return Exit code (0 on success)
 */
int main(int argc, char *argv[])
{
std::cout << "\033[41m";
#ifdef RELEASE_BUILD
    std::cout << "Running in Release mode";
#elif defined(DEBUG_BUILD)

    std::cout << "Running in Debug mode";
#else
    std::cout << "Running in Unknown mode";
#endif
std::cout << "\e[m" << std::endl;

    cmdline::parser p;

    p.add<std::string>("input_index_path", 'i', "Input index file path (.ds)", true);
    p.add<std::string>("input_query_path", 'q', "Input query file path (TSV format)", true);
    p.add<std::string>("output_log_path", 'w', "Output log file path", true);
    p.add<std::string>("output_index_path", 'o', "Save updated index (optional)", false, "");
    p.add<std::string>("alternative_tab_key", 't', "The alternative tab key for the query file (optional)", false, "");
    p.add<std::string>("alternative_line_break_key", 'n', "The alternative line break key for the query file (optional)", false, "");


    p.parse_check(argc, argv);
    std::string input_index_path = p.get<std::string>("input_index_path");
    std::string input_query_path = p.get<std::string>("input_query_path");
    std::string log_file_path = p.get<std::string>("output_log_path");
    std::string output_index_path = p.get<std::string>("output_index_path");
    std::string alternative_tab_key = p.get<std::string>("alternative_tab_key");
    std::string alternative_line_break_key = p.get<std::string>("alternative_line_break_key");

    std::cout << "Loading the dynamic string from the file..." << std::endl;
    std::ifstream ifs(input_index_path, std::ios::binary);
    dynRLSLP::DynamicRLSLPString ds = dynRLSLP::DynamicRLSLPString::load_from_file(ifs);
    ifs.close();

    std::ifstream query_ifs(input_query_path);
    std::ofstream query_result_os(log_file_path);

    process_query_file(ds, query_ifs, query_result_os, alternative_tab_key, alternative_line_break_key);

    query_ifs.close();
    query_result_os.close();

    if(output_index_path.size() > 0){
        std::ofstream ofs(output_index_path, std::ios::binary);
        dynRLSLP::DynamicRLSLPString::store_to_file(ds, ofs);
        ofs.close();
    }

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Index File:                 " << input_index_path << std::endl;
    std::cout << "Query File:                 " << input_query_path << std::endl;
    std::cout << "Log File:                   " << log_file_path << std::endl;
    std::cout << "Output Index File:          " << output_index_path << std::endl;
    std::cout << "Alternative Tab Key:        " << alternative_tab_key << std::endl;
    std::cout << "Alternative Line Break Key: " << alternative_line_break_key << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}
