/**
 *  \file CLASSNAMEModifier.cpp   \brief A function on Particles.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/CLASSNAMEModifier.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/ModelObject.h"

IMPKERNEL_BEGIN_NAMESPACE

CLASSNAMEModifier::CLASSNAMEModifier(std::string name):
  Object(name){
}

IMP_INPUTS_DEF(CLASSNAMEModifier);
IMP_OUTPUTS_DEF(CLASSNAMEModifier);

IMPKERNEL_END_NAMESPACE
