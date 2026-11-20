/**
 *  \file test_pca_angle.cpp
 *  \brief Test the angle calculation used by PCAFitRestraint.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/internal/pca_utils.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/flags.h>
#include <IMP/exception.h>
#include <IMP/constants.h>
#include <cmath>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test PCA angle calculation.");

  // Rounding can make the cosine of the angle between parallel vectors
  // slightly larger than 1 (or less than -1), which must not give NaN
  for (unsigned int i = 0; i < 10000; ++i) {
    IMP::algebra::Vector3D v = IMP::algebra::get_random_vector_in(
        IMP::algebra::BoundingBox3D(IMP::algebra::Vector3D(-10, -10, -10),
                                    IMP::algebra::Vector3D(10, 10, 10)));
    if (v.get_magnitude() < 1e-3) continue;
    double same = IMP::em::internal::get_angle(v, v);
    double opposite = IMP::em::internal::get_angle(v, -v);
    double scaled = IMP::em::internal::get_angle(v, v * 3.7);
    IMP_ALWAYS_CHECK(!std::isnan(same) && std::abs(same) < 1e-6,
                     "Angle between " << v << " and itself is " << same,
                     IMP::ValueException);
    IMP_ALWAYS_CHECK(!std::isnan(scaled) && std::abs(scaled) < 1e-6,
                     "Angle between parallel vectors is " << scaled,
                     IMP::ValueException);
    IMP_ALWAYS_CHECK(!std::isnan(opposite)
                     && std::abs(opposite - IMP::PI) < 1e-6,
                     "Angle between antiparallel vectors is " << opposite,
                     IMP::ValueException);
  }
  return EXIT_SUCCESS;
}
