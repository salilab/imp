/**
 *  \file PairScore.cpp  \brief Define PairScore
 *
 *  WARNING This file was generated from NAMEScore.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/PairScore.h>
#include <IMP/internal/utility.h>
#include <IMP/Restraint.h>
#include <IMP/pair_macros.h>
#include <IMP/internal/TupleRestraint.h>
IMP_BEGIN_NAMESPACE

PairScore::PairScore(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}

double PairScore::evaluate(const ParticlePair& vt,
                                DerivativeAccumulator *da) const {
  return evaluate_index(internal::get_model(vt),
                        internal::get_index(vt),
                        da);
}

// old versions of gcc don't like having the pragma inside the function
// hopefully these also have the _Pragma bug.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
double PairScore::evaluate_index(Model *m, const ParticleIndexPair& vt,
                                      DerivativeAccumulator *da) const {
  // see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53469
  IMP_DEPRECATED_IGNORE( return evaluate(internal::get_particle(m, vt), da));
}
#pragma GCC diagnostic pop

double PairScore::evaluate_indexes(Model *m,
                                        const ParticleIndexPairs &o,
                                        DerivativeAccumulator *da,
                                        unsigned int lower_bound,
                                        unsigned int upper_bound) const {
  double ret=0;
  for (unsigned int i=lower_bound; i< upper_bound; ++i) {
    ret+= evaluate_index(m, o[i], da);
  }
  return ret;
}


double PairScore::evaluate_if_good_index(Model *m,
                                              const ParticleIndexPair& vt,
                                              DerivativeAccumulator *da,
                                              double max)
  const {
  IMP_UNUSED(max);
  return evaluate_index(m, vt, da);
}

double PairScore::evaluate_if_good_indexes(Model *m,
                                                const ParticleIndexPairs &o,
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
PairScore
::create_current_decomposition(Model *m,
                               const ParticleIndexPair& vt) const {
  return internal::create_score_current_decomposition(this,
                                                      m, vt);
}

IMP_INPUTS_DEF(PairScore);

IMP_END_NAMESPACE
