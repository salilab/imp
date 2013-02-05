/**
 *  \file QuadPredicate.cpp  \brief Define QuadPredicate
 *
 *  WARNING This file was generated from NAMEPredicate.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/kernel/QuadPredicate.h>
#include <IMP/kernel/functor.h>
IMPKERNEL_BEGIN_NAMESPACE

QuadPredicate::QuadPredicate(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairPredicate
     symbols are present in the kernel DSO */
}

void QuadPredicate::remove_if_equal(Model *m,
                                         ParticleIndexQuads& ps,
                                         int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_equal(this, m, value)),
           ps.end());

}

void QuadPredicate::remove_if_not_equal(Model *m,
                                             ParticleIndexQuads& ps,
                                             int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_not_equal(this, m, value)),
           ps.end());
}

IMP_INPUTS_DEF(QuadPredicate);

IMPKERNEL_END_NAMESPACE
