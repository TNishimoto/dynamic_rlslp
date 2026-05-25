#pragma once
#include "stool/include/all.hpp"
#include "./fast_parent_dictionary.hpp"
#include "./occurrence_node.hpp"
#include "./sub/occurrence_pointer.hpp"

namespace dynRLSLP
{
        /**
         * @brief Static queries for enumerating node occurrences in a layered RLSLP grammar.
         * @ingroup ParentClasses
         */
        class NodeOccurrenceQuery
        {
        private:
            /**
             * @brief Finds a type-2 primary occurrence by walking up a single-ancestor chain.
             * @param sig Base nonterminal to locate.
             * @param fastParentDictionary Parent dictionary of the grammar.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param explicit_nonterminal_length_list Derived string lengths indexed by base nonterminal.
             * @return Pair of the primary temporary occurrence and the walk depth.
             */
            static std::pair<TemporaryOccurrence, uint64_t> find_type_2_primary_occurrence_of_nonterminal(ExplicitNonterminal sig, const FastParentDictionary &fastParentDictionary, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list,
                                                                                                        const std::vector<uint64_t> &explicit_nonterminal_length_list)
            {
                if (fastParentDictionary.is_empty(sig))
                {
                    return {TemporaryOccurrence(sig, 0), 0};
                }
                else
                {
                    if (fastParentDictionary.has_single_ancestor(sig, explicit_nonterminal_rule_list))
                    {
                        NonterminalWithRelativeLevel parent = fastParentDictionary.take_any_important_ancestor(sig);
                        uint64_t position = ManyParentsManager::get_any_occurrence_position(sig, parent, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, true);
                        //FastParentDictionary::MUDA_COUNT++;
                        auto q = find_type_2_primary_occurrence_of_nonterminal(parent, fastParentDictionary, explicit_nonterminal_rule_list, explicit_nonterminal_length_list);
                        uint64_t new_pos = position + q.first.position;
                        TemporaryOccurrence new_to(q.first.nonterminal, new_pos);
                        uint64_t depth = q.second + 1;
                        return {new_to, depth};
                    }
                    else
                    {
                        return {TemporaryOccurrence(sig, 0), 0};
                    }
                }
            }
            /**
             * @brief Finds a type-2 primary occurrence using a precomputed occurrence cache.
             * @param sig Base nonterminal to locate.
             * @param position_offset Accumulated position offset along the cache chain.
             * @param occCacheList Per-base-nonterminal cached primary occurrences.
             * @return Primary temporary occurrence with absolute position offset.
             */
            static TemporaryOccurrence find_type_2_primary_occurrence_of_nonterminal(ExplicitNonterminal sig, int64_t position_offset, const std::vector<TemporaryOccurrence> &occCacheList)
            {
                TemporaryOccurrence temp_occurrence = occCacheList[sig];
                if (temp_occurrence.nonterminal != sig)
                {
                    //FastParentDictionary::MUDA_COUNT++;
                    int64_t new_offset = position_offset + temp_occurrence.position;
                    return find_type_2_primary_occurrence_of_nonterminal(temp_occurrence.nonterminal, new_offset, occCacheList);
                }
                else
                {
                    return TemporaryOccurrence(sig, position_offset + temp_occurrence.position);
                }
            }

            /**
             * @brief Resolves type-2 secondary occurrences, optionally expanding through type-1 parents.
             * @param sig Base nonterminal to locate.
             * @param fastParentDictionary Parent dictionary of the grammar.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param explicit_nonterminal_length_list Derived string lengths indexed by base nonterminal.
             * @param occCacheList Optional per-base-nonterminal occurrence cache; nullptr disables the cache.
             * @param output Stack receiving expanded temporary occurrences when multiple parents exist.
             * @return -1 if multiple occurrences were pushed onto output; otherwise the single position offset.
             */
            static int64_t find_type_2_secondary_occurrences_of_nonterminal(ExplicitNonterminal sig, const FastParentDictionary &fastParentDictionary, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list,
                                                                          const std::vector<uint64_t> &explicit_nonterminal_length_list, const std::vector<TemporaryOccurrence> *occCacheList, VStack<TemporaryOccurrence> &output)
            {
                TemporaryOccurrence temp_occurrence = TemporaryOccurrence::create_null_occurrence();

                if (occCacheList == nullptr)
                {
                    auto p = find_type_2_primary_occurrence_of_nonterminal(sig, fastParentDictionary, explicit_nonterminal_rule_list, explicit_nonterminal_length_list);
                    temp_occurrence = p.first;
                }
                else
                {
                    temp_occurrence = find_type_2_primary_occurrence_of_nonterminal(sig, 0, *occCacheList);
                }

                uint64_t base_size = output.size();
                fastParentDictionary.get_all_type_1_primary_occurrences_of_nonterminal(temp_occurrence.nonterminal, temp_occurrence.position, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, output);
                //FastParentDictionary::NOT_MUDA_COUNT++;
                uint64_t new_size = output.size();

                if (new_size - base_size >= 1)
                {
                    return -1;
                }
                else
                {
                    return temp_occurrence.position;
                }
            }

