/**
 *  \file TransformSingletonParticle.cpp
 *  \brief Transform a particle
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */


#include <IMP/core/TransformSingletonModifier.h>
#include "IMP/core/XYZDecorator.h"

IMPCORE_BEGIN_NAMESPACE

TransformSingletonModifier::TransformSingletonModifier(
     const algebra::Transformation3D &t,
     bool ignore_non_xyz)
{
  t_=t;
  ignore_non_xyz_=ignore_non_xyz_;
}

void TransformSingletonModifier::apply(Particle *p) const
{
  if (!XYZDecorator::is_instance_of(p)) {
    IMP_assert(ignore_non_xyz_,"The particle does not have XYZ attributes");
    return;
  }
  XYZDecorator xyz = XYZDecorator(p);
  xyz.set_coordinates(t_.transform(xyz.get_coordinates()));
}
void TransformSingletonModifier::show(std::ostream &out) const
{
  out<<"TransformParticle :";
  t_.show(out);
  out<<std::endl;
}

IMPCORE_END_NAMESPACE
