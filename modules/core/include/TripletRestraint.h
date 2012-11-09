/**
 *  \file IMP/core/TripletRestraint.h
 *  \brief Apply a TripletScore to a Triplet.
 *
 *  WARNING This file was generated from NAMERestraint.hpp
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_TRIPLET_RESTRAINT_H
#define IMPCORE_TRIPLET_RESTRAINT_H

#include "core_config.h"

#include <IMP/internal/TupleRestraint.h>
#include <IMP/TripletScore.h>
#include <IMP/restraint_macros.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a TripletScore to a Triplet.
/** This restraint stores a Triplet.
    \see TripletRestraint
 */
class TripletRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
  public Restraint
#else
  public IMP::internal::TupleRestraint<TripletScore>
#endif
{
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Triplet and the Triplet.
   */
  TripletRestraint(TripletScore *ss,
                     const ParticleTriplet& vt,
                     std::string name="TripletRestraint %1%"):
      IMP::internal::TupleRestraint<TripletScore>(ss, vt, name)
  {}

#if defined(SWIG) || defined(IMP_DOXYGEN)

  TripletScore* get_score() const {
    return ss_;
  }
  ParticleTriplet get_argument() const {
    return IMP::internal::get_particle(get_model(), v_);
  }

  IMP_RESTRAINT(TripletRestraint);

  double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                      double max) const;
#endif
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_TRIPLET_RESTRAINT_H */
