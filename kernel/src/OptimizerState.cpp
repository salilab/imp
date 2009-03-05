/**
 *  \file OptimizerState.cpp \brief Shared optimizer state.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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


//! Show the state.
/** \param[in] out Stream to send state description to.
 */
void OptimizerState::show(std::ostream& out) const
{
  out << "unknown state:" << std::endl;

  get_version_info().show(out);
}

IMP_END_NAMESPACE
