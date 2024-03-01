/*
* This file defines the class ExplicitCompositionalBestEffortSynthesizer
* which implements the explicit-compositional approach to best-effort synthesis
*/

#include "ExplicitCompositionalBestEffortSynthesizer.h"
#include <boost/algorithm/string.hpp>
#include <queue>

namespace Syft {

    ExplicitCompositionalBestEffortSynthesizer::ExplicitCompositionalBestEffortSynthesizer(
                                                std::shared_ptr<VarMgr> var_mgr,
                                                std::string agent_specification,
                                                std::string environment_specification,
                                                InputOutputPartition partition,
                                                Player starting_player,
                                                bool dominance_check) :   var_mgr_(var_mgr),
                                                                            agent_specification_(agent_specification),
                                                                            environment_specification_(environment_specification),
                                                                            partition_(partition),
                                                                            starting_player_(starting_player),
                                                                            dominance_check_(dominance_check) 
    {
        // step 1. Construct symbolic DFAs of formulas {E -> phi, !E, E /\ phi}
        // Build MONA DFAs for agent and environment specifications

        Stopwatch ltlf2dfa;
        ltlf2dfa.start();

        std::string adversarial_formula = 
            "(" + environment_specification + ") -> (" + agent_specification +")"; 

        ExplicitStateDfaMona agent_spec_dfa =
            ExplicitStateDfaMona::dfa_of_formula(agent_specification); // DFA A_{phi}
        ExplicitStateDfaMona environment_spec_dfa =
            ExplicitStateDfaMona::dfa_of_formula(environment_specification); // DFA A_{E}
        ExplicitStateDfaMona no_empty_dfa = 
            ExplicitStateDfaMona::dfa_of_formula("F(true)"); // DFA A_{tt}, i.e. accepts all non-empty traces

        // constructs DFA A_{E -> Phi}
        // a. Build DFA for implication (E -> phi) as !(E && (!phi))
        std::vector<ExplicitStateDfaMona> implication_dfas;
        implication_dfas.push_back(environment_spec_dfa);
        implication_dfas.push_back(ExplicitStateDfaMona::dfa_negation(agent_spec_dfa));
        implication_dfas.push_back(no_empty_dfa);

        ExplicitStateDfaMona implication = ExplicitStateDfaMona::dfa_negation(ExplicitStateDfaMona::dfa_product(implication_dfas));

        // b. Apply non-empty traces semantics
        std::vector<ExplicitStateDfaMona> adv_dfas;
        adv_dfas.push_back(implication);
        adv_dfas.push_back(no_empty_dfa);

        ExplicitStateDfaMona adversarial_dfa = ExplicitStateDfaMona::dfa_product(adv_dfas);

        // constructs DFA A_{!E}
        std::vector<ExplicitStateDfaMona> neg_dfas;
        neg_dfas.push_back(ExplicitStateDfaMona::dfa_negation(environment_spec_dfa));
        neg_dfas.push_back(no_empty_dfa); // i.e. apply non-empty traces semantics

        ExplicitStateDfaMona negated_env_dfa = ExplicitStateDfaMona::dfa_product(neg_dfas);

        // construts DFA A_{E /\ Phi}
        std::vector<ExplicitStateDfaMona> coop_dfas; 
        coop_dfas.push_back(environment_spec_dfa);
        coop_dfas.push_back(agent_spec_dfa);
        coop_dfas.push_back(no_empty_dfa); // i.e. apply non-empty traces semantics

        ExplicitStateDfaMona cooperative_dfa = ExplicitStateDfaMona::dfa_product(coop_dfas);

        double t_ltlf2dfa = ltlf2dfa.stop().count() / 1000.0;
        running_times_.push_back(t_ltlf2dfa);
        std::cout << "[BeSyft] LTLf-to-DFA construction DONE in: " << t_ltlf2dfa << " s" << std::endl;

        Syft::Stopwatch dfa2sym;
        dfa2sym.start();

        // formula parsed_adversarial_formula = 
            // parse_formula(adversarial_formula.c_str()); // parses (E -> phi)

        // Extract propositions from formula and partition
        // var_mgr_->create_named_variables(get_props(parsed_adversarial_formula)); // (E -> phi) includes all problem variables
        var_mgr_->create_named_variables(agent_spec_dfa.names);
        var_mgr_->create_named_variables(environment_spec_dfa.names);
        var_mgr_->partition_variables(partition_.input_variables,
                                        partition_.output_variables);

        // Get explicit-state DFA from MONA DFA
        ExplicitStateDfa explicit_adversarial_dfa = 
            ExplicitStateDfa::from_dfa_mona(var_mgr_, adversarial_dfa);
        ExplicitStateDfa explicit_negated_environment_dfa = 
            ExplicitStateDfa::from_dfa_mona(var_mgr_, negated_env_dfa);
        ExplicitStateDfa explicit_co_operative_dfa = 
            ExplicitStateDfa::from_dfa_mona(var_mgr_, cooperative_dfa);

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
        running_times_.push_back(t_dfa2sym);
        std::cout << "[BeSyft] Symbolic DFA construction DONE in " << t_dfa2sym << std::endl;

    }

