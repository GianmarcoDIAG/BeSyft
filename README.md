# BeSyft

BeSyft is a tool for symbolic best-effort synthesis with LTLf goals and assumptions. The tool has been described in [1].

# Usage

The output of `BeSyft --help` is the following:

```
BeSyft: a tool for Reactive and Best-Effort Synthesis with LTLf Goals and Assumptions
Usage: ./BeSyft [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -d,--print-dot              Print the output function(s)
  -c,--dominance-check        Performs the dominance check
  -i,--interactive            Executes the synthesized strategy in interactive mode
  -a,--agent-file TEXT:FILE REQUIRED
                              File to agent specification
  -e,--environment-file TEXT:FILE REQUIRED
                              File to environment assumption
  -p,--partition-file TEXT:FILE REQUIRED
                              File to partition
  -s,--starting-player INT REQUIRED
                              Starting player:
                              agent=1;
                              environment=0.
  -t,--algorithm INT REQUIRED Specifies algorithm to use:
                              Direct Best-Effort Synthesis=1;
                              Compositional-Minimal Best-Effort Synthesis=2;
                              Compositional Best-Effort Synthesis=3;
                              Compositional-Minimal Reactive Synthesis=4
                              Compositional Reactive Synthesis=5
  -f,--save-results TEXT      If specified, save results in the passed file. Stores:
                              Algorithm;
                              Goal file;
                              Environment file;
                              Starting player;
                              LTLf2DFA (s);
                              DFA2Sym (s);
                              Adv Game (s);
                              Coop Game (s); 	#best-effort synthesis algorithms only
                              Dominance Test (s); 	# best-effort synthesis algorithms only with -c option
                              Run time(s);
                              Realizability;
                              Dominance;	#best-effort synthesis algorithms only with -c option
```

LTLf formulas in agent and environment files should be written in Lydia's syntax. For further details, refer to https://github.com/whitemech/lydia . 

To perform best-effort synthesis for an LTLf goal in some LTLf environment, you have to provide both the path to the agent goal and the environment specification, e.g., `counter_2.ltlf` and `add_request.ltlf`, and the path to the partition file, e.g., `counter_2.part` (see the `Examples` folder).

For instance, the command:

```
./BeSyft -a counter_2.ltlf -e add_request.ltlf -p counter_2.part -s 1 -t 3 -c -i
```

Performs best-effort synthesis using the compositional algorithm, checks the existence of a dominant strategy, and executes the synthesized strategy in interactive mode.

# Build from source

Compilation instruction using CMake (https://cmake.org/). We recommend the use of Ubuntu 20.04 LTS. Problems can occur between some libraries on which BeSyft relies and newer versions of Ubuntu (more information below).

## Install the dependencies

### Flex and Bison

The project uses Flex and Bison for parsing purposes.

First check that you have them: `whereis flex bison`

If no item occurs, then you have to install them: `sudo apt-get install -f flex bison`

### CUDD 3.0.0

The project depends on CUDD 3.0.0. To install it, run the following commands

```
wget https://github.com/whitemech/cudd/releases/download/v3.0.0/cudd_3.0.0_linux-amd64.tar.gz
tar -xf cudd_3.0.0_linux-amd64.tar.gz
cd cudd_3.0.0_linux-amd64
sudo cp -P lib/* /usr/local/lib/
sudo cp -Pr include/* /usr/local/include/
```

Otherwise, build from source (customize `PREFIX` variable as you see fit).

```
git clone https://github.com/whitemech/cudd && cd cudd
PREFIX="/usr/local"
./configure --enable-silent-rules --enable-obj --enable-dddmp --prefix=$PREFIX
sudo make install
```

If you get an error about aclocal, this might be due to either

* Not having automake: `sudo apt-get install automake`
* Needing to reconfigure, do this before `configuring: autoreconf -i`
* Using a version of aclocal other than 1.14: modify the version 1.14 in configure accordingly.

### MONA

The projects depends on the MONA library, version v1.4 (patch 19). We require that the library is compiled with different values for parameters such as `MAX_VARIABLES`, and `BDD_MAX_TOTAL_TABLE_SIZE` (you can have a look at the details at https://github.com/whitemech/MONA/releases/tag/v1.4-19.dev0).

To install the MONA library, run the following commands:

```
wget https://github.com/whitemech/MONA/releases/download/v1.4-19.dev0/mona_1.4-19.dev0_linux-amd64.tar.gz
tar -xf mona_1.4-19.dev0_linux-amd64.tar.gz
cd mona_1.4-19.dev0_linux-amd64
sudo cp -P lib/* /usr/local/lib/
sudo cp -Pr include/* /usr/local/include
```

### SPOT

The project relies on SPOT (https://spot.lre.epita.fr/). To install it, follows the instructions at https://spot.lre.epita.fr/install.html

### Graphviz

The project uses Graphviz to display automata and strategies. Follow the install instructions on the official website: https://graphviz.gitlab.io/download/.

On Ubuntu, this should work:

```
sudo apt-get install libgraphviz-dev
```

### Syft

BeSyft depends on Syft. First, install the Boost libraries.

```
sudo apt-get install libboost-dev-all
```

For further information see https://www.boost.org/ 

Install Syft with

```
git clone https://github.com/whitemech/Syft.git
cd Syft
git checkout v0.1.1
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
sudo make install
```

### Lydia:

Unzip the repository and move into it `cd BeSyft`

Clone Lydia within the submodules folder.

```
mkdir submodules
cd submodules 
git clone https://github.com/whitemech/lydia.git --recursive
```


NOTE: Users of Ubuntu 22.04 LTS might encounter compilation errors due to incompatibilities Lydia's Catch2 library and newer versions of Ubuntu. To address it, take the following steps

I. Go to https://github.com/catchorg/Catch2/tree/v2.x and get the .zip file.

II. unzip the file.

III. substitute in repository `submodules/lydia/third_party/Catch2` with the unzipped folder of Catch2.

IV. delete any CMakeCache.txt file which may have been generated by previous compilation processes.

### Building BeSyft

To build, run the following commands.

```
cd ..
mkdir build && cd build
cmake ..
make -j2
```

## Performing the Experiments

To plot the results of our experiments on counter games execute:

```
cd EmpiricalResults/CounterGames
python3 besyft_comparison.py
python3 besyft_vs_syft.py
python3 relative_time_cost.py
```

To plot the results of our experiments on random conjunction benchmarks execute

```
cd EmpiricalResults/Random
python3 random_benchmarks.py
```

Else, to execute your own experiments on counter games run `sudo chmod "u+x" run-counters.sh run-random.sh`

Then run: `./run-counters.sh` or `./run-random.sh`

To plot results on counter games execute:

```
cd Benchmarks/CounterGames
python3 besyft_comparison.py
python3 besyft_vs_syft.py
python3 relative_time_cost.py
```

To plot results on random conjunction benchmarks:

```
cd Benchmarks/Random
python3 random_benchmarks.py
```

## Contacts

For any question, feedback, or suggestion, please reach to: parretti@diag.uniroma1.it

## References

[1] De Giacomo, Giuseppe; Parretti, Gianmarco; and Zhu, Shufang 2023. Symbolic LTLf Best-Effort Synthesis. In European Conference on Multi-Agent Systems (EUMAS). Cham: Springer Nature Switzerland, 2023. p. 228-243.
