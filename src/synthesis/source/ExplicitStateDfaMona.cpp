//
// Created by shufang on 7/2/21.
//

#include "ExplicitStateDfaMona.h"

#include "spotparser.h"
#include <iostream>
#include <istream>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <lydia/mona_ext/mona_ext_base.hpp>
#include <lydia/dfa/mona_dfa.hpp>
#include <lydia/parser/ldlf/driver.cpp>
#include <lydia/parser/ltlf/driver.cpp>
#include <lydia/to_dfa/core.hpp>
#include <lydia/to_dfa/strategies/compositional/base.hpp>
#include <lydia/utils/print.hpp>

namespace Syft
{

    void ExplicitStateDfaMona::dfa_print()
    {
        std::cout << "Number of states " +
                         std::to_string(get_nb_states())
                  << "\n";

        std::cout << "Computed automaton: ";
        whitemech::lydia::dfaPrint(get_dfa(),
                                   get_nb_variables(),
                                   names, indices.data());
    }

    // ExplicitStateDfaMona& ExplicitStateDfaMona::operator=(const ExplicitStateDfaMona& other) {
    //     if(this == &other) {
    //         return *this;
    //     }
    //     if(this->dfa_ != 0) {
    //         std::cout << "Free here\n";
    //         dfaFree(this->dfa_);
    //     }
    //     this->dfa_ = dfaCopy(other.dfa_);
    //     this->names = other.names;
    //     this->indices = other.indices;
    //     return *this;
    // }
    // the input d is a bad prefix DFA and agent_winning is the set of winning states that should be kept
    ExplicitStateDfaMona ExplicitStateDfaMona::prune_dfa_with_states(ExplicitStateDfaMona &d, std::vector<size_t> agent_winning)
    {
        std::cout << "here ....\n";
        int d_ns = d.get_nb_states();
        int new_ns = agent_winning.size();
        int n = d.get_nb_variables();
        int new_len = d.names.size();

        std::cout << "here ...." << d_ns << ", " << new_ns << ", " << n << ", " << (n == new_len) << "\n";

        bool safe_states[d_ns];
        int state_map[d_ns];
        memset(safe_states, false, sizeof(safe_states));
        memset(state_map, -1, sizeof(state_map));

        for (auto s : agent_winning)
        {
            safe_states[s] = true;
        }

        int index = 0;
        for (int i = 0; i < d_ns; i++)
        {
            if (!safe_states[i])
                continue;
            state_map[i] = index++;
        }

        DFA *a = d.dfa_;
        DFA *result;
        paths state_paths, pp;
        std::string statuses;

        int indices[new_len];
        for (int i = 0; i < d.indices.size(); i++)
        {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns + 1, new_len, indices);

        for (int i = 0; i < a->ns; i++)
        {
            // ignore non-safe_states
            if (!safe_states[i])
                continue;
            int next_state;
            std::string next_guard;

            auto transitions = std::vector<std::pair<int, std::string>>();
            state_paths = pp = make_paths(a->bddm, a->q[i]);
            while (pp)
            {
                auto guard = whitemech::lydia::get_path_guard(n, pp->trace);
                // ignore non safe_states
                if (safe_states[pp->to])
                {
                    transitions.emplace_back(pp->to, guard);
                }
                pp = pp->next;
            }

            statuses += "-";
            // transitions
            int nb_transitions = transitions.size();
            dfaAllocExceptions(nb_transitions);
            for (const auto &p : transitions)
            {
                std::tie(next_state, next_guard) = p;
                dfaStoreException(state_map[next_state], next_guard.data());
            }
            dfaStoreState(new_ns);
            kill_paths(state_paths);
        }

        statuses += "+";
        dfaAllocExceptions(0);
        dfaStoreState(new_ns);

        DFA *tmp = dfaBuild(statuses.data());

        std::cout << "Pruned DFA:\n"
                  << statuses << "\n";
        ExplicitStateDfaMona res1(tmp, d.names);
        res1.dfa_print();
        result = dfaMinimize(tmp);
        //dfaFree(tmp);

        ExplicitStateDfaMona res(result, d.names);
        return res;
    }

