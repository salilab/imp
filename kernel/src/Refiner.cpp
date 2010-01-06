/**
 *  \file Refiner.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Refiner.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE

namespace {
  unsigned int next_index=0;
}


Refiner::Refiner(std::string name):
  Object(internal::make_object_name(name, next_index++)){
}

IMP_END_NAMESPACE
