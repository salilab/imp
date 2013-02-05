/**
 *  \file IMP/core/PairRestraint.h
 *  \brief Apply a PairScore to a Pair.
 *
 *  WARNING This file was generated from NAMERestraint.hpp
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_PAIR_RESTRAINT_H
#define IMPCORE_PAIR_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/internal/TupleRestraint.h>
#include <IMP/PairScore.h>
#include <IMP/restraint_macros.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a PairScore to a Pair.
/** This restraint stores a Pair.
    \see PairRestraint
 */
class PairRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
  public Restraint
#else
  public IMP::kernel::internal::TupleRestraint<PairScore>
#endif
{
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Pair and the Pair.
   */
  PairRestraint(PairScore *ss,
                     const ParticlePair& vt,
                     std::string name="PairRestraint %1%"):
      IMP::kernel::internal::TupleRestraint<PairScore>(ss,
                                         IMP::kernel::internal::get_model(vt),
                                         IMP::kernel::internal::get_index(vt),
                                                    name)
  {}

#if defined(SWIG) || defined(IMP_DOXYGEN)
 protected:
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  IMP::ModelObjectsTemp do_get_inputs() const;
  IMP_OBJECT_METHODS(PairRestraint)
#endif
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_PAIR_RESTRAINT_H */
