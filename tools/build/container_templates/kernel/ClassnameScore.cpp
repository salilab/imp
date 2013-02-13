/**
 *  \file ClassnameScore.cpp  \brief Define ClassnameScore
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/kernel/ClassnameScore.h>
#include <IMP/kernel/internal/utility.h>
#include <IMP/kernel/Restraint.h>
#include <IMP/kernel/classname_macros.h>
#include <IMP/kernel/internal/TupleRestraint.h>
IMPKERNEL_BEGIN_NAMESPACE

ClassnameScore::ClassnameScore(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}

double ClassnameScore::evaluate(ARGUMENTTYPE vt,
                                DerivativeAccumulator *da) const {
  return evaluate_index(internal::get_model(vt),
                        internal::get_index(vt),
                        da);
}

// old versions of gcc don't like having the pragma inside the function
IMP_DEPRECATED_IGNORE(
double ClassnameScore::evaluate_index(Model *m, PASSINDEXTYPE vt,
                                      DerivativeAccumulator *da) const {
  // see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53469
   return evaluate(internal::get_particle(m, vt), da);
})

double ClassnameScore::evaluate_indexes(Model *m,
                                        const PLURALINDEXTYPE &o,
                                        DerivativeAccumulator *da,
                                        unsigned int lower_bound,
                                        unsigned int upper_bound) const {
  double ret=0;
  for (unsigned int i=lower_bound; i< upper_bound; ++i) {
    ret+= evaluate_index(m, o[i], da);
  }
  return ret;
}


double ClassnameScore::evaluate_if_good_index(Model *m,
                                              PASSINDEXTYPE vt,
                                              DerivativeAccumulator *da,
                                              double max)
  const {
  IMP_UNUSED(max);
  return evaluate_index(m, vt, da);
}

double ClassnameScore::evaluate_if_good_indexes(Model *m,
                                                const PLURALINDEXTYPE &o,
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
ClassnameScore
::do_create_current_decomposition(Model *m,
                                  PASSINDEXTYPE vt) const {
  double score=evaluate_index(m, vt, nullptr);
  if (score==0) {
    return Restraints();
  } else {
    return Restraints(1, IMP::kernel::internal::create_tuple_restraint(this,
                                                               m,
                                                               vt,
                                                               get_name()));
  }
}

Restraints
ClassnameScore
::create_current_decomposition(Model *m,
                               PASSINDEXTYPE vt) const {
  return do_create_current_decomposition(m, vt);
}

IMP_INPUTS_DEF(ClassnameScore);

IMPKERNEL_END_NAMESPACE
