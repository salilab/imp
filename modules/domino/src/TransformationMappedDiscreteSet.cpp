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
   (const Particles &ps_target): MappedDiscreteSet(ps_target)
{
  //translation attributes
  FloatKeys xyz = IMP::domino::Transformation::get_translation_keys();
  atts_.insert(atts_.end(),xyz.begin(),xyz.end());
  //rotation attributes
  FloatKeys abcd = IMP::domino::Transformation::get_rotation_keys();
  atts_.insert(atts_.end(),abcd.begin(),abcd.end());
}

IMPDOMINO_END_NAMESPACE
