/**
 *  \file IMP/core/SingletonRestraint.h
 *  \brief Apply a SingletonScore to a Singleton.
 *
 *  WARNING This file was generated from NAMERestraint.hpp
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SINGLETON_RESTRAINT_H
#define IMPCORE_SINGLETON_RESTRAINT_H

#include "core_config.h"

#include <IMP/internal/TupleRestraint.h>
#include <IMP/SingletonScore.h>
#include <IMP/restraint_macros.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a SingletonScore to a Singleton.
/** This restraint stores a Singleton.
    \see SingletonRestraint
 */
class SingletonRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
  public Restraint
#else
  public IMP::internal::TupleRestraint<SingletonScore>
#endif
{
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Singleton and the Singleton.
   */
  SingletonRestraint(SingletonScore *ss,
                     Particle* vt,
                     std::string name="SingletonRestraint %1%"):
      IMP::internal::TupleRestraint<SingletonScore>(ss,
                                                IMP::internal::get_model(vt),
                                                IMP::internal::get_index(vt),
                                                    name)
  {}

#if defined(SWIG) || defined(IMP_DOXYGEN)

  SingletonScore* get_score() const {
    return ss_;
  }
  Particle* get_argument() const {
    return IMP::internal::get_particle(get_model(), v_);
  }

  IMP_RESTRAINT(SingletonRestraint);

  double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                      double max) const;
#endif
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SINGLETON_RESTRAINT_H */
