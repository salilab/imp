/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/log.h"
#include "IMP/ScoreState.h"
#include "IMP/Model.h"
#include "IMP/internal/utility.h"
#include <algorithm>
#include <cmath>
#include <limits>

IMP_BEGIN_NAMESPACE

ScoreState::ScoreState(std::string name) :
  ModelObject(name)
{
  order_=-1;
}

ScoreState::ScoreState(Model *m, std::string name) :
  ModelObject(m, name)
{
  order_=-1;
  // later change to just registering with ModelObject
  m->add_score_state(this);
}


void ScoreState::before_evaluate() {
  do_before_evaluate();
}

ModelObjectsTemp ScoreState::do_get_inputs() const {
  return ModelObjectsTemp(get_input_particles())
    + ModelObjectsTemp(get_input_containers());
}
ModelObjectsTemp ScoreState::do_get_outputs() const {
  return  ModelObjectsTemp(get_output_particles())
    + ModelObjectsTemp(get_output_containers());
}
void ScoreState::do_update_dependencies(const DependencyGraph &,
                                        const DependencyGraphVertexIndex &) {}

void ScoreState::after_evaluate(DerivativeAccumulator *da) {
  do_after_evaluate(da);
}


namespace {
struct CompOrder {
  bool operator()(const ScoreState*a,
                  const ScoreState*b) const {
    return a->order_ < b->order_;
  }
};
}


ScoreStatesTemp get_ordered_score_states( ScoreStatesTemp in) {
  if (in.empty()) return in;
  // make sure the order_ entries are up to date
  if (!in[0]->get_model()->get_has_dependencies()) {
    in[0]->get_model()->compute_dependencies();
  }
  std::sort(in.begin(), in.end(), CompOrder());
  return in;
}

IMP_END_NAMESPACE
