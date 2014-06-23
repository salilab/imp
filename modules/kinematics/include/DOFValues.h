/**
 * \file IMP/kinematics/DOFValues.h
 * \brief a simple class for storage of DOF values.
 *
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_DOF_VALUES_H
#define IMPKINEMATICS_DOF_VALUES_H

#include "DOF.h"

IMPKINEMATICS_BEGIN_NAMESPACE

/*
  A class that hold DOF values for DOFs
 */
class IMPKINEMATICSEXPORT DOFValues : public std::vector<double> {
 public:
  // Constructor from DOFs
  DOFValues(const DOFs& dofs) {
    reserve(dofs.size());
    for (unsigned int i = 0; i < dofs.size(); i++)
      push_back(dofs[i]->get_value());
  }

  // Empty Constructor
  DOFValues() {}

  // TODO: provide an option for more general distance definition
  // through RRT class
  double get_distance2(const DOFValues& other_dof_values) const {
    double dist = 0.0;
    for(unsigned int i=0; i<size(); i++) {
      double diff1 = ((*this)[i] - other_dof_values[i]);
      double diff2 = std::fabs(diff1 - 2.0*IMP::algebra::PI);
      double diff3 = std::fabs(diff1 + 2.0*IMP::algebra::PI);
      double diff = std::min(std::fabs(diff1), std::min(diff2, diff3));
      dist += (diff*diff);

    }
    return dist / size();
  }

  double get_distance(const DOFValues& other_dof_values) const {
    return sqrt(get_distance2(other_dof_values));
  }

 public:
  IMP_SHOWABLE_INLINE(DOFValues, {
    out << "[";
    if (size() > 0) {
      out << operator[](0);
    }
    for (unsigned int i = 1; i < size(); i++) {
      out << "," << operator[](i);
    }
    out << "]";
  });
};

IMP_VALUES(DOFValues, DOFValuesList);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_DOF_VALUES_H */
