/**
 *  \file ClassnamePredicate.cpp  \brief Define ClassnamePredicate
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/HELPERNAME_predicates.h>
#include <IMP/base/random.h>

IMPCORE_BEGIN_NAMESPACE

ConstantClassnamePredicate::ConstantClassnamePredicate(int v,
                                                 std::string name):
    ClassnamePredicate(name), v_(v){}

UnorderedTypeClassnamePredicate::UnorderedTypeClassnamePredicate(
                                                 std::string name):
    ClassnamePredicate(name){}

OrderedTypeClassnamePredicate::OrderedTypeClassnamePredicate(
                                                 std::string name):
    ClassnamePredicate(name){}


AllSameClassnamePredicate::AllSameClassnamePredicate(
    std::string name):
    ClassnamePredicate(name){}

CoinFlipClassnamePredicate::CoinFlipClassnamePredicate( double p,
    std::string name):
  ClassnamePredicate(name), p_(p), rng_(0.,1.){}


IMPCORE_END_NAMESPACE
