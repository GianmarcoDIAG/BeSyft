/*
* This header declares the class MonolithicBestEffortSynthesizer
* which implements the monolithic approach to best-effort synthesis
*/

#ifndef SYFT_MONOLITHICBESTEFFORTSYNTHESIZER_H
#define SYFT_MONOLITHICBESTEFFORTSYNTHESIZER_H

#include"ExplicitStateDfaMona.h"
#include"ExplicitStateDfa.h"
#include"SymbolicStateDfa.h"
#include"ReachabilitySynthesizer.h"
#include"CoOperativeReachabilitySynthesizer.h"
#include"InputOutputPartition.h"
#include"Stopwatch.h"
#include"spotparser.h"
#include<unordered_set>

namespace Syft {

	class MonolithicBestEffortSynthesizer {
	
		protected:
			std::shared_ptr<Syft::VarMgr> var_mgr_;

			std::string agent_specification_;
			std::string environment_specification_;

			Player starting_player_;

			std::vector<SymbolicStateDfa> symbolic_dfas_;
			std::vector<SymbolicStateDfa> arena_;

			InputOutputPartition partition_;

			std::vector<double> running_times_;

			bool dominance_check_;

			/**
			 * \brief Gets all cooperative winning moves in arena
			 * 
			 * \param arena Symbolic state DFA where to get cooperative moves
			 * \param cooperative_states Cooperative states in arena
			 * \param cooperative_moves Cooperative moves in arena
			 * 
			 * \return Boolean function representing cooperative states and all cooperative moves
			*/
			CUDD::BDD get_all_cooperative_moves(const SymbolicStateDfa& arena, const CUDD::BDD& cooperative_states, const CUDD::BDD& cooperative_moves) const;

			/**
			 * If a dominant strategy does not exist, finds an alternative strategy that proves it
			 * 
			 * \param witness Boolean function with a witness that dominant strategy does not exist 
			 * 
			 * \return a strategy proving that no dominant strategy exists
			*/
			std::unordered_map<int, CUDD::BDD> get_witness_strategy(const CUDD::BDD& witness) const;

			std::vector<std::vector<int>> get_assignments(int n) const;
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
			MonolithicBestEffortSynthesizer(std::shared_ptr<VarMgr> var_mgr,
									std::string agent_specification,
									std::string environment_specification,
									InputOutputPartition partition,
									Player starting_player,
									bool dominance_check);
			
			/**
			 * @brief Solves symbolic DFA games to compute adversarially and cooperatively winning strategies
			 * 
			 * @return std::pair<SynthesisResult, SynthesisResult>. First and second are the adversarial and cooperatively winning strategies, respectively.
			 */
			virtual BestEffortSynthesisResult run() final;

			/**
			* \brief Merges two transducers into a best-effort strategy
			*
			* \param adversarial_result Reactive synthesis result
			* \param coopeartive_result Cooperative synthesis result
			* \param filename A path to the output .dot file
			* \return void. Prints implementation function of best-effort strategy into a .dot file
			*/
			void merge_and_dump_dot(const SynthesisResult& adversarial_result, 
									const SynthesisResult& cooperative_result, 
									const string& filename) const;

			/**
			 * @brief Returns running times of major operations during synthesis
			 * 
			 * @return std::vector<double> storing running times  
			 */
			std::vector<double> get_running_times() const;

			void interactive(
        		const BestEffortSynthesisResult& best_effort_result
    		) const;

			
	};
}
#endif