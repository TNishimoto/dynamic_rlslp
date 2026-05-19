#pragma once
#include "../rules/all.hpp"
#include "./run_rule_vector.hpp"
#include "../../local_parsings/locally_consistent_parsing.hpp"
#include "./level_sequence_function.hpp"

namespace dynRLSLP
{

        /**
         * @brief Builds common run sequences for locally consistent parsing.
         * @ingroup StaticOperationsClasses
         */
        /**
         * @brief Builds common run sequences for locally consistent parsing.
         * @ingroup StaticOperationsClasses
         */
        class FastCommonSequenceBuilder
        {

        public:
            static inline int left_context_length = -1;
            static inline int right_context_length = -1;

            static void initialize(bool is_restricted_block_compression)
            {
                if (is_restricted_block_compression)
                {
                    FastCommonSequenceBuilder::left_context_length = 1;
                    FastCommonSequenceBuilder::right_context_length = 1;
                }
                else
                {
                    FastCommonSequenceBuilder::left_context_length = LocallyConsistentParsing::get_consistent_L();
                    FastCommonSequenceBuilder::right_context_length = LocallyConsistentParsing::get_consistent_R();
                }
            }

            /**
             * @brief Build a RunRuleVector for a nonterminal substring.
             * @param item Rule body item.
             * @param pos Start position in the represented string.
             * @param len Length of the substring or prefix/suffix.
             * @param dic Layered RLSLP dictionary.
             * @return Computed integer value.
             */
            static RunRuleVector build(NonterminalWithRelativeLevel item, int64_t pos, int64_t len, const DictionaryForLayeredRLSLP &dic)
            {
                int64_t item_len = NonterminalFunctions::get_length(item, dic.get_explicit_nonterminal_length_list());
                if (pos >= item_len)
                {
                    throw std::runtime_error("Error in create_sub_sequence: pos >= item_len");
                }
                if (pos + len > item_len)
                {
                    throw std::runtime_error("Error in create_sub_sequence: pos + len > item_len");
                }

                std::vector<RunRuleBody> seq = LevelSequenceFunction::substring(item, pos, len, dic);

                std::vector<RunRuleBody> seq2 = break_loop(seq, dic.get_explicit_nonterminal_rule_list(), dic.get_explicit_nonterminal_level_list());

                return RunRuleVector(seq2, dic);
            }
            /**
             * @brief Build a RunRuleVector for a nonterminal substring.
             * @param item Rule body item.
             * @param dic Layered RLSLP dictionary.
             * @return Run-rule vector.
             */
            static RunRuleVector build(NonterminalWithRelativeLevel item, const DictionaryForLayeredRLSLP &dic)
            {
                uint64_t len = NonterminalFunctions::get_length(item, dic.get_explicit_nonterminal_length_list());
                return build(item, 0, len, dic);
            }
            static RunRuleVector build(NonterminalWithRelativeLevel item, int64_t pos, const DictionaryForLayeredRLSLP &dic)
            {
                uint64_t len = NonterminalFunctions::get_length(item, dic.get_explicit_nonterminal_length_list());
                return build(item, pos, len - pos, dic);
            }

