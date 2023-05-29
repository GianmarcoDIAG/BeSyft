#include<string>
#include<vector>
#include<iostream>
#include<fstream>

std::string get_init(int n_bits) {
    std::string init = "";
    init += "(!c0 &";
    for (int i=0; i <= n_bits-1; ++i) {
        if (i < n_bits - 1) {
            init += " !b" + std::to_string(i) + " & " + "!c" + std::to_string(i+1) + " & "; 
        } else if (i == n_bits - 1) {
            //init += " !b" + std::to_string(i) +  ")";
            init += " !b" + std::to_string(i) + " & " + "!c" + std::to_string(i+1) + ")";
        }
    }
    return init;
}

std::string get_goal(int n_bits) {
    std::string goal = "";
    goal += "F(";
    for (int i=0; i<=n_bits-1; ++i) {
        if (i < n_bits - 1) {
            goal += " b" + std::to_string(i) + " & "; 
        } else if (i == n_bits-1) {
            goal += " b" + std::to_string(i) + ")";
        }
    }
    return goal;
}

std::string get_constraint() {
    return "G((!add) -> X(!c0))";
}

std::string get_bits_constraint(int n_bits) {
    std::string bits_constraint = "";
    for (int i=0; i < n_bits; ++i) {
        std::string case_1 = "G(((!c" + std::to_string(i) + ") & (!b" + std::to_string(i) + 
                            ")) -> X((!b" + std::to_string(i) + ") & (!c" + std::to_string(i+1) + ")))";
        std::string case_2 = "G(((!c" + std::to_string(i) + ") & b" + std::to_string(i) +
                            ") -> X(b" + std::to_string(i) + " & (!c" + std::to_string(i+1) + ")))";
        std::string case_3 = "G((c" + std::to_string(i) + " & (!b" + std::to_string(i) + 
                            ")) -> X(b" + std::to_string(i) + " & (!c" + std::to_string(i+1) + ")))";
        std::string case_4 = "G((c" + std::to_string(i) + " & b" + std::to_string(i) +
                            ") -> X((!b" + std::to_string(i) + ") & c" + std::to_string(i+1) + "))";
        if (i < n_bits - 1) {
            bits_constraint += case_1 + " & " + case_2 + " & " + case_3 + " & " + case_4 + " & ";
        } else if (i == n_bits - 1) {
            bits_constraint += case_1 + " & " + case_2 + " & " + case_3 + " & " + case_4;
        }
     }
     return bits_constraint;
}

//Unrealizable counter game
std::string get_unrealizable_bits_constraint(int n_bits) {
    std::string bits_constraint = "";
    // Wrong constraint on first bit.
    std::string case_1 = "G(((!c" + std::to_string(0) + ") & (!b" + std::to_string(0) + 
                            ")) -> X((!b" + std::to_string(0) + ") & (!c" + std::to_string(1) + ")))";
    std::string case_2 = "G(((!c" + std::to_string(0) + ") & b" + std::to_string(0) +
                            ") -> X(b" + std::to_string(0) + " & (!c" + std::to_string(1) + ")))";
    std::string case_3 = "G((c" + std::to_string(0) + " & (!b" + std::to_string(0) + 
                            ")) -> X((!b" + std::to_string(0) + ") & c" + std::to_string(1) + "))";
    std::string case_4 = "G((c" + std::to_string(0) + " & b" + std::to_string(0) +
                            ") -> X(b" + std::to_string(0) + " & c" + std::to_string(1) + "))";
    if (n_bits > 1) {
        bits_constraint += case_1 + " & " + case_2 + " & " + case_3 + " & " + case_4 + " & ";
    } else {
        bits_constraint += case_1 + " & " + case_2 + " & " + case_3 + " & " + case_4;
    }
    // for bits 1 ... n remains unchanged
    for (int i=1; i < n_bits; ++i) {
        std::string case_1 = "G(((!c" + std::to_string(i) + ") & (!b" + std::to_string(i) + 
                            ")) -> X((!b" + std::to_string(i) + ") & (!c" + std::to_string(i+1) + ")))";
        std::string case_2 = "G(((!c" + std::to_string(i) + ") & b" + std::to_string(i) +
                            ") -> X(b" + std::to_string(i) + " & (!c" + std::to_string(i+1) + ")))";
        std::string case_3 = "G((c" + std::to_string(i) + " & (!b" + std::to_string(i) + 
                            ")) -> X(b" + std::to_string(i) + " & (!c" + std::to_string(i+1) + ")))";
        std::string case_4 = "G((c" + std::to_string(i) + " & b" + std::to_string(i) +
                            ") -> X((!b" + std::to_string(i) + ") & c" + std::to_string(i+1) + "))";
        if (i < n_bits - 1) {
            bits_constraint += case_1 + " & " + case_2 + " & " + case_3 + " & " + case_4 + " & ";
        } else if (i == n_bits - 1) {
            bits_constraint += case_1 + " & " + case_2 + " & " + case_3 + " & " + case_4;
        }
     }
     return bits_constraint;
}

