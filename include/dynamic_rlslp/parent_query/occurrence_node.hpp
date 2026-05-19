#pragma once
#include "stool/include/all.hpp"
#include "./fast_parent_dictionary.hpp"

namespace dynRLSLP
{
        /**
         * @brief Node in an implicit occurrence tree used for low-memory occurrence enumeration.
         * @ingroup ParentClasses
         */
        struct OccurrenceNode
        {
            NonterminalWithRelativeLevel nonterminal;
            uint64_t occ;            
            uint64_t children_start_index;
            uint64_t children_end_index;
            //std::vector<int64_t> positionOffsets;
            //std::vector<uint64_t> children;

            /**
             * @brief Constructs an occurrence tree node with child range indices.
             * @param nonterminal The nonterminal associated with this node.
             * @param occ Total number of leaf occurrences in the subtree rooted here.
             * @param children_start_index Inclusive start index into the children range list.
             * @param children_end_index Inclusive end index into the children range list.
             */
            OccurrenceNode(NonterminalWithRelativeLevel nonterminal, uint64_t occ, uint64_t children_start_index, uint64_t children_end_index) : nonterminal(nonterminal), occ(occ), children_start_index(children_start_index), children_end_index(children_end_index){
            }
            /**
             * @brief Default constructor initializing all fields to sentinel maximum values.
             */
            OccurrenceNode() : nonterminal(std::numeric_limits<NonterminalWithRelativeLevel>::max()), occ(std::numeric_limits<uint64_t>::max()), children_start_index(std::numeric_limits<uint64_t>::max()), children_end_index(std::numeric_limits<uint64_t>::max()){
            }

            /**
             * @brief Collects all leaf occurrence positions by iterative depth-first traversal.
             * @param occurrence_nodes The occurrence tree nodes.
             * @param children_range_list Child pointers as (node index, position offset) pairs; UINT64_MAX denotes a leaf.
             * @param root_idx Index of the root node in occurrence_nodes.
             * @return Sorted list of absolute occurrence positions.
             */
            static std::vector<uint64_t> get_all_occurrences(const std::vector<OccurrenceNode> &occurrence_nodes, const std::vector<std::pair<uint64_t, uint64_t>> &children_range_list, uint64_t root_idx){
                std::vector<uint64_t> output;
                VStack<std::pair<uint64_t, uint64_t>> stk;
                stk.push({root_idx, 0});


                while (stk.size() > 0)
                {
                    auto top = stk.top();
                    uint64_t current_idx = top.first;
                    uint64_t current_position = top.second;
                    stk.pop();
                    if(current_idx == UINT64_MAX){
                        output.push_back(current_position);
                    }else{
                        uint64_t children_start_index = occurrence_nodes[current_idx].children_start_index;
                        uint64_t children_end_index = occurrence_nodes[current_idx].children_end_index;
                        for(uint64_t i = children_start_index; i <= children_end_index; i++){
                            uint64_t child_idx = children_range_list[i].first;
                            uint64_t offset = children_range_list[i].second;
                            stk.push({child_idx, current_position + offset});
                        }
                    }
                }
                return output;
            }

            /**
             * @brief Recursively writes leaf occurrence positions into a pre-sized output buffer.
             * @param occurrence_nodes The occurrence tree nodes.
             * @param children_range_list Child pointers as (node index, position offset) pairs; UINT64_MAX denotes a leaf.
             * @param idx Index of the current node in occurrence_nodes.
             * @param position_offset Position offset accumulated from ancestors.
             * @param output_i Next write index in output.
             * @param output Pre-allocated buffer to fill with occurrence positions.
             * @return Number of leaf occurrences in the subtree rooted at idx.
             */
            static uint64_t get_all_occurrences_recursive(const std::vector<OccurrenceNode> &occurrence_nodes, const std::vector<std::pair<uint64_t, uint64_t>> &children_range_list, 
                uint64_t idx, uint64_t position_offset, uint64_t output_i, std::vector<uint64_t> &output){

                const OccurrenceNode& current_node = occurrence_nodes[idx];
                uint64_t children_start_index = current_node.children_start_index;
                uint64_t children_end_index = current_node.children_end_index;
                for(uint64_t i = children_start_index; i <= children_end_index; i++){
                    uint64_t child_idx = children_range_list[i].first;
                    uint64_t offset = children_range_list[i].second;
                    if(child_idx != UINT64_MAX){
                        output_i += get_all_occurrences_recursive(occurrence_nodes, children_range_list, child_idx, position_offset + offset, output_i, output);
                    }else{
                        output[output_i++] = position_offset + offset;                        
                    }
                }
                return occurrence_nodes[idx].occ;
            }

        };
    
}