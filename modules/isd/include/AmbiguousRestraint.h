/**
 *  \file IMP/isd/AmbiguousRestraint.h
 *  \brief An implementation of the d-norm to make an ambiguous restraint.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_AMBIGUOUS_RESTRAINT_H
#define IMPISD_AMBIGUOUS_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/restraint_macros.h>

IMPISD_BEGIN_NAMESPACE

//! Apply an ambiguous restraint by computing the d-norm.
//  d is an integer.
//  If d>0, behaves like an "and" function.
//  If d<0, behaves like an "or" function.
//  Limits are fuzzy "and" (min) and fuzzy "or" (max) when d is infinite.
/*
    The source code is as follows:
    \include AmbiguousRestraint.h
    \include AmbiguousRestraint.cpp
*/
class IMPISDEXPORT AmbiguousRestraint : public ISDRestraint
{
  int d_;
  Restraints rs_;
public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
      Two ways to call it: pass it two restraints, or a list of restraints.
   */
  AmbiguousRestraint(int d, Restraint *r0, Restraint *r1);
  AmbiguousRestraint(int d, Restraints rs);

  double get_probability() const
  {
      return exp(-unprotected_evaluate(nullptr));
  }

  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(AmbiguousRestraint);

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_AMBIGUOUS_RESTRAINT_H */
