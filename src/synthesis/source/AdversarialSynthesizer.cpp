/*
* This file defines the class AdversarialSynthesizer
* which implements reactive synthesis under environment assumptions
* by reducing it to reactive synthesis
*/

#include "AdversarialSynthesizer.h"
#include <boost/algorithm/string.hpp>
#include <queue>

namespace Syft {

    AdversarialSynthesizer::AdversarialSynthesizer(
                            std::shared_ptr<VarMgr> var_mgr,
                            std::string agent_specification,
                            std::string environment_specification,
                            InputOutputPartition partition,
                            Player starting_player) :   
                                var_mgr_(var_mgr),
                                agent_specification_(agent_specification),
                                environment_specification_(environment_specification),
                                partition_(partition),
                                starting_player_(starting_player)
    {

        // step 1. Construct symbolic DFA of LTLf formula E -> Phi
        Stopwatch ltlf2dfa;
        ltlf2dfa.start();

        std::string adversarial_formula = 
            "(" + environment_specification_ + ") -> (" + agent_specification_ + ")";

        ExplicitStateDfaMona adversarial_formula_dfa = 
            ExplicitStateDfaMona::dfa_of_formula(adversarial_formula); 

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

        ExplicitStateDfa explicit_adversarial_dfa = 
            ExplicitStateDfa::from_dfa_mona(var_mgr_, adversarial_formula_dfa);
        
        symbolic_dfa_.push_back(SymbolicStateDfa::from_explicit(std::move(explicit_adversarial_dfa)));

        double t_dfa2sym = dfa2sym.stop().count() / 1000.0;
        std::cout << "[BeSyft] Symbolic DFA construction DONE in " << t_dfa2sym << std::endl;
        running_times_.push_back(t_dfa2sym);
    }                        

    SynthesisResult AdversarialSynthesizer::run() 
    {
        SynthesisResult adv_result;
        CUDD::BDD adv_goal = symbolic_dfa_[0].final_states();

        // Step 2. Compute a winning strategy in the adversarial game, if it exists
        Stopwatch advGame;
        advGame.start();
        std::cout << "[BeSyft] Constructing and solving adversarial game...";
        ReachabilitySynthesizer adv_synthesizer(symbolic_dfa_[0],
                                                starting_player_,
                                                Player::Agent,
                                                adv_goal, // Lifting
                                                var_mgr_->cudd_mgr()->bddOne());
        adv_result = adv_synthesizer.run();
        double t_advGame = advGame.stop().count() / 1000.0;
        running_times_.push_back(t_advGame);
        std::cout << "DONE in " << t_advGame << " s" << std::endl;
        return adv_result;
    }

    std::vector<double> AdversarialSynthesizer::get_running_times() const {
        return running_times_;
    }                                      
}