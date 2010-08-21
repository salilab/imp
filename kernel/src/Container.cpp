/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container_base.h"
#include "IMP/internal/utility.h"
#include "IMP/Particle.h"

IMP_BEGIN_NAMESPACE

namespace {
  unsigned int restraint_index=0;
}

Container::Container(Model *m, std::string name):
  Object(name),
  m_(m)
{
}

bool Container::is_ok(Particle *p) {
  return p && p->get_model()==m_;
}


Model *Container::get_model(Particle *p) {return p->get_model();}

IMP_END_NAMESPACE
