/**
 *  \file DiameterRestraint.h
 *  \brief A restraint to maintain the diameter of a set of points
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_DIAMETER_RESTRAINT_H
#define IMPCORE_DIAMETER_RESTRAINT_H

#include "config.h"

#include "internal/version_info.h"
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
  Pointer<Restraint> r_;
  Pointer<ScoreState> ss_;
  Pointer<Particle> p_;
  Float diameter_;
  Pointer<SingletonContainer> sc_;
  Pointer<UnaryFunction> f_;
public:
  //! Use f to restraint sc to be withing diameter of one another
  /** f should have a minimum at 0 and be an upper bound-style function.
   */
  DiameterRestraint(UnaryFunction *f,
                    SingletonContainer *sc, Float diameter);

  IMP_RESTRAINT(DiameterRestraint, internal::version_info);

  virtual ParticlesList get_interacting_particles() const;

  void set_model(Model *m);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DIAMETER_RESTRAINT_H */
