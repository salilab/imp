/**
 *  \file ClassnamePredicate.cpp  \brief Define ClassnamePredicate
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/kernel/ClassnamePredicate.h>
#include <IMP/kernel/functor.h>
IMPKERNEL_BEGIN_NAMESPACE

ClassnamePredicate::ClassnamePredicate(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairPredicate
     symbols are present in the kernel DSO */
}

void ClassnamePredicate::remove_if_equal(Model *m,
                                         PLURALINDEXTYPE& ps,
                                         int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_equal(this, m, value)),
           ps.end());

}

void ClassnamePredicate::remove_if_not_equal(Model *m,
                                             PLURALINDEXTYPE& ps,
                                             int value) const {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          make_predicate_not_equal(this, m, value)),
           ps.end());
}

IMP_INPUTS_DEF(ClassnamePredicate);

IMPKERNEL_END_NAMESPACE
