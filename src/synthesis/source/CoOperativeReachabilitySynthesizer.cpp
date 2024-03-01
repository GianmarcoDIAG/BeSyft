#include "CoOperativeReachabilitySynthesizer.h"
#include <cassert>

namespace Syft {

CoOperativeReachabilitySynthesizer::CoOperativeReachabilitySynthesizer(SymbolicStateDfa spec,
						Player starting_player, Player protagonist_player,
						 CUDD::BDD goal_states,
						 CUDD::BDD state_space)
    : CoOperativeDfaGameSynthesizer(spec, starting_player, protagonist_player)
    , goal_states_(goal_states), state_space_(state_space)
{}


SynthesisResult CoOperativeReachabilitySynthesizer::run() {
  SynthesisResult result;
  CUDD::BDD winning_states = state_space_ & goal_states_;
  CUDD::BDD winning_moves = winning_states;

  while (true) {
    CUDD::BDD new_winning_moves = winning_moves |
                                  (state_space_ & (!winning_states) & preimage(winning_states));

    CUDD::BDD new_winning_states = project_into_states(new_winning_moves);

    if (includes_initial_state(new_winning_states)) {
        result.realizability = true;
        result.winning_states = new_winning_states;
        std::unordered_map<int, CUDD::BDD> strategy = synthesize_strategy(
              new_winning_moves);
        result.transducer = std::make_unique<Transducer>(
              var_mgr_, initial_vector_, strategy, spec_.transition_function(),
              starting_player_, protagonist_player_);
        winning_states_ = new_winning_states;
        winning_moves_ = new_winning_moves;
        return result;

    } else if (new_winning_states == winning_states) {
        result.realizability = false;
        result.winning_states = new_winning_states;
        // result.transducer = nullptr;
        std::unordered_map<int, CUDD::BDD> strategy = synthesize_strategy(
          new_winning_moves);
        result.transducer = std::make_unique<Transducer>(
              var_mgr_, initial_vector_, strategy, spec_.transition_function(),
              starting_player_, protagonist_player_);
        winning_states_ = new_winning_states;
        winning_moves_ = new_winning_moves;
        return result;
    }
    winning_moves = new_winning_moves;
    winning_states = new_winning_states;
  }

}

CUDD::BDD CoOperativeReachabilitySynthesizer::get_winning_states() const {
      return winning_states_;
 }

 CUDD::BDD CoOperativeReachabilitySynthesizer::get_winning_moves() const {
      return winning_moves_;
 }

}
