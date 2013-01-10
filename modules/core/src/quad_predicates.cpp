/**
 *  \file QuadPredicate.cpp  \brief Define QuadPredicate
 *
 *  WARNING This file was generated from name_predicates.cc
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/quad_predicates.h>
#include <IMP/base/random.h>

IMPCORE_BEGIN_NAMESPACE

ConstantQuadPredicate::ConstantQuadPredicate(int v,
                                                 std::string name):
    QuadPredicate(name), v_(v){}

UnorderedTypeQuadPredicate::UnorderedTypeQuadPredicate(
                                                 std::string name):
    QuadPredicate(name){}

OrderedTypeQuadPredicate::OrderedTypeQuadPredicate(
                                                 std::string name):
    QuadPredicate(name){}


AllSameQuadPredicate::AllSameQuadPredicate(
    std::string name):
    QuadPredicate(name){}

CoinFlipQuadPredicate::CoinFlipQuadPredicate( double p,
    std::string name):
  QuadPredicate(name), p_(p), rng_(0.,1.){}


IMPCORE_END_NAMESPACE
