import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib import cm
from matplotlib.colors import ListedColormap, LinearSegmentedColormap
import os

MAX_COUNTER_SIZE = 8
MIN_REQUESTS_SIZE = 1
MAX_REQUESTS_SIZE = 10
TIMEOUT = 1000.0

IMPLEMENTATION_INDEX = 0
GOAL_INDEX = 1
ENV_INDEX = 2
STARTING_PLAYER_INDEX = 3
LTLF_TO_DFA_RUNTIME_INDEX = 4
DFA_TO_SYMBOLIC_RUNTIME_INDEX = 5
ADV_GAME_RUNTIME_INDEX = 6
COOP_GAME_RUNTIME_INDEX = 7
DOMINANCE_TEST_RUNTIME_INDEX = 8
RUNTIME_INDEX = 9
REALIZABILITY_INDEX = 10
DOMINANT_EXISTENCE_INDEX = 11

PERCENT = 100.0

COLOR_LIST = np.array([[0.0, 0.0, 0.75, 1.0],
                [0.0, 0.0, 0.75, 0.3],
                [0.75, 0.0, 0.0, 1.0],
                [0.75, 0.0, 0.0, 0.3],
                [0.0, 0.75, 0.0, 1.0]])
MY_CMAP = ListedColormap(COLOR_LIST)

MAJOR_OPERATIONS_COUNT = 5

FONT_SIZE = 14

besyft_1_results = pd.read_csv(r"outfl_1.csv", header = None)
besyft_2_results = pd.read_csv(r"outfl_2.csv", header = None)
besyft_3_results = pd.read_csv(r"outfl_3.csv", header = None)

for besyft in [besyft_1_results, besyft_2_results, besyft_3_results]:
    besyft = besyft.reset_index()

barhs = []

id = 0
for results in [besyft_1_results, besyft_2_results, besyft_3_results]:
    relative_time_df = pd.DataFrame(
        columns = [
        "LTLf2DFA",
        "DFA2Sym",
        "AdvGame",
        "CoopGame",
        "DomCheck"
    ]
    )
    for k in range(MIN_REQUESTS_SIZE, MAX_REQUESTS_SIZE+1):
        for i, row in results.iterrows():
            if row[GOAL_INDEX] == "goal_"+str(MAX_COUNTER_SIZE)+".ltlf" and row[ENV_INDEX]=="env_"+str(k)+".ltlf":
                relative_time_df = relative_time_df.append({
                    "LTLf2DFA": row[LTLF_TO_DFA_RUNTIME_INDEX],
                    "DFA2Sym": row[DFA_TO_SYMBOLIC_RUNTIME_INDEX],
                    "AdvGame": row[ADV_GAME_RUNTIME_INDEX],
                    "CoopGame": row[COOP_GAME_RUNTIME_INDEX],
                    "DomCheck": row[DOMINANCE_TEST_RUNTIME_INDEX]
                }, ignore_index = True)
    
    for i in range(len(relative_time_df)):
        relative_time_df.iloc[[i]] = relative_time_df.iloc[[i]].to_numpy() / np.sum(relative_time_df.iloc[[i]].to_numpy())

    print(relative_time_df * PERCENT)
    
    b = relative_time_df.plot(kind="barh", stacked=True, mark_right=True, colormap = MY_CMAP)

    xlabels = ["0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%"]
    xticks = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1]
    ylabels = ["ND-1", "ND-2", "ND-3", "ND-4", "ND-5", "ND-6", "ND-7", "ND-8", "ND-9", "ND-10"]

    b.yaxis.set_ticklabels(ylabels, fontsize = FONT_SIZE)
    b.xaxis.set_ticks(xticks)
    b.xaxis.set_ticklabels(xlabels, fontsize = FONT_SIZE)
    b.xaxis.set_label("Time (%)")
    b.yaxis.set_label("Instance")
    b.set_xlim([0, 1])

    b.xaxis.grid(color = 'k')

    plt.legend(ncol = 3, bbox_to_anchor = (0.5, 1.25), loc='upper center', fontsize=FONT_SIZE)

    plt.tight_layout()

    plt.show()