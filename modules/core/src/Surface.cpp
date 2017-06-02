/**
 *  \file Surface.cpp   \brief Simple surface decorator.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/Surface.h>
#include <IMP/score_functor/internal/surface_helpers.h>
#include <IMP/score_functor/internal/direction_helpers.h>
#include <IMP/core/direction.h>
#include <IMP/algebra/Rotation3D.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

void Surface::do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::Vector3D &center,
                                const algebra::Vector3D &normal) {
  if (!XYZ::get_is_setup(m, pi)) {
    XYZ::setup_particle(m, pi);
  }
  XYZ(m, pi).set_coordinates(center);
  if (!Direction::get_is_setup(m, pi)) {
    Direction::setup_particle(m, pi, normal);
  }
}

void Surface::do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::ReferenceFrame3D &rf) {
  algebra::Vector3D c = rf.get_global_coordinates(algebra::Vector3D(0, 0, 0));
  algebra::Vector3D n = rf.get_global_coordinates(algebra::Vector3D(0, 0, 1)) - c;
  do_setup_particle(m, pi, c, n);
}

bool Surface::get_is_setup(Model *m, ParticleIndex pi) {
  return (m->get_has_attribute(get_coordinate_key(2), pi) &&
          m->get_has_attribute(get_normal_key(2), pi));
}

FloatKey Surface::get_normal_key(unsigned int i) {
  IMP_USAGE_CHECK(i < 3, "Out of range coordinate");
  return score_functor::internal::get_direction_key(i);
}

algebra::Vector3D Surface::get_normal_derivatives() const {
  return Direction(get_particle()).get_direction_derivatives();
}

void Surface::add_to_normal_derivatives(const algebra::Vector3D &v,
                                        DerivativeAccumulator &d) {
  Direction(get_particle()).add_to_direction_derivatives(v, d);
}

bool Surface::get_normal_is_optimized() const {
  return Direction(get_particle()).get_direction_is_optimized();
}

void Surface::set_normal_is_optimized(bool tf) const {
  Direction(get_particle()).set_direction_is_optimized(tf);
}

algebra::Vector3D Surface::get_normal() const {
  return Direction(get_particle()).get_direction();
}

void Surface::set_normal(const algebra::Vector3D &normal) {
  Direction(get_particle()).set_direction(normal);
}

double Surface::get_height(const algebra::Vector3D &v) const {
  return score_functor::internal::get_height_above_surface(
    get_coordinates(), get_normal(), v, nullptr);
}

double Surface::get_depth(const algebra::Vector3D &v) const {
  return score_functor::internal::get_depth_below_surface(
    get_coordinates(), get_normal(), v, nullptr);
}

double Surface::get_distance_to(const algebra::Vector3D &v) const {
  return score_functor::internal::get_distance_from_surface(
    get_coordinates(), get_normal(), v, nullptr);
}

double Surface::get_distance_to_center(const algebra::Vector3D &v) const {
  return algebra::get_distance(get_coordinates(), v);
}

algebra::ReferenceFrame3D Surface::get_reference_frame() const {
  algebra::Rotation3D rot = algebra::get_rotation_taking_first_to_second(
      algebra::Vector3D(0, 0, 1), get_normal());
  algebra::Transformation3D trans = algebra::Transformation3D(rot, get_coordinates());
  return algebra::ReferenceFrame3D(trans);
}

void Surface::show(std::ostream &out) const {
    out << "Surface || " << get_coordinates() << " " << get_normal();
}


SurfaceGeometry::SurfaceGeometry(std::string n) : display::Geometry(n) {}
SurfaceGeometry::SurfaceGeometry(Surface s, const std::string n)
    : display::Geometry(n), s_(s)
    , n_(new display::SegmentGeometry(get_segment()))
    , c_(new display::CylinderGeometry(get_cylinder())) {}

SurfaceGeometry::SurfaceGeometry(Surface s, const display::Color &c, std::string n)
    : display::Geometry(c, n), s_(s)
    , n_(new display::SegmentGeometry(get_segment()))
    , c_(new display::CylinderGeometry(get_cylinder())) {}

void SurfaceGeometry::set_geometry(Surface s) {
  s_ = s;
  c_->set_geometry(get_cylinder());
  n_->set_geometry(get_segment());
}

algebra::Segment3D SurfaceGeometry::get_segment(double length) const {
  algebra::Vector3D c = s_.get_coordinates();
  algebra::Vector3D n = s_.get_normal();
  algebra::Segment3D seg = algebra::Segment3D(c, c + length * n);
  return seg;
}

algebra::Cylinder3D SurfaceGeometry::get_cylinder(double radius,
                                                  double thick) const {
  algebra::Vector3D c = s_.get_coordinates();
  algebra::Vector3D n = s_.get_normal();
  algebra::Vector3D v;
  for (unsigned int i=0; i < 3; ++i) {
    v[i] = c[i] - n[i] * thick;
  }
  algebra::Segment3D seg = algebra::Segment3D(v, c);
  algebra::Cylinder3D cyl = algebra::Cylinder3D(seg, radius);
  return cyl;
}

display::Geometries SurfaceGeometry::get_components() const {
  display::Geometries ret;
  ret.push_back(c_);
  ret.push_back(n_);
  return ret;
}


IMPCORE_END_NAMESPACE
