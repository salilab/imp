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

class BinormalTerm;

//! Modeller-style multiple binormal (phi/psi) restraint.
/** This implements a multiple binormal restraint on the two dihedral angles
    between the two quads of Particles passed to the restraint, by implementing
    equation A.76 in the
    \external{salilab.org/modeller/9v7/manual/node441.html, Modeller manual}.
    The two angles are typically the phi and psi dihedrals of a residue.
 */
class IMPMODELLEREXPORT MultipleBinormalRestraint : public Restraint
{
  std::vector<BinormalTerm> terms_;
  ParticleQuad q1_, q2_;
public:
  //! Create the multiple binormal restraint.
  /** After creating the restraint, call add_term one or more times to add
      BinormalTerms to the restraint.
      \param[in] q1 First quad of particles.
      \param[in] q2 Second quad of particles.
   */
  MultipleBinormalRestraint(const ParticleQuad &q1, const ParticleQuad &q2);

  //! Add a single BinormalTerm to the restraint.
  void add_term(const BinormalTerm &term) {
    terms_.push_back(term);
  }

  IMP_RESTRAINT(MultipleBinormalRestraint)
};

//! A single binormal term in a MultipleBinormalRestraint.
class BinormalTerm
{
  double correlation_, weight_;
  std::pair<double, double> means_, stdevs_;

  double evaluate(const double dihedral[2], double &sin1, double &sin2,
                  double &cos1, double &cos2, double &rho) const;
public:
  friend class MultipleBinormalRestraint;

  void set_correlation(double correlation) { correlation_ = correlation; }
  void set_weight(double weight) { weight_ = weight; }
  void set_means(std::pair<double, double> means) { means_ = means; }
  void set_standard_deviations(std::pair<double, double> stdevs) {
    stdevs_ = stdevs;
  }
};

IMPMODELLER_END_NAMESPACE

#endif  /* IMPMODELLER_MULTIPLE_BINORMAL_RESTRAINT_H */
