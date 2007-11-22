/**
 *  \file State.cpp \brief Shared state.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/ModelData.h"
#include "IMP/log.h"
#include "IMP/State.h"

namespace IMP
{

//! Constructor
State::State(std::string name) : name_(name)
{
  model_ = NULL;
  IMP_LOG(VERBOSE, "State constructed");
}


//! Destructor
State::~State()
{
  IMP_LOG(VERBOSE, "State deleted");
}


//! Give accesss to model particle data.
/** \param[in] model_data All particle data in the model.
 */
void State::set_model(Model* model)
{
  model_ = model;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void State::show(std::ostream& out) const
{
  out << "unknown state:" << std::endl;

  out << "version: " << version() << std::endl;
  out << "last_modified_by: " << last_modified_by() << std::endl;
}


}  // namespace IMP
