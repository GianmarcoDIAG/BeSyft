/*
* This file defines the class SymbolicCompositionalBestEffortSynthesizer
* which implements the symbolic-compositional approach to best-effort synthesis
*/

#include "SymbolicCompositionalBestEffortSynthesizer.h"
#include <boost/algorithm/string.hpp>
#include <queue>

namespace Syft
{

    SymbolicCompositionalBestEffortSynthesizer::SymbolicCompositionalBestEffortSynthesizer(std::shared_ptr<VarMgr> var_mgr,
                                                 std::string agent_specification,
                                                 std::string environment_specification,
                                                 InputOutputPartition partition,
                                                 Player starting_player)    :   var_mgr_(var_mgr),
                                                                                agent_specification_(agent_specification),
                                                                                environment_specification_(environment_specification),
                                                                                partition_(partition),
                                                                                starting_player_(starting_player)
    {
        // 1. Step 1. Construct symbolic DFAs formulas {E -> phi, !E, E && phi}
        // Build MONA DFAs for agent and environment specifications
        Syft::Stopwatch ltlf2dfa;
        ltlf2dfa.start();

        ExplicitStateDfaMona agent_spec_dfa =
            ExplicitStateDfaMona::dfa_of_formula(agent_specification); // DFA A_{phi}
        ExplicitStateDfaMona environment_spec_dfa =
            ExplicitStateDfaMona::dfa_of_formula(environment_specification); // DFA A_{E}
        ExplicitStateDfaMona tautology_dfa =
            ExplicitStateDfaMona::dfa_of_formula("tt"); // DFA A_{tt}. Accepts non-empty traces only

        // DFA A_{phi}
        // std::cout << std::endl;
        // std::cout << "Agent goal DFA\n";
        // agent_spec_dfa.dfa_print();
        // std::cout << std::endl;

        // DFA A_{E}
        // std::cout << std::endl;
        // std::cout << "Environment Specification DFA\n";
        // environment_spec_dfa.dfa_print();
        // std::cout << std::endl;

        // tautoloty DFA
        // std::cout << std::endl;
        // std::cout << "Tautology DFA\n";
        // tautology_dfa.dfa_print();
        // std::cout << std::endl;

        double t_ltlf2dfa = ltlf2dfa.stop().count() / 1000.0;
        running_times_.push_back(t_ltlf2dfa);
        std::cout << "[BeSyft] MONA DFA construction DONE in: " << t_ltlf2dfa << " s" << std::endl;

        // Obtain parsed formulas (requirement to construct symbolic DFAs)
        Syft::Stopwatch dfa2sym;
        dfa2sym.start();

        std::string adversarial_formula = 
            "(" + environment_specification + ") -> (" + agent_specification +")"; 

        formula parsed_adversarial_formula = 
            parse_formula(adversarial_formula.c_str()); // parses (E -> phi)

        // Extract propositions from formula and partition
        var_mgr_->create_named_variables(get_props(parsed_adversarial_formula)); // (E -> phi) includes all problem variables
        var_mgr_->partition_variables(partition_.input_variables,
                                        partition_.output_variables);

        // Get explicit state DFA from MONA DFA
        ExplicitStateDfa explicit_agent_dfa =
            ExplicitStateDfa::from_dfa_mona(var_mgr_, agent_spec_dfa);
        ExplicitStateDfa explicit_env_dfa =
            ExplicitStateDfa::from_dfa_mona(var_mgr_, environment_spec_dfa); 
        ExplicitStateDfa explicit_tau_dfa =
            ExplicitStateDfa::from_dfa_mona(var_mgr_, tautology_dfa);

        // Get Symbolic State DFA from Explicit DFA
        symbolic_dfas_.push_back(SymbolicStateDfa::from_explicit(std::move(explicit_agent_dfa)));
        symbolic_dfas_.push_back(SymbolicStateDfa::from_explicit(std::move(explicit_env_dfa)));
        symbolic_dfas_.push_back(SymbolicStateDfa::from_explicit(std::move(explicit_tau_dfa)));
        
        // f_{phi} is stored in symbolic_dfas_[0].final_states()
        // f_{E} is stored in symbolic_dfas_[1].final_states()


        // Step 2. Construct symbolic arena for best-effort synthesis through product
        SymbolicStateDfa arena = 
            SymbolicStateDfa::product(symbolic_dfas_);
        arena_.push_back(arena);

        double t_dfa2sym = dfa2sym.stop().count() / 1000.0;
        running_times_.push_back(t_dfa2sym);
        std::cout << "[BeSyft] Symbolic DFA construction DONE in " << t_dfa2sym << " s" << std::endl;
    }

    std::pair<SynthesisResult, SynthesisResult> SymbolicCompositionalBestEffortSynthesizer::run() {

        std::pair<SynthesisResult, SynthesisResult> best_effort_result;

        CUDD::BDD adv_goal = ((!symbolic_dfas_[1].final_states()) + symbolic_dfas_[0].final_states()) * (!arena_[0].initial_state_bdd()); // f_{E} -> f_{Phi}
        // CUDD::BDD adv_goal = (!(symbolic_dfas_[1].final_states() * (!symbolic_dfas_[0].final_states()))) * (!arena_[0].initial_state_bdd());
        CUDD::BDD neg_goal = ((!symbolic_dfas_[1].final_states()) * (!arena_[0].initial_state_bdd())); // ! f_{E}
        CUDD::BDD coop_goal = (symbolic_dfas_[1].final_states()) * (symbolic_dfas_[0].final_states()) * (!arena_[0].initial_state_bdd()); // F{E} /\ f_{Phi}

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

    void SymbolicCompositionalBestEffortSynthesizer::merge_and_dump_dot(const SynthesisResult& adversarial_result, const SynthesisResult& cooperative_result, const string& filename) const {
        
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

    std::vector<double> SymbolicCompositionalBestEffortSynthesizer::get_running_times() const {
        return running_times_;
    }
}
