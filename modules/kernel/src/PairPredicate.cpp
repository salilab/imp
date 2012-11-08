/**
 *  \file PairPredicate.cpp  \brief Define PairPredicate
 *
 *  WARNING This file was generated from NAMEPredicate.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/PairPredicate.h>
#include <IMP/functor.h>
IMP_BEGIN_NAMESPACE

PairPredicate::PairPredicate(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairPredicate
     symbols are present in the kernel DSO */
}

void PairPredicate::remove_if_equal(Model *m,
                                         ParticleIndexPairs& ps,
                                         int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_equal(this, m, value)),
           ps.end());

}

void PairPredicate::remove_if_not_equal(Model *m,
                                             ParticleIndexPairs& ps,
                                             int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_not_equal(this, m, value)),
           ps.end());
}

IMP_INPUTS_DEF(PairPredicate);

IMP_END_NAMESPACE
