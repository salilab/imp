/**
 *  \file ScoreState.cpp \brief Shared score state.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base//log.h"
#include "IMP/kernel/ScoreState.h"
#include "IMP/kernel/Model.h"
#include "IMP/kernel/container_base.h"
#include "IMP/kernel/input_output.h"
#include "IMP/kernel/internal/utility.h"
#include <algorithm>
#include <cmath>
#include <limits>

IMPKERNEL_BEGIN_NAMESPACE

ScoreState::ScoreState(std::string name) : ModelObject(name) {
  IMPKERNEL_DEPRECATED_METHOD_DEF(
      2.1, "Use the ScoreState constructor that takes the model and a name.");
  order_ = -1;
}
ScoreState::ScoreState(Model *m) : ModelObject(m, "ScoreState%1%") {
  IMPKERNEL_DEPRECATED_METHOD_DEF(
      2.1, "Use the ScoreState constructor that takes the model and a name.");
  order_ = -1;
  m->set_has_dependencies(false);
}
ScoreState::ScoreState(Model *m, std::string name) : ModelObject(m, name) {
  order_ = -1;
  m->set_has_dependencies(false);
}

void ScoreState::before_evaluate() { do_before_evaluate(); }

void ScoreState::after_evaluate(DerivativeAccumulator *da) {
  do_after_evaluate(da);
}

ScoreState::~ScoreState() {
  if (get_model()) {
    // make sure I get removed from dependency graph
    get_model()->set_has_dependencies(false);
  }
}

namespace {
struct CompOrder {
  bool operator()(const ScoreState *a, const ScoreState *b) const {
    IMP_INTERNAL_CHECK(a->order_ != -1 && b->order_ != -1,
                       "No order assigned yet.");
    return a->order_ < b->order_;
  }
};
}

ScoreStatesTemp get_update_order(ScoreStatesTemp in) {
  IMP_FUNCTION_LOG;
  if (in.empty()) return in;
  std::sort(in.begin(), in.end());
  in.erase(std::unique(in.begin(), in.end()), in.end());
  // make sure the order_ entries are up to date
  if (!in[0]->get_model()->get_has_dependencies()) {
    in[0]->get_model()->compute_dependencies();
  }
  std::sort(in.begin(), in.end(), CompOrder());
  IMP_LOG_TERSE("Order: " << in << std::endl);
  return in;
}

void ScoreState::do_set_model(Model *m) {
  if (m) {
    m->set_has_dependencies(false);
  }
}

ParticlesTemp ScoreState::get_input_particles() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_inputs() instead");
  return IMP::kernel::get_input_particles(get_inputs());
}
ContainersTemp ScoreState::get_input_containers() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_inputs() instead");
  return IMP::kernel::get_input_containers(get_inputs());
}
ParticlesTemp ScoreState::get_output_particles() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_outputs() instead");
  return IMP::kernel::get_output_particles(get_outputs());
}
ContainersTemp ScoreState::get_output_containers() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1, "Use get_outputs() instead");
  return IMP::kernel::get_output_containers(get_outputs());
}

IMPKERNEL_END_NAMESPACE
