/**
 * \file IMP/em2d/internal/ProjectionSphere.h
 * \brief A class for generation and storage of
 * projection directions on a sphere
 *
 * \authors Dina Schneidman
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_INTERNAL_PROJECTION_SPHERE_H
#define IMPEM2D_INTERNAL_PROJECTION_SPHERE_H

#include <IMP/em2d/em2d_config.h>
#include <IMP/algebra/SphericalVector3D.h>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

class IMPEM2DEXPORT ProjectionSphere {
public:
  ProjectionSphere(unsigned int n, double r = 1.0);

  void get_all_rotations_and_axes(IMP::algebra::Rotation3Ds& rotations,
                                  IMP::algebra::Vector3Ds& axes) const {
    rotations = rotations_;
    axes = axes_;
  }

  void get_adjacent_rotations_and_axes(const IMP::algebra::Vector3Ds& curr_axes,
                                       double angle_thr,
                                       IMP::algebra::Rotation3Ds& rotations,
                                       IMP::algebra::Vector3Ds& new_axes) const;

 private:
  void quasi_evenly_spherical_distribution(unsigned int n,
                                           IMP::algebra::SphericalVector3Ds& vs,
                                           double r);
 private:
  IMP::algebra::Rotation3Ds rotations_;
  IMP::algebra::Vector3Ds axes_;
  typedef IMP::algebra::DenseGrid3D<IMP::Ints> Grid;
  Grid grid_;

};

IMPEM2D_END_INTERNAL_NAMESPACE

#endif /* IMPEM2D_INTERNAL_PROJECTION_SPHERE_H */
