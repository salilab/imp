/**
 *  \file FretrRestraint.h
 *  \brief FRET_R Restraint
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_FRETR_RESTRAINT_H
#define IMPMEMBRANE_FRETR_RESTRAINT_H

#include "membrane_config.h"
#include "IMP/Restraint.h"
#include <IMP/Particle.h>
#include <IMP/container/CloseBipartitePairContainer.h>


IMPMEMBRANE_BEGIN_NAMESPACE

//! FRET_R Restraint
/** FRET_R Restraint description here

 */
class IMPMEMBRANEEXPORT FretrRestraint : public Restraint
{

Particles pd_;
Particles pa_;
double R0_;
double Sd_;
double Sa_;
double Nd_;
double Na_;
double gamma_;
double Ida_;
double fretr_;
double kappa_;

public:
  FretrRestraint(Particles pd, Particles pa,
                 double R0, double Sd, double Sa,
                 double gamma, double Ida,
                 double fretr, double kappa,
                 std::string name);

 IMP_RESTRAINT(FretrRestraint);

};


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_FRETR_RESTRAINT_H */
