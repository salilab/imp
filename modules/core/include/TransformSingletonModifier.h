/**
 *  \file TransformSingletonModifier.h     \brief Transform a particle
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPCORE_TRANSFORM_SINGLETON_MODIFIER_H
#define IMPCORE_TRANSFORM_SINGLETON_MODIFIER_H
#include "config.h"
#include "internal/version_info.h"
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonModifier.h>

IMPCORE_BEGIN_NAMESPACE


//! Apply a transformation to a passed particle
/**
   \relates algebra::Transformation3D
 */
class IMPCOREEXPORT TransformSingletonModifier : public SingletonModifier
{
public:
  //! Initialize
  /**
  /param[in] t a 3d transformation to be applied on a particle
  /param[in] ignore_non_xyz if false then appling the tranformation
                            on a non XYZ particle will result in an exception.
  */
  TransformSingletonModifier(const algebra::Transformation3D &t,
                    bool ignore_non_xyz=false);
  ~TransformSingletonModifier(){}

  IMP_SINGLETON_MODIFIER(internal::version_info);
private:
  algebra::Transformation3D t_;
  bool ignore_non_xyz_;
};


IMPCORE_END_NAMESPACE
#endif  /* IMPCORE_TRANSFORM_SINGLETON_MODIFIER_H */
