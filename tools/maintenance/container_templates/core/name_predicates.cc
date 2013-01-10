/**
 *  \file CLASSNAMEPredicate.cpp  \brief Define CLASSNAMEPredicate
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/HELPERNAME_predicates.h>
#include <IMP/base/random.h>

IMPCORE_BEGIN_NAMESPACE

ConstantCLASSNAMEPredicate::ConstantCLASSNAMEPredicate(int v,
                                                 std::string name):
    CLASSNAMEPredicate(name), v_(v){}

UnorderedTypeCLASSNAMEPredicate::UnorderedTypeCLASSNAMEPredicate(
                                                 std::string name):
    CLASSNAMEPredicate(name){}

OrderedTypeCLASSNAMEPredicate::OrderedTypeCLASSNAMEPredicate(
                                                 std::string name):
    CLASSNAMEPredicate(name){}


AllSameCLASSNAMEPredicate::AllSameCLASSNAMEPredicate(
    std::string name):
    CLASSNAMEPredicate(name){}

CoinFlipCLASSNAMEPredicate::CoinFlipCLASSNAMEPredicate( double p,
    std::string name):
  CLASSNAMEPredicate(name), p_(p), rng_(0.,1.){}


IMPCORE_END_NAMESPACE
