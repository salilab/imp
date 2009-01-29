/**
 *  \file TransformationCartesianProductSampler.h
 *  \brief The class samples all  combinations in a discrete set of
 *    transformatios.
 *    For example, for a discrete set of trasnformations:
 *    t11,t12       for particle p1
 *    t21,t22,t23   for particle p2
 *    t31,t32       for particle p3
 *    the sampling space would be:
 *    [p1^t11,p2^t21,p3^t31],[p1^t11,p2^t21,p3^t32], ... ,[p1^t12,p2^t23,p3^t31]
 *     p^t indicates the p is transforms by t
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#ifndef IMPDOMINO_TRANSFORMATION_CARTESIAN_PRODUCT_SAMPLER_H
#define IMPDOMINO_TRANSFORMATION_CARTESIAN_PRODUCT_SAMPLER_H
#include "IMP/Particle.h"
#include <map>
#include  <sstream>
#include "IMP/base_types.h"
#include "IMP/domino/TransformationMappedDiscreteSet.h"
#include "IMP/domino/CartesianProductSampler.h"
#include <algorithm>
#include "TransformationUtils.h"
IMPDOMINO_BEGIN_NAMESPACE

class  IMPDOMINOEXPORT TransformationCartesianProductSampler :
        public CartesianProductSampler
{
public:
  TransformationCartesianProductSampler(){}
  //! Create a combination sampler.
  /**
    \param[in] ds              the discrete sampling space
    \param[in] ps              the sampled particles
    \param[in] trans_from_orig if true then a tranformations are applied from
                               the original position/orientation rather than
                               the current one.
  */
  TransformationCartesianProductSampler(TransformationMappedDiscreteSet *ds,
                                        Particles *ps,
                                        bool trans_from_orig = false);
  void move2state(const CombState *cs) ;

protected:
  TransformationUtils tu_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_TRANSFORMATION_CARTESIAN_PRODUCT_SAMPLER_H */
