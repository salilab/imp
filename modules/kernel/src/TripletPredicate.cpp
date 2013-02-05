/**
 *  \file TripletPredicate.cpp  \brief Define TripletPredicate
 *
 *  WARNING This file was generated from NAMEPredicate.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/kernel/TripletPredicate.h>
#include <IMP/kernel/functor.h>
IMPKERNEL_BEGIN_NAMESPACE

TripletPredicate::TripletPredicate(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairPredicate
     symbols are present in the kernel DSO */
}

void TripletPredicate::remove_if_equal(Model *m,
                                         ParticleIndexTriplets& ps,
                                         int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_equal(this, m, value)),
           ps.end());

}

void TripletPredicate::remove_if_not_equal(Model *m,
                                             ParticleIndexTriplets& ps,
                                             int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_not_equal(this, m, value)),
           ps.end());
}

IMP_INPUTS_DEF(TripletPredicate);

IMPKERNEL_END_NAMESPACE
