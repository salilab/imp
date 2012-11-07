/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container_base.h"
#include "IMP/internal/utility.h"
#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/internal/graph_utility.h"
#include "IMP/dependency_graph.h"

IMP_BEGIN_NAMESPACE

Container::Container(Model *m, std::string name):
  Constraint(m, name) {
  IMP_USAGE_CHECK(m, "Must pass model to container constructor.");
  changed_=false;
}

void Container::set_is_changed(bool tr) {
  changed_=tr;
}

void Container::do_after_evaluate(DerivativeAccumulator *) {
  changed_=false;
}

IMP_END_NAMESPACE