        private:
            /**
             * @brief Refine a run sequence using consistent parsing context lengths.
             * @param seq Input run sequence.
             * @param explicit_nonterminal_rule_list Base-nonterminal rule list (D).
             * @param explicit_nonterminal_level_list Base-nonterminal level list (H).
             * @return Level or height value.
             */
            static std::vector<RunRuleBody> break_loop(const std::vector<RunRuleBody> &seq, const std::vector<RLSLPRuleBody> &explicit_nonterminal_rule_list, const std::vector<uint16_t> &explicit_nonterminal_level_list)
            {
                std::deque<RunRuleBody> deq;
                for (auto it : seq)
                {
                    deq.push_back(it);
                }
                VStack<RunRuleBody> front_stack;
                VStack<RunRuleBody> back_stack;
                int64_t h = 0;
                while (deq.size() > 0)
                {
                    int64_t currentContextL = 0;

                    while (currentContextL < FastCommonSequenceBuilder::left_context_length && deq.size() > 0)
                    {
                        RunRuleBody fstRun = deq.front();
                        deq.pop_front();
                        int64_t level = NonterminalFunctions::get_level(fstRun.number, explicit_nonterminal_level_list);
                        NonterminalWithRelativeLevel explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(fstRun.number);
                        int64_t bottom_level = NonterminalFunctions::get_level(explicit_nonterminal, explicit_nonterminal_level_list);

                        if (h < bottom_level)
                        {
                            if (fstRun.power > 1)
                            {
                                deq.push_front(RunRuleBody(fstRun.number, fstRun.power - 1));
                            }

                            std::vector<RunRuleBody> tmp;
                            RunRuleBody::y_break(explicit_nonterminal, explicit_nonterminal_rule_list, explicit_nonterminal_level_list, tmp);
                            for (int64_t i = tmp.size() - 1; i >= 0; i--)
                            {
                                assert(NonterminalFunctions::get_level(tmp[i].number, explicit_nonterminal_level_list) == bottom_level - 1);
                                deq.push_front(RunRuleBody(tmp[i].number, tmp[i].power));
                            }
                        }
                        else if (h < level)
                        {
                            int64_t m = std::min((int64_t)fstRun.power, (int64_t)(FastCommonSequenceBuilder::left_context_length - currentContextL));
                            int64_t diff = fstRun.power - m;
                            if (diff > 0)
                            {
                                deq.push_front(RunRuleBody(fstRun.number, diff));
                            }
                            NonterminalWithRelativeLevel new_nonterminal = NonterminalFunctions::get_nonterminal(h - bottom_level, explicit_nonterminal);

                            front_stack.push(RunRuleBody(new_nonterminal, m));
                            currentContextL += m;
                        }
                        else if (h == level)
                        {
                            front_stack.push(fstRun);
                            currentContextL += fstRun.power;
                        }
                        else
                        {
                            std::cout << "fstRun.level: " << level << ", h: " << h << std::endl;
                            throw std::runtime_error("Error in break_loop: fst.level < h");
                        }
                    }
                    while (deq.size() > 0)
                    {
                        RunRuleBody fstRun = deq.front();
                        int64_t level = NonterminalFunctions::get_level(fstRun.number, explicit_nonterminal_level_list);
                        if (level == h)
                        {
                            front_stack.push(fstRun);
                            deq.pop_front();
                        }
                        else if (level > h)
                        {
                            break;
                        }
                        else
                        {
                            throw std::runtime_error("Error in break_loop: fstRun.level < h");
                        }
                    }

                    int64_t currentContextR = 0;

                    if (deq.size() > 0)
                    {
                        while (currentContextR < FastCommonSequenceBuilder::right_context_length && deq.size() > 0)
                        {
                            RunRuleBody lastRun = deq.back();
                            deq.pop_back();
                            int64_t level = NonterminalFunctions::get_level(lastRun.number, explicit_nonterminal_level_list);
                            NonterminalWithRelativeLevel explicit_nonterminal = NonterminalFunctions::get_explicit_nonterminal(lastRun.number);
                            int64_t bottomLevel = NonterminalFunctions::get_level(explicit_nonterminal, explicit_nonterminal_level_list);

                            if (h < bottomLevel)
                            {
                                if (lastRun.power > 1)
                                {
                                    deq.push_back(RunRuleBody(lastRun.number, lastRun.power - 1));
                                }

                                std::vector<RunRuleBody> tmp;
                                RunRuleBody::y_break(explicit_nonterminal, explicit_nonterminal_rule_list, explicit_nonterminal_level_list, tmp);
                                for (auto it : tmp)
                                {
                                    assert(NonterminalFunctions::get_level(it.number, explicit_nonterminal_level_list) == bottomLevel - 1);
                                    deq.push_back(RunRuleBody(it.number, it.power));
                                }
                            }
                            else if (h < level)
                            {
                                int64_t m = std::min((int64_t)lastRun.power, (int64_t)(FastCommonSequenceBuilder::right_context_length - currentContextR));
                                int64_t diff = lastRun.power - m;
                                if (diff > 0)
                                {
                                    deq.push_back(RunRuleBody(lastRun.number, diff));
                                }

                                NonterminalWithRelativeLevel new_nonterminal = NonterminalFunctions::get_nonterminal(h - bottomLevel, explicit_nonterminal);
                                back_stack.push(RunRuleBody(new_nonterminal, m));
                                currentContextR += m;
                            }
                            else if (h == level)
                            {
                                back_stack.push(lastRun);
                                currentContextR += lastRun.power;
                            }
                            else
                            {
                                throw std::runtime_error("Error in break_loop: lastRun.level < h");
                            }
                        }
                    }
                    while (deq.size() > 0)
                    {
                        RunRuleBody lastRun = deq.back();
                        int64_t level = NonterminalFunctions::get_level(lastRun.number, explicit_nonterminal_level_list);
                        if (level == h)
                        {
                            back_stack.push(lastRun);
                            deq.pop_back();
                        }
                        else if (level > h)
                        {
                            break;
                        }
                        else
                        {
                            throw std::runtime_error("Error in break_loop: lastRun.level < h");
                        }
                    }
                    h++;
                }

                std::vector<RunRuleBody> r;
                uint64_t front_size = front_stack.size();
                r.resize(front_stack.size() + back_stack.size());
                int64_t p = front_size - 1;
                int64_t q = front_size;
                while (front_stack.size() > 0 && p >= 0)
                {
                    RunRuleBody current = front_stack.top();
                    front_stack.pop();
                    r[p] = current;
                    p--;
                }
                while (back_stack.size() > 0)
                {
                    RunRuleBody current = back_stack.top();
                    back_stack.pop();
                    r[q] = current;
                    q++;
                }
                return r;
            }
        };
    
}