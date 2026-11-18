/**
 *  \file test_cache_lookup.cpp
 *  \brief Test lookups in the kernel parameter and distance mask caches.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/KernelParameters.h>
#include <IMP/em/masking.h>
#include <IMP/em/DensityHeader.h>
#include <IMP/flags.h>
#include <IMP/exception.h>
#include <cmath>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test kernel parameter cache lookups.");

  // A lookup on an empty cache (or for a radius larger than any cached
  // radius) must not dereference map::end()
  {
    IMP::em::KernelParameters kp(3.0);
    const IMP::em::internal::RadiusDependentKernelParameters &p =
        kp.get_params(0.f);
    IMP_ALWAYS_CHECK(std::abs(p.get_normfac() - kp.get_rnormfac()) < 1e-6,
                     "Wrong parameters returned for radius 0",
                     IMP::ValueException);
    const IMP::em::internal::RadiusDependentKernelParameters &p2 =
        kp.get_params(5.f);
    const IMP::em::internal::RadiusDependentKernelParameters &p3 =
        kp.get_params(10.f);
    IMP_ALWAYS_CHECK(&p2 != &p3 && &p2 == &kp.get_params(5.f),
                     "Kernel parameter cache returned wrong entry",
                     IMP::ValueException);
    IMP_ALWAYS_CHECK(std::abs(p3.get_vsig() - 10. / std::sqrt(2.)) < 1e-4,
                     "Wrong parameters returned for radius 10",
                     IMP::ValueException);
  }

  {
    IMP::em::DensityHeader h;
    IMP::em::DistanceMask dm(&h);
    IMP_ALWAYS_CHECK(!dm.is_mask_set(0.f),
                     "Empty mask cache should not contain a mask",
                     IMP::ValueException);
    IMP_ALWAYS_CHECK(!dm.is_mask_set(1e6f),
                     "Empty mask cache should not contain a mask",
                     IMP::ValueException);
  }
  return EXIT_SUCCESS;
}
