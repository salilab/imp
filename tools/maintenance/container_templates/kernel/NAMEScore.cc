/**
 *  \file CLASSNAMEScore.cpp  \brief Define CLASSNAMEScore
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/CLASSNAMEScore.h>
#include <IMP/internal/utility.h>
#include <IMP/Restraint.h>
#include <IMP/LCCLASSNAME_macros.h>
#include <IMP/internal/TupleRestraint.h>
IMP_BEGIN_NAMESPACE

CLASSNAMEScore::CLASSNAMEScore(std::string name):
  Object(name)
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}


Restraints
CLASSNAMEScore
::create_current_decomposition(Model *m,
                               PASSINDEXTYPE vt) const {
  return internal::create_score_current_decomposition(this,
                                                      m, vt);
}

IMP_INPUTS_DEF(CLASSNAMEScore);

IMP_END_NAMESPACE
