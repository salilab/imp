/**
 * \file local_planners
 * \brief
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/local_planners.h>

IMPKINEMATICS_BEGIN_NAMESPACE

/***************** LocalPlanner **************/

LocalPlanner::LocalPlanner(Model* model, DOFsSampler* dofs_sampler)
    : Object("IMP_KINEMATICS_LOCALPLANNER"),
      model_(model),
      dofs_sampler_(dofs_sampler) {
}

/***************** PathLocalPlanner **************/

// default path sampling is linear
PathLocalPlanner::PathLocalPlanner(Model* model,
                                   DOFsSampler* dofs_sampler,
                                   DirectionalDOF* directional_dof,
                                   int save_step_interval)
    : LocalPlanner(model, dofs_sampler),
      d_(directional_dof),
      save_step_interval_(save_step_interval) {}

std::vector<DOFValues> PathLocalPlanner::plan(DOFValues q_from,
                                              DOFValues q_rand,
                                              ScoringFunction *sf) {
  std::vector<DOFValues> dofs_list;
  d_->set_end_points(q_from, q_rand);
  int step = 0;
  bool is_collision_detected = false;

  // iterate path nodes and check validity
  while (d_->get_value() < 1.0) {
    // Increase DOF value by delta and current_step_number_ by one. 
    // Do we increase each DOF by delta at each step?
    (*d_)++;
    //std::cout << is_valid(d_->get_dofs_values(), sf) << "  |  " << dofs_list.size() << " | SSI=" << save_step_interval_ << std::endl;
    if (!is_valid(d_->get_dofs_values(), sf)) {  // TODO
      is_collision_detected = true;
      (*d_)--;  // re-validate d to point to valid node
      break;
    }
    step++;
    // save node each save_step_interval
    if (save_step_interval_ > 0 && step % save_step_interval_ == 0) {
      dofs_list.push_back(d_->get_dofs_values());
    }
  }
  // handle q_rand separately to avoid floating point errors
  if (!is_collision_detected) {
    if (is_valid(q_rand, sf)) {
      dofs_list.push_back(q_rand);
    }
  }
      // add last valid path node in any case
      else {
    if (dofs_list.size() > 0 && d_->get_dofs_values() != dofs_list.back()) {
      dofs_list.push_back(d_->get_dofs_values());
    }
  }
  return dofs_list;
}

IMPKINEMATICS_END_NAMESPACE
