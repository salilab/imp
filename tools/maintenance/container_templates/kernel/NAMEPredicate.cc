/**
 *  \file CLASSNAMEPredicate.cpp  \brief Define CLASSNAMEPredicate
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/kernel/CLASSNAMEPredicate.h>
#include <IMP/kernel/functor.h>
IMPKERNEL_BEGIN_NAMESPACE

CLASSNAMEPredicate::CLASSNAMEPredicate(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairPredicate
     symbols are present in the kernel DSO */
}

void CLASSNAMEPredicate::remove_if_equal(Model *m,
                                         PLURALINDEXTYPE& ps,
                                         int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_equal(this, m, value)),
           ps.end());

}

void CLASSNAMEPredicate::remove_if_not_equal(Model *m,
                                             PLURALINDEXTYPE& ps,
                                             int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_not_equal(this, m, value)),
           ps.end());
}

IMP_INPUTS_DEF(CLASSNAMEPredicate);

IMPKERNEL_END_NAMESPACE
