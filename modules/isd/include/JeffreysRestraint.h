/**
 *  \file isd/JeffreysRestraint.h
 *  \brief A restraint on a nuisance parameter.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_ISD_JEFFREYSRESTRAINT_H
#define IMPISD_ISD_JEFFREYSRESTRAINT_H

#include "isd_config.h"
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>

IMPISD_BEGIN_NAMESPACE

//! Score a nuisance parameter with 1/nuisance

class IMPISDEXPORT JeffreysRestraint : public Restraint
{
  Pointer<Particle> p_;

public:
  //! Create the restraint.
  JeffreysRestraint(Particle *p);

  /** This macro declares the basic needed methods: evaluate and show
   */
  IMP_RESTRAINT(JeffreysRestraint);
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_RESTRAINT_H */
