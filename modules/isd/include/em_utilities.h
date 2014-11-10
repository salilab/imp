/**
 *  \file isd/em_utilities.h
 *  \brief Common scoring functions
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_EM_UTILITIES_H
#define IMPISD_EM_UTILITIES_H

#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/Gaussian3D.h>
#include <IMP/core/Gaussian.h>

IMPISD_BEGIN_NAMESPACE

Float score_gaussian_overlap(kernel::Model *m,
                             kernel::ParticleIndexPair pp,
                             IMP_Eigen::Vector3d &deriv){
  double determinant;
  bool invertible;
  IMP_Eigen::Matrix3d inverse = IMP_Eigen::Matrix3d::Zero();
  Float mass12 = atom::Mass(m,pp[0]).get_mass() *
                  atom::Mass(m,pp[1]).get_mass();
  core::Gaussian g1(m,pp[0]);
  core::Gaussian g2(m,pp[1]);
  IMP_Eigen::Matrix3d covar = g1.get_global_covariance() +
                               g2.get_global_covariance();
  IMP_Eigen::Vector3d v = IMP_Eigen::Vector3d(g2.get_coordinates().get_data())
    - IMP_Eigen::Vector3d(g1.get_coordinates().get_data());
  covar.computeInverseAndDetWithCheck(inverse,determinant,invertible);
  IMP_Eigen::Vector3d tmp = inverse*v;
  Float score = mass12 * 0.06349363593424097 / (std::sqrt(determinant)) *
    std::exp(-0.5*v.transpose()*tmp);
  deriv = -score*tmp;
  return score;
}

IMPISD_END_NAMESPACE

#endif  /* IMPISD_EM_UTILITIES_H */
