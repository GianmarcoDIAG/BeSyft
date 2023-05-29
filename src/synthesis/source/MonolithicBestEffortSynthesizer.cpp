/*
* This file defines the class MonolithicBestEffortSynthesizer
* which implements the monolithic approach to best-effort synthesis
*/

#include "MonolithicBestEffortSynthesizer.h"
#include <boost/algorithm/string.hpp>
#include <queue>

namespace Syft {

    MonolithicBestEffortSynthesizer::MonolithicBestEffortSynthesizer(
                                                std::shared_ptr<VarMgr> var_mgr,
                                                std::string agent_specification,
                                                std::string environment_specification,
                                                InputOutputPartition partition,
                                                Player starting_player) :   var_mgr_(var_mgr),
                                                                            agent_specification_(agent_specification),
                                                                            environment_specification_(environment_specification),
                                                                            partition_(partition),
                                                                            starting_player_(starting_player)
    {
        // step 1. Convert LTLf formulas {E -> Phi, !E, E /\ Phi} to symbolic DFAs
        // constructs LTLf formulas
        Stopwatch ltlf2dfa;
        ltlf2dfa.start();

        std::string adversarial_formula = 
            "(" + environment_specification_ + ") -> (" + agent_specification_ + ")";
        std::string negated_environment_formula = 
            "!(" + environment_specification_ + ")";
        std::string co_operative_formula = 
            "(" + agent_specification_ + ") && (" + environment_specification_ + ")";

        // transforms LTLf formulas into explicit-state DFAs
        ExplicitStateDfaMona adversarial_formula_dfa = 
            ExplicitStateDfaMona::dfa_of_formula(adversarial_formula); 
        ExplicitStateDfaMona negated_environment_formula_dfa = 
            ExplicitStateDfaMona::dfa_of_formula(negated_environment_formula);
        ExplicitStateDfaMona co_operative_formula_dfa = 
            ExplicitStateDfaMona::dfa_of_formula(co_operative_formula);

        double t_ltlf2dfa = ltlf2dfa.stop().count() / 1000.0;
        running_times_.push_back(t_ltlf2dfa);
        std::cout << "[BeSyft] MONA DFA construction DONE in: " << t_ltlf2dfa << " s" << std::endl;

        Syft::Stopwatch dfa2sym;
        dfa2sym.start();

        formula parsed_adversarial_formula = 
            parse_formula(adversarial_formula.c_str()); // parses (E -> phi)

        // Extract propositions from formula and partition
        var_mgr_->create_named_variables(get_props(parsed_adversarial_formula)); // (E -> phi) includes all problem variables
        var_mgr_->partition_variables(partition_.input_variables,
                                        partition_.output_variables);

        // Get explicit-state DFA from MONA DFA
        ExplicitStateDfa explicit_adversarial_dfa = 
            ExplicitStateDfa::from_dfa_mona(var_mgr_, adversarial_formula_dfa);
        ExplicitStateDfa explicit_negated_environment_dfa = 
            ExplicitStateDfa::from_dfa_mona(var_mgr_, negated_environment_formula_dfa);
        ExplicitStateDfa explicit_co_operative_dfa = 
            ExplicitStateDfa::from_dfa_mona(var_mgr_, co_operative_formula_dfa);

        // Get symbolic-state DFA
        SymbolicStateDfa symbolic_adversarial_dfa = 
            SymbolicStateDfa::from_explicit(std::move(explicit_adversarial_dfa));
        SymbolicStateDfa symbolic_negated_environment_dfa = 
            SymbolicStateDfa::from_explicit(std::move(explicit_negated_environment_dfa));
        SymbolicStateDfa symbolic_co_operative_dfa = 
            SymbolicStateDfa::from_explicit(std::move(explicit_co_operative_dfa));

        // stores symbolic-state DFAs
        symbolic_dfas_.push_back(symbolic_adversarial_dfa);         // f_{E -> Phi} stored in symbolic_dfas_[0].final_states() 
        symbolic_dfas_.push_back(symbolic_negated_environment_dfa); // f_{!E} stored in symbolic_dfas_[1].final_states() 
        symbolic_dfas_.push_back(symbolic_co_operative_dfa);        // f_{E /\ Phi} stored in symbolic_dfas_[2].final_states() 

        // step 2. Construct symbolic arena
        SymbolicStateDfa arena = 
            SymbolicStateDfa::product(symbolic_dfas_);
        arena_.push_back(arena);

        double t_dfa2sym = dfa2sym.stop().count() / 1000.0;
        std::cout << "[BeSyft] Symbolic DFA construction DONE in " << t_dfa2sym << std::endl;
        running_times_.push_back(t_dfa2sym);
    }

