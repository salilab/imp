/**
 *  \file ClassnameModifier.cpp   \brief A function on Particles.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/ClassnameModifier.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/ModelObject.h"

IMPKERNEL_BEGIN_NAMESPACE

ClassnameModifier::ClassnameModifier(std::string name):
  Object(name){
}

IMP_INPUTS_DEF(ClassnameModifier);
IMP_OUTPUTS_DEF(ClassnameModifier);

IMPKERNEL_END_NAMESPACE
