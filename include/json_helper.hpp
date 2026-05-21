#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <filesystem>
#include "stool/include/all.hpp"

namespace dynRLSLP
{
    class JsonHelper
    {
    public:
    static std::string escapeJsonChar(char c) {
        switch (c) {
            case '"':
                return "\\\"";
            case '\\':
                return "\\\\";
            case '\b':
                return "\\b";
            case '\f':
                return "\\f";
            case '\n':
                return "\\n";
            case '\r':
                return "\\r";
            case '\t':
                return "\\t";
            default:
                break;
        }
    
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x20) {
            std::ostringstream oss;
            oss << "\\u"
                << std::hex << std::setw(4) << std::setfill('0')
                << static_cast<int>(uc);
            return oss.str();
        }
    
        return std::string(1, c);
    }

        template <typename T>
        static void write_content_as_json_format(std::string name, const std::vector<T> &data, std::function<std::string(const T&)> display_converter, 
        bool use_new_line_per_item,
        std::ofstream &ofs, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) {
            
            ofs << stool::Message::get_paragraph_string(message_paragraph) << "\"" << name << "\": " << "["; 
            if(use_new_line_per_item){
                ofs << std::endl;
            }
            for(uint64_t i = 0; i < data.size(); i++){
                if(use_new_line_per_item){
                    ofs << stool::Message::get_paragraph_string(message_paragraph+1); 
                }
                ofs << "\"" << display_converter(data[i]) << "\"";
                if(i != data.size() - 1){
                    ofs << ", ";
                }
                if(use_new_line_per_item){
                    ofs << std::endl;
                }
            }

            if(use_new_line_per_item){
                ofs << stool::Message::get_paragraph_string(message_paragraph);
            }
            ofs << "]";

        }

        template <typename T, typename U>
        static void write_content_as_json_format(std::string name, const std::unordered_map<T, U> &data, 
            std::function<std::string(const T&)> key_display_converter, 
            std::function<std::string(const U&)> value_display_converter, 
        bool use_new_line_per_item,
        std::ofstream &ofs, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) {
            uint64_t item_count = data.size();
            uint64_t index = 0;

            ofs << stool::Message::get_paragraph_string(message_paragraph) << "\"" << name << "\": " << "{"; 
            if(use_new_line_per_item){
                ofs << std::endl;
            }
            for(auto& [key, value] : data){
                index++;
                if(use_new_line_per_item){
                    ofs << stool::Message::get_paragraph_string(message_paragraph+1); 
                }
                ofs << "\"" << key_display_converter(key) << "\": \"" << value_display_converter(value) << "\"";
                if(index < item_count){
                    ofs << ", ";
                }
                if(use_new_line_per_item){
                    ofs << std::endl;
                }
            }
            if(use_new_line_per_item){
                ofs << stool::Message::get_paragraph_string(message_paragraph);
            }
            ofs << "}";
        }
        template <typename T, typename U>
        static void write_content_as_json_format(std::string name, const std::map<T, U> &data, 
            std::function<std::string(const T&)> key_display_converter, 
            std::function<std::string(const U&)> value_display_converter, 
        bool use_new_line_per_item,
        std::ofstream &ofs, int64_t message_paragraph = stool::Message::SHOW_MESSAGE) {
            uint64_t item_count = data.size();
            uint64_t index = 0;

            ofs << stool::Message::get_paragraph_string(message_paragraph) << "\"" << name << "\": " << "{"; 
            if(use_new_line_per_item){
                ofs << std::endl;
            }
            for(auto& [key, value] : data){
                index++;
                if(use_new_line_per_item){
                    ofs << stool::Message::get_paragraph_string(message_paragraph+1); 
                }
                ofs << "\"" << key_display_converter(key) << "\": \"" << value_display_converter(value) << "\"";
                if(index < item_count){
                    ofs << ", ";
                }
                if(use_new_line_per_item){
                    ofs << std::endl;
                }
            }
            if(use_new_line_per_item){
                ofs << stool::Message::get_paragraph_string(message_paragraph);
            }
            ofs << "}";
        }



    };
}
