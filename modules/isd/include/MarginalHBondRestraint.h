/**
 *  \file IMP/isd/MarginalHBondRestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model HBond-derived
 *  distance fit.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_MARGINAL_HBOND_RESTRAINT_H
#define IMPISD_MARGINAL_HBOND_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>

IMPISD_BEGIN_NAMESPACE

//! Apply a lognormal distance restraint between two particles.
/** Marginal of the lognormal model for NOEs where only \f$\sigma\f$ was
    marginalized, and \f$\gamma\f$ was set to 1.
    Since the restraint is complicated, pass individual particles to
    add_contribution() command.

   \f[p(D|X,I) =
    \left(\sum_{i=1}^N \log^2\left(\frac{V_i^{exp}}
                     {d_i^{-6}(X)}\right)\right)^{-\frac{N}{2}}
    \f]
 */
class IMPISDEXPORT MarginalHBondRestraint : public Restraint {
  PairContainers contribs_;
  std::vector<double> volumes_;
  double logsquares_;
  void set_logsquares(double logsquares) { logsquares_ = logsquares; }

 public:
  //! Create the restraint.
  MarginalHBondRestraint(Model *m)
      : Restraint(m, "MarginalHBondRestraint%1%") {};

  // add a contribution: simple case
  void add_contribution(Particle *p1, Particle *p2,
                        double Iexp);

  // add a contribution: general case
  void add_contribution(PairContainer *pc, double Iexp);

  // return the sum inside the parentheses
  double get_logsquares() const { return logsquares_; }

  unsigned get_number_of_contributions() const { return volumes_.size(); }

  /* call for probability */
  double get_probability() const { return exp(-unprotected_evaluate(nullptr)); }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MarginalHBondRestraint);
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_MARGINAL_HBOND_RESTRAINT_H */
