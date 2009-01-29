/**
 *  \file TransformationUtils.h
 *  \brief Handeling tranformation of hierarchical particles
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_TRANSFORMATION_UTILS_H
#define IMPDOMINO_TRANSFORMATION_UTILS_H
#include "IMP/domino/TransformationMappedDiscreteSet.h"
#include <IMP/algebra/Transformation3D.h>
#include "IMP/Particle.h"
#include "IMP/base_types.h"
#include "IMP/domino/CombState.h"
IMPDOMINO_BEGIN_NAMESPACE
class TransformationUtils {
public:
  TransformationUtils(){}
  //! Constructor
  /**
  /param[in] ps particles to apply transformation on.
  /param[in] trans_from_orig if true then a tranformations are applied from
                               the original position/orientation rather than
                               the current one.
   */
  TransformationUtils(Particles *ps,bool trans_from_orig = false);
  //! Transform a particle
  /**
  /param[in] p_sample The particle to apply the trasnformation all. The
            transformation is applied on all of the leaves of the particle.
  /param[in] t_trans a particle that holds the transformation (has xyz and abcd
             attributes)
   */
  void move2state(Particle *p_sample, Particle *p_trans);
protected:
  //! Apply a transformation on all of the leaves of a particle
  void apply(Particle *p , const algebra::Transformation3D &t);
  algebra::Transformation3D get_trans(Particle *p) const;
  bool go_back_;
  std::map<Particle *,algebra::Transformation3D> last_transform_;
  std::vector<FloatKey> atts_;
};
IMPDOMINO_END_NAMESPACE
#endif  /* IMPDOMINO_TRANSFORMATION_UTILS_H */
