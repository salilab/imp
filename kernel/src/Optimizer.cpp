/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/Optimizer.h"

namespace IMP
{

//! Constructor
Optimizer::Optimizer()
{
  IMP_LOG(VERBOSE, "MEMORY: Optimizer created " << this << std::endl);
}


//! Destructor
Optimizer::~Optimizer()
{
  IMP_LOG(VERBOSE, "MEMORY: Optimizer destroyed " << this << std::endl);
}

//! Update optimizer state, at each successful step
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

IMP_CONTAINER_IMPL(Optimizer, OptimizerState, optimizer_state,
                   OptimizerStateIndex, obj->set_optimizer(this));

}  // namespace IMP
