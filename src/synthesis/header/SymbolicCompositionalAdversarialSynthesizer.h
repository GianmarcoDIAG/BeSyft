#ifndef SYFT_SYMBOLICCOMPOSITIONALSymbolicCompositionalAdversarialSynthesizer_H
#define SYFT_SYMBOLICCOMPOSITIONALSymbolicCompositionalAdversarialSynthesizer_H

#include"ExplicitStateDfaMona.h"
#include"ExplicitStateDfa.h"
#include"SymbolicStateDfa.h"
#include"ReachabilitySynthesizer.h"
#include"CoOperativeReachabilitySynthesizer.h"
#include"InputOutputPartition.h"
#include"Stopwatch.h"
#include"spotparser.h"

namespace Syft {

	class SymbolicCompositionalAdversarialSynthesizer {
	
		protected:
			std::shared_ptr<Syft::VarMgr> var_mgr_;

			std::string agent_specification_;
			std::string environment_specification_;

			Player starting_player_;

			std::vector<SymbolicStateDfa> symbolic_dfa_;

			std::vector<SymbolicStateDfa> arena_;

			InputOutputPartition partition_;

			std::vector<double> running_times_;
		public:
		
			/**
			* \brief Construct an object presenting best-effort synthesis problem (E, Phi)
			* 
			* \param var_mgr Dictionary storing variables of the problem
			* \param agent_specification LTLf agent goal in Lydia syntax
			* \param environment_assumption LTLf environment specification in Lydia syntax
			* \param partition Partitioning of problem variables
			* \param starting_player Player who moves first each turn
			* 
			*/
			SymbolicCompositionalAdversarialSynthesizer(std::shared_ptr<VarMgr> var_mgr,
									std::string agent_specification,
									std::string environment_specification,
									InputOutputPartition partition,
									Player starting_player);
			
			/**
			 * @brief Solves symbolic DFA games to compute adversarially and cooperatively winning strategies
			 * 
			 * @return std::pair<SynthesisResult, SynthesisResult>. First and second are the adversarial and cooperatively winning strategies, respectively.
			 */
			virtual SynthesisResult run() final;

			/**
			 * @brief Returns running times of major operations during synthesis
			 * 
			 * @return std::vector<double> storing running times  
			 */
			std::vector<double> get_running_times() const;
	};
}
#endif