/**
 *  \file BoundingBox3DSingletonScore.h
 *  \brief Score particles based on a bounding box
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H
#define IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H

#include "config.h"
#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/algebra/BoundingBoxD.h>

IMPCORE_BEGIN_NAMESPACE

//! Score particles based on how far outside a box they are.
/** The radius of the particle is ignored, only the center coordinates
    are used. A particle that is contained within the bounding box has
    a score of 0. The UnaryFunction passed should return 0 when given
    a feature size of 0.
 */
class IMPCOREEXPORT BoundingBox3DSingletonScore: public SingletonScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  algebra::BoundingBoxD<3> bb_;
public:
  BoundingBox3DSingletonScore(UnaryFunction *f,
                            const algebra::BoundingBoxD<3> &bb);

  IMP_SIMPLE_SINGLETON_SCORE(BoundingBox3DSingletonScore)
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H */
