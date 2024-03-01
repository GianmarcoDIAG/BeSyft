// This header defines the interface of
// CoOperativeDfaGameSynthesizer type
// Used to perfrom co operative synthesis

#ifndef CO_OP_DFA_GAME_SYNTHESIZER_H
#define CO_OP_DFA_GAME_SYNTHESIZER_H

#include"Quantification.h"
#include"SymbolicStateDfa.h"
#include"Synthesizer.h"
#include"Transducer.h"

namespace Syft {

    class CoOperativeDfaGameSynthesizer : public Synthesizer<SymbolicStateDfa> {

        protected:
            std::shared_ptr<VarMgr> var_mgr_;
            Player starting_player_;
            Player protagonist_player_;
            std::vector<int> initial_vector_;
            std::vector<CUDD::BDD> transition_vector_;
            std::unique_ptr<Quantification> quantify_independent_variables_;
            std::unique_ptr<Quantification> quantify_non_state_variables_;

            CUDD::BDD preimage(const CUDD::BDD &winning_states) const;  // Used to compute function t in symbolic synthesis

            CUDD::BDD project_into_states(const CUDD::BDD &winning_moves) const;    // Used to compute function w in symbolic synthesis

            std::unordered_map<int, CUDD::BDD> synthesize_strategy(const CUDD::BDD &winning_moves) const;

            bool includes_initial_state(const CUDD::BDD &winning_states) const;

            public:
                CoOperativeDfaGameSynthesizer(SymbolicStateDfa spec, Player starting_player, Player protagonist_player);

                virtual SynthesisResult run() override = 0;
    };

} 
#endif // CO_OP_DFA_GANE_SYNTHESIZER_H