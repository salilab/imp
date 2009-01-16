/**
 *  \file TransformationFunction.h
 *  \brief Transforms a vector of Particles
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_TRANSFORMATION_FUNCTION_H
#define IMPCORE_TRANSFORMATION_FUNCTION_H

#include "config.h"

#include <IMP/algebra/Transformation3D.h>

#include <IMP/core/ParticleFunction.h>
#include "IMP/core/MolecularHierarchyDecorator.h"
#include "IMP/core/XYZDecorator.h"
#include <iostream>

IMPCORE_BEGIN_NAMESPACE
//! This class transforms a vector of particles
/**
 */
class TransformationFunction : public ParticleFunction
{
public:
  TransformationFunction(IMP::algebra::Transformation3D t)
  {
    t_=t;
  }
  ~TransformationFunction()
  {
  }
  //! Apply the transformation on all of the particle-leaves of
  //! the input particle.
  void apply(Particles ps)
  {
    IMP::core::XYZDecorator xyz;
    Vector3D v;
    for(Particles::iterator it = ps.begin(); it != ps.end();it++) {
      xyz = IMP::core::XYZDecorator::cast(*it);
      v = xyz.get_coordinates();
      xyz.set_coordinates(t_.transform(v));
    }
  }
private:
  IMP::algebra::Transformation3D t_;
};

IMPCORE_END_NAMESPACE
#endif /* IMPCORE_TRANSFORMATION_FUNCTION_H */
