/**
 * \file IMP/kinematics/directional_DOFs.h
 * \brief
 * \authors Dina Schneidman, Barak Raveh
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_DIRECTIONAL_DO_FS_H
#define IMPKINEMATICS_DIRECTIONAL_DO_FS_H

#include "DOF.h"

IMPKINEMATICS_BEGIN_NAMESPACE

/**
  this is a linear one.
  \note We may want to make it more general. This will require
  turning it into a virtual class.
*/
class IMPKINEMATICSEXPORT DirectionalDOF : public IMP::Object {
 public:
  DirectionalDOF(const DOFs& dofs) : Object("DirectionalDOF%1%"), dofs_(dofs) {}

  void set_end_points(const DOFValues& q1, const DOFValues& q2) {
    endpoint1_ = q1;
    endpoint2_ = q2;
    current_delta_.resize(dofs_.size());
    // TODO: check that the size of dofs_, q1 and q2 is the same
    // IMP_CHECK...

    // determine step size based on each DOF
    // Total number of steps between endpoint1 and endpoint2
    // determined by the number of steps between the largest DOF
    step_number_ = 0;
    for (unsigned int i = 0; i < dofs_.size(); i++) {
      int dof_step_number =
          dofs_[i]->get_number_of_steps(endpoint1_[i], endpoint2_[i]);
      if (dof_step_number > step_number_) step_number_ = dof_step_number;
    }
    // current_delta_ is the delta in values of q between start and endpoints
    for (unsigned int i = 0; i < dofs_.size(); i++) {
      current_delta_[i] = (endpoint2_[i] - endpoint1_[i]) / step_number_;
    }

    current_step_number_ = 0;
    delta_ = 1.0 / step_number_;
    value_ = 0.0;
  }

  // return current DOF
  DOFValues get_dofs_values() {
    DOFValues ret = endpoint1_;
    for (unsigned int i = 0; i < ret.size(); i++) {
      ret[i] += current_step_number_ * current_delta_[i];
    }
    return ret;
  }

  double get_value() const { return value_; }

#ifndef SWIG
  void operator++(int) {
    current_step_number_++;
    value_ += delta_;
  }
  void operator--(int) {
    current_step_number_--;
    value_ -= delta_;
  }
#endif

  IMP_OBJECT_METHODS(DirectionalDOF);

 protected:
  DOFs dofs_;  // can be a pointer
  DOFValues endpoint1_;
  DOFValues endpoint2_;

  // step size for each DOF for endpoint1 and endpoint2
  DOFValues current_delta_;

  // step number based on current_delta_
  int step_number_;

  // counter for step_number_
  int current_step_number_;

  // follows curr_step_number_ between 0 and 1
  double value_;

  // delta for value
  double delta_;
};

IMP_OBJECTS(DirectionalDOF, DirectionalDOFs);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_DIRECTIONAL_DO_FS_H */
