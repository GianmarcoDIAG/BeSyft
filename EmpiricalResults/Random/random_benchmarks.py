import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import os

# for f in os.listdir("./"):
    # print(f)

# init parameters
TIMESTEP = 5
TIMEOUT = 1000
LTLF_TO_DFA_RUNTIME_INDEX = 4
DFA_TO_SYMBOLIC_RUNTIME_INDEX = 5
ADVERSARIAL_RUNTIME_INDEX = 6
COOPERATION_RUNTIME_INDEX = 7
DOMINANCE_TEST_RUNTIME_INDEX = 8
RUNTIME_INDEX = 9
REALIZABILITY_INDEX = 10
DOMINANCE_INDEX = 11
IMPLEMENTATION_IDS = {
    1: "direct BeSyft",
    2: "direct BeSyft (no dominance)",
    3: "compositional-minimal BeSyft",
    4: "compositional-minimal BeSyft (no dominance)",
    5: "compositional BeSyft",
    6: "compositional BeSyft (no dominance)",
    7: "compositional-minimal Syft",
    8: "compositional Syft"
    }
IMPLEMENTATION_LINESTYLE = {
    1: "solid",
    2: "dashed",
    3: "solid",
    4: "dashed",
    5: "solid",
    6: "dashed",
    7: "solid",
    8: "dashed"
}
IMPLEMENTATION_COLORS = {
    1: (0, 0, 1.0, 1.0), #blue 
    2: (0, 0, 1.0, 0.5),  #blue (light)
    3: (1.0, 0, 0, 1.0), #red
    4: (1.0, 0, 0, 0.5), #red (light)
    5: (0, 1.0, 0.0, 1.0), #green
    6: (0, 1.0, 0.0, 0.5), #gree (light)
    7: (1.0, 1.0, 0.0, 1.0), #yellow
    8: (1.0, 0.0, 1.0, 1.0) #purple
}
FONT_SIZE = 18
LINEWIDTH = 2.1

DOMINANCE_ONLY_IMPLEMENTATION_IDS = [1, 3, 5]
DOMINANCE_IMPLEMENTATION_IDS = [1, 3, 5, 7, 8]
BEST_EFFORT_IMPLEMENTATION_IDS = [1, 2, 3, 4, 5, 6]

if os.path.isfile("timeout_vs_solved_instances.csv"):
    to_vs_solved_df = pd.read_csv("timeout_vs_solved_instances.csv")
