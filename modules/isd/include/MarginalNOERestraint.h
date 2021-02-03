/**
 *  \file IMP/isd/MarginalNOERestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_MARGINAL_NOE_RESTRAINT_H
#define IMPISD_MARGINAL_NOE_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>

IMPISD_BEGIN_NAMESPACE

//! Apply an NOE distance restraint between two particles.
/** Marginal of the NOE lognormal model. Since restraint is complicated,
    pass individual particles to add_contribution() command. Supports
    ambiguous NOEs and derivatives.

   \f[p(D|X,I) = SS^{-\frac{N-1}{2}} \quad
    SS = \sum_{i=1}^N \log^2\left(\frac{V_i^{exp}}{d_i^{-6}(X)
          \hat{\gamma}}\right) \quad
    \hat{\gamma} = \left(\prod_{i=1}^N \frac{V_i^{exp}}{d_i^{-6}}\right)^{1/N}
    \f]
 */
class IMPISDEXPORT MarginalNOERestraint : public Restraint {
  PairContainers contribs_;
  std::vector<double> volumes_;
  double loggammahat_;
  void set_log_gammahat(double loggammahat) { loggammahat_ = loggammahat; }
  double SS_;
  void set_SS(double SS) { SS_ = SS; }

 public:
  //! Create the restraint.
  MarginalNOERestraint(Model *m)
      : Restraint(m, "MarginalNOERestraint%1%") {};

  // add a contribution: simple case
  void add_contribution(Particle *p1, Particle *p2,
                        double Iexp);

  // add a contribution: general case
  void add_contribution(PairContainer *pc, double Iexp);

  // return the estimate of gamma given the current structure.
  double get_log_gammahat() const { return loggammahat_; }

  // return the sum of squares wrt current structure.
  double get_SS() const { return SS_; }

  unsigned get_number_of_contributions() const { return volumes_.size(); }

  /* call for probability */
  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MarginalNOERestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_MARGINAL_NOE_RESTRAINT_H */
