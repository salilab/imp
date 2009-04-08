/**
 *  \file OptimizerState.cpp \brief Shared optimizer state.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */


#include "IMP/log.h"
#include "IMP/OptimizerState.h"

IMP_BEGIN_NAMESPACE

//! Constructor
OptimizerState::OptimizerState()
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
