/**
 * \file IMP/em2d/internal/Projector.cpp
 * \brief projection generation class
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em2d/internal/Projector.h>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

Projector::Projector(const kernel::Particles& particles,
                     unsigned int projection_number,
                     double pixel_size,
                     double resolution) :
  particles_(particles),
  projection_number_(projection_number),
  pixel_size_(pixel_size),
  resolution_(resolution),
  projection_sphere_(projection_number) {

  for (unsigned int i = 0; i < particles_.size(); i++) {
    radii_.push_back(core::XYZR(particles[i]).get_radius());
    mass_.push_back(atom::Mass(particles[i]).get_mass());
  }
}

int Projector::estimate_image_size(const IMP::algebra::Vector3Ds& points) const {
  // estimate max image size
  IMP::algebra::Vector3D centroid = IMP::algebra::get_centroid(points);
  double max_rad2 = 0;
  for (unsigned int i = 0; i < points.size(); i++) {
    double dist2 = IMP::algebra::get_squared_distance(points[i], centroid);
    if (dist2 > max_rad2) max_rad2 = dist2;
  }

  // estimate max image size
  double max_dist =  2*sqrt(max_rad2); //compute_max_distance(points);
  double max_radius = *std::max_element(radii_.begin(), radii_.end());
  static IMP::em::KernelParameters kp(resolution_);
  const IMP::em::RadiusDependentKernelParameters& params =
    kp.get_params(max_radius);
  double wrap_length = 2 * params.get_kdist() + 1.0;
  int axis_size =
      (int)((max_dist + 2 * wrap_length + 2 * pixel_size_) / pixel_size_ + 2);
  return axis_size;
}

void Projector::compute_projections(boost::ptr_vector<Projection>& projections,
                                    int image_size) const {

  // get coordinates
  IMP::algebra::Vector3Ds points(particles_.size());
  for (unsigned int i = 0; i < particles_.size(); i++) {
    points[i] = core::XYZ(particles_[i]).get_coordinates();
  }

  int axis_size = estimate_image_size(points);
  if (axis_size <= image_size) axis_size = image_size;

  // storage for rotated points
  IMP::algebra::Vector3Ds rotated_points(points.size());
  IMP::algebra::Rotation3Ds rotations;
  IMP::algebra::Vector3Ds axes;
  projection_sphere_.get_all_rotations_and_axes(rotations, axes);

  for (unsigned int i = 0; i<rotations.size(); i++) {
    // rotate points
    for (unsigned int point_index = 0; point_index < points.size();
         point_index++) {
      rotated_points[point_index] = rotations[i] * points[point_index];
    }
    // project
    std::auto_ptr<Projection> p(new Projection(rotated_points, radii_, mass_,
                                               pixel_size_, resolution_,
                                               axis_size));
    p->set_rotation(rotations[i]);
    p->set_axis(axes[i]);
    p->set_id(i);
    projections.push_back(p.release());
  }
}

void Projector::compute_projections(const algebra::Vector3Ds& axis,
                                    double angle_thr,
                                    boost::ptr_vector<Projection>& projections,
                                    int image_size) const {

  // get coordinates
  IMP::algebra::Vector3Ds points(particles_.size());
  for (unsigned int i = 0; i < particles_.size(); i++) {
    points[i] = core::XYZ(particles_[i]).get_coordinates();
  }

  int axis_size = estimate_image_size(points);
  if (axis_size <= image_size) axis_size = image_size;

  // storage for rotated points
  IMP::algebra::Vector3Ds rotated_points(points.size());
  IMP::algebra::Rotation3Ds rotations;
  IMP::algebra::Vector3Ds axes;
  projection_sphere_.get_adjacent_rotations_and_axes(axis, angle_thr, rotations, axes);

  for (unsigned int i = 0; i<rotations.size(); i++) {
    // rotate points
    for (unsigned int point_index = 0; point_index < points.size();
         point_index++) {
      rotated_points[point_index] = rotations[i] * points[point_index];
    }
    // project
    std::auto_ptr<Projection> p(new Projection(rotated_points, radii_, mass_,
                                               pixel_size_, resolution_,
                                               axis_size));
    p->set_rotation(rotations[i]);
    p->set_axis(axes[i]);
    p->set_id(i);
    projections.push_back(p.release());
  }
}

IMPEM2D_END_INTERNAL_NAMESPACE
