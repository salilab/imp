/**
 *  \file IMP/em2d/Fine2DRegistrationRestraint.h
 *  \brief Alignment of 2D projections of a 3D volume
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H
#define IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H

#include <IMP/em2d/em2d_config.h>
#include "internal/Fine2DRegistrationRestraint.h"

IMPEM2D_BEGIN_NAMESPACE

class IMPEM2DEXPORT Fine2DRegistrationRestraint
         : public internal::Fine2DRegistrationRestraint {
 public:
  IMPEM2D_DEPRECATED_OBJECT_DECL(2.19)
  Fine2DRegistrationRestraint(Model *m)
           : internal::Fine2DRegistrationRestraint(m) {
    IMPEM2D_DEPRECATED_OBJECT_DEF(
                 2.19, "Use internal::Fine2DRegistrationRestraint instead");
  }
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_FINE_2DREGISTRATION_RESTRAINT_H */
