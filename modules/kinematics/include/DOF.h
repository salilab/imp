/**
 * \file IMP/kinematics/DOF.h
 * \brief single degree of freedom
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_DO_F_H
#define IMPKINEMATICS_DO_F_H

#include <IMP/kinematics/kinematics_config.h>
#include <IMP/Object.h>
#include <IMP/object_macros.h>

IMPKINEMATICS_BEGIN_NAMESPACE

//! Representation of one degree of freedom (DOF).
/**
  A general class for representing one degree of freedom (DOF) that is
  a double.  the class holds min/max range for the DOF, as well as the
  step size for passing the DOFs values around. It relies on a
  DOFValues class that only holds DOF value for each DOF

  @note Typically, a DOF would correspond to one joint, with the exception
  of transformation joint that has 6 DOFs (though collective joints
  such as normal modes might be supported in principle)

 */
class IMPKINEMATICSEXPORT DOF : public IMP::Object {
  IMP_OBJECT_METHODS(DOF);

 public:
  // maybe delete this one, only leave the next constructor?
  DOF(double v);

  DOF(double v, double min, double max, double step_size);

  double get_value() const { return value_; }

  void set_value(double v) { value_ = v; }

  std::pair<double, double> get_range() const { return range_; }

  void set_range(std::pair<double, double> range) { range_ = range; }

  //! Set step size for going in the direction of the sampled value
  double get_step_size() const { return step_size_; }

  void set_step_size(double step_size) { step_size_ = step_size; }

  //! Return the number of steps between the input value and current DOF value
  int get_number_of_steps(double value) const;

  //! Return the number of steps between between the two values
  int get_number_of_steps(double value1, double value2) const;

 private:
  // initial value of DOF, the sampling samples around it
  double value_;

  // sampling range
  // TODO: provide an option for multiple ranges?
  std::pair<double, double> range_;

  // step size
  double step_size_;
};

IMP_OBJECTS(DOF, DOFs);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_DO_F_H */
