/**
 * \file DOF.cpp
 * \brief single degree of freedom
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/DOF.h>
#include <IMP/base/Object.h>

IMPKINEMATICS_BEGIN_NAMESPACE

DOF::DOF(double v) : Object("IMP_KINEMATICS_DOF"), value_(v) {}

DOF::DOF(double v, double min, double max, double step_size)
    : Object("IMP_KINEMATICS_DOF"),
      value_(v),
      range_(std::make_pair(min, max)),
      step_size_(step_size) {}

int DOF::get_number_of_steps(double value) const {
  return get_number_of_steps(value_, value);
}

int DOF::get_number_of_steps(double value1, double value2) const {
  return std::ceil(std::fabs(value1 - value2) / step_size_);
}

IMPKINEMATICS_END_NAMESPACE
