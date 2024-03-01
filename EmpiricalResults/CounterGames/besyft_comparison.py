import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import os

MAX_COUNTER_SIZE = 8
MIN_REQUESTS_SIZE = 1
MAX_REQUESTS_SIZE = 10
INSTANCES = 2 * MAX_REQUESTS_SIZE
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

BARWITDH = 0.30
FONT_SIZE = 16

IMPLEMENTATION_IDS = {
    1: "direct BeSyft",
    2: "compositional-minimal BeSyft",
    3: "compositional BeSyft"
    }

IMPLEMENTATION_COLORS = {
    1: (0, 0, 0.8, 1.0), #blue 
    2: (0.8, 0, 0, 1.0), #red
    3: (0, 0.8, 0.0, 1.0), #green
    4: (0.8, 0.8, 0.0, 1.0), #yellow
    5: (0.8, 0.0, 0.8, 1.0) #purple
}

besyft_1 = pd.read_csv(r"outfl_1.csv", header = None)
besyft_2 = pd.read_csv(r"outfl_2.csv", header = None)
besyft_3 = pd.read_csv(r"outfl_3.csv", header = None)

for besyft in [besyft_1, besyft_2, besyft_3]:
    besyft = besyft.reset_index()

besyft_comparison = pd.DataFrame(
    columns = [
        "Instance ID",
        "Number of Increment Requests",
        IMPLEMENTATION_IDS[1],
        IMPLEMENTATION_IDS[2],
        IMPLEMENTATION_IDS[3]
    ]
)

for k in range(MIN_REQUESTS_SIZE, MAX_REQUESTS_SIZE+1):
    besyft_1_time = besyft_2_time = besyft_3_time = TIMEOUT
    instance_id = ""
    for i, row in besyft_1.iterrows():
        if "goal_"+str(MAX_COUNTER_SIZE)+".ltlf"==row[GOAL_INDEX] and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            besyft_1_time = row[RUNTIME_INDEX]
            instance_id = "ND-"+str(k)
    for i, row in besyft_2.iterrows():
        if "goal_"+str(MAX_COUNTER_SIZE)+".ltlf"==row[GOAL_INDEX] and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            besyft_2_time = row[RUNTIME_INDEX]
            instance_id = "ND-"+str(k)
    for i, row in besyft_3.iterrows():
        if "goal_"+str(MAX_COUNTER_SIZE)+".ltlf"==row[GOAL_INDEX] and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            besyft_3_time = row[RUNTIME_INDEX]
            instance_id = "ND-"+str(k)
    besyft_comparison = besyft_comparison.append(
        {
            "Instance ID": instance_id,
            "Number of Increment Requests": str(int(k)),
            IMPLEMENTATION_IDS[1]: besyft_1_time,
            IMPLEMENTATION_IDS[2]: besyft_2_time,
            IMPLEMENTATION_IDS[3]: besyft_3_time
        }, ignore_index = True
    )

for k in range(MIN_REQUESTS_SIZE, MAX_REQUESTS_SIZE+1):
    besyft_1_time = besyft_2_time = besyft_3_time = TIMEOUT
    instance_id = ""
    for i, row in besyft_1.iterrows():
        if "goal_dominance_"+str(MAX_COUNTER_SIZE)+".ltlf" == row[GOAL_INDEX] and "env_"+str(k)+".ltlf" == row[ENV_INDEX]:
            besyft_1_time = row[RUNTIME_INDEX]
            instance_id = "D-"+str(k)
    for i, row in besyft_2.iterrows():
        if "goal_dominance_"+str(MAX_COUNTER_SIZE)+".ltlf" == row[GOAL_INDEX] and "env_"+str(k)+".ltlf" == row[ENV_INDEX]:
            besyft_2_time = row[RUNTIME_INDEX]
            instance_id = "D-"+str(k)
    for i, row in besyft_3.iterrows():
        if "goal_dominance_"+str(MAX_COUNTER_SIZE)+".ltlf" == row[GOAL_INDEX] and "env_"+str(k)+".ltlf" == row[ENV_INDEX]:
            besyft_3_time = row[RUNTIME_INDEX]
            instance_id = "D-"+str(k)

    besyft_comparison = besyft_comparison.append(
        {
            "Instance ID": instance_id,
            "Number of Increment Requests": str(int(k)),
            IMPLEMENTATION_IDS[1]: besyft_1_time,
            IMPLEMENTATION_IDS[2]: besyft_2_time,
            IMPLEMENTATION_IDS[3]: besyft_3_time
        }, ignore_index = True
    )

print(besyft_comparison)

besyft_1_times = besyft_comparison.loc[:,IMPLEMENTATION_IDS[1]]
besyft_2_times = besyft_comparison.loc[:,IMPLEMENTATION_IDS[2]]
besyft_3_times = besyft_comparison.loc[:,IMPLEMENTATION_IDS[3]]

besyft_times = [besyft_1_times, besyft_2_times, besyft_3_times]

p1 = plt.subplot()

pos_bar1 = np.arange(INSTANCES) # both no dominant and dominant benchmarks
pos_bar2 = [x + BARWITDH for x in pos_bar1]
pos_bar3 = [x + BARWITDH for x in pos_bar2]

pos_bar = [pos_bar1, pos_bar2, pos_bar3]

for i in range(len(besyft_times)):
    p1.bar(pos_bar[i],
       besyft_times[i],
       label = IMPLEMENTATION_IDS[i+1],
       width= BARWITDH,
       color = IMPLEMENTATION_COLORS[i+1]
       )

p1.set_xlabel("Instance", fontsize = FONT_SIZE)
p1.set_ylabel("Time (s)", fontsize = FONT_SIZE)

p1.set_xticks([r + BARWITDH for r in range(INSTANCES)], 
           besyft_comparison.loc[:,"Instance ID"], rotation=45)

plt.setp(p1.get_xticklabels(), fontsize=FONT_SIZE)
plt.setp(p1.get_yticklabels(), fontsize=FONT_SIZE)

p1.set_ylim(0, 400) # visualization purposes

p1.legend(loc = 'upper right', fontsize = FONT_SIZE)

plt.show()


