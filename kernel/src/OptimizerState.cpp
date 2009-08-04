/**
 *  \file OptimizerState.cpp \brief Shared optimizer state.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */


#include "IMP/log.h"
#include "IMP/OptimizerState.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE
namespace {
  unsigned int optimizerstate_index=0;
}

OptimizerState::OptimizerState(std::string name)
{
  set_name(internal::make_object_name(name, optimizerstate_index++));
}


//! Give accesss to optimizer.
/** \param[in] optimizer The optimizer this state will act on.
 */
void OptimizerState::set_optimizer(Optimizer* optimizer)
{
  optimizer_ = optimizer;
}


IMP_END_NAMESPACE
