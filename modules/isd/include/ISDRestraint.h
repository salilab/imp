/**
 *  \file IMP/isd/ISDRestraint.h
 *  \brief A lognormal restraint that uses the ISPA model to model NOE-derived
 *  distance fit.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_ISD_RESTRAINT_H
#define IMPISD_ISD_RESTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>
#include <IMP/PairScore.h>
#include <string>

IMPISD_BEGIN_NAMESPACE

//! Apply an NOE distance restraint between two particles.
class IMPISDEXPORT ISDRestraint : public Restraint
{
public:

  ISDRestraint() : Restraint() {}
  ISDRestraint(std::string name) : Restraint(name) {}
  ISDRestraint(Model *m, std::string name) : Restraint(m,name) {}

  /* call for probability, pure method */
  virtual double get_probability() const = 0;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_RESTRAINT_H */
