/**
 *  \file IMP/modeller/MultipleBinormalRestraint.h
 *  \brief Modeller-style multiple binormal (phi/psi) restraint.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMODELLER_MULTIPLE_BINORMAL_RESTRAINT_H
#define IMPMODELLER_MULTIPLE_BINORMAL_RESTRAINT_H

#include <IMP/modeller/modeller_config.h>

#include <IMP/kernel/Restraint.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/ParticleTuple.h>
#include <IMP/generic.h>

IMPMODELLER_BEGIN_NAMESPACE

class BinormalTerm;

//! Modeller-style multiple binormal (phi/psi) restraint.
/** This implements a multiple binormal restraint on the two dihedral angles
    between the two quads of kernel::Particles passed to the restraint, by
   implementing
    equation A.76 in the
    \external{http://salilab.org/modeller/9v7/manual/node441.html,
   Modeller manual}.
    The two angles are typically the phi and psi dihedrals of a residue.
 */
class IMPMODELLEREXPORT MultipleBinormalRestraint : public kernel::Restraint {
  std::vector<BinormalTerm> terms_;
  kernel::ParticleQuad q1_, q2_;

 public:
  //! Create the multiple binormal restraint.
  /** After creating the restraint, call add_term one or more times to add
      BinormalTerms to the restraint.
      \param[in] q1 First quad of particles.
      \param[in] q2 Second quad of particles.
   */
  MultipleBinormalRestraint(const kernel::ParticleQuad &q1,
                            const kernel::ParticleQuad &q2);

  //! Add a single BinormalTerm to the restraint.
  void add_term(const BinormalTerm &term) { terms_.push_back(term); }

  virtual double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MultipleBinormalRestraint);
};

//! A single binormal term in a MultipleBinormalRestraint.
class BinormalTerm {
  double correlation_, weight_;
  std::pair<double, double> means_, stdevs_;

  double evaluate(const double dihedral[2], double &sin1, double &sin2,
                  double &cos1, double &cos2, double &rho) const;

 public:
  BinormalTerm()
      : correlation_(-1), weight_(-1), means_(-1, -1), stdevs_(-1, -1) {}
  friend class MultipleBinormalRestraint;

  void set_correlation(double correlation) { correlation_ = correlation; }
  void set_weight(double weight) { weight_ = weight; }
  void set_means(FloatPair means) { means_ = means; }
  void set_standard_deviations(FloatPair stdevs) { stdevs_ = stdevs; }
  IMP_SHOWABLE_INLINE(BinormalTerm, {
    out << "correlation: " << correlation_ << "; weight: " << weight_
        << "; means: " << means_.first << ", " << means_.second
        << "; standard deviations: " << stdevs_.first << ", " << stdevs_.second;
  });
};
IMP_VALUES(BinormalTerm, BinormalTermList);

IMPMODELLER_END_NAMESPACE

#endif /* IMPMODELLER_MULTIPLE_BINORMAL_RESTRAINT_H */
