/**
 *  \file Optimizer.cpp   \brief Base class for all optimizers.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/Optimizer.h"
#include "IMP/OptimizerState.h"
#include "IMP/internal/utility.h"
#include <limits>

IMP_BEGIN_NAMESPACE

namespace {
  unsigned int optimizer_index=0;
}

Optimizer::Optimizer(Model *m, std::string name)
{
  if (m) set_model(m);
  set_name(internal::make_object_name(name, optimizer_index++));
  set_was_used(true);
  min_score_= -std::numeric_limits<double>::max();
}

Optimizer::~Optimizer()
{
}

void Optimizer::update_states() const
{
  IMP_LOG(VERBOSE,
          "Updating OptimizerStates " << std::flush);
  for (OptimizerStateConstIterator it = optimizer_states_begin();
       it != optimizer_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->update();
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(VERBOSE, "done." << std::endl);
}

IMP_LIST_IMPL(Optimizer, OptimizerState, optimizer_state,
              OptimizerState*, OptimizerStates, {
                Optimizer::set_optimizer_state_optimizer(obj, this);
                obj->set_was_used(true);
              },{},
              {Optimizer::set_optimizer_state_optimizer(obj, NULL);});

void Optimizer::set_optimizer_state_optimizer(OptimizerState *os, Optimizer *o)
{
  os->set_optimizer(o);
}

IMP_END_NAMESPACE
