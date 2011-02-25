/**
 *  \file isd/ISDRestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_ISD_RESTRAINT_H
#define IMPISD_ISD_RESTRAINT_H

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
    \include ISDRestraint.h
    \include ISDRestraint.cpp
*/
class IMPISDEXPORT ISDRestraint : public Restraint
{
public:

  /* call for probability, pure method */
  virtual double get_probability() const = 0;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_RESTRAINT_H */
