/**
 *  \file ListCLASSNAMEContainer.cpp   \brief A list of PLURALVARIABLETYPE.
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/internal/ListLikeCLASSNAMEContainer.h>
#include <IMP/CLASSNAMEModifier.h>
#include <IMP/CLASSNAMEScore.h>

IMP_BEGIN_INTERNAL_NAMESPACE

void ListLikeCLASSNAMEContainer
::do_show(std::ostream &out) const {
  out << "contains " << data_.size() << std::endl;
}



IMP_END_INTERNAL_NAMESPACE
