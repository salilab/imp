/**
 *  \file IMP/core/Transform.h     \brief Transform a particle
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_TRANSFORM_H
#define IMPCORE_TRANSFORM_H
#include <IMP/core/core_config.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonModifier.h>
#include <IMP/singleton_macros.h>
IMPCORE_BEGIN_NAMESPACE

//! Apply a transformation to a passed particle
/**
   \see algebra::Transformation3D
   See IMP::SingletonFunctor for an example of how to transform a set of
   particles.
 */
class IMPCOREEXPORT Transform : public SingletonModifier {
 public:
  //! Initialize
  /**
  \param[in] t a 3d transformation to be applied on a particle
  \param[in] ignore_non_xyz if false then applying the transformation
                            on a non XYZ particle will result in an exception.
  */
  Transform(const algebra::Transformation3D &t, bool ignore_non_xyz = false);

  virtual void apply_index(kernel::Model *m, kernel::ParticleIndex p) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_outputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(Transform);
  IMP_OBJECT_METHODS(Transform);
  ;

 private:
  algebra::Transformation3D t_;
  bool ignore_non_xyz_;
};

IMPCORE_END_NAMESPACE
#endif /* IMPCORE_TRANSFORM_H */
