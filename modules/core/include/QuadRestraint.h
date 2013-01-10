/**
 *  \file IMP/core/QuadRestraint.h
 *  \brief Apply a QuadScore to a Quad.
 *
 *  WARNING This file was generated from NAMERestraint.hpp
 *  in tools/maintenance/container_templates/core
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_QUAD_RESTRAINT_H
#define IMPCORE_QUAD_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/internal/TupleRestraint.h>
#include <IMP/QuadScore.h>
#include <IMP/restraint_macros.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a QuadScore to a Quad.
/** This restraint stores a Quad.
    \see QuadRestraint
 */
class QuadRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
  public Restraint
#else
  public IMP::internal::TupleRestraint<QuadScore>
#endif
{
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Quad and the Quad.
   */
  QuadRestraint(QuadScore *ss,
                     const ParticleQuad& vt,
                     std::string name="QuadRestraint %1%"):
      IMP::internal::TupleRestraint<QuadScore>(ss,
                                                IMP::internal::get_model(vt),
                                                IMP::internal::get_index(vt),
                                                    name)
  {}

#if defined(SWIG) || defined(IMP_DOXYGEN)

  QuadScore* get_score() const {
    return ss_;
  }
  ParticleQuad get_argument() const {
    return IMP::internal::get_particle(get_model(), v_);
  }

  IMP_RESTRAINT(QuadRestraint);

  double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                      double max) const;
#endif
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_QUAD_RESTRAINT_H */
