#include "Common.h"
#include <bitset>

struct BestEffortExplicitState {

    // In Best-effort arena states are represented as triples
    int adversarial_state_;
    int negated_env_state_;
    int cooperative_state_;

    BestEffortExplicitState() = default;

    BestEffortExplicitState(const int& adversarial_state, 
                            const int& negated_env_state, 
                            const int& cooperative_state) :
                                                            adversarial_state_(adversarial_state),
                                                            negated_env_state_(negated_env_state),
                                                            cooperative_state_(cooperative_state) {};
};

std::string state2bin(int n){
  std::string res;
    while (n)
    {
        res.push_back((n & 1) + '0');
        n >>= 1;
    }

    if (res.empty())
        res = "0";
    else
        reverse(res.begin(), res.end());
   return res;
}

int bin2state(const std::string& binary) {
    int state = stoi(binary, 0, 2);
    return state;
}

bool is_prefix(const std::string& prefix, const std::string& check_string) {
    auto res = std::mismatch(prefix.begin(), prefix.end(), check_string.begin());
    if (res.first == prefix.end()) {
        return true;
    }
    return false;
}

std::string state_to_binary_encoding(const int& state, const int& encoding_length) {
    std::string padded_encoding = "";
    std::string bin = state2bin(state);

    int pad = encoding_length - bin.size();

    for (int i=0; i<pad; ++i) {
        padded_encoding += "0";
    }

    padded_encoding += bin;
    
    return padded_encoding;
}

std::vector<int> from_binary_to_vector(const std::string& binary_string) {
    std::vector<int> binary_vector;

    for (int i=0; i<binary_string.size(); ++i) {
        binary_vector.push_back((int) (binary_string[i] - 48));
    }

    return binary_vector;
}

std::vector<int> from_triple_to_vector(const BestEffortExplicitState& triple,
                            int environment_bits,
                            int adversarial_bits,
                            int cooperative_bits)
    {
        std::string neg_env_bin = state_to_binary_encoding(triple.negated_env_state_, environment_bits);
        reverse(neg_env_bin.begin(), neg_env_bin.end());
        std::string adversarial_bin = state_to_binary_encoding(triple.adversarial_state_, adversarial_bits);
        reverse(adversarial_bin.begin(), adversarial_bin.end());
        std::string cooperative_bin = state_to_binary_encoding(triple.cooperative_state_, cooperative_bits);
        reverse(cooperative_bin.begin(), cooperative_bin.end());

        std::string binary_encoding = neg_env_bin + cooperative_bin + adversarial_bin;
        std::vector<int> binary_vector = from_binary_to_vector(binary_encoding);

        return binary_vector;
    }

BestEffortExplicitState from_vector_to_triple(const std::vector<int>& triple_vector,
                                                int environment_bits,
                                                int adversarial_bits,
                                                int cooperative_bits)
    {                                            
        
    }

std::vector<int> concatenate_vectors(const std::vector<int>& lhv, const std::vector<int> rhv) {
    std::vector<int> concatenated_vector;
    for (int b : lhv) {
        concatenated_vector.push_back(b);
    }
    for (int b : rhv) {
        concatenated_vector.push_back(b);
    }
    return concatenated_vector;
}

void print_vector(const std::vector<int>& vec) {
    std::cout << "Vector: ";
    for (auto b : vec) {
        std::cout << b;
    }
    std::cout << std::endl;
}

std::vector<std::vector<int>> get_binary_vectors(int length) {

    std::vector<std::vector<int>> binary_vectors;
    if (length == 0) {
        return binary_vectors;
    }
    std::vector<int> v0 = {0};
    // print_vector(v0);
    std::vector<int> v1 = {1};
    // print_vector(v1);
    
    binary_vectors.push_back(v0);
    binary_vectors.push_back(v1);

    for (int i=2; i<=length; ++i) {
        for (std::vector<int> vec : binary_vectors) {
            if (vec.size() == (i-1)) {
                std::vector<int> v_1 = concatenate_vectors(vec, v1);
                std::vector<int> v_0 = concatenate_vectors(vec, v0);
                binary_vectors.push_back(v_0);
                binary_vectors.push_back(v_1);
            }
        }
    }

    std::vector<std::vector<int>> result;
    for (std::vector<int> vec : binary_vectors) {
        if (vec.size() == length) {
            result.push_back(vec);
        }
    }
    return result;
}

    





/*
    std::vector<int> input({ 1, 2, 3, 4, 5 });
 
    int arr[input.size()];
    std::copy(input.begin(), input.end(), arr);
 
    for (int i: arr) {
        std::cout << i << ' ';
    }
*/ // Code to convert from a vector<int> into an array



