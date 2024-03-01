//
// Created by shufang on 7/2/21.
//

#ifndef EXPLICITSTATEDFAMONA_H
#define EXPLICITSTATEDFAMONA_H

extern "C" {
#include <mona/bdd.h>
#include <mona/dfa.h>
#include <mona/mem.h>
}

#include "lydia/dfa/mona_dfa.hpp"

namespace Syft {

/*
 * Wrapper to Lydia DFA.
 */
    class ExplicitStateDfaMona : public whitemech::lydia::mona_dfa {
    public:

        ExplicitStateDfaMona(DFA* dfa, int nb_variables)
        : whitemech::lydia::mona_dfa(dfa, nb_variables) {
        }

        ExplicitStateDfaMona(DFA* dfa, const std::vector<std::string>& names)
                : whitemech::lydia::mona_dfa(dfa, names) {
        }

        ExplicitStateDfaMona(const ExplicitStateDfaMona& other)
        : whitemech::lydia::mona_dfa(dfaCopy(other.dfa_), other.names) {

        }

        ExplicitStateDfaMona& operator=(ExplicitStateDfaMona other)
        {
            // std::cout << "copy assignment of ExplicitStateDfaMona\n";
            this->names = other.names;
            this->dfa_ = dfaCopy(other.get_dfa());
            return *this;
        }
        // ExplicitStateDfaMona& operator=(const ExplicitStateDfaMona& other);

        ~ExplicitStateDfaMona(){
            
        }

        // rewrite this function, since nb_vairiables_ in mona_dfa is private
        int get_nb_variables() {
            return this->names.size();
        }

        /**
         * \brief Print the DFA.
         */
        void dfa_print();


        /**
         * \brief Prune a DFA with given set of states.
         *
         * Basically remove a set of states from the DFA, and return a minimized one.
         *
         * \param d The DFA to be pruned.
         * \param states The set of states to be removed.
         * \return The pruned DFA.
         */
        static ExplicitStateDfaMona prune_dfa_with_states(ExplicitStateDfaMona& d, std::vector<size_t> states);

        /**
         * \brief Take the product of a vector of DFAs.
         *
         * \param dfa_vector The DFAs to be processed.
         * \return The product DFA.
         */
        static ExplicitStateDfaMona dfa_product(const std::vector<ExplicitStateDfaMona>& dfa_vector);

        static ExplicitStateDfaMona dfa_negation(const ExplicitStateDfaMona &d);

        /**
         * \brief Minimize a given DFA.
         *
         * \param d The DFA to be minimized.
         * \return The minimal DFA.
         */

        static ExplicitStateDfaMona dfa_minimize(const ExplicitStateDfaMona& d);


        /**
         * \brief Prune a DFA with given set of states and set of transitions.
         *
         * Basically remove a set of states and set of transitions from the DFA, and return a minimized one.
         *
         * \param d The DFA to be pruned.
         * \param states The set of states to be removed.
         * \return The complemented DFA.
         */
        // TODO: How to store the maximal winning strategy?)
        ExplicitStateDfaMona prune_dfa_with_strategy(ExplicitStateDfaMona& d, std::vector<int> states, std::vector<int> transitions);


        /**
         * \brief Construct DFA from a given formula
         *
         *
         * \param formula An LTLf formula.
         * \return The corresponding explicit-state DFA.
         */
        static ExplicitStateDfaMona dfa_of_formula(const std::string& formula);




    };

}
#endif //EXPLICITSTATEDFAMONA_H
