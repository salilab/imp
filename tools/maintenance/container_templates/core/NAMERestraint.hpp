/**
 *  \file IMP/core/CLASSNAMERestraint.h
 *  \brief Apply a CLASSNAMEScore to a CLASSNAME.
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_HEADERNAME_RESTRAINT_H
#define IMPCORE_HEADERNAME_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/internal/TupleRestraint.h>
#include <IMP/CLASSNAMEScore.h>
#include <IMP/restraint_macros.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a CLASSNAMEScore to a CLASSNAME.
/** This restraint stores a CLASSNAME.
    \see CLASSNAMERestraint
 */
class CLASSNAMERestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
  public Restraint
#else
  public IMP::internal::TupleRestraint<CLASSNAMEScore>
#endif
{
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored CLASSNAME and the CLASSNAME.
   */
  CLASSNAMERestraint(CLASSNAMEScore *ss,
                     ARGUMENTTYPE vt,
                     std::string name="CLASSNAMERestraint %1%"):
      IMP::internal::TupleRestraint<CLASSNAMEScore>(ss,
                                                IMP::internal::get_model(vt),
                                                IMP::internal::get_index(vt),
                                                    name)
  {}

#if defined(SWIG) || defined(IMP_DOXYGEN)

  CLASSNAMEScore* get_score() const {
    return ss_;
  }
  VARIABLETYPE get_argument() const {
    return IMP::internal::get_particle(get_model(), v_);
  }

  IMP_RESTRAINT(CLASSNAMERestraint);

  double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                      double max) const;
#endif
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_HEADERNAME_RESTRAINT_H */
