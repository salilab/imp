/**
 *  \file OptimizerState.cpp \brief Shared optimizer state.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */


#include "IMP/log.h"
#include "IMP/OptimizerState.h"
#include "IMP/internal/utility.h"

IMP_BEGIN_NAMESPACE

OptimizerState::OptimizerState(std::string name): Object(name)
{
}


//! Give accesss to optimizer.
/** \param[in] optimizer The optimizer this state will act on.
 */
void OptimizerState::set_optimizer(Optimizer* optimizer)
{
  optimizer_ = optimizer;
}


IMP_END_NAMESPACE
