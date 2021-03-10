/**
 *  \file IMP/spb/UniformBoundedRestraint.h
 *  \brief A uniform bounded restraint
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_UNIFORM_BOUNDED_RESTRAINT_H
#define IMPSPB_UNIFORM_BOUNDED_RESTRAINT_H
#include <IMP/Pointer.h>
#include <IMP/isd/Scale.h>
#include <IMP/spb/ISDRestraint.h>
#include "IMP/Restraint.h"
#include <IMP/spb/spb_config.h>

IMPSPB_BEGIN_NAMESPACE
/** A uniform bounded restraint

    The source code is as follows:
    \include  UniformBoundedRestraint.h
    \include  UniformBoundedRestraint.cpp
 */

class IMPSPBEXPORT UniformBoundedRestraint
    : public spb::ISDRestraint {
  IMP::PointerMember<Particle> p_;
  FloatKey fk_;
  IMP::PointerMember<Particle> a_;
  IMP::PointerMember<Particle> b_;

 public:
  //! Create the restraint.
  UniformBoundedRestraint(Particle *p, FloatKey fk, Particle *a, Particle *b);

  /* call for probability */
  double get_probability() const;

  /** This macro declares the basic needed methods: evaluate and show
   */
  // IMP_RESTRAINT( UniformBoundedRestraint);
  //
  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const
      IMP_OVERRIDE;
  IMP::ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(UniformBoundedRestraint);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_UNIFORM_BOUNDED_RESTRAINT_H */
