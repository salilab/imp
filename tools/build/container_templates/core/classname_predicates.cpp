/**
 *  \file ClassnamePredicate.cpp  \brief Define ClassnamePredicate
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#include <IMP/core/HELPERNAME_predicates.h>
#include <IMP/core/ClassnameRestraint.h>
#include <IMP/core/ClassnameConstraint.h>
#include <IMP/random.h>

IMPCORE_BEGIN_NAMESPACE

ConstantClassnamePredicate::ConstantClassnamePredicate(int v, std::string name)
    : ClassnamePredicate(name), v_(v) {}

UnorderedTypeClassnamePredicate::UnorderedTypeClassnamePredicate(
    std::string name)
    : ClassnamePredicate(name) {}

OrderedTypeClassnamePredicate::OrderedTypeClassnamePredicate(std::string name)
    : ClassnamePredicate(name) {}

AllSameClassnamePredicate::AllSameClassnamePredicate(std::string name)
    : ClassnamePredicate(name) {}

CoinFlipClassnamePredicate::CoinFlipClassnamePredicate(double p,
                                                       std::string name)
    : ClassnamePredicate(name), p_(p), rng_(0., 1.) {}

IMP_OBJECT_SERIALIZE_IMPL(IMP::core::ClassnameRestraint);
IMP_OBJECT_SERIALIZE_IMPL(IMP::core::ClassnameConstraint);
IMP_OBJECT_SERIALIZE_IMPL(IMP::core::ConstantClassnamePredicate);
IMP_OBJECT_SERIALIZE_IMPL(IMP::core::UnorderedTypeClassnamePredicate);
IMP_OBJECT_SERIALIZE_IMPL(IMP::core::AllSameClassnamePredicate);

IMPCORE_END_NAMESPACE
