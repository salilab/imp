/**
 *  \file Mover.cpp \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/MonteCarloMover.h"

IMPCORE_BEGIN_NAMESPACE

MonteCarloMover::MonteCarloMover(kernel::Model *m, std::string name)
    : kernel::ModelObject(m, name), has_move_(false) {
  reset_statistics();
}

IMPCORE_END_NAMESPACE
