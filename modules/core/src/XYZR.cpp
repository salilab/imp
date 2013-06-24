/**
 *  \file XYZR.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/XYZR.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/vector_generators.h>

IMPCORE_BEGIN_NAMESPACE

void XYZR::show(std::ostream &out) const {
  out << "(" << get_x() << ", " << get_y() << ", " << get_z() << ": "
      << get_radius() << ")";
}

algebra::Sphere3D get_enclosing_sphere(const XYZs &v) {
  algebra::Sphere3Ds ss(v.size());
  for (unsigned int i = 0; i < v.size(); ++i) {
    XYZ d(v[i]);
    Float r = 0;
    if (v[i].get_particle()->has_attribute(XYZR::get_radius_key())) {
      r = v[i].get_particle()->get_value(XYZR::get_radius_key());
    }
    ss[i] = algebra::Sphere3D(d.get_coordinates(), r);
  }
  algebra::Sphere3D s = algebra::get_enclosing_sphere(ss);
  return s;
}

void set_enclosing_sphere(XYZR out, const XYZs &v, double slack) {
  algebra::Sphere3D s = get_enclosing_sphere(v);
  algebra::Sphere3D s2(s.get_center(), s.get_radius() + slack);
  out.set_sphere(s2);
}

void set_enclosing_radius(XYZR out, const XYZs &v) {
  double r = 0;
  for (unsigned int i = 0; i < v.size(); ++i) {
    if (XYZR::get_is_setup(v[i])) {
      XYZR d(v[i]);
      double dist = get_distance(static_cast<XYZ>(out), static_cast<XYZ>(d));
      dist += d.get_radius();
      r = std::max(dist, r);
    } else {
      XYZ d(v[i]);
      double dist = get_distance(static_cast<XYZ>(out), static_cast<XYZ>(d));
      r = std::max(dist, r);
    }
  }
  out.set_radius(r);
}

XYZRs create_xyzr_particles(Model *m, unsigned int num, Float radius,
                            Float box_side) {
  XYZRs ret;
  for (unsigned int i = 0; i < num; ++i) {
    Particle *p = new Particle(m);
    XYZR d = XYZR::setup_particle(p);
    d.set_coordinates(algebra::get_random_vector_in(
        algebra::BoundingBox3D(algebra::get_ones_vector_d<3>(-box_side),
                               algebra::get_ones_vector_d<3>(box_side))));
    d.set_radius(radius);
    d.set_coordinates_are_optimized(true);
    ret.push_back(d);
  }
  return ret;
}

namespace {
bool check_radius(Model *m, ParticleIndex pi) {
  XYZR d(m, pi);
  if (d.get_radius() < 0) {
    IMP_THROW("Invalid radius: " << d.get_radius(), ValueException);
  }
  return true;
}
}
IMP_CHECK_DECORATOR(XYZR, check_radius);

IMPCORE_END_NAMESPACE