    BestEffortSynthesisResult ExplicitCompositionalBestEffortSynthesizer::run() {

        BestEffortSynthesisResult best_effort_result;

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
        best_effort_result.adversarial = adv_synthesizer.run();
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
        // arena_.push_back(arena_[0].restriction(non_environment_winning_region));

        // Step 6. Compute a cooperatively winning strategy in restricted game
        CoOperativeReachabilitySynthesizer coop_synthesizer(arena_[0],
                                                            starting_player_,
                                                            Player::Agent,
                                                            coop_goal, // Lifting
                                                            !non_environment_winning_region); 
        best_effort_result.cooperative = coop_synthesizer.run();
        double t_coopGame = coopGame.stop().count() / 1000.0;
        running_times_.push_back(t_coopGame);
        std::cout << "DONE in " << t_coopGame << " s" << std::endl; 

        if (dominance_check_) {
        
            Stopwatch domTest;
            domTest.start();
            std::cout << "[BeSyft] Checking existence of a dominant strategy...";

            if (best_effort_result.adversarial.realizability) {
                best_effort_result.dominant = true;
                double domTest_t = domTest.stop().count() / 1000.0;
                std::cout << "succeeds. Synthesized best-effort strategy is winning. DONE in " <<  domTest_t << " s" << std::endl;
                running_times_.push_back(domTest_t);
                return best_effort_result;
            } else if (!best_effort_result.cooperative.realizability) {
                best_effort_result.dominant = true;
                double domTest_t = domTest.stop().count() / 1000.0;
                std::cout << "succeeds. Synthesized best-effort strategy is losing. DONE in " <<  domTest_t << " s" << std::endl;
                running_times_.push_back(domTest_t);
                return best_effort_result;
            }

            // number of state variables
            int non_state_vars = var_mgr_->get_index_to_name().size();

            // initial state vector
            std::vector<int> vars_init(non_state_vars, 0);
            std::vector<int> goal_init = symbolic_dfas_[0].initial_state();
            std::vector<int> env_init = symbolic_dfas_[1].initial_state();
            std::vector<int> tau_init = symbolic_dfas_[2].initial_state();

            std::vector<int> initial_state;
            initial_state.insert(initial_state.end(), vars_init.begin(), vars_init.end());
            initial_state.insert(initial_state.end(), goal_init.begin(), goal_init.end());
            initial_state.insert(initial_state.end(), env_init.begin(), env_init.end());
            initial_state.insert(initial_state.end(), tau_init.begin(), tau_init.end());

            // transition function and regions
            std::vector<CUDD::BDD> transition_function = arena_[0].transition_function();
            CUDD::BDD winning_region = best_effort_result.adversarial.winning_states;
            CUDD::BDD cooperative_region = best_effort_result.cooperative.winning_states;
            CUDD::BDD cooperative_only_region = cooperative_region * (!winning_region);

            // possible environment choices
            std::vector<std::vector<int>> env_moves = get_assignments(var_mgr_->input_variable_count());

            // initially, only reached state is the initial state
            std::unordered_set<std::vector<int>, VectorHash> reached;
            std::queue<std::vector<int>> frontier;
            reached.insert(initial_state);
            frontier.push(initial_state);
            std::unordered_set<std::vector<int>, VectorHash> expanded;
            std::unordered_map<int, std::string> id_to_var = var_mgr_->get_index_to_name(); 

            // cooperative winning strategy
            std::unordered_map<int, CUDD::BDD> cooperative_output_function 
                = best_effort_result.cooperative.transducer.get()->get_output_function();

            if (starting_player_ == Player::Agent) {

            // stores all cooperative agent moves
            CUDD::BDD all_cooperative_moves = 
                get_all_cooperative_moves(arena_[0], 
                coop_synthesizer.get_winning_states(), 
                coop_synthesizer.get_winning_moves());

            while (!frontier.empty()){ // forall Z in frontier

                std::vector<int> state = frontier.front();
                std::vector<int> z_state;
                z_state.insert(z_state.end(), state.begin() + non_state_vars, state.end());
                frontier.pop();

                // search symbolically states witnessing no dominant strategy exists
                CUDD::BDD state_bdd = var_mgr_->state_vector_to_bdd(arena_[0].automaton_id(), z_state);
                CUDD::BDD cooperative_moves = state_bdd * all_cooperative_moves;
                CUDD::BDD witness_move = 
                    cooperative_moves * (!cooperative_moves.CProjection(var_mgr_->output_cube()));
                if (witness_move != var_mgr_->cudd_mgr()->bddZero()) { // state witnesses no dominant strategy exists
                    best_effort_result.dominant = false;
                    double domTest_t = domTest.stop().count() / 1000.0;
                    std::cout << "failed. Synthesized strategy is best-effort and no dominant strategy exists. DONE in " << domTest_t << " s" << std::endl;
                    running_times_.push_back(domTest_t);
                    return best_effort_result;
                } else { // if current state is not a witness, it should be expanded
                    std::vector<int> transition = state;
                    for (int i = 0; i < id_to_var.size(); ++i) { // fixes agent choice
                        std::string var = id_to_var[i];
                        int agent_eval;
                        if (var_mgr_->is_output_variable(var)) {
                            transition[i] = cooperative_output_function[i].Eval(state.data()).IsOne();
                        }
                    }

                    for (const auto& env_choice : env_moves) { // for every environment choice

                        int curr_x = 0;

                        for (int i = 0; i < id_to_var.size(); ++i) {
                            std::string var = id_to_var[i];
                            if (var_mgr_->is_input_variable(var)) {
                                transition[i] = env_choice[curr_x];
                                curr_x = curr_x + 1;
                            }
                        }

                    // construct successor state
                    std::vector<int> successor_state;
                    successor_state.insert(successor_state.end(), vars_init.begin(), vars_init.end());
                    for (const auto& bdd : transition_function) successor_state.push_back(bdd.Eval(transition.data()).IsOne());

                    if (cooperative_only_region.Eval(successor_state.data()).IsOne()) {
                        reached.insert(successor_state);
                        if (expanded.find(successor_state) == expanded.end()) {
                            frontier.push(successor_state);
                        }
                    }
                    expanded.insert(state);
                    }

                    if (reached == expanded) { // if a fixpoint is reached
                        best_effort_result.dominant = true;
                        double domTest_t = domTest.stop().count() / 1000.0;
                        std::cout << "Synthesized strategy is best-effort and dominant. DONE in " << domTest_t << " s" << std::endl;
                        running_times_.push_back(domTest_t);
                        return best_effort_result;
                    }
                }
            }
        }
    }
    return best_effort_result;
}

