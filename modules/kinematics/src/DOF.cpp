/**
 * \file DOF.cpp
 * \brief single degree of freedom
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/DOF.h>
#include <IMP/Object.h>

IMPKINEMATICS_BEGIN_NAMESPACE

DOF::DOF(double v) :
  Object("IMP_KINEMATICS_DOF") ,
  value_(v) {}

DOF::DOF(double v, double min, double max, double step_size) :
  Object("IMP_KINEMATICS_DOF"),
  value_(v), range_(std::make_pair(min, max)), step_size_(step_size) {}


int DOF::number_of_steps(double value) const {
  return number_of_steps(value_, value);
}

int DOF::number_of_steps(double value1, double value2) const {
  return std::ceil(std::fabs(value1-value2)/step_size_);
}

void DOF::do_show(std::ostream & os) const
{
  os << "(DOF " << value_ << " range [ " << range_.first
     << " : " << range_.second << " step size " << step_size_ << ")";
}
IMPKINEMATICS_END_NAMESPACE
