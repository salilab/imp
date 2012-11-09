/**
 *  \file TripletScore.cpp  \brief Define TripletScore
 *
 *  WARNING This file was generated from NAMEScore.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/TripletScore.h>
#include <IMP/internal/utility.h>
#include <IMP/Restraint.h>
#include <IMP/triplet_macros.h>
#include <IMP/internal/TupleRestraint.h>
IMP_BEGIN_NAMESPACE

TripletScore::TripletScore(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}

double TripletScore::evaluate(const ParticleTriplet& vt,
                                DerivativeAccumulator *da) const {
  return evaluate_index(internal::get_model(vt),
                        internal::get_index(vt),
                        da);
}

double TripletScore::evaluate_index(Model *m, const ParticleIndexTriplet& vt,
                                      DerivativeAccumulator *da) const {
  // see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53469
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  IMP_DEPRECATED_IGNORE( return evaluate(internal::get_particle(m, vt), da));
#pragma GCC diagnostic pop
}

double TripletScore::evaluate_indexes(Model *m,
                                        const ParticleIndexTriplets &o,
                                        DerivativeAccumulator *da,
                                        unsigned int lower_bound,
                                        unsigned int upper_bound) const {
  double ret=0;
  for (unsigned int i=lower_bound; i< upper_bound; ++i) {
    ret+= evaluate_index(m, o[i], da);
  }
  return ret;
}


double TripletScore::evaluate_if_good_index(Model *m,
                                              const ParticleIndexTriplet& vt,
                                              DerivativeAccumulator *da,
                                              double max)
  const {
  IMP_UNUSED(max);
  return evaluate_index(m, vt, da);
}

double TripletScore::evaluate_if_good_indexes(Model *m,
                                                const ParticleIndexTriplets &o,
                                                DerivativeAccumulator *da,
                                                double max,
                                                unsigned int lower_bound,
                                                unsigned int upper_bound)
  const {
  double ret=0;
  for (unsigned int i=lower_bound; i< upper_bound; ++i) {
    double cur= evaluate_if_good_index(m, o[i], da, max-ret);
    max-=cur;
    ret+=cur;
    if (max<0) break;
  }
  return ret;
}


Restraints
TripletScore
::create_current_decomposition(Model *m,
                               const ParticleIndexTriplet& vt) const {
  return internal::create_score_current_decomposition(this,
                                                      m, vt);
}

IMP_INPUTS_DEF(TripletScore);

IMP_END_NAMESPACE
