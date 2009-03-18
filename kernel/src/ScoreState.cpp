/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/ScoreState.h"

#include <cmath>
#include <limits>

IMP_BEGIN_NAMESPACE

//! Constructor
ScoreState::ScoreState(std::string name) : name_(name)
{
  update_iteration_= std::numeric_limits<unsigned int>::max();
  after_iteration_= std::numeric_limits<unsigned int>::max();
}


void ScoreState::before_evaluate(unsigned int iter) {
  if (update_iteration_ != iter) {
    update_iteration_= iter;
    do_before_evaluate();
  }
}


void ScoreState::after_evaluate(unsigned int iter,
                                DerivativeAccumulator *da) {
  if (after_iteration_ != iter) {
    after_iteration_= iter;
    do_after_evaluate(da);
  }
}


void ScoreState::before_evaluate() {
  do_before_evaluate();
}


void ScoreState::after_evaluate(DerivativeAccumulator *da) {
  do_after_evaluate(da);
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

IMP_END_NAMESPACE
