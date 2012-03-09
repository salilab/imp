/**
 *  \file EzRestraint.h
 *  \brief FRET_R Restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_EZ_RESTRAINT_H
#define IMPMEMBRANE_EZ_RESTRAINT_H

#include "membrane_config.h"
#include "IMP/Restraint.h"
#include <IMP/Particle.h>
#include <string>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Ez Potential Restraint
/** Ez Restraint description here

 */
class IMPMEMBRANEEXPORT EzRestraint : public Restraint
{

Particles ps_;
UnaryFunctions ufs_;
Floats get_parameters(std::string restype);


public:
  EzRestraint(Particles ps);

  IMP_RESTRAINT(EzRestraint);

};


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_EZ_RESTRAINT_H */
