/**
 *  \file IMP/em/KernelParameters.h
 *  \brief Calculates and stores Gaussian kernel parameters.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_KERNEL_PARAMETERS_H
#define IMPEM_KERNEL_PARAMETERS_H
#include "def.h"
#include <IMP/base_types.h>
#include <boost/scoped_array.hpp>
#include <IMP/exception.h>
#include <IMP/log.h>
#include <map>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include <limits>
#include <IMP/em/internal/RadiusDependentKernelParameters.h>

IMPEM_BEGIN_NAMESPACE

//! Calculates and stores Gaussian kernel parameters
class IMPEMEXPORT KernelParameters {
 public:
  KernelParameters() { initialized_ = false; }

  KernelParameters(float resolution) {
    init(resolution);
    initialized_ = true;
  }

  ~KernelParameters();

  //! Get sigma as a function of the resolution according to the
  //! full width at half maximum criterion
  inline float get_rsig() const { return rsig_; }

  //! Get squared sigma as a function of the resolution according to the
  //! full width at half maximum criterion
  inline float get_rsigsq() const { return rsigsq_; }

  //! Get the inverse of sigma squared
  inline float get_inv_rsigsq() const { return inv_rsigsq_; }

  //! Gets the number of sigma used.
  /**
    \note We use 3, which means that 99% of the density is considered
   */
  inline float get_timessig() const { return timessig_; }

  //! Get the non-sigma portion of the Gaussian normalization factor
  inline float get_sq2pi3() const { return sq2pi3_; }

  //! Get the Gaussian normalization factor
  inline float get_rnormfac() const { return rnormfac_; }

  //! Get the length of the Gaussian (sigma*number_of_sigmas_used)
  inline float get_rkdist() const { return rkdist_; }

  //! Get the squared length of the Gaussian (sigma*number_of_sigmas_used)
  inline float get_rkdistsq() const { return rkdistsq_; }

  //! Gets the value of lim parameter
  inline float get_lim() const { return lim_; }

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  //! Get radius dependent kernel parameters
  /**
      \param[in] radius searching for parameters of this radius
      \param[in] eps tolerance around radius to search for
      \note cached values are used if available for radius +/- eps
   */
  const internal::RadiusDependentKernelParameters& get_params(float radius,
                                                         float eps = 0.001);
#endif

  IMP_SHOWABLE_INLINE(KernelParameters, out << "rsig: " << rsig_ << std::endl;);

 protected:
  float timessig_; // 3.0 is used by default
  float rsig_, rsigsq_, inv_rsigsq_; // resolution dependent sigma
  float sq2pi3_, rnormfac_;
  float rkdist_, rkdistsq_, lim_;
  bool initialized_;
  typedef std::map<float, const internal::RadiusDependentKernelParameters *>
                    KernelMap;
  KernelMap radii2params_;

  void init(float resolution);
};

IMP_VALUES(KernelParameters, KernelParametersList);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class IMPEMEXPORT Kernel3D {
 public:
  Kernel3D() {}
  Kernel3D(const Kernel3D& other) {
    size_ = other.size_;
    dim_ext_ = other.dim_ext_;
    data_.reset(new double[size_]);
    std::copy(other.data_.get(), other.data_.get() + size_, data_.get());
  }
  Kernel3D(int size, int ext) {
    size_ = size;
    dim_ext_ = ext;
    data_.reset(new double[size_]);
    IMP_INTERNAL_CHECK(data_, "Can not allocate vector\n");
    for (int i = 0; i < size_; i++) data_[i] = 0.;
  }
  double* get_data() const { return data_.get(); }
  int get_size() const { return size_; }
  int get_extent() const { return dim_ext_; }

 protected:
  boost::scoped_array<double> data_;
  int size_;
  int dim_ext_;
};
IMPEMEXPORT
Kernel3D create_3d_gaussian(double sigma, double sigma_factor);
IMPEMEXPORT
Kernel3D create_3d_laplacian();
//! Truncate a kernel according to an input sigma
IMPEMEXPORT
Kernel3D get_truncated(double* data, int extent, double sigmap,
                       double sigma_fac);
#endif
IMPEM_END_NAMESPACE

#endif /* IMPEM_KERNEL_PARAMETERS_H */
