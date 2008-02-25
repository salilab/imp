/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/ScoreState.h"

#include <cmath>

namespace IMP
{

//! Constructor
ScoreState::ScoreState(std::string name) : name_(name)
{
  model_ = NULL;
  IMP_LOG(VERBOSE, "ScoreState constructed " << name << std::endl);
}


//! Destructor
ScoreState::~ScoreState()
{
  IMP_LOG(VERBOSE, "ScoreState deleted " << get_name() << std::endl);
}


//! Give accesss to model particle data.
/** \param[in] model_data All particle data in the model.
 */
void ScoreState::set_model(Model* model)
{
  model_ = model;
}


//! Show the state.
/** \param[in] out Stream to send state description to.
 */
void ScoreState::show(std::ostream& out) const
{
  out << "unknown state:" << std::endl;

  get_version_info().show(out);
}


}  // namespace IMP
