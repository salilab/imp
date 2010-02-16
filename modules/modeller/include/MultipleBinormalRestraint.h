/**
 *  \file MultipleBinormalRestraint.h
 *  \brief Modeller-style multiple binormal (phi/psi) restraint.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMODELLER_MULTIPLE_BINORMAL_RESTRAINT_H
#define IMPMODELLER_MULTIPLE_BINORMAL_RESTRAINT_H

#include "config.h"

#include <IMP/Restraint.h>
#include <IMP/Particle.h>

IMPMODELLER_BEGIN_NAMESPACE

//! A single binormal term in a MultipleBinormalRestraint.
class BinormalTerm
{
  double correlation_;
  std::pair<double, double> weights_, means_, stdevs_;
public:
  void set_correlation(double correlation) { correlation_ = correlation; }
  void set_weights(std::pair<double, double> weights) { weights_ = weights; }
  void set_means(std::pair<double, double> means) { means_ = means; }
  void set_standard_deviations(std::pair<double, double> stdevs) {
    stdevs_ = stdevs;
  }
};

//! Modeller-style multiple binormal (phi/psi) restraint.
class IMPMODELLEREXPORT MultipleBinormalRestraint : public Restraint
{
  std::vector<BinormalTerm> terms_;
  ParticleQuad q1_, q2_;
public:
  //! Create the multiple binormal restraint.
  /** \param[in] q1 First quad of particles.
      \param[in] q2 Second quad of particles.
   */
  MultipleBinormalRestraint(const ParticleQuad &q1, const ParticleQuad &q2);

  void add_term(const BinormalTerm &term) {
    terms_.push_back(term);
  }

  IMP_RESTRAINT(MultipleBinormalRestraint)
};

IMPMODELLER_END_NAMESPACE

#endif  /* IMPMODELLER_MULTIPLE_BINORMAL_RESTRAINT_H */
