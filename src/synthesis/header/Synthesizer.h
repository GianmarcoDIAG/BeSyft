#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <memory>

#include "Transducer.h"
#include <tuple>

namespace Syft {

    struct VectorHash {
    size_t operator()(const std::vector<int>& v) const {
        std::hash<int> hasher;
        size_t seed = 0;
        for (int i : v) {
            seed ^= hasher(i) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }
        return seed;
    }
    };

    struct SynthesisResult{
        bool realizability;
        CUDD::BDD winning_states;
        std::unique_ptr<Transducer> transducer;
    };

    /**
     * class BestEffortSynthesisResult returns the result of best-effort synthesis
     * 
     * adversarial is the result of adversarial synthesis
     * cooperative is the result of cooperative synthesis
     * dominance is the result of dominance check
    */
    struct BestEffortSynthesisResult{
      SynthesisResult adversarial;
      SynthesisResult cooperative;
      bool dominant;
    };

/**
 * \brief Abstract class for synthesizers.
 *
 * Can be inherited to implement synthesizers for different specification types.
 */
template <class Spec>
class Synthesizer {
 protected:

  Spec spec_;
  
 public:

  Synthesizer(Spec spec)
    : spec_(std::move(spec))
    {}

  virtual ~Synthesizer()
    {}

  /**
   * \brief Solves the synthesis problem of the specification.
   *
   * \return The result consists of
   * realizability
   * a set of agent winning states
   * a transducer representing a winning strategy for the specification or nullptr if the specification is unrealizable.
   */
  virtual SynthesisResult run() = 0;
};

}

#endif // SYNTHESIZER_H
