/**
 * \file local_planners
 * \brief
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/local_planners.h>

IMPKINEMATICS_BEGIN_NAMESPACE

std::vector<DOFValues> PathLocalPlanner::plan(DOFValues q_from,
                                              DOFValues q_rand) {
  std::vector<DOFValues> dofs_list;
  d_->set_end_points(q_from, q_rand);
  int step=0;
  bool is_collision_detected = false;

  // iterate path nodes and check validity
  while (d_->get_value() < 1.0) {
    (*d_)++;
    if( ! is_valid( d_->get_dofs_values() ) ){ // TODO
      is_collision_detected = true;
      (*d_)--; // re-validate d to point to valid node
      break;
    }
    step++;
    // save node each save_step_interval
    if( save_step_interval_ > 0 &&
        step % save_step_interval_ == 0 ) {
      dofs_list.push_back( d_->get_dofs_values() );
    }
  }
  // handle q_rand separately to avoid floating point errors
  if(! is_collision_detected) {
    if( is_valid( q_rand ) ){
      dofs_list.push_back( q_rand );
    }
  }
  // add last valid path node in any case
  else {
    if(dofs_list.size() > 0 &&
       d_->get_dofs_values() != dofs_list.back()) {
      dofs_list.push_back( d_->get_dofs_values() );
    }
  }
  return dofs_list;
}

IMPKINEMATICS_END_NAMESPACE
