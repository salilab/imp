/**
 * \file directional_DOFs
 * \brief
 * \authors Dina Schneidman, Barak Raveh
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_DIRECTIONAL_DO_FS_H
#define IMPKINEMATICS_DIRECTIONAL_DO_FS_H

#include "DOF.h"

IMPKINEMATICS_BEGIN_NAMESPACE

/*
  this is a linear one.
  we may want to make it more general. this will require
  turning it into virtual class
*/
class IMPKINEMATICSEXPORT DirectionalDOF {
public:
  DirectionalDOF(const DOFs& dofs): dofs_(dofs) {}

  void set_end_points(const DOFValues& q1, const DOFValues& q2) {
    endpoint1_ = q1;
    endpoint2_ = q2;
    current_delta_.resize(dofs_.size());
    // TODO: check that the size of dofs_, q1 and q2 is the same
    // IMP_CHECK...

    // determine step size based on each DOF
    step_number_ = 0;
    for(unsigned int i=0; i<dofs_.size(); i++) {
      int dof_step_number =
        dofs_[i]->number_of_steps(endpoint1_[i], endpoint2_[i]);
      if(dof_step_number > step_number_)
        step_number_ = dof_step_number;
    }

    for(unsigned int i=0; i<dofs_.size(); i++) {
      current_delta_[i] = (endpoint2_[i] - endpoint1_[i])/step_number_;
    }

    current_step_number_ = 0;
    delta_ = 1.0/step_number_;
    value_ = 0.0;
  }

  // return current DOF
  DOFValues get_dofs_values() {
    DOFValues ret = endpoint1_;
    for(unsigned int i=0; i<ret.size(); i++) {
      ret[i]+= current_step_number_ * current_delta_[i];
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

protected:
  DOFs dofs_; // can be a pointer
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

 public:
  IMP_SHOWABLE_INLINE(DirectionalDOF, {
      out << "(direction dof from " << endpoint1_ << " to " << endpoint2_
          << " ; step number " << current_step_number_ << ")";
    });

};

IMP_VALUES(DirectionalDOF, DirectionalDOFs);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_DIRECTIONAL_DO_FS_H */
