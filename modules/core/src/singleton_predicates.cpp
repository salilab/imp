/**
 *  \file SingletonPredicate.cpp  \brief Define SingletonPredicate
 *
 *  WARNING This file was generated from name_predicates.cc
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/singleton_predicates.h>
#include <IMP/base/random.h>

IMPCORE_BEGIN_NAMESPACE

ConstantSingletonPredicate::ConstantSingletonPredicate(int v,
                                                 std::string name):
    SingletonPredicate(name), v_(v){}

UnorderedTypeSingletonPredicate::UnorderedTypeSingletonPredicate(
                                                 std::string name):
    SingletonPredicate(name){}

OrderedTypeSingletonPredicate::OrderedTypeSingletonPredicate(
                                                 std::string name):
    SingletonPredicate(name){}


AllSameSingletonPredicate::AllSameSingletonPredicate(
    std::string name):
    SingletonPredicate(name){}

CoinFlipSingletonPredicate::CoinFlipSingletonPredicate( double p,
    std::string name):
  SingletonPredicate(name), p_(p), rng_(0.,1.){}


IMPCORE_END_NAMESPACE
