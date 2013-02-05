/**
 *  \file Refiner.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Refiner.h"
#include "IMP/kernel/internal/utility.h"

IMPKERNEL_BEGIN_NAMESPACE

Refiner::Refiner(std::string name):
  Object(name){
}

IMP_INPUTS_DEF(Refiner);

IMPKERNEL_END_NAMESPACE
