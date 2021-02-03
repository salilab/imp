/**
 *  \file IMP/core/DiameterRestraint.h
 *  \brief A restraint to maintain the diameter of a set of points
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DIAMETER_RESTRAINT_H
#define IMPCORE_DIAMETER_RESTRAINT_H

#include <IMP/core/core_config.h>

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

    \note Currently, decomposing this restraint results in pieces which
    score deviations from the diameter with a harmonic upper bound of strength
    1.
 */
class IMPCOREEXPORT DiameterRestraint : public Restraint {
  IMP::PointerMember<ScoreState> ss_;
  IMP::PointerMember<Particle> p_;
  Float diameter_;
  IMP::PointerMember<SingletonContainer> sc_;
  IMP::PointerMember<UnaryFunction> f_;
  FloatKey dr_;
  void init();

 public:
  //! Use f to restrain sc to be within diameter of one another
  /** f should have a minimum at 0 and be an upper bound-style function.
   */
  DiameterRestraint(UnaryFunction *f, SingletonContainer *sc, Float diameter);

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DiameterRestraint);
#ifndef IMP_DOXYGEN
  Restraints do_create_decomposition() const;
  Restraints do_create_current_decomposition() const;
#endif
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DIAMETER_RESTRAINT_H */
