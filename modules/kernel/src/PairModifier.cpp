/**
 *  \file PairModifier.cpp   \brief A function on Particles.
 *
 *  WARNING This file was generated from NAMEModifier.cc
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/PairModifier.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/ModelObject.h"

IMPKERNEL_BEGIN_NAMESPACE

PairModifier::PairModifier(std::string name):
  Object(name){
}

IMP_INPUTS_DEF(PairModifier);
IMP_OUTPUTS_DEF(PairModifier);

IMPKERNEL_END_NAMESPACE