std::string get_agent_specification(int n_bits) {
    std::string agent_specification = "";
    std::string initial_state = get_init(n_bits);
    std::string goal = get_goal(n_bits);
    std::string constraint = get_constraint();
    std::string bits_constraint = get_bits_constraint(n_bits);
    agent_specification += initial_state + " & " + goal + " & " + constraint + " & " + bits_constraint;
    return agent_specification;
}

std::string get_unrealizable_agent_specifcation(int n_bits) {
    std::string agent_specification = "";
    std::string initial_state = get_init(n_bits);
    std::string goal = get_goal(n_bits);
    std::string constraint = get_constraint();
    std::string bits_constraint = get_unrealizable_bits_constraint(n_bits);
    agent_specification += initial_state + " & " + goal + " & " + constraint + " & " + bits_constraint;
    return agent_specification;
}

void print_partition(int n_bits, std::string partition_path) {
    std::ofstream partition_stream(partition_path);
    std::string input_variables = ".inputs: add\n";
    std::string output_variables = ".outputs: c0 ";
    for (int i=0; i < n_bits; ++i) {
        output_variables += "b" + std::to_string(i) + " c" + std::to_string(i+1) + " ";
    }
    // std::cout << input_variables << std::endl;
    // std::cout << output_variables << std::endl;
    partition_stream << input_variables;
    partition_stream << output_variables;
}

std::string get_nested_add(int nested_next) {
    std::string nested_stat = "";

    for (int i = 1; i < nested_next; ++i) {
        nested_stat += "X(";
    }
    nested_stat += "add";

    for (int j = 1; j < nested_next; ++j) {
        nested_stat += ")";
    }

    return nested_stat;
}

std::string get_nested_assumption(int sequence_length) {
    std::string assumption = "F(";

    for (int i=0; i < sequence_length; ++i) {
        if (i == (sequence_length - 1)) {
            assumption += get_nested_add(i+1);
        } else {
            assumption += get_nested_add(i+1) + " & ";
        }
    }
    assumption += ")";
    return assumption;
}

void print_ltlf_goal(int bits, std::string path) {
    std::ofstream outstrm(path);
    outstrm << get_agent_specification(bits);
}

void print_ltlf_unrgoal(int bits, std::string path) {
    std::ofstream outstrm(path);
    outstrm << get_unrealizable_agent_specifcation(bits);
}

void print_ltlf_env(int adds, std::string path) {
    std::ofstream outstrm(path);
    outstrm << get_nested_assumption(adds);
}

std::string exeInstance(int to, int alg, int bits, int envs) {
    std::string bs = "timeout " + std::to_string(to) + " ./../BeSyft ";
    std::string goal = "-a goal_" + std::to_string(bits) + ".ltlf ";
    std::string env = "-e env_" + std::to_string(envs) + ".ltlf ";
    std::string part = "-p part_" + std::to_string(bits) + ".part ";
    std::string salg = "-t " + std::to_string(alg) + " ";
    std::string out = "-f outfl_" + std::to_string(alg) + ".csv ";
    std::string sp = "-s 1 ";
    std::string nl = " ;";
    return bs + goal + env + part + salg + out + sp + nl;   
}

std::string exeUnrInstance(int to, int alg, int bits, int envs) {
    std::string bs = "timeout " + std::to_string(to) + " ./../BeSyft ";
    std::string goal = "-a unrgoal_" + std::to_string(bits) + ".ltlf ";
    std::string env = "-e env_" + std::to_string(envs) + ".ltlf ";
    std::string part = "-p part_" + std::to_string(bits) + ".part ";
    std::string salg = "-t " + std::to_string(alg) + " ";
    std::string out = "-f outfl_" + std::to_string(alg) + ".csv ";
    std::string sp = "-s 1 ";
    std::string nl = " ;";
    return bs + goal + env + part + salg + out + sp + nl;
}

void exeBenchs(int to, int max_algs, int max_bits, int max_envs, std::string outfl) {
    std::ofstream outstrm(outfl);
    for (int i = 1; i <= max_algs; ++i) {
        for (int j = 1; j <= max_bits; ++j) {
            for (int k = 1; k <= max_envs; ++k) {
                outstrm << exeInstance(to, i, j, k) << std::endl;
            }
        }
    }
    for (int i = 1; i <= max_algs; ++i) {
        for (int j = 1; j <= max_bits; ++j) {
            for (int k = 1; k <= max_envs; ++k) {
                outstrm << exeUnrInstance(to, i, j, k) << std::endl;
            }
        }
    }
}

int main() {

    int TIMEOUT = 1000, ALGS=4, MAX_BITS = 10, MAX_ENVS = 10;

    for (int i = 1; i <= MAX_BITS; ++i) {
        print_ltlf_goal(i, "benchs/goal_"+std::to_string(i)+".ltlf");
        print_ltlf_unrgoal(i, "benchs/unrgoal_"+std::to_string(i)+".ltlf");
        print_partition(i, "benchs/part_"+std::to_string(i)+".part");
    }

    for (int i = 1; i <= MAX_ENVS; ++i) {
        print_ltlf_env(i, "benchs/env_"+std::to_string(i)+".ltlf");
    }

    exeBenchs(TIMEOUT, ALGS, MAX_BITS, MAX_ENVS, "benchs/exe.sh");

    std::cout << "Counter benchmarks printed" << std::endl;

    return 0;

}