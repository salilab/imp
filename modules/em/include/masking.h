/**
 *  \file IMP/em/masking.h
 *  \brief masking tools
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_MASKING_H
#define IMPEM_MASKING_H

#include <IMP/em/em_config.h>
#include <IMP/base_types.h>
#include "DensityHeader.h"
#include "KernelParameters.h"
IMPEM_BEGIN_NAMESPACE
/** Mask based on the distance. */
class IMPEMEXPORT RadiusDependentDistanceMask {
  // class  RadiusDependentDistanceMask {
 public:
  RadiusDependentDistanceMask(float sampling_radius,
                              const DensityHeader &header);
  inline const std::vector<double> *get_neighbor_shift() const {
    return &neighbor_shift_;
  }
  inline const std::vector<double> *get_neighbor_dist() const {
    return &neighbor_dist_;
  }
  inline const std::vector<double> *get_neighbor_dist_exp() const {
    return &neighbor_dist_exp_;
  }

  void show(std::ostream &out) const { out << "RadiusDependentDistanceMask"; }

 protected:
  std::vector<double> neighbor_shift_;
  std::vector<double> neighbor_dist_;
  std::vector<double> neighbor_dist_exp_;
};

//! Calculates and stores a distance mask
class IMPEMEXPORT DistanceMask
    // class  DistanceMask
    {
 public:
  DistanceMask() { initialized_ = false; }

  DistanceMask(const DensityHeader *header) {
    header_ = header;
    initialized_ = true;
  }

  void show(std::ostream &out) const { out << "DistanceMask"; }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  //! Sets the parameters that depend on the radius of a given particle.
  const RadiusDependentDistanceMask *set_mask(float radius);

  //! Finds the precomputed mask given a particle radius.
  /**
    \param[in] radius searching for parameters of this radius
    \param[in] eps used for numerical stability
    \note The parameters are indexes by the radius. To maintain
    numeratical stability, look for a radius within +-eps from the
    queried radius.
   \note the function return nullptr and writes a warning if parameters
   for this radius were not found.
  */
  const RadiusDependentDistanceMask *get_mask(float radius,
                                              float eps = 0.001) const;
#endif
  bool is_mask_set(float radius, float eps = 0.001) {
    return get_mask(radius, eps) != nullptr;
  }

 protected:
  typedef std::map<float, const RadiusDependentDistanceMask *> MASK_MAP;
  bool initialized_;
  MASK_MAP masks_;
  const DensityHeader *header_;
  KernelParameters kernel_params_;
};

IMPEM_END_NAMESPACE
#endif /* IMPEM_MASKING_H */
