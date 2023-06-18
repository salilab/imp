/**
 *  \file IMP/core/Transform.h     \brief Transform a particle
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_TRANSFORM_H
#define IMPCORE_TRANSFORM_H
#include <IMP/core/core_config.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonModifier.h>
#include <IMP/singleton_macros.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Apply a transformation to a passed particle
/**
   \see algebra::Transformation3D
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

  Transform() {}

  virtual void apply_index(Model *m, ParticleIndex p) const
      override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_SINGLETON_MODIFIER_METHODS(Transform);
  IMP_OBJECT_METHODS(Transform);
  ;

 private:
  algebra::Transformation3D t_;
  bool ignore_non_xyz_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<SingletonModifier>(this), t_, ignore_non_xyz_);
  }
  IMP_OBJECT_SERIALIZE_DECL(Transform);

};

IMPCORE_END_NAMESPACE
#endif /* IMPCORE_TRANSFORM_H */
