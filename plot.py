# TODO. Polish this script

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

def pad(s, sz, to):
    k = sz - len(s)
    padding = pd.Series([to] * k)
    return s.append(padding, ignore_index = True)

MAX_GOALS = 10
MAX_ENVS = 10
TIMEOUT = 1000.0

HEADER = ["Synthesizer","Goal","Env","First Player","LTLf2DFA (s)","DFA2Sym (s)", "Adv Game (s)","Coop Game (s)","Run Time (s)","Realizability"]

outfl_1 = pd.read_csv(r"./build/bin/benchs/outfl_1.csv", header=None)
outfl_2 = pd.read_csv(r"./build/bin/benchs/outfl_2.csv", header=None)
outfl_3 = pd.read_csv(r"./build/bin/benchs/outfl_3.csv", header=None)
outfl_4 = pd.read_csv(r"./build/bin/benchs/outfl_4.csv", header=None)

dfs = [outfl_1, outfl_2, outfl_3, outfl_4]

for df in dfs: df.columns = HEADER

# plots comparison of 3 best-effort synthesis approaches
dfs = [outfl_1, outfl_2, outfl_3]
for i in range(1, MAX_GOALS+1):
    bench_df = pd.DataFrame()
    bench_df["K"] = [i for i in range(1, MAX_ENVS+1)]
    for df in dfs:
        goal_df = df.iloc[:,[1,8]]
        goal_df = (goal_df.loc[goal_df['Goal'] == "goal_"+str(i)+".ltlf"])
        bench_df[df.iloc[0].tolist()[0]] = pad(goal_df.iloc[:,[1]]["Run Time (s)"], MAX_ENVS, TIMEOUT).values
    fig = plt.subplots()

    plt.plot(bench_df["Symbolic-Compositional Best-Effort Synthesizer"], label="Symbolic-Compositional", color = "green")
    plt.plot(bench_df["Monolithic Best-Effort Synthesizer"], label="Monolithic", color = "red")
    plt.plot(bench_df["Explicit-Compositional Best-Effort Synthesizer"], label="Explicit-Compositional", color = "blue")
    
    plt.yscale("log")
    plt.xlabel("Environment requests (K)")
    plt.ylabel("Running time (s)")
    plt.title("BeSyft performance (log scale) on "+str(i)+"-bits counter games")
    plt.xticks(np.arange(MAX_ENVS), np.arange(1, MAX_ENVS+1))
    plt.yticks([0.001, 0.01, 0.1, 1, 10, 100, 1000], [0.001, 0.01, 0.1, 1, 10, 100, 1000])

    plt.legend()
    plt.show()

# plots comparison of symbolic-compositional and reactive synthesis
dfs = [outfl_3, outfl_4]
for i in range(1, MAX_GOALS+1):
    bench_df = pd.DataFrame()
    bench_df["K"] = [i for i in range(1, MAX_ENVS+1)]
    for df in dfs:
        goal_df = df.iloc[:,[1,8]]
        goal_df = (goal_df.loc[goal_df['Goal'] == "goal_"+str(i)+".ltlf"])
        bench_df[df.iloc[0].tolist()[0]] = pad(goal_df.iloc[:,[1]]["Run Time (s)"], MAX_ENVS, TIMEOUT).values
    print(bench_df)
    fig = plt.subplots()

    plt.plot(bench_df["Symbolic-Compositional Best-Effort Synthesizer"], label="Symbolic-Compositional Best-Effort Synthesis", color = "green")
    plt.plot(bench_df["Adversarial Synthesizer"], label="Reactive Synthesis", color = "red")
    
    plt.yscale("log")
    plt.xlabel("Environment requests (K)")
    plt.ylabel("Running time (s)")
    plt.title("Symbolic-compositional best-effort synthesis vs reactive synthesis on "+str(i)+"-bits counter games")
    plt.xticks(np.arange(MAX_ENVS), np.arange(1, MAX_ENVS+1))
    plt.yticks([0.001, 0.01, 0.1, 1, 10, 100, 1000], [0.001, 0.01, 0.1, 1, 10, 100, 1000])

    plt.legend()
    plt.show()

# plots the stacked bar plots
# determine hardest instance solved
dfs = [outfl_1, outfl_2, outfl_3]
for df in dfs:
    m = 0
    for i in range(1, MAX_GOALS+1):
        goal_df = df.iloc[:,[1, 4, 5, 6, 7]]
        goal_df = (goal_df.loc[goal_df['Goal'] == "goal_"+str(i)+".ltlf"])
        if (len(goal_df) > 0): m += 1
    # print(m)

    # get data
    goal_df = df.iloc[:,[1, 4, 5, 6, 7]]
    goal_df = ((goal_df.loc[goal_df['Goal'] == "goal_"+str(m)+".ltlf"]))
    goal_df = goal_df.iloc[:,[1, 2, 3, 4]]
    # print(goal_df)

    # normalize data
    for i in range(len(goal_df)):
        goal_df.iloc[[i]] = goal_df.iloc[[i]].to_numpy() / np.sum(goal_df.iloc[[i]].to_numpy())

    barh = goal_df.plot(kind="barh", stacked=True, mark_right=True, colormap = "bwr")

    xlabels = ["0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%"]
    xticks = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1]
    exps = len(goal_df)
    barh.yaxis.set_ticklabels([i for i in range(1, exps+1)])
    barh.xaxis.set_ticks(xticks)
    barh.xaxis.set_ticklabels(xlabels)
    plt.xlabel("Time (%)")
    plt.ylabel("Environment Requests (K)")
    plt.xlim([0, 1])
    barh.xaxis.grid(color = 'k')
    plt.tight_layout()
    if df.equals(outfl_1): plt.title("Relative time cost of monolithic BES in "+str(m)+"-bits counter games")
    elif df.equals(outfl_2): plt.title("Relative time cost of explicit-compositional BES in "+str(m)+"-bits counter games")
    elif df.equals(outfl_3): plt.title("Relative time cost of symbolic-compositional BES in "+str(m)+"-bits counter games")
    plt.show()