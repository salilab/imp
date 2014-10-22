/**
 *  \file IMP/em2d/internal/ProjectionSphere.cpp
 *  \brief A class for generation and storage of
 * projection directions on a sphere
 *
 *  \authors Dina Schneidman
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em2d/internal/ProjectionSphere.h>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

ProjectionSphere::ProjectionSphere(unsigned int n, double r) :
  grid_(0.1, // voxel size
        IMP::algebra::BoundingBox3D(IMP::algebra::Vector3D(-1.01, -1.01, -1.01),
                                    IMP::algebra::Vector3D(1.01, 1.01, 1.01))) {

  IMP::algebra::SphericalVector3Ds spherical_coords;
  quasi_evenly_spherical_distribution(n, spherical_coords, r);

  // convert sphere coordinate to rotation and axis
  for (unsigned int i = 0; i < spherical_coords.size(); i++) {
    IMP::algebra::SphericalVector3D v = spherical_coords[i];
    double cy = cos(v[1] / 2.0);
    double cz = cos(v[2] / 2.0);
    double sy = sin(v[1] / 2.0);
    double sz = sin(v[2] / 2.0);
    // this is a rotation about z axis by an angle v[2]
    // followed by rotation about y axis by an angle v[1]
    IMP::algebra::Rotation3D r(cy * cz, sy * sz, sy * cz, cy * sz);
    rotations_.push_back(r);
    axes_.push_back(v.get_cartesian_coordinates());
  }

  // fill grid
  for (unsigned int i = 0; i < axes_.size(); i++) {
    Grid::Index grid_index = grid_.get_nearest_index(axes_[i]);
    grid_[grid_index].push_back(i);
  }
}

void ProjectionSphere::get_adjacent_rotations_and_axes(const IMP::algebra::Vector3Ds& curr_axes,
                                                       double angle_thr,
                                                       IMP::algebra::Rotation3Ds& rotations,
                                                       IMP::algebra::Vector3Ds& axes) const {

  // convert angle threshold into euclidean distance
  double radius = 2*sin(angle_thr/2);
  double radius2 = radius*radius;

  for (unsigned int i = 0; i < curr_axes.size(); i++) {
    IMP::algebra::BoundingBox3D bb(curr_axes[i]);
    bb += radius;
    Grid::ExtendedIndex lb = grid_.get_extended_index(bb.get_corner(0)),
                        ub = grid_.get_extended_index(bb.get_corner(1));

    for (Grid::IndexIterator it = grid_.indexes_begin(lb, ub);
       it != grid_.indexes_end(lb, ub); ++it) {
      for (unsigned int vIndex = 0; vIndex < grid_[*it].size(); vIndex++) {
        int direction_index = grid_[*it][vIndex];
        double dist2 = algebra::get_squared_distance(curr_axes[i],
                                                     axes_[direction_index]);
        if (dist2 <= radius2) {
          rotations.push_back(rotations_[direction_index]);
          axes.push_back(axes_[direction_index]);
        }
      }
    }
  }
}

void ProjectionSphere::quasi_evenly_spherical_distribution(unsigned int N,
                                                           IMP::algebra::SphericalVector3Ds& vs,
                                                           double r) {
  vs.resize(N);
  double theta, psi;
  for (unsigned long k = 1; k <= N; ++k) {
    double h = (2.0 * (k - 1)) / (N - 1) - 1.0;
    theta = std::acos(h);
    if (k == 1 || k == N) {
      psi = 0;
    } else {
      psi = (vs[k - 2][2] + 3.6 / sqrt((double)N * (1.0 - h * h)));
      int div = psi / (2 * IMP::PI);
      psi -= div * 2 * IMP::PI;
    }
    // Set the values of the spherical vector
    vs[k - 1][0] = r;
    vs[k - 1][1] = theta;
    vs[k - 1][2] = psi;
  }
}

IMPEM2D_END_INTERNAL_NAMESPACE
