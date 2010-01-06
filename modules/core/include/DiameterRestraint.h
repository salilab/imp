/**
 *  \file core/DiameterRestraint.h
 *  \brief A restraint to maintain the diameter of a set of points
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_DIAMETER_RESTRAINT_H
#define IMPCORE_DIAMETER_RESTRAINT_H

#include "config.h"

#include "internal/remove_pointers.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Restrain the diameter of a set of points
/** This class also serves as an example of how to build restraints which
    have internal ScoreStates and perhaps more than one actual restraint
    object.
 */
class IMPCOREEXPORT DiameterRestraint: public Restraint
{
  IMP::internal::OwnerPointer<ScoreState> ss_;
  IMP::internal::OwnerPointer<Particle> p_;
  Float diameter_;
  IMP::internal::OwnerPointer<SingletonContainer> sc_;
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  FloatKey dr_;
public:
  //! Use f to restraint sc to be withing diameter of one another
  /** f should have a minimum at 0 and be an upper bound-style function.
   */
  DiameterRestraint(UnaryFunction *f,
                    SingletonContainer *sc, Float diameter);

  IMP_RESTRAINT(DiameterRestraint, get_module_version_info());

  void set_model(Model *m);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DIAMETER_RESTRAINT_H */