    void ExplicitCompositionalBestEffortSynthesizer::merge_and_dump_dot(const SynthesisResult& adversarial_result, const SynthesisResult& cooperative_result, const std::string& filename) const {
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

    std::vector<double> ExplicitCompositionalBestEffortSynthesizer::get_running_times() const {
        return running_times_;
    }

    CUDD::BDD ExplicitCompositionalBestEffortSynthesizer::get_all_cooperative_moves(const SymbolicStateDfa& arena, const CUDD::BDD& cooperative_states, const CUDD::BDD& cooperative_moves) const {
        std::vector<CUDD::BDD> substitution_vector = 
            var_mgr_->make_compose_vector(arena.automaton_id(), arena.transition_function());
        CUDD::BDD composed_cooperative_states = 
            cooperative_states.VectorCompose(substitution_vector);
        if (starting_player_ == Player::Agent) {
            composed_cooperative_states = 
                composed_cooperative_states.ExistAbstract(var_mgr_->input_cube());
        }
        return cooperative_moves + (cooperative_states * composed_cooperative_states);
    }


    std::unordered_map<int, CUDD::BDD> ExplicitCompositionalBestEffortSynthesizer::get_witness_strategy(const CUDD::BDD& witness) const {
        // this code follows same scheme as that in class DfaGameSynthesizer
        std::vector<CUDD::BDD> parameterized_output_function;
        int* output_indices;
        CUDD::BDD output_cube = var_mgr_->output_cube();
        std::size_t output_count = var_mgr_->output_variable_count();

        CUDD::BDD pre = (!witness).SolveEqn(
            output_cube,
            parameterized_output_function,
            &output_indices,
            output_count
        );

        std::vector<int> index_copy(output_count);

        for (std::size_t i = 0; i < output_count; ++i) {
            index_copy[i] = output_indices[i];
        }

        CUDD::BDD verified = (!witness).VerifySol(
            parameterized_output_function,
            output_indices
        );

        assert(pre == verified);

        std::unordered_map<int, CUDD::BDD> output_function;

        for (int i = output_count - 1; i >= 0; --i) {
            int output_index = index_copy[i];

            output_function[output_index] = parameterized_output_function[i];

            for (int j = output_count - 1; j >= i; --j) {
                int parameter_index = index_copy[j];

	        CUDD::BDD parameter_value = var_mgr_->cudd_mgr()->bddOne();

	        output_function[output_index] =
	          output_function[output_index].Compose(parameter_value,
	        					  parameter_index);
      }
  }

  return output_function;

}

void ExplicitCompositionalBestEffortSynthesizer::interactive(
        const BestEffortSynthesisResult& best_effort_result
    ) const {
        std::cout << "[BeSyft][interactive] Interactive strategy execution" << std::endl;

        // var_mgr_->print_varmgr();

        // initial state. Order of variables is: X \/ Y, Z_{phi}, Z_{E}, Z_{tau}
        std::vector<int> vars_init(var_mgr_->get_index_to_name().size(), 0);
        std::vector<int> implication_init = symbolic_dfas_[0].initial_state();
        std::vector<int> neg_env_init = symbolic_dfas_[1].initial_state();
        std::vector<int> conjunction_init = symbolic_dfas_[2].initial_state();
        std::vector<int> state;
        state.insert(state.end(), vars_init.begin(), vars_init.end());
        state.insert(state.end(), implication_init.begin(), implication_init.end());
        state.insert(state.end(), neg_env_init.begin(), neg_env_init.end());
        state.insert(state.end(), conjunction_init.begin(), conjunction_init.end());

        CUDD::BDD winning_region = best_effort_result.adversarial.winning_states;
        CUDD::BDD cooperative_region = best_effort_result.cooperative.winning_states;
        // CUDD::BDD witness_region = best_effort_result.dominance.witness_states;
        std::unordered_map<int, CUDD::BDD> output_function;
        std::unordered_map<int, CUDD::BDD> alternative_output_function;

        std::unordered_map<int, std::string> id_to_var = var_mgr_->get_index_to_name(); 

        bool running = true;
        while (running) {

            std::cout << "[BeSyft][interactive] Current state: ";
            for (const auto&b : state) std::cout << b;
            // std::cout << " Size. " << state.size() << std::endl;
            std::cout << std::endl;
        
            // gets output function and alternative output function if a state is a witness
            bool state_is_witness = false;
            if (winning_region.Eval(state.data()).IsOne()) {
                std::cout << "[BeSyft][interactive] Agent in winning region uses winning strategy" << std::endl;
                output_function = best_effort_result.adversarial.transducer.get()->get_output_function();
            } else if (cooperative_region.Eval(state.data()).IsOne()) {
                std::cout << "[BeSyft][interactive] Agent in cooperative region uses cooperative strategy" << std::endl;
                output_function = best_effort_result.cooperative.transducer.get()->get_output_function();
                // if (dominance_check_ & !best_effort_result.dominance.existence) {
                //     if (witness_region.Eval(state.data()).IsOne()) {
                //         state_is_witness = true;
                //         std::cout << "[BeSyft][interactive] State witnesses that no dominant strategy exists" << std::endl;
                //         alternative_output_function = best_effort_result.dominance.witness_transducer.get()->get_output_function();
                //         // std::cout << "alternative output function selected..." << std::endl;
                //     }                                                        
                // }
            } else {
                std::cout << "[BeSyft][interactive] Agent in losing region. Termination" << std::endl;
                return;
            }

            std::vector<int> transition = state;
        
            if (starting_player_ == Player::Agent) {

                // agent turn. Agent moves first
                std::cout << "[BeSyft][interactive] Agent move: " << std::endl;
                for (int i = 0; i < id_to_var.size(); ++i) {
                    std::string var = id_to_var[i];
                    int agent_eval;
                    if (var_mgr_->is_output_variable(var)) {
                        std::cout << "Variable: " << var;
                        std::cout << ". Agent output (0 = false, 1 = true): ";
                        agent_eval = output_function[i].Eval(state.data()).IsOne();
                        std::cout << agent_eval << std::endl;
                        transition[i] = agent_eval;
                    }
                }

                // shows witness if current state has one
                if (state_is_witness) {
                    std::cout << "[BeSyft][interactive] Alternative agent move (witness no dominant strategy exist): " << std::endl;
                    for (int i = 0; i < id_to_var.size(); ++i) {
                        std::string var = id_to_var[i];
                        int agent_eval;
                        if (var_mgr_->is_output_variable(var)) {
                            std::cout << "Variable: " << var;
                            std::cout << ". Agent output (0 = false, 1 = true): ";
                            agent_eval = alternative_output_function[i].Eval(state.data()).IsOne();
                            std::cout << agent_eval << std::endl;
                            // transition[i] = agent_eval; // do not update transitions
                        }
                    }
                } 

                // environment turn
                std::cout << "[BeSyft][interactive] Environment move (type 1 if var is true, else 0): " << std::endl;
                for (int i = 0; i < id_to_var.size(); ++i) {
                    std::string var = id_to_var[i];
                    int env_eval;
                    if (var_mgr_->is_input_variable(var)) {
                        std::cout << "Variable: " << var;
                        std::cout << ". Env Input (0=false, 1=true): ";
                        std::cin >> env_eval;
                        transition[i] = env_eval;
                    } 
                }

            } else if (starting_player_ == Player::Environment) {

                // environment turn. Environment moves first
                std::cout << "[BeSyft][interactive] Environment move (type 1 if var is true, else 0): " << std::endl;
                for (int i = 0; i < id_to_var.size(); ++i) {
                    std::string var = id_to_var[i];
                    int env_eval;
                    if (var_mgr_->is_input_variable(var)) {
                        std::cout << "Variable: " << var;
                        std::cout << ". Env Input (0=false, 1=true): ";
                        std::cin >> env_eval;
                        state[i] = env_eval; // if environment moves first, state must be updated
                        transition[i] = env_eval;
                    } 
                }

                // agent turn
                std::cout << "[BeSyft][interactive] Agent move: " << std::endl;
                for (int i = 0; i < id_to_var.size(); ++i) {
                    std::string var = id_to_var[i];
                    int agent_eval;
                    if (var_mgr_->is_output_variable(var)) {
                        std::cout << "Variable: " << var;
                        std::cout << ". Agent output (0 = false, 1 = true): ";
                        agent_eval = output_function[i].Eval(state.data()).IsOne();
                        std::cout << agent_eval << std::endl;
                        transition[i] = agent_eval;
                    }
                }

                // shows witness if current state has one
                if (state_is_witness) {
                    std::cout << "[BeSyft][interactive] Alternative agent move (witness no dominant strategy exist): " << std::endl;
                    for (int i = 0; i < id_to_var.size(); ++i) {
                        std::string var = id_to_var[i];
                        int agent_eval;
                        if (var_mgr_->is_output_variable(var)) {
                            std::cout << "Variable: " << var;
                            std::cout << ". Agent output (0 = false, 1 = true): ";
                            agent_eval = alternative_output_function[i].Eval(state.data()).IsOne();
                            std::cout << agent_eval << std::endl;
                            // transition[i] = agent_eval; // do not update transitions
                        }
                    } 
                }
            }

            std::cout << "[BeSyft][interactive] Input to transitions: ";
            for (const auto&b : transition) std::cout << b;
            // std::cout << " Size. "<< transition.size() << std::endl;
            std::cout << std::endl;
            // successor state
            int curr_state_var = id_to_var.size();
            std::vector<int> new_state = state;
            for (int i = 0; i < symbolic_dfas_[0].transition_function().size(); ++i) {
                new_state[curr_state_var] = symbolic_dfas_[0].transition_function()[i].Eval(transition.data()).IsOne();
                ++curr_state_var;
            }
            for (int i = 0; i < symbolic_dfas_[1].transition_function().size(); ++i) {
                new_state[curr_state_var] = symbolic_dfas_[1].transition_function()[i].Eval(transition.data()).IsOne();
                ++curr_state_var;
            }
            for (int i = 0; i < symbolic_dfas_[2].transition_function().size(); ++i) {
                new_state[curr_state_var] = symbolic_dfas_[2].transition_function()[i].Eval(transition.data()).IsOne();
                ++curr_state_var;
            }
            std::cout << "[BeSyft][interactive] Successor state: ";
            for (const auto& b: new_state) std::cout << b;
            std::cout << std::endl;

            // update state
            state = new_state;

            // evaluate whether we can stop the loop
            if (symbolic_dfas_[0].final_states().Eval(state.data()).IsOne() || symbolic_dfas_[2].final_states().Eval(state.data()).IsOne()) {
                std::cout << "[BeSyft][interactive] The goal has been reached. Termination" << std::endl;
                running = false;
            }
            if (symbolic_dfas_[1].final_states().Eval(state.data()).IsOne()) {
                std::cout << "[BeSyft][interactive] The environment has been negated. Termination" << std::endl;
                running = false; 
            }
        }
    }

    std::vector<std::vector<int>> ExplicitCompositionalBestEffortSynthesizer::get_assignments(int n) const {

        // std::cout << n << std::endl;

        if (n == 1) return {{0}, {1}};

        std::vector<std::vector<int>> rest = get_assignments(n-1);
        std::vector<std::vector<int>> appended;
        for (const auto& v: rest) {
            std::vector<int> v0 = v;
            std::vector<int> v1 = v;
            v0.push_back(0);
            appended.push_back(v0);
            v1.push_back(1);
            appended.push_back(v1);
        }
        return appended;
    }

}