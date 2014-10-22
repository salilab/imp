/**
 * \file IMP/em2d/internal/Projector.h
 * \brief projection generation class
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_INTERNAL_PROJECTOR_H
#define IMPEM2D_INTERNAL_PROJECTOR_H

#include <IMP/em2d/em2d_config.h>

#include "Projection.h"
#include "ProjectionSphere.h"

#include <IMP/kernel/Particle.h>
#include <boost/ptr_container/ptr_vector.hpp>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

class IMPEM2DEXPORT Projector {
public:
  Projector(const kernel::Particles& particles,
            unsigned int projection_number,
            double pixel_size,
            double resolution);

  // compute all projections
  void compute_projections(boost::ptr_vector<Projection>& projections,
                           int image_size) const;

  // compute projections in some radius of specific direction
  void compute_projections(const algebra::Vector3Ds& axis,
                           double angle_thr,
                           boost::ptr_vector<Projection>& projections,
                           int image_size) const;
private:
  int estimate_image_size(const IMP::algebra::Vector3Ds& points) const;

private:
  kernel::Particles particles_;
  unsigned int projection_number_;
  double pixel_size_;
  double resolution_;
  ProjectionSphere projection_sphere_;
  std::vector<double> radii_;
  std::vector<double> mass_;

};

IMPEM2D_END_INTERNAL_NAMESPACE

#endif /* IMPEM2D_INTERNAL_PROJECTOR_H */
