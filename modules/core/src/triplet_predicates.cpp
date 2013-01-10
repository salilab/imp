/**
 *  \file TripletPredicate.cpp  \brief Define TripletPredicate
 *
 *  WARNING This file was generated from name_predicates.cc
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/triplet_predicates.h>
#include <IMP/base/random.h>

IMPCORE_BEGIN_NAMESPACE

ConstantTripletPredicate::ConstantTripletPredicate(int v,
                                                 std::string name):
    TripletPredicate(name), v_(v){}

UnorderedTypeTripletPredicate::UnorderedTypeTripletPredicate(
                                                 std::string name):
    TripletPredicate(name){}

OrderedTypeTripletPredicate::OrderedTypeTripletPredicate(
                                                 std::string name):
    TripletPredicate(name){}


AllSameTripletPredicate::AllSameTripletPredicate(
    std::string name):
    TripletPredicate(name){}

CoinFlipTripletPredicate::CoinFlipTripletPredicate( double p,
    std::string name):
  TripletPredicate(name), p_(p), rng_(0.,1.){}


IMPCORE_END_NAMESPACE
