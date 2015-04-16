/**
 *  \file IMP/core/VolumeRestraint.h
 *  \brief A restraint that prevents spheres from inter-penetrating
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_VOLUME_RESTRAINT_H
#define IMPCORE_VOLUME_RESTRAINT_H

#include <IMP/core/core_config.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE
#if defined(IMP_CORE_USE_IMP_CGAL) || defined(IMP_DOXYGEN)
//! A restraint that prevents spheres from inter-penetrating
/** \note This restraint does not support derivatives.
    \note This restraint requires CGAL.*/
class IMPCOREEXPORT VolumeRestraint : public Restraint {
  IMP::PointerMember<SingletonContainer> sc_;
  IMP::PointerMember<UnaryFunction> f_;
  double volume_;

 public:
  VolumeRestraint(UnaryFunction *f, SingletonContainer *sc, double volume);

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(VolumeRestraint);
};
#endif

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_VOLUME_RESTRAINT_H */
