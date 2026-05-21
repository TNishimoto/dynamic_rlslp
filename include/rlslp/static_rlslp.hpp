#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <unordered_set>
#include <functional>
#include "./components/random_bit_dictionary.hpp"
#include "./components/dictionary_for_layered_rlslp.hpp"
#include "../local_parsings/locally_consistent_parsing.hpp"
#include "./helper/nonterminal_less_comparer.hpp"
#include "./helper/derivation_tree_visualizer.hpp"
#include "./static_operations/all.hpp"


namespace dynRLSLP
{
    class StaticRLSLP
    {
        public:
        std::vector<NonterminalWithRelativeLevel> nonterminal_list;
        std::vector<RLSLPRuleBody> rule_list;
        NonterminalWithRelativeLevel root_id;


        

        void write_to_file_as_json(const std::string &file_path) const{
            std::ofstream ofs(file_path);
            ofs << "{" << std::endl;
            ofs << " \"root_id\": \"" << std::to_string(this->root_id) << "\"," << std::endl;
            ofs << " \"production_rules\": \"" << "{" << std::endl;

            for(uint64_t i = 0; i < this->rule_list.size(); i++){
                RLSLPRuleBody rule = this->rule_list[i];
                NonterminalWithRelativeLevel nonterminal = this->nonterminal_list[i];
                ofs << stool::Message::get_paragraph_string(1) << " \"" << NonterminalFunctions::to_string(nonterminal) << "\": \"" << rule.to_string() << "\"";
                if(i != this->rule_list.size() - 1){
                    ofs << ", " << std::endl;
                }else{
                    ofs << std::endl;
                }
            }
            ofs << stool::Message::get_paragraph_string(1) << "}" << std::endl;
            ofs << "}" << std::endl;
            ofs.close();
            
        }
    };
}
