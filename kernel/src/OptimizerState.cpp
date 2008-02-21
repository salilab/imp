/**
 *  \file OptimizerState.cpp \brief Shared optimizer state.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */


#include "IMP/log.h"
#include "IMP/OptimizerState.h"

namespace IMP
{

//! Constructor
OptimizerState::OptimizerState(std::string name) : name_(name)
{
  IMP_LOG(VERBOSE, "OptimizerState constructed " << name << std::endl);
}


//! Destructor
OptimizerState::~OptimizerState()
{
  IMP_LOG(VERBOSE, "OptimizerState deleted " << get_name() << std::endl);
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

  out << "version: " << version() << std::endl;
  out << "last_modified_by: " << last_modified_by() << std::endl;
}


}  // namespace IMP
