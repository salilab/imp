/**
 * \file  TransformationMappedDiscreteSet.cpp
 * \brief Holds a mapped discrete sampling space of rigid transformations.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include "IMP/domino/TransformationMappedDiscreteSet.h"
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Transformation3D.h>
IMPDOMINO_BEGIN_NAMESPACE
TransformationMappedDiscreteSet::TransformationMappedDiscreteSet
   (Particles *ps_target): MappedDiscreteSet(ps_target)
{
  //translation attributes
  atts_.push_back(FloatKey("x"));
  atts_.push_back(FloatKey("y"));
  atts_.push_back(FloatKey("z"));
  //rotation attributes
  atts_.push_back(FloatKey("a"));
  atts_.push_back(FloatKey("b"));
  atts_.push_back(FloatKey("c"));
  atts_.push_back(FloatKey("d"));
}

IMPDOMINO_END_NAMESPACE
