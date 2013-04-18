/**
 * \file DOFValues
 * \brief a simple class for storage of DOF values.
 *
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
    for(unsigned int i=0; i<dofs.size(); i++)
      push_back(dofs[i]->get_value());
  }

  // Empty Constructor
  DOFValues() {}

  // TODO: provide an option for more general distance definition
  // through RRT class
  double distance2(const DOFValues& other_dof_values) const {
    double dist=0.0;
    for(unsigned int i=0; i<size(); i++) {
      double diff = ((*this)[i] - other_dof_values[i]);
      dist += (diff*diff);
    }
    return dist/size();
  }

  double distance(const DOFValues& other_dof_values) const {
    return sqrt(distance2(other_dof_values));
  }

};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_DOF_VALUES_H */