    std::pair<SynthesisResult, SynthesisResult> MonolithicBestEffortSynthesizer::run() {

        std::pair<SynthesisResult, SynthesisResult> best_effort_result;

        CUDD::BDD adv_goal = symbolic_dfas_[0].final_states();
        CUDD::BDD neg_goal = symbolic_dfas_[1].final_states();
        CUDD::BDD coop_goal = symbolic_dfas_[2].final_states();

        // Step 3. Compute a winning strategy in the adversarial game
        Stopwatch advGame;
        advGame.start();
        std::cout << "[BeSyft] Constructing and solving adversarial game...";
        ReachabilitySynthesizer adv_synthesizer(arena_[0],
                                                starting_player_,
                                                Player::Agent,
                                                adv_goal, // Lifting
                                                var_mgr_->cudd_mgr()->bddOne());
        best_effort_result.first = adv_synthesizer.run();
        double t_advGame = advGame.stop().count() / 1000.0;
        running_times_.push_back(t_advGame);
        std::cout << "DONE in " << t_advGame << " s" << std::endl;

        // Step 4. Compute environment's winning region in negation of environment game
        Stopwatch coopGame;
        coopGame.start();
        std::cout << "[BeSyft] Constructing and solving cooperative game...";
        ReachabilitySynthesizer neg_env_synthesizer(arena_[0],
                                                    starting_player_,
                                                    Player::Agent,  // gets env winning region from agent's
                                                    neg_goal, // Lifting
                                                    var_mgr_->cudd_mgr()->bddOne());
        SynthesisResult env_result = neg_env_synthesizer.run();
        CUDD::BDD non_environment_winning_region = env_result.winning_states;

        // Step 5. Restrict arena to environemt winning region.
        // i.e. all states that are in non_environment_winning_region have to be pruned as invalid
        arena_.push_back(arena_[0].restriction(non_environment_winning_region));

        // Step 6. Compute a cooperatively winning strategy in restricted game
        CoOperativeReachabilitySynthesizer coop_synthesizer(arena_[1],
                                                            starting_player_,
                                                            Player::Agent,
                                                            coop_goal, // Lifting
                                                            var_mgr_->cudd_mgr()->bddOne()); 
        best_effort_result.second = coop_synthesizer.run();
        double t_coopGame = coopGame.stop().count() / 1000.0;
        running_times_.push_back(t_coopGame);
        std::cout << "DONE in " << t_coopGame << " s" << std::endl; 

        return best_effort_result;
    }

    void MonolithicBestEffortSynthesizer::merge_and_dump_dot(const SynthesisResult& adversarial_result, const SynthesisResult& cooperative_result, const std::string& filename) const {
        
        Syft::Stopwatch merge;
        merge.start();
        std::cout << "[BeSyft] Merging strategies...";

        std::vector<std::string> output_labels = var_mgr_->output_variable_labels(); // i.e. Y variables

        std::size_t output_count = cooperative_result.transducer.get()->output_function_.size();
        std::vector<CUDD::ADD> output_vector(output_count);

        // Cooperatively only winning states, i.e. states in cooperatively, but not reactively, winning region
        CUDD::BDD cooperative_only_winning_states = (!adversarial_result.winning_states) * cooperative_result.winning_states;
        for(std::size_t i=0; i < output_count; ++i) {
            std::string label = output_labels[i];
            int index = var_mgr_->name_to_variable(label).NodeReadIndex();
            // i. For winning states use adversarial output function
            CUDD::BDD restricted_adversarial_bdd = 
                adversarial_result.transducer.get()->output_function_.at(index) * adversarial_result.winning_states; 
            // ii. For cooperatively only winning states use cooperative output function
            CUDD::BDD restricted_cooperative_bdd = 
                cooperative_result.transducer.get()->output_function_.at(index) * cooperative_only_winning_states; 
            /// iii. For any state keep best-effort output
            CUDD::BDD merged_bdd = restricted_adversarial_bdd + restricted_cooperative_bdd;
            output_vector[i] = merged_bdd.Add();
        }
        var_mgr_->dump_dot(output_vector, output_labels, filename);

        double t_merge = merge.stop().count() / 1000.0;
        std::cout << "DONE in " <<  t_merge << " s" << std::endl;
    }


std::vector<double> MonolithicBestEffortSynthesizer::get_running_times() const {
    return running_times_;
}

}