    ExplicitStateDfaMona ExplicitStateDfaMona::dfa_of_formula(const std::string &formula)
    {
        whitemech::lydia::Logger logger("main");
        whitemech::lydia::Logger::level(whitemech::lydia::LogLevel::info);

        std::shared_ptr<whitemech::lydia::AbstractDriver> driver;
        driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
        std::stringstream formula_stream(formula);
        logger.info("Parsing {}", formula);
        driver->parse(formula_stream);
        auto parsed_formula = driver->get_result();

        logger.info("Apply no-empty semantics.");
        auto context = driver->context;
        auto end = context->makeLdlfEnd();
        auto not_end = context->makeLdlfNot(end);
        parsed_formula = context->makeLdlfAnd({parsed_formula, not_end});

        auto dfa_strategy = whitemech::lydia::CompositionalStrategy();
        auto translator = whitemech::lydia::Translator(dfa_strategy);

        auto t_start = std::chrono::high_resolution_clock::now();

        logger.info("Transforming to DFA...");
        auto t_dfa_start = std::chrono::high_resolution_clock::now();

        auto my_dfa = translator.to_dfa(*parsed_formula);

        auto my_mona_dfa =
            std::dynamic_pointer_cast<whitemech::lydia::mona_dfa>(my_dfa);

        DFA *d = dfaCopy(my_mona_dfa->dfa_);

        ExplicitStateDfaMona exp_dfa(d, my_mona_dfa->names);

        // std::cout << "Number of states " +
        //                  std::to_string(exp_dfa.get_nb_variables())
        //           << "\n";

        return exp_dfa;
    }

    // all the names may not be the same, needs a map for right indices
    ExplicitStateDfaMona ExplicitStateDfaMona::dfa_product(const std::vector<ExplicitStateDfaMona> &dfa_vector)
    {
        // first record all variables, as they may not have the same alphabet
        std::unordered_map<std::string, int> name_to_index = {};
        std::vector<std::string> name_vector;
        std::vector<DFA *> renamed_dfa_vector;

        for (auto dfa : dfa_vector)
        {
            // for each DFA, record its names and assign with global indices
            int map[dfa.names.size()];
            for (int i = 0; i < dfa.names.size(); i++)
            {
                int local_index;

                std::unordered_map<std::string, int>::const_iterator
                    got = name_to_index.find(dfa.names[i]);
                if (got != name_to_index.end())
                {
                    // found this proposition before
                    local_index = name_to_index[dfa.names[i]];
                }
                else
                {
                    // not found
                    local_index = name_to_index.size();
                    name_to_index[dfa.names[i]] = local_index;
                    name_vector.push_back(dfa.names[i]);
                }

                map[i] = local_index;
            }

            // replace indices
            DFA *copy = dfaCopy(dfa.dfa_);
            dfaReplaceIndices(copy, map);
            renamed_dfa_vector.push_back(copy);
        }

        auto cmp = [](const DFA *d1, const DFA *d2)
        { return d1->ns > d2->ns; };
        std::priority_queue<DFA *, std::vector<DFA *>, decltype(cmp)>
            queue(renamed_dfa_vector.begin(), renamed_dfa_vector.end(), cmp);
        while (queue.size() > 1)
        {
            DFA *lhs = queue.top();
            queue.pop();
            DFA *rhs = queue.top();
            queue.pop();
            DFA *tmp = dfaProduct(lhs, rhs, dfaProductType::dfaAND);
            dfaFree(lhs);
            dfaFree(rhs);
            DFA *res = dfaMinimize(tmp);
            dfaFree(tmp);
            queue.push(res);
        }

        ExplicitStateDfaMona res_dfa(queue.top(), name_vector);
        return res_dfa;
    }

    ExplicitStateDfaMona ExplicitStateDfaMona::dfa_negation(const ExplicitStateDfaMona &d)
    {
        DFA *d_copy = dfaCopy(d.dfa_);
        dfaNegation(d_copy);
        ExplicitStateDfaMona res_dfa(d_copy, d.names);
        return res_dfa;
    }

    ExplicitStateDfaMona ExplicitStateDfaMona::dfa_minimize(const ExplicitStateDfaMona &d)
    {
        //logger.info("Determinizing DFA...");
        DFA *res = dfaMinimize(d.dfa_);
        ExplicitStateDfaMona res_dfa(res, d.names);
        std::cout << "Number of states in the minimized DFA" +
                         std::to_string(res_dfa.get_nb_variables())
                  << "\n";
        return res_dfa;
    }

}
