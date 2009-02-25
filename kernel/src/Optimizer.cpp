/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/Optimizer.h"
#include "IMP/OptimizerState.h"

IMP_BEGIN_NAMESPACE

Optimizer::Optimizer()
{
  IMP_LOG(MEMORY, "MEMORY: Optimizer created " << this << std::endl);
}


Optimizer::~Optimizer()
{
  IMP_LOG(MEMORY, "MEMORY: Optimizer destroyed " << this << std::endl);
}


void Optimizer::update_states()
{
  IMP_LOG(VERBOSE,
          "Updating OptimizerStates " << std::flush);
  for (OptimizerStateIterator it = optimizer_states_begin();
       it != optimizer_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->update();
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(VERBOSE, "done." << std::endl);
}

IMP_LIST_IMPL(Optimizer, OptimizerState, optimizer_state,
              OptimizerState*, obj->set_optimizer(this),,
              obj->set_optimizer(NULL));

IMP_END_NAMESPACE
