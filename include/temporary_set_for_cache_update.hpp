#pragma once
#include <unordered_set>
#include "dynamic_rlslp/dynamic_grammar_for_layered_rlslp.hpp"

namespace dynRLSLP
{
    class TemporarySetForCacheUpdate{
        public: 
        std::unordered_set<ExplicitNonterminal> removed_nonterminal_set;
        std::unordered_set<ExplicitNonterminal> unremoved_chidlren_of_removed_nonterminal_set;
        std::unordered_set<ExplicitNonterminal> added_nonterminal_set;
        std::unordered_set<ExplicitNonterminal> nonadded_chidlren_of_added_nonterminal_set;

        TemporarySetForCacheUpdate(){

        }
        void import(TemporarySetForCacheUpdate& other){
            this->removed_nonterminal_set.insert(other.removed_nonterminal_set.begin(), other.removed_nonterminal_set.end());
            this->unremoved_chidlren_of_removed_nonterminal_set.insert(other.unremoved_chidlren_of_removed_nonterminal_set.begin(), other.unremoved_chidlren_of_removed_nonterminal_set.end());
            this->added_nonterminal_set.insert(other.added_nonterminal_set.begin(), other.added_nonterminal_set.end());
            this->nonadded_chidlren_of_added_nonterminal_set.insert(other.nonadded_chidlren_of_added_nonterminal_set.begin(), other.nonadded_chidlren_of_added_nonterminal_set.end());

            other.removed_nonterminal_set.clear();
            other.unremoved_chidlren_of_removed_nonterminal_set.clear();
            other.added_nonterminal_set.clear();
            other.nonadded_chidlren_of_added_nonterminal_set.clear();
        }        
    }; 
}