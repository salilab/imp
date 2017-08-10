/**
 *  \brief A distance restraint between protein termini.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/constants.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Scale.h>
#include <IMP/spb/DistanceTerminiRestraint.h>
#include <IMP/random.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
IMPSPB_BEGIN_NAMESPACE

DistanceTerminiRestraint::DistanceTerminiRestraint(
    IMP::Particle* nterm, IMP::Particle* cterm, IMP::Particle* reqd_dist_term,
    double sigma0_dist)
    : Restraint(nterm->get_model(), "DistanceTerminiRestraint%1%"),
      nterm_(nterm),
      cterm_(cterm),
      sigma0_dist_(sigma0_dist),
      reqd_dist_term_(reqd_dist_term){};

double DistanceTerminiRestraint::get_model_termini_distance() const {
  double dist =
      std::max(core::get_distance(core::XYZ(nterm_), core::XYZ(cterm_)),
               std::numeric_limits<double>::epsilon());
  // using sphere center to center distance

  return dist;
}

/* call for probability */
// Probability is a log-normal distribution

double DistanceTerminiRestraint::unprotected_evaluate(
    DerivativeAccumulator* accum) const {
  double score;

  double model_dist = get_model_termini_distance();

  double ref_dist = isd::Scale(reqd_dist_term_).get_scale();

  double log_eps = log(model_dist / ref_dist);

  double before_exp = 1. / sqrt(2. * IMP::PI) / sigma0_dist_ / model_dist;

  if (before_exp <= std::numeric_limits<double>::epsilon()) {
    before_exp = std::numeric_limits<double>::epsilon();
  }

  score = -1.0 * log(before_exp)  // inside log guaranteed to be positive
          + (log_eps * log_eps / (2. * sigma0_dist_ * sigma0_dist_));

  if (accum) {
  }

  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp DistanceTerminiRestraint::do_get_inputs() const {
  IMP::ParticlesTemp ret;
  ret.push_back(nterm_);
  ret.push_back(cterm_);
  ret.push_back(reqd_dist_term_);
  return ret;
}

IMPSPB_END_NAMESPACE
