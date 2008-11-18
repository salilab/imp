/**
 *  \file PropagateBondDerivativesSingletonModifier.h
 *  \brief Split the spatial derivatives between the endpoints of the bond
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_PROPAGATE_BOND_DERIVATIVES_SINGLETON_MODIFIER_H
#define IMPCORE_PROPAGATE_BOND_DERIVATIVES_SINGLETON_MODIFIER_H

#include "core_exports.h"
#include "internal/core_version_info.h"

#include "SingletonModifier.h"
#include "SingletonContainer.h"
#include "XYZRDecorator.h"
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! This class propagates the spatial derivatives of a bond to its endpoints.
/** Currently the derivative is split in half and half goes to each endpoint.
    Only the x,y,z derivatives are used.

    \ingroup bond
 */
class IMPCOREEXPORT PropagateBondDerivativesSingletonModifier:
  public SingletonModifier
{
  FloatKey rk_;
  Float slack_;
public:
  PropagateBondDerivativesSingletonModifier();
  ~PropagateBondDerivativesSingletonModifier();

  IMP_SINGLETON_MODIFIER(internal::core_version_info);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_PROPAGATE_BOND_DERIVATIVES_SINGLETON_MODIFIER_H */
