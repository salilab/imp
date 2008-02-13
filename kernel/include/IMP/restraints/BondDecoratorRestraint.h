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

namespace IMP
{

class BondDecoratorListScoreState;
class UnaryFunction;

//! Restrain all pairs of non-bonded particles
/** This restraint currently only works for bonds which have their 
    length set explicitly. Eventually we should add a table for standard
    molecular bonds.

    The bond stiffness is assumed to be 1 if it is not specified in
    the bond. This can become a parameter eventually.

    \ingroup bond
 */
class IMPDLLEXPORT BondDecoratorRestraint : public Restraint
{
public:
  //! Create the bond restraint.
  /** \param[in] bl The BondDecoratorListScoreState to use to get the list
      of bonds.
      \param[in] f The UnaryFunction to apply to the particles.
   */
  BondDecoratorRestraint(BondDecoratorListScoreState *bl, UnaryFunction *f);
  virtual ~BondDecoratorRestraint(){}

  IMP_RESTRAINT("0.5", "Daniel Russel");

protected:
  BondDecoratorListScoreState *bl_;
  std::auto_ptr<UnaryFunction> f_;
};

} // namespace IMP

#endif  /* __IMP_BOND_DECORATOR_RESTRAINT_H */
