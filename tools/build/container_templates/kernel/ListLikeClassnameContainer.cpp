/**
 *  \file ListClassnameContainer.cpp   \brief A list of PLURALVARIABLETYPE.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kernel/internal/ListLikeClassnameContainer.h>
#include <IMP/kernel/ClassnameModifier.h>
#include <IMP/kernel/ClassnameScore.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

void ListLikeClassnameContainer
::do_show(std::ostream &out) const {
  out << "contains " << data_.size() << std::endl;
}



IMPKERNEL_END_INTERNAL_NAMESPACE
