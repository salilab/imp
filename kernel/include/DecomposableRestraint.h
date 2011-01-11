/**
 *  \file IMP/DecomposableRestraint.h
 *  \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_DECOMPOSABLE_RESTRAINT_H
#define IMP_DECOMPOSABLE_RESTRAINT_H

#include "Restraint.h"

#include <vector>
#include <iostream>
#include <limits>

IMP_BEGIN_NAMESPACE


class Model;

//! Abstract class for representing a restraint which is decomposable
/** A decomposable restraint is one in which it makes sense to evaluate
    the restraint over only a subset of all the input particles. Examples
    include IMP::container::PairsRestraint where the scoring function
    is a sum of terms over a list of pairs. Hence, it makes sense to
    talk of the score over a subset of the particles (namely the
    sum of pairs where both particles in the pair are in the subset).

 */
class IMPEXPORT DecomposableRestraint : public Restraint
{
public:
  DecomposableRestraint(std::string name="DecomposableRestraint %1%");

#ifndef IMP_DOXYGEN
  virtual double unprotected_evaluate_subset(DerivativeAccumulator *) const;
#endif
  IMP_REF_COUNTED_DESTRUCTOR(DecomposableRestraint);
};

IMP_OBJECTS(DecomposableRestraint, DecomposableRestraints);

IMP_END_NAMESPACE

#endif  /* IMP_DECOMPOSABLE_RESTRAINT_H */