else:
    # read input data
    besyft_1 = pd.read_csv(r"outfl_1_dominance.csv", header = None)
    besyft_2 = pd.read_csv(r"outfl_1.csv", header = None)
    besyft_3 = pd.read_csv(r"outfl_2_dominance.csv", header = None)
    besyft_4 = pd.read_csv(r"outfl_2.csv", header = None)
    besyft_5 = pd.read_csv(r"outfl_3_dominance.csv", header = None)
    besyft_6 = pd.read_csv(r"outfl_3.csv", header = None)
    besyft_7 = pd.read_csv(r"outfl_4.csv", header = None)
    besyft_8 = pd.read_csv(r"outfl_5.csv", header = None)

    besyft = [besyft_1, besyft_2, besyft_3, besyft_4, besyft_5, besyft_6, besyft_7, besyft_8]
    besyft_be_only = [besyft_1, besyft_3, besyft_5]

    for bs in besyft:
        bs.reset_index()

    # init dataframe to store results
    to_vs_solved_df = pd.DataFrame(columns=["Timeout (s)", 
                                IMPLEMENTATION_IDS[1], 
                                IMPLEMENTATION_IDS[2],
                                IMPLEMENTATION_IDS[3],
                                IMPLEMENTATION_IDS[4],
                                IMPLEMENTATION_IDS[5],
                                IMPLEMENTATION_IDS[6],
                                IMPLEMENTATION_IDS[7],
                                IMPLEMENTATION_IDS[8]])
    
    realizability_df = pd.DataFrame(
        columns=["Implementation",
                 "Adversarial",
                 "Cooperative Dominant",
                 "Cooperative No Dominant",
                 "Unrealizable"]
                )
        
    for implementation in DOMINANCE_ONLY_IMPLEMENTATION_IDS:
        realizability_df = realizability_df.append({
            "Implementation": IMPLEMENTATION_IDS[implementation],
            "Adversarial": 0,
            "Cooperative Dominant": 0,
            "Cooperative No Dominant": 0,
            "Unrealizable": 0
        }, ignore_index = True)

    relative_time_cost = {}
    # for impl_id in DOMINANCE_ONLY_IMPLEMENTATION_IDS:
        # relative_time_cost[IMPLEMENTATION_IDS[impl_id]] = (0.0, 0.0)

    # store results about realizability of each instance
    id = 1 # iterate over implementations of interest [1, 3, 5]
    for results in besyft_be_only:
        print("Processing: " + str(IMPLEMENTATION_IDS[id]) + "'s realizability results...")
        total_ltlf_2_dfa_time = 0.0
        total_dfa_2_symbolic_time = 0.0
        total_adv_time = 0.0
        total_coop_time = 0.0
        total_dom_time = 0.0
        for i, row in results.iterrows():
            total_ltlf_2_dfa_time += row[LTLF_TO_DFA_RUNTIME_INDEX] / row[RUNTIME_INDEX]
            total_dfa_2_symbolic_time += row[DFA_TO_SYMBOLIC_RUNTIME_INDEX] / row[RUNTIME_INDEX]
            total_adv_time += row[ADVERSARIAL_RUNTIME_INDEX] / row[RUNTIME_INDEX]
            total_coop_time += row[COOPERATION_RUNTIME_INDEX] / row[RUNTIME_INDEX]
            total_dom_time += row[DOMINANCE_TEST_RUNTIME_INDEX] / row[RUNTIME_INDEX]
            for j, realizability_row in realizability_df.iterrows():
                if realizability_row["Implementation"] == IMPLEMENTATION_IDS[id]:
                    if row[REALIZABILITY_INDEX] == "Adv":
                        realizability_row["Adversarial"] += 1
                    elif row[REALIZABILITY_INDEX] == "Coop":
                        if row[DOMINANCE_INDEX] == "Dom":
                            realizability_row["Cooperative Dominant"] += 1
                        elif row[DOMINANCE_INDEX] == "NoDom":
                            realizability_row["Cooperative No Dominant"] += 1
                    elif row[REALIZABILITY_INDEX] == "Unr":
                        realizability_row["Unrealizable"] += 1
                    break
        relative_time_cost[IMPLEMENTATION_IDS[id]] = (
                round((total_ltlf_2_dfa_time / len(results)) * 100.0, 2),
                round((total_dfa_2_symbolic_time / len(results)) * 100.0, 2),
                round((total_adv_time / len(results)) * 100.0, 2),
                round((total_coop_time / len(results)) * 100.0, 2),
                round((total_dom_time / (realizability_row["Cooperative Dominant"] + realizability_row["Cooperative No Dominant"])) * 100.0, 2)
        )    
        id += 2

    print(realizability_df)

    print("Relative costs as (LTLf2DFA, DFA2Sym, AdvGame, CoopGame, DomTest) (%).")
    for (impl, rel_time_cost) in relative_time_cost.items():
        print(str(impl) + "relative time cost: " + str(rel_time_cost)) 
                         
    for i in range(TIMESTEP, TIMEOUT+1, TIMESTEP):
        to_vs_solved_df = to_vs_solved_df.append(
            {
                "Timeout (s)": i,
                IMPLEMENTATION_IDS[1]: 0,
                IMPLEMENTATION_IDS[2]: 0,
                IMPLEMENTATION_IDS[3]: 0,
                IMPLEMENTATION_IDS[4]: 0,
                IMPLEMENTATION_IDS[5]: 0,
                IMPLEMENTATION_IDS[6]: 0,
                IMPLEMENTATION_IDS[7]: 0,
                IMPLEMENTATION_IDS[8]: 0
            }, ignore_index = True
        )

    # store number of solved instances within timeout for each implementation
    id = 1
    for implementation in besyft:
        print("Processing: " + str(IMPLEMENTATION_IDS[id]) + "'s results...")
        for j, time_row in implementation.iterrows():
            for i, to_row in to_vs_solved_df.iterrows():
                if time_row[RUNTIME_INDEX] < to_row["Timeout (s)"]:
                    to_row[IMPLEMENTATION_IDS[id]] += 1
        id += 1

    to_vs_solved_df.to_csv("timeout_vs_solved_instances.csv",index=False)
    realizability_df.to_csv("implementation_realizability.csv", index=False)

print(to_vs_solved_df)

p1 = plt.subplot()

# compares all results simultaneously
for implementation_id in DOMINANCE_IMPLEMENTATION_IDS:
    p1.plot(
        to_vs_solved_df.loc[:,IMPLEMENTATION_IDS[implementation_id]],
        to_vs_solved_df.loc[:,"Timeout (s)"],
        color = IMPLEMENTATION_COLORS[implementation_id],
        label= IMPLEMENTATION_IDS[implementation_id],
        linewidth = LINEWIDTH 
    )

p1.set_xlabel("Number of Solved Instances", fontsize=FONT_SIZE)
p1.set_ylabel("Timeout (s)", fontsize=FONT_SIZE)

plt.setp(p1.get_xticklabels(), fontsize=FONT_SIZE)
plt.setp(p1.get_yticklabels(), fontsize=FONT_SIZE)

# p1.legend(fontsize=20) # using a size in points
p1.legend(fontsize=FONT_SIZE) # using a named size

plt.show()

# compare results of best-effort implementations
p2 = plt.subplot()

for implementation_id in BEST_EFFORT_IMPLEMENTATION_IDS:
    p2.plot(
        to_vs_solved_df.loc[:,IMPLEMENTATION_IDS[implementation_id]],
        to_vs_solved_df.loc[:,"Timeout (s)"],
        color = IMPLEMENTATION_COLORS[implementation_id],
        label= IMPLEMENTATION_IDS[implementation_id],
        linestyle = IMPLEMENTATION_LINESTYLE[implementation_id],
        linewidth = LINEWIDTH 
    )

p2.set_xlabel("Number of Solved Instances", fontsize=FONT_SIZE)
p2.set_ylabel("Timeout (s)", fontsize=FONT_SIZE)

p2.legend()

plt.setp(p2.get_xticklabels(), fontsize=FONT_SIZE)
plt.setp(p2.get_yticklabels(), fontsize=FONT_SIZE)

p2.legend(fontsize=FONT_SIZE)

plt.show()