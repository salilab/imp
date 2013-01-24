/**
 *  \file QuadScore.cpp  \brief Define QuadScore
 *
 *  WARNING This file was generated from NAMEScore.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/QuadScore.h>
#include <IMP/internal/utility.h>
#include <IMP/Restraint.h>
#include <IMP/quad_macros.h>
#include <IMP/internal/TupleRestraint.h>
IMP_BEGIN_NAMESPACE

QuadScore::QuadScore(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}

double QuadScore::evaluate(const ParticleQuad& vt,
                                DerivativeAccumulator *da) const {
  return evaluate_index(internal::get_model(vt),
                        internal::get_index(vt),
                        da);
}

// old versions of gcc don't like having the pragma inside the function
IMP_DEPRECATED_IGNORE(
double QuadScore::evaluate_index(Model *m, const ParticleIndexQuad& vt,
                                      DerivativeAccumulator *da) const {
  // see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53469
   return evaluate(internal::get_particle(m, vt), da);
})

double QuadScore::evaluate_indexes(Model *m,
                                        const ParticleIndexQuads &o,
                                        DerivativeAccumulator *da,
                                        unsigned int lower_bound,
                                        unsigned int upper_bound) const {
  double ret=0;
  for (unsigned int i=lower_bound; i< upper_bound; ++i) {
    ret+= evaluate_index(m, o[i], da);
  }
  return ret;
}


double QuadScore::evaluate_if_good_index(Model *m,
                                              const ParticleIndexQuad& vt,
                                              DerivativeAccumulator *da,
                                              double max)
  const {
  IMP_UNUSED(max);
  return evaluate_index(m, vt, da);
}

double QuadScore::evaluate_if_good_indexes(Model *m,
                                                const ParticleIndexQuads &o,
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
QuadScore
::do_create_current_decomposition(Model *m,
                                  const ParticleIndexQuad& vt) const {
  double score=evaluate_index(m, vt, nullptr);
  if (score==0) {
    return Restraints();
  } else {
    return Restraints(1, IMP::internal::create_tuple_restraint(this,
                                                               m,
                                                               vt,
                                                               get_name()));
  }
}

Restraints
QuadScore
::create_current_decomposition(Model *m,
                               const ParticleIndexQuad& vt) const {
  return do_create_current_decomposition(m, vt);
}

IMP_INPUTS_DEF(QuadScore);

IMP_END_NAMESPACE
