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

ScoreState::ScoreState(std::string name) :
  ModelObject(name)
{
  order_=-1;
}

ScoreState::ScoreState(Model *m, std::string name) :
  ModelObject(m, name)
{
  order_=-1;
}


void ScoreState::before_evaluate() {
  do_before_evaluate();
}

void ScoreState::after_evaluate(DerivativeAccumulator *da) {
  do_after_evaluate(da);
}

ScoreState::~ScoreState() {
}

void ScoreState::do_update_dependencies() {}

namespace {
struct CompOrder {
  bool operator()(const ScoreState*a,
                  const ScoreState*b) const {
    IMP_INTERNAL_CHECK(a->order_ != -1 && b->order_!= -1,
                    "No order assigned yet.");
    return a->order_ < b->order_;
  }
};
}

ScoreStatesTemp get_update_order( ScoreStatesTemp in) {
  IMP_FUNCTION_LOG;
  if (in.empty()) return in;
  std::sort(in.begin(), in.end());
  in.erase(std::unique(in.begin(), in.end()), in.end());
  // make sure the order_ entries are up to date
  if (!in[0]->get_model()->get_has_dependencies()) {
    in[0]->get_model()->compute_dependencies();
  }
  std::sort(in.begin(), in.end(), CompOrder());
  IMP_LOG_TERSE( "Order: " << in << std::endl);
  return in;
}

#if IMP_HAS_DEPRECATED

 ParticlesTemp ScoreState::get_input_particles() const {
   IMP_DEPRECATED_FUNCTION(get_inputs());
   return IMP::kernel::get_input_particles(get_inputs());
 }
ContainersTemp ScoreState::get_input_containers() const {
  IMP_DEPRECATED_FUNCTION(get_inputs());
  return IMP::kernel::get_input_containers(get_inputs());
}
ParticlesTemp ScoreState::get_output_particles() const {
  IMP_DEPRECATED_FUNCTION(get_outputs());
  return IMP::kernel::get_output_particles(get_outputs());
}
ContainersTemp ScoreState::get_output_containers() const {
  IMP_DEPRECATED_FUNCTION(get_outputs());
  return IMP::kernel::get_output_containers(get_outputs());
}
#endif
IMPKERNEL_END_NAMESPACE
