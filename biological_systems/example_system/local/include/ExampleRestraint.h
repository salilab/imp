/**
 *  \file local/ExampleRestraint.h
 *  \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_RESTRAINT_H
#define IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_RESTRAINT_H

#include "example_system_local_config.h"
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>

IMPEXAMPLESYSTEMLOCAL_BEGIN_NAMESPACE

//! Constraint a particle to be in the x,y plane
/** \note Be sure to check out the swig wrapper file and how it
    wraps this class.

    The source code is as follows:
    \include ExampleRestraint.h
    \include ExampleRestraint.cpp
*/
class IMPEXAMPLESYSTEMLOCALEXPORT ExampleRestraint : public Restraint
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

IMPEXAMPLESYSTEMLOCAL_END_NAMESPACE

#endif  /* IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_RESTRAINT_H */
