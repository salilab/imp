/**
 * \file  TransformationMappedDiscreteSet.h
 * \brief Holds a discrete sampling space of rigid transformations.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_TRANSFORMATION_MAPPED_DISCRETE_SET_H
#define IMPDOMINO_TRANSFORMATION_MAPPED_DISCRETE_SET_H

#include "IMP/Particle.h"
#include <map>
#include  <sstream>
#include "IMP/base_types.h"
#include "config.h"
#include "DiscreteSet.h"
#include <IMP/algebra/Transformation3D.h>
#include "IMP/domino/MappedDiscreteSet.h"

IMPDOMINO_BEGIN_NAMESPACE
//! TransformationMappedDiscreteSet
/**
Holds a discrete sampling space of rigid transformations.
 */
class IMPDOMINOEXPORT TransformationMappedDiscreteSet : public MappedDiscreteSet
{
public:
  //! Constructor
  /**
  /param[in] ps_target particles to be mapped on a discrete set
   */
  TransformationMappedDiscreteSet(Particles *ps_target);
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_TRANSFORMATION_MAPPED_DISCRETE_SET_H */
