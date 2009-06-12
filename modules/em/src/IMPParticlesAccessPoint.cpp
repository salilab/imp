/**
 *  \file IMPParticlesAccessPoint.cpp
 *  \brief Provision of EMBED structural data using the IMP framework.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/IMPParticlesAccessPoint.h>
#include <IMP/core/utility.h>

IMPEM_BEGIN_NAMESPACE

IMPParticlesAccessPoint::IMPParticlesAccessPoint(
    const Particles & ps,
    FloatKey radius_key,
    FloatKey weight_key)
{
  particles_ = ps;
  radius_key_ = radius_key;
  weight_key_ = weight_key;
}

algebra::Vector3D IMPParticlesAccessPoint::get_centroid() const {
  return core::centroid(core::XYZs(particles_));
}

void IMPParticlesAccessPoint::reselect(const Particles& ps)
{
  particles_ = ps;
}


IMPEM_END_NAMESPACE
