/**
 *  \file IMP/core/TripletRestraint.h
 *  \brief Apply a TripletScore to a Triplet.
 *
 *  WARNING This file was generated from NAMERestraint.hpp
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_TRIPLET_RESTRAINT_H
#define IMPCORE_TRIPLET_RESTRAINT_H

#include <IMP/core/core_config.h>

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
  public IMP::kernel::internal::TupleRestraint<TripletScore>
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
      IMP::kernel::internal::TupleRestraint<TripletScore>(ss,
                                         IMP::kernel::internal::get_model(vt),
                                         IMP::kernel::internal::get_index(vt),
                                                    name)
  {}

#if defined(SWIG) || defined(IMP_DOXYGEN)
 protected:
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  IMP::ModelObjectsTemp do_get_inputs() const;
  IMP_OBJECT_METHODS(TripletRestraint)
#endif
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_TRIPLET_RESTRAINT_H */
