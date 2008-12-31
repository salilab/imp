/**
 *  \file BondDecoratorRestraint.h
 *  \brief Apply a UnaryFunction to all bonded pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_BOND_DECORATOR_RESTRAINT_H
#define IMPCORE_BOND_DECORATOR_RESTRAINT_H

#include "config.h"
#include "internal/core_version_info.h"
#include "BondDecoratorListScoreState.h"

#include <IMP/Restraint.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! This class is deprecated, use SingletonsRestraint or PairsRestraint instead.
/** This restraint currently only works for bonds which have their
    length set explicitly. Eventually we should add a table for standard
    molecular bonds.

    The bond stiffness is assumed to be 1 if it is not specified in
    the bond. This can become a parameter eventually.
    \deprecated Use SingletonsRestraint or PairsRestraint instead.
 */
class IMPCOREEXPORT BondDecoratorRestraint : public Restraint
{
public:
  //! Create the bond restraint.
  /** \param[in] f The UnaryFunction to apply to the particles.
      \param[in] bl The BondDecoratorListScoreState to use to get the list
      of bonds.
   */
  BondDecoratorRestraint(UnaryFunction *f, BondDecoratorListScoreState *bl);
  virtual ~BondDecoratorRestraint(){}

  IMP_RESTRAINT(internal::core_version_info)

  void set_function(UnaryFunction *f) {f_=f;}

protected:
  BondDecoratorListScoreState *bl_;
  Pointer<UnaryFunction> f_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_DECORATOR_RESTRAINT_H */