        public:
            /**
             * @brief Finds a type-2 primary occurrence with a bounded ancestor-walk depth.
             * @param sig Base nonterminal to locate.
             * @param fastParentDictionary Parent dictionary of the grammar.
             * @param explicit_nonterminal_rule_list Rule bodies indexed by base nonterminal.
             * @param explicit_nonterminal_length_list Derived string lengths indexed by base nonterminal.
             * @param max_depth Maximum number of ancestor hops allowed.
             * @param current_depth Current depth in the ancestor walk.
             * @return Primary temporary occurrence, or (sig, 0) if the depth limit is reached.
             */
            static TemporaryOccurrence find_type_2_primary_occurrence_of_nonterminal_using_limited_depth(ExplicitNonterminal sig, const FastParentDictionary &fastParentDictionary, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list,
                                                                                                       const std::vector<uint64_t> &explicit_nonterminal_length_list, uint64_t max_depth, uint64_t current_depth)
            {
                if (current_depth >= max_depth)
                {
                    return TemporaryOccurrence(sig, 0);
                }
                else
                {
                    if (fastParentDictionary.is_empty(sig))
                    {
                        return TemporaryOccurrence(sig, 0);
                    }
                    else if (fastParentDictionary.has_single_ancestor(sig, explicit_nonterminal_rule_list))
                    {
                        NonterminalWithRelativeLevel parent = fastParentDictionary.take_any_important_ancestor(sig);
                        uint64_t position = ManyParentsManager::get_any_occurrence_position(sig, parent, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, true);

                        TemporaryOccurrence q = find_type_2_primary_occurrence_of_nonterminal_using_limited_depth(parent, fastParentDictionary, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, max_depth, current_depth + 1);
                        uint64_t new_pos = position + q.position;
                        TemporaryOccurrence new_to(q.nonterminal, new_pos);
                        return new_to;
                    }
                    else
                    {
                        return TemporaryOccurrence(sig, 0);
                    }
                }
            }
            /**
             * @brief Enumerates all leaf occurrence positions by expanding type-1 primary occurrences.
             * @param input Seed temporary occurrences to start enumeration from.
             * @param fastParentDictionary Parent dictionary of the grammar.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param explicit_nonterminal_length_list Derived string lengths indexed by base nonterminal.
             * @return All absolute occurrence positions.
             */
            static std::vector<uint64_t> faster_get_all_occurrences(const std::vector<TemporaryOccurrence> &input, 
                const FastParentDictionary &fastParentDictionary, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, const std::vector<uint64_t> &explicit_nonterminal_length_list)
            {

                VStack<TemporaryOccurrence> stk;
                std::vector<uint64_t> output;

                for (uint64_t i = 0; i < input.size(); i++)
                {
                    stk.push(input[i]);
                }

                while (stk.size() > 0)
                {
                    auto top = stk.top();
                    stk.pop();
                    TemporaryOccurrence temporary_occurrence = top;

                    /*
                    uint64_t dif = stk.size();
                    uint64_t dif2 = stk.size();
                    if (dif2 - dif <= 1)
                    {
                        FastParentDictionary::MUDA_COUNT++;
                    }
                    else
                    {
                        FastParentDictionary::NOT_MUDA_COUNT++;
                    }
                    */

                    bool b = fastParentDictionary.get_all_type_1_primary_occurrences_of_nonterminal(temporary_occurrence.nonterminal, temporary_occurrence.position, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, stk);
                    if (!b)
                    {
                        output.push_back(temporary_occurrence.position);
                    }
                }
                return output;
            }

