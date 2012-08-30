/**
 *  \file ListCLASSNAMEContainer.cpp   \brief A list of PLURALVARIABLETYPE.
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DynamicListCLASSNAMEContainer.h"
#include "IMP/CLASSNAMEModifier.h"
#include "IMP/CLASSNAMEScore.h"
#include <IMP/internal/InternalListCLASSNAMEContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


DynamicListCLASSNAMEContainer
::DynamicListCLASSNAMEContainer(Container *m,  std::string name):
  P(m, name){
}

void DynamicListCLASSNAMEContainer::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out <<  get_number_of_FUNCTIONNAMEs()
      << " CLASSNAMEs" << std::endl;
}

IMPCONTAINER_END_NAMESPACE
