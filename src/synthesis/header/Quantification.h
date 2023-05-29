#ifndef QUANTIFICATION_H
#define QUANTIFICATION_H

#include <cuddObj.hh>

namespace Syft {

/**
 * \brief Abstract class representing a quantification operation on BDDs.
 */
class Quantification {
 public:
  virtual ~Quantification() {}
  virtual CUDD::BDD apply(const CUDD::BDD& bdd) const = 0;
};

/**
 * \brief Performs no quantification.
 */
class NoQuantification final : public Quantification {
 public:
  CUDD::BDD apply(const CUDD::BDD& bdd) const override;
};

/**
 * \brief Performs universal quantification.
 */
class Forall final : public Quantification {
 private:
  CUDD::BDD universal_variables_;
  
 public:
  Forall(CUDD::BDD universal_variables);

  CUDD::BDD apply(const CUDD::BDD& bdd) const override;
};

/**
 * \brief Performs existential quantification.
 */
class Exists final : public Quantification {
 private:
  CUDD::BDD existential_variables_;
  
 public:
  Exists(CUDD::BDD existential_variables);

  CUDD::BDD apply(const CUDD::BDD& bdd) const override;
};

/**
 * \brief Performs both universal and existential quantification.
 */
class ForallExists final : public Quantification {
 private:
  Forall forall_;
  Exists exists_;
  
 public:
  ForallExists(CUDD::BDD universal_variables,
	       CUDD::BDD existential_variables);

  CUDD::BDD apply(const CUDD::BDD& bdd) const override;
};


// ABSTRACTION FOR DOBULE EXISTENTIAL QUANTIFICATION
/**
 * \brief Performs two existential quantifications
 */
class ExistsExists final: public Quantification {
  private:
    Exists exists_a_; // First group of variables to be existentially quantified
    Exists exists_b_; // Second group of variables to be existentially quantified

  public:
    ExistsExists(CUDD::BDD existential_variables_a,
                  CUDD::BDD existential_variables_b);

    CUDD::BDD apply(const CUDD::BDD &bdd) const override;
};
// END

/**
 * \brief Performs both universal and existential quantification.
 */
    class ExistsForall final : public Quantification {
    private:
        Exists exists_;
        Forall forall_;

    public:
        ExistsForall(CUDD::BDD existential_variables,
                    CUDD::BDD universal_variables);

        CUDD::BDD apply(const CUDD::BDD& bdd) const override;
    };



}

#endif // QUANTIFICATION_H
