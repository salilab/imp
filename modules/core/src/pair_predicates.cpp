/**
 *  \file PairPredicate.cpp  \brief Define PairPredicate
 *
 *  WARNING This file was generated from name_predicates.cc
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/core/pair_predicates.h>
#include <IMP/base/random.h>

IMPCORE_BEGIN_NAMESPACE

ConstantPairPredicate::ConstantPairPredicate(int v,
                                                 std::string name):
    PairPredicate(name), v_(v){}

UnorderedTypePairPredicate::UnorderedTypePairPredicate(
                                                 std::string name):
    PairPredicate(name){}

OrderedTypePairPredicate::OrderedTypePairPredicate(
                                                 std::string name):
    PairPredicate(name){}


AllSamePairPredicate::AllSamePairPredicate(
    std::string name):
    PairPredicate(name){}

CoinFlipPairPredicate::CoinFlipPairPredicate( double p,
    std::string name):
  PairPredicate(name), p_(p), rng_(0.,1.){}


IMPCORE_END_NAMESPACE
