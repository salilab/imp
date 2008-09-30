/**
 *  \file BondDecoratorRestraint.h
 *  \brief Apply a UnaryFunction to all bonded pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BOND_DECORATOR_RESTRAINT_H
#define __IMP_BOND_DECORATOR_RESTRAINT_H

#include <iostream>

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../internal/kernel_version_info.h"
#include "../Pointer.h"
#include "../UnaryFunction.h"

IMP_BEGIN_NAMESPACE

class BondDecoratorListScoreState;

//! Restrain all pairs of non-bonded particles
/** This restraint currently only works for bonds which have their
    length set explicitly. Eventually we should add a table for standard
    molecular bonds.

    The bond stiffness is assumed to be 1 if it is not specified in
    the bond. This can become a parameter eventually.

    \ingroup bond
    \ingroupd restraint
 */
class IMPDLLEXPORT BondDecoratorRestraint : public Restraint
{
public:
  //! Create the bond restraint.
  /** \param[in] f The UnaryFunction to apply to the particles.
      \param[in] bl The BondDecoratorListScoreState to use to get the list
      of bonds.
   */
  BondDecoratorRestraint(UnaryFunction *f, BondDecoratorListScoreState *bl);
  virtual ~BondDecoratorRestraint(){}

  IMP_RESTRAINT(internal::kernel_version_info)

  void set_function(UnaryFunction *f) {f_=f;}

protected:
  BondDecoratorListScoreState *bl_;
  Pointer<UnaryFunction> f_;
};

IMP_END_NAMESPACE

#endif  /* __IMP_BOND_DECORATOR_RESTRAINT_H */
