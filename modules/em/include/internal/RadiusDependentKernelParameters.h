/**
 *  \file IMP/em/internal/RadiusDependentKernelParameters.h
 *  \brief Calculates kernel parameters as a function of a specific radius.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_INTERNAL_RADIUS_DEPENDENT_KERNEL_PARAMETERS_H
#define IMPEM_INTERNAL_RADIUS_DEPENDENT_KERNEL_PARAMETERS_H

#include <limits>

IMPEM_BEGIN_INTERNAL_NAMESPACE

//! Calculates kernel parameters as a function of a specific radius.
class RadiusDependentKernelParameters {
 public:
  RadiusDependentKernelParameters(float radii, float rsigsq, float timessig,
                                  float sq2pi3, float inv_rsigsq,
                                  float rnormfac, float rkdist) {
    IMP_USAGE_CHECK(std::abs(radii) < std::numeric_limits<float>::max(),
                    "Radius out of range");
    IMP_USAGE_CHECK(std::abs(rsigsq) < std::numeric_limits<float>::max(),
                    "rsigsq out of range");
    IMP_USAGE_CHECK(std::abs(timessig) < std::numeric_limits<float>::max(),
                    "timessig out of range");
    IMP_USAGE_CHECK(std::abs(sq2pi3) < std::numeric_limits<float>::max(),
                    "sq2pi3 out of range");
    IMP_USAGE_CHECK(std::abs(inv_rsigsq) < std::numeric_limits<float>::max(),
                    "inv_rsigsq out of range");
    IMP_USAGE_CHECK(std::abs(rnormfac) < std::numeric_limits<float>::max(),
                    "rnormfac out of range");
    IMP_USAGE_CHECK(std::abs(timessig) < std::numeric_limits<float>::max(),
                    "rkdist out of range");
    if (radii > EPS) {
      vsig_ = 1. / (sqrt(2.)) * radii;  // volume sigma
      vsigsq_ = vsig_ * vsig_;
      inv_sigsq_ = rsigsq + vsigsq_;
      sig_ = sqrt(inv_sigsq_);
      kdist_ = timessig * sig_;
      inv_sigsq_ = 1. / inv_sigsq_ * .5;
      normfac_ = sq2pi3 * 1. / (sig_ * sig_ * sig_);
    } else {
      inv_sigsq_ = inv_rsigsq;
      normfac_ = rnormfac;
      kdist_ = rkdist;
    }
  }

  //! Gets the value of the volume sigma
  inline float get_vsig() const { return vsig_; }
  //! Gets the value of volume sigma squared
  inline float get_vsigsq() const { return vsigsq_; }
  //! Gets the inverse sigma squared
  inline float get_inv_sigsq() const { return inv_sigsq_; }
  //! Gets the sigma value
  inline float get_sig() const { return sig_; }
  //! Gets the value of kdist parameter
  inline float get_kdist() const { return kdist_; }
  //! Gets the value of normfac parameter
  inline float get_normfac() const { return normfac_; }

private:
  //! vsig
  float vsig_;
  //! square of vsig
  float vsigsq_;
  //! the inverse of sigma square
  float inv_sigsq_;
  //! the sigma
  float sig_;
  //! the kernel distance (= elements for summation)
  float kdist_;
  //! normalization factor
  float normfac_;
};

IMPEM_END_INTERNAL_NAMESPACE

#endif /* IMPEM_INTERNAL_RADIUS_DEPENDENT_KERNEL_PARAMETERS_H */
