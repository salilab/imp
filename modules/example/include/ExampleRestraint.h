/**
 *  \file example/ExampleRestraint.h
 *  \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_RESTRAINT_H
#define IMPEXAMPLE_EXAMPLE_RESTRAINT_H

#include "example_config.h"
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! Constraint a particle to be in the x,y plane
/** \note Be sure to check out the swig wrapper file and how it
    wraps this class.

    The source code is as follows:
    \include ExampleRestraint.h
    \include ExampleRestraint.cpp
*/
class IMPEXAMPLEEXPORT ExampleRestraint : public Restraint
{
  Pointer<Particle> p_;
  double k_;
public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  ExampleRestraint(Particle *p, double k);

  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(ExampleRestraint);
};

IMPEXAMPLE_END_NAMESPACE

#endif  /* IMPEXAMPLE_EXAMPLE_RESTRAINT_H */
