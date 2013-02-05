/**
 *  \file SingletonModifier.cpp   \brief A function on Particles.
 *
 *  WARNING This file was generated from NAMEModifier.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/SingletonModifier.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/ModelObject.h"

IMPKERNEL_BEGIN_NAMESPACE

SingletonModifier::SingletonModifier(std::string name):
  Object(name){
}

IMP_INPUTS_DEF(SingletonModifier);
IMP_OUTPUTS_DEF(SingletonModifier);

IMPKERNEL_END_NAMESPACE
