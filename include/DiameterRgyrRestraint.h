/**
 *  \file DiameterRgyrRestraint.h
 *  \brief Diameter Restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_DIAMETER_RGYR_RESTRAINT_H
#define IMPMEMBRANE_DIAMETER_RGYR_RESTRAINT_H

#include "membrane_config.h"
#include "IMP/Restraint.h"
//#include "IMP/restraint_macros.h"
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <string>
#include <map>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Diameter and radius of gyration Restraint
/** description here

 */
class IMPMEMBRANEEXPORT DiameterRgyrRestraint : public Restraint
{

private:

 Particles ps_;
 Float diameter_;
 Float rgyr_;
 Float kappa_;

public:

 DiameterRgyrRestraint(kernel::Model *m,Particles ps, Float diameter,
               Float rgyr, Float kappa);
 IMP_OBJECT_METHODS(DiameterRgyrRestraint);

};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DIAMETER_RGYR_RESTRAINT_H */