            /**
             * @brief Enumerates occurrences level-by-level using extra memory for grouping by nonterminal level.
             * @param input Seed temporary occurrences to start enumeration from.
             * @param fastParentDictionary Parent dictionary of the grammar.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param explicit_nonterminal_length_list Base-nonterminal length list (L).
             * @param explicit_nonterminal_level_list Base-nonterminal level list (H).
             * @param tree_max_level Maximum level in the derivation forest.
             * @param occCacheList Optional per-base-nonterminal occurrence cache; nullptr disables the cache.
             * @return All absolute occurrence positions.
             */
            static std::vector<uint64_t> faster_get_all_occurrences_using_memory(const std::vector<TemporaryOccurrence> &input, const FastParentDictionary &fastParentDictionary, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list,
                                                                                 const std::vector<uint64_t> &explicit_nonterminal_length_list, const std::vector<uint16_t> &explicit_nonterminal_level_list, uint64_t tree_max_level, const std::vector<TemporaryOccurrence> *occCacheList)
            {
                std::vector<std::vector<TemporaryOccurrence>> level_to_occurrences_map;
                level_to_occurrences_map.resize(tree_max_level + 1);
                std::vector<uint64_t> output;
                VStack<TemporaryOccurrence> stk;
                std::vector<TemporaryOccurrence> h_temp_occurrences;

                for (uint64_t i = 0; i < input.size(); i++)
                {
                    uint64_t level = NonterminalFunctions::get_level(input[i].nonterminal, explicit_nonterminal_level_list);
                    level_to_occurrences_map[level].push_back(input[i]);
                }

                uint64_t h = 0;
                while (h <= tree_max_level)
                {
                    if (level_to_occurrences_map[h].size() == 0)
                    {
                        h++;
                    }
                    else
                    {
                        h_temp_occurrences.swap(level_to_occurrences_map[h]);
                        std::sort(h_temp_occurrences.begin(), h_temp_occurrences.end(), [](const TemporaryOccurrence &a, const TemporaryOccurrence &b)
                                  { return a.nonterminal < b.nonterminal; });

                        uint64_t i = 0;
                        while (i < h_temp_occurrences.size())
                        {
                            TemporaryOccurrence temporary_occurrence(h_temp_occurrences[i].nonterminal, 0);
                            int64_t p = find_type_2_secondary_occurrences_of_nonterminal(temporary_occurrence.nonterminal, fastParentDictionary, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, occCacheList, stk);
                            uint64_t k = i + 1;
                            while (k < h_temp_occurrences.size() && h_temp_occurrences[k].nonterminal == temporary_occurrence.nonterminal)
                            {
                                k++;
                            }

                            if (p == -1)
                            {

                                while (stk.size() > 0)
                                {
                                    auto top = stk.top();
                                    stk.pop();
                                    uint64_t new_level = NonterminalFunctions::get_level(top.nonterminal, explicit_nonterminal_level_list);
                                    assert(new_level >= h);
                                    for (uint64_t j = i; j < k; j++)
                                    {
                                        level_to_occurrences_map[new_level].push_back(TemporaryOccurrence(top.nonterminal, top.position + h_temp_occurrences[j].position));
                                    }
                                }
                            }
                            else
                            {
                                for (uint64_t j = i; j < k; j++)
                                {
                                    output.push_back(h_temp_occurrences[j].position + p);
                                }
                            }
                            i = k;
                        }
                        h_temp_occurrences.clear();
                    }
                }
                return output;
            }
            /**
             * @brief Enumerates occurrences level-by-level using an implicit occurrence tree for lower memory use.
             * @param input Seed temporary occurrences to start enumeration from.
             * @param fastParentDictionary Parent dictionary of the grammar.
             * @param rlslp_dictionary Layered RLSLP dictionary providing rule and length tables.
             * @param tree_max_level Maximum level in the derivation forest.
             * @param occCacheList Optional per-base-nonterminal occurrence cache; nullptr disables the cache.
             * @return All absolute occurrence positions.
             */
            static std::vector<uint64_t> faster_get_all_occurrences_using_low_memory(const std::vector<TemporaryOccurrence> &input, const FastParentDictionary &fastParentDictionary, const DictionaryForLayeredRLSLP &rlslp_dictionary, uint64_t tree_max_level, const std::vector<TemporaryOccurrence> *occCacheList)
            {
                const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list = rlslp_dictionary.get_explicit_nonterminal_rule_list();
                const std::vector<uint64_t> &explicit_nonterminal_length_list = rlslp_dictionary.get_explicit_nonterminal_length_list();
                const std::vector<uint16_t> &explicit_nonterminal_level_list = rlslp_dictionary.get_explicit_nonterminal_level_list();

                std::vector<TemporaryOccurrence> copy_input(input);
                std::vector<OccurrenceNode> occurrence_nodes;
                std::vector<std::pair<uint64_t, uint64_t>> children_range_list;
                std::vector<std::vector<OccurrencePointer>> level_to_occurrences_map;
                level_to_occurrences_map.resize(tree_max_level + 1);
                // std::vector<uint64_t> output;
                VStack<TemporaryOccurrence> stk;
                std::vector<OccurrencePointer> h_temp_occurrences;
                std::vector<std::pair<uint64_t, uint64_t>> temp_vector_for_root;

                for (uint64_t i = 0; i < input.size(); i++)
                {
                    uint64_t level = NonterminalFunctions::get_level(input[i].nonterminal, explicit_nonterminal_level_list);
                    level_to_occurrences_map[level].push_back(OccurrencePointer(input[i].nonterminal, UINT64_MAX, input[i].position));
                }

                uint64_t h = 0;
                uint64_t root_idx = UINT64_MAX;
                while (h <= tree_max_level)
                {
                    if (level_to_occurrences_map[h].size() == 0)
                    {
                        h++;
                    }
                    else
                    {
                        h_temp_occurrences.swap(level_to_occurrences_map[h]);
                        std::sort(h_temp_occurrences.begin(), h_temp_occurrences.end(), [](const OccurrencePointer &a, const OccurrencePointer &b)
                                  { return a.nonterminal < b.nonterminal; });

                        uint64_t i = 0;
                        while (i < h_temp_occurrences.size())
                        {
                            NonterminalWithRelativeLevel sig = h_temp_occurrences[i].nonterminal;
                            int64_t p = find_type_2_secondary_occurrences_of_nonterminal(sig, fastParentDictionary, explicit_nonterminal_rule_list, explicit_nonterminal_length_list, occCacheList, stk);
                            uint64_t k = i;
                            uint64_t x = occurrence_nodes.size();
                            uint64_t children_start_index = children_range_list.size();
                            uint64_t occ = 0;
                            while (k < h_temp_occurrences.size() && h_temp_occurrences[k].nonterminal == sig)
                            {
                                uint64_t pointer = h_temp_occurrences[k].pointer;
                                uint64_t positionOffset = h_temp_occurrences[k].positionOffset;

                                if (pointer == UINT64_MAX)
                                {
                                    occ++;
                                }
                                else
                                {
                                    occ += occurrence_nodes[pointer].occ;
                                }

                                if (pointer != UINT64_MAX && occurrence_nodes[pointer].children_start_index == occurrence_nodes[pointer].children_end_index)
                                {
                                    uint64_t new_pointer = children_range_list[occurrence_nodes[pointer].children_start_index].first;
                                    uint64_t new_positionOffset = children_range_list[occurrence_nodes[pointer].children_start_index].second;
                                    children_range_list.push_back({new_pointer, new_positionOffset + positionOffset});
                                }
                                else
                                {
                                    children_range_list.push_back({pointer, positionOffset});
                                }
                                k++;
                            }

                            uint64_t children_end_index = children_range_list.size() - 1;
                            occurrence_nodes.push_back(OccurrenceNode(sig, occ, children_start_index, children_end_index));
                            if (p == -1)
                            {
                                while (stk.size() > 0)
                                {
                                    auto top = stk.top();
                                    stk.pop();
                                    uint64_t new_level = NonterminalFunctions::get_level(top.nonterminal, explicit_nonterminal_level_list);
                                    level_to_occurrences_map[new_level].push_back(OccurrencePointer(top.nonterminal, x, top.position));
                                    assert(new_level >= h);
                                }
                            }
                            else
                            {
                                temp_vector_for_root.push_back({x, p});
                            }
                            i = k;
                        }
                        h_temp_occurrences.clear();
                    }
                }

                uint64_t root_children_start_index = children_range_list.size();
                uint64_t root_occ = 0;
                for (uint64_t i = 0; i < temp_vector_for_root.size(); i++)
                {
                    if (temp_vector_for_root[i].first == UINT64_MAX)
                    {
                        root_occ++;
                    }
                    else
                    {
                        root_occ += occurrence_nodes[temp_vector_for_root[i].first].occ;
                    }
                    children_range_list.push_back(temp_vector_for_root[i]);
                }
                uint64_t root_children_end_index = children_range_list.size() - 1;
                occurrence_nodes.push_back(OccurrenceNode(UINT64_MAX, root_occ, root_children_start_index, root_children_end_index));
                root_idx = occurrence_nodes.size() - 1;
                std::vector<uint64_t> output;
                output.resize(root_occ, UINT64_MAX);

                OccurrenceNode::get_all_occurrences_recursive(occurrence_nodes, children_range_list, root_idx, 0, 0, output);
                return output;
            }
        };

    
}
