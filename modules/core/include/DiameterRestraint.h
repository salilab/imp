/**
 *  \file core/DiameterRestraint.h
 *  \brief A restraint to maintain the diameter of a set of points
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DIAMETER_RESTRAINT_H
#define IMPCORE_DIAMETER_RESTRAINT_H

#include "core_config.h"

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

    The diameter scored includes the radius of the involved particles.

    \note, Currently, decomposing this restraint results in pieces which
    score deviations from the diameter with a harmonic upper bound of strength
    1.
 */
class IMPCOREEXPORT DiameterRestraint: public Restraint
{
  IMP::OwnerPointer<ScoreState> ss_;
  IMP::OwnerPointer<Particle> p_;
  Float diameter_;
  IMP::OwnerPointer<SingletonContainer> sc_;
  IMP::OwnerPointer<UnaryFunction> f_;
  FloatKey dr_;
public:
  //! Use f to restraint sc to be withing diameter of one another
  /** f should have a minimum at 0 and be an upper bound-style function.
   */
  DiameterRestraint(UnaryFunction *f,
                    SingletonContainer *sc, Float diameter);

  IMP_RESTRAINT(DiameterRestraint);
  Restraints create_decomposition() const;
  Restraints create_current_decomposition() const;
  void set_model(Model *m);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DIAMETER_RESTRAINT_H */
