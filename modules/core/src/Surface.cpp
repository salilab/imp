/**
 *  \file Surface.cpp   \brief Simple surface decorator.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/Surface.h>
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
  algebra::Vector3D u = normal.get_unit_vector();
  m->add_attribute(get_normal_key(0), pi, u[0]);
  m->add_attribute(get_normal_key(1), pi, u[1]);
  m->add_attribute(get_normal_key(2), pi, u[2]);
}

void Surface::do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::ReferenceFrame3D &rf) {
  algebra::Vector3D c = rf.get_global_coordinates(algebra::Vector3D(0, 0, 0));
  algebra::Vector3D n = rf.get_global_coordinates(algebra::Vector3D(0, 0, 1)) - c;
  do_setup_particle(m, pi, c, n);
}

bool Surface::get_is_setup(Model *m, ParticleIndex pi) {
  return m->get_has_attribute(get_normal_key(2), pi);
}

FloatKey Surface::get_normal_key(unsigned int i) {
  IMP_USAGE_CHECK(i < 3, "Out of range coordinate");
  static const FloatKey normal_keys[] = {FloatKey("normal_x"),
                                         FloatKey("normal_y"),
                                         FloatKey("normal_z")};
  return normal_keys[i];
}

algebra::Vector3D Surface::get_normal_derivatives() const {
  return algebra::Vector3D(get_normal_derivative(0),
                           get_normal_derivative(1),
                           get_normal_derivative(2));
}

void Surface::add_to_normal_derivatives(const algebra::Vector3D &v,
                                        DerivativeAccumulator &d) {
  add_to_normal_derivative(0, v[0], d);
  add_to_normal_derivative(1, v[1], d);
  add_to_normal_derivative(2, v[2], d);
}

bool Surface::get_normal_is_optimized() const {
  return get_particle()->get_is_optimized(get_normal_key(0)) &&
         get_particle()->get_is_optimized(get_normal_key(1)) &&
         get_particle()->get_is_optimized(get_normal_key(2));
}

void Surface::set_normal_is_optimized(bool tf) const {
  get_particle()->set_is_optimized(get_normal_key(0), tf);
  get_particle()->set_is_optimized(get_normal_key(1), tf);
  get_particle()->set_is_optimized(get_normal_key(2), tf);
}

algebra::Vector3D Surface::get_normal() const {
  Model *m = get_model();
  ParticleIndex pi = get_particle_index();
  return algebra::Vector3D(m->get_attribute(get_normal_key(0), pi),
                           m->get_attribute(get_normal_key(1), pi),
                           m->get_attribute(get_normal_key(2), pi));
}

void Surface::set_normal(const algebra::Vector3D &normal) {
  Model *m = get_model();
  ParticleIndex pi = get_particle_index();
  algebra::Vector3D u = normal.get_unit_vector();
  m->set_attribute(get_normal_key(0), pi, u[0]);
  m->set_attribute(get_normal_key(1), pi, u[1]);
  m->set_attribute(get_normal_key(2), pi, u[2]);
}

double Surface::get_height(const algebra::Vector3D &v) const {
  return get_normal() * (v - get_coordinates());
}

double Surface::get_depth(const algebra::Vector3D &v) const {
  return -get_height(v);
}

double Surface::get_distance_to(const algebra::Vector3D &v) const {
  return std::abs(get_height(v));
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
    , c_(new display::CylinderGeometry(get_cylinder()))
    , n_(new display::SegmentGeometry(get_segment())) {}
SurfaceGeometry::SurfaceGeometry(Surface s, const display::Color &c, std::string n)
    : display::Geometry(c, n), s_(s)
    , c_(new display::CylinderGeometry(get_cylinder()))
    , n_(new display::SegmentGeometry(get_segment())) {}

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
