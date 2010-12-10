/**
 *  \file isd/NOERestraint.h
 *  \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_NOE_RESTRAINT_H
#define IMPISD_NOE_RESTRAINT_H

#include "isd_config.h"
#include <IMP/SingletonScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>

IMPISD_BEGIN_NAMESPACE

//! Apply an NOE distance restraint between two particles.
/** \note Be sure to check out the swig wrapper file and how it
    wraps this class.

    The source code is as follows:
    \include NOERestraint.h
    \include NOERestraint.cpp
*/
class IMPISDEXPORT NOERestraint : public Restraint
{
  Pointer<Particle> p0_;
  Pointer<Particle> p1_;
  Pointer<Particle> sigma_;
  Pointer<Particle> gamma_;
  double Iexp_;
public:
  //! Create the restraint.
  /** Restraints should store the particles they are to act on,
      preferably in a Singleton or PairContainer as appropriate.
   */
  NOERestraint(Particle *p0, Particle *p1, Particle *sigma, 
	       Particle *gamma,double Iexp);

  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(NOERestraint);
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_NOE_RESTRAINT_H */
