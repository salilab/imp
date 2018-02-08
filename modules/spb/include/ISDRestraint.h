/**
 *  \file IMP/spb/ISDRestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_ISD_RESTRAINT_H
#define IMPSPB_ISD_RESTRAINT_H

#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>
#include <IMP/Restraint.h>
#include <IMP/SingletonScore.h>
#include <IMP/core/XYZ.h>
#include <string>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

//! Apply an NOE distance restraint between two particles.
/** \note Be sure to check out the swig wrapper file and how it
    wraps this class.

    The source code is as follows:
    \include ISDRestraint.h
*/
class IMPSPBEXPORT ISDRestraint : public Restraint {
 public:
  // ISDRestraint() : Restraint() {}
  // ISDRestraint(std::string name) : Restraint(name) {}
  ISDRestraint(Model *mdl, std::string name) : Restraint(mdl, name) {}

  /* call for probability, pure method */
  virtual double get_probability() const = 0;
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_ISD_RESTRAINT_H */
