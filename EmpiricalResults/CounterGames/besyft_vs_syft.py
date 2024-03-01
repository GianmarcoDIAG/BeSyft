import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import os

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

BARWITDH = 0.25
FONT_SIZE = 16

IMPLEMENTATION_IDS = {
    1: "direct BeSyft",
    2: "compositional-minimal BeSyft",
    3: "compositional BeSyft",
    4: "compositional-minimal Syft",
    5: "compositional Syft"
    }

IMPLEMENTATION_COLORS = {
    1: (0, 0, 0.8, 1.0), #blue 
    2: (0.8, 0, 0, 1.0), #red
    3: (0, 0.8, 0.0, 1.0), #green
    4: (0.95, 0.95, 0.0, 1.0), #yellow
    5: (0.9, 0.0, 0.9, 1.0) #purple
}

besyft_3 = pd.read_csv(r"outfl_3.csv", header=None)
besyft_4 = pd.read_csv(r"outfl_4.csv", header=None)
besyft_5 = pd.read_csv(r"outfl_5.csv", header=None)

for besyft in [besyft_3, besyft_4, besyft_5]:
    besyft = besyft.reset_index()

besyft_vs_syft = pd.DataFrame(
    columns = [
        "Instance ID",
        "Number of Increment Requests",
        IMPLEMENTATION_IDS[3],
        IMPLEMENTATION_IDS[4],
        IMPLEMENTATION_IDS[5]
    ]
)

for k in range(MIN_REQUESTS_SIZE, MAX_REQUESTS_SIZE+1):
    besyft_time = min_syft_time = syft_time = TIMEOUT
    instance_id = ""
    for i, row in besyft_3.iterrows():
        if "goal_"+str(MAX_COUNTER_SIZE)+".ltlf" and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            besyft_time = row[RUNTIME_INDEX]
            instance_id = "ND-"+str(k)
    for i, row in besyft_4.iterrows():
        if "goal_"+str(MAX_COUNTER_SIZE)+".ltlf" and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            min_syft_time = row[RUNTIME_INDEX]
            instance_id = "ND-"+str(k)
    for i, row in besyft_5.iterrows():
        if "goal_"+str(MAX_COUNTER_SIZE)+".ltlf" and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            syft_time = row[RUNTIME_INDEX]
            instance_id = "ND-"+str(k)
    besyft_vs_syft = besyft_vs_syft.append({
        "Instance ID": instance_id,
        "Number of Increment Requests": str(int(k)),
        IMPLEMENTATION_IDS[3]: besyft_time,
        IMPLEMENTATION_IDS[4]: min_syft_time,
        IMPLEMENTATION_IDS[5]: syft_time
    }, ignore_index = True)

for k in range(MIN_REQUESTS_SIZE, MAX_REQUESTS_SIZE+1):
    besyft_time = min_syft_time = syft_time = TIMEOUT
    instance_id = ""
    for i, row in besyft_3.iterrows():
        if "goal_dominance_"+str(MAX_COUNTER_SIZE)+".ltlf" and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            besyft_time = row[RUNTIME_INDEX]
            instance_id = "D-"+str(k)
    for i, row in besyft_4.iterrows():
        if "goal_dominance_"+str(MAX_COUNTER_SIZE)+".ltlf" and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            min_syft_time = row[RUNTIME_INDEX]
            instance_id = "D-"+str(k)
    for i, row in besyft_5.iterrows():
        if "goal_dominance"+str(MAX_COUNTER_SIZE)+".ltlf" and "env_"+str(k)+".ltlf"==row[ENV_INDEX]:
            syft_time = row[RUNTIME_INDEX]
            instance_id = "D-"+str(k)
    besyft_vs_syft = besyft_vs_syft.append({
        "Instance ID": instance_id,
        "Number of Increment Requests": str(int(k)),
        IMPLEMENTATION_IDS[3]: besyft_time,
        IMPLEMENTATION_IDS[4]: min_syft_time,
        IMPLEMENTATION_IDS[5]: syft_time
    }, ignore_index = True)

print(besyft_vs_syft)

besyft_times = besyft_vs_syft.loc[:,IMPLEMENTATION_IDS[3]]
min_syft_times = besyft_vs_syft.loc[:,IMPLEMENTATION_IDS[4]]
syft_times = besyft_vs_syft.loc[:,IMPLEMENTATION_IDS[5]]

comparisons = [besyft_times, min_syft_times, syft_times]

p1 = plt.subplot()

pos_bar1 = np.arange(INSTANCES) # both no dominant and dominant benchmarks
pos_bar2 = [x + BARWITDH for x in pos_bar1]
pos_bar3 = [x + BARWITDH for x in pos_bar2]

pos_bar = [pos_bar1, pos_bar2, pos_bar3]

for i in range(len(comparisons)):
    p1.bar(pos_bar[i],
       comparisons[i],
       label = IMPLEMENTATION_IDS[i+3],
       width= BARWITDH,
       color = IMPLEMENTATION_COLORS[i+3]
       )

p1.set_xlabel("Instance", fontsize = FONT_SIZE)
p1.set_ylabel("Time (s)", fontsize = FONT_SIZE)

p1.set_xticks([r + BARWITDH for r in range(INSTANCES)], 
           besyft_vs_syft.loc[:,"Instance ID"], rotation=45)

plt.setp(p1.get_xticklabels(), fontsize=FONT_SIZE)
plt.setp(p1.get_yticklabels(), fontsize=FONT_SIZE)

p1.set_ylim(0, 25) # visualization purposes

p1.legend(loc = 'upper right', fontsize = FONT_SIZE)

plt.show()

