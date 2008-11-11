/**
 *  \file TransformationFunction.h
 *  \brief Transforms a vector of Particles
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_TRANSFORMATION_FUNCTION_H
#define IMPMISC_TRANSFORMATION_FUNCTION_H

#include "misc_exports.h"

#include <IMP/misc/ParticleFunction.h>
#include "IMP/misc/Transformation3D.h"
#include "IMP/core/MolecularHierarchyDecorator.h"
#include "IMP/core/XYZDecorator.h"
#include <iostream>

IMPMISC_BEGIN_NAMESPACE
//! This class transforms a vector of particles
/**
 */
class IMPMISCEXPORT TransformationFunction : public ParticleFunction
{
public:
  TransformationFunction(Transformation3D t)
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
  Transformation3D t_;
};

IMPMISC_END_NAMESPACE
#endif /* IMPMISC_TRANSFORMATION_FUNCTION_H */
