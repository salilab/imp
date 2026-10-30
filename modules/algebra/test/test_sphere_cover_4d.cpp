/**
 *  \file test_sphere_cover_4d.cpp
 *  \brief Test uniform covers of 4D spheres (refined with IMP.cgal, if
 *         available)
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/vector_generators.h>
#include <IMP/algebra/SphereD.h>
#include <IMP/check_macros.h>
#include <IMP/flags.h>
#include <IMP/random.h>
#include <cmath>

namespace {

using IMP::algebra::VectorD;
typedef IMP::Vector<VectorD<4> > Vector4Ds;

// All points are on the sphere, and (for a hemisphere cover) on its upper
// half; and no two points coincide
void check_cover(const Vector4Ds &vs, const IMP::algebra::SphereD<4> &s,
                 unsigned int n, bool upper) {
  IMP_ALWAYS_CHECK(vs.size() == n, "Wrong number of points: " << vs.size(),
                   IMP::ValueException);
  for (const VectorD<4> &v : vs) {
    VectorD<4> d = v - s.get_center();
    IMP_ALWAYS_CHECK(std::abs(d.get_magnitude() - s.get_radius()) < 1e-6,
                     "Point not on the sphere: " << v, IMP::ValueException);
    if (upper) {
      IMP_ALWAYS_CHECK(d[3] >= -1e-6, "Point not on the upper hemisphere: "
                       << v, IMP::ValueException);
    }
  }
  for (unsigned int i = 0; i < vs.size(); ++i) {
    for (unsigned int j = i + 1; j < vs.size(); ++j) {
      IMP_ALWAYS_CHECK((vs[i] - vs[j]).get_magnitude() > 1e-6,
                       "Coincident points " << i << " and " << j,
                       IMP::ValueException);
    }
  }
}
}  // namespace

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test uniform covers of 4D spheres");
  IMP::random_number_generator.seed(42);
  IMP::algebra::SphereD<4> unit(IMP::algebra::get_zero_vector_d<4>(), 1.0);
  IMP::algebra::SphereD<4> other(VectorD<4>(1.0, -2.0, 0.5, 3.0), 2.5);
  // Hemisphere covers need at least 4 points (the basis vectors), and
  // full sphere covers at least 8 (the basis vectors and their opposites),
  // or an even number of points
  for (unsigned int n : {4, 5, 10, 30, 60}) {
    for (const IMP::algebra::SphereD<4> &s : {unit, other}) {
      check_cover(IMP::algebra::get_uniform_upper_hemisphere_cover(s, n), s, n,
                  true);
      if (n >= 8) {
        check_cover(IMP::algebra::get_uniform_surface_cover(s, n), s, n,
                    false);
      }
    }
  }
  // Full sphere covers with fewer points than the basis vectors and their
  // opposites
  for (unsigned int n : {2, 4, 6}) {
    check_cover(IMP::algebra::get_uniform_surface_cover(unit, n), unit, n,
                false);
  }
  return 0;
}
