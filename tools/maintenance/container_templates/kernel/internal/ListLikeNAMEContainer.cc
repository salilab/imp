/**
 *  \file ListCLASSNAMEContainer.cpp   \brief A list of PLURALVARIABLETYPE.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kernel/internal/ListLikeCLASSNAMEContainer.h>
#include <IMP/kernel/CLASSNAMEModifier.h>
#include <IMP/kernel/CLASSNAMEScore.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

void ListLikeCLASSNAMEContainer
::do_show(std::ostream &out) const {
  out << "contains " << data_.size() << std::endl;
}



IMPKERNEL_END_INTERNAL_NAMESPACE
