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


IMPMEMBRANE_BEGIN_NAMESPACE

//! FRET_R Restraint
/** FRET_R Restraint description here

 */
class IMPMEMBRANEEXPORT FretrRestraint : public Restraint
{

Particles pd_;
Particles pa_;
double R0_;
double gamma_;
double Ida_;
double fretr_;
double kappa_;
int    mcsteps_;
Floats bleach0_;
Floats bleach1_;
bool   photobleach_;

double get_bleach_fretr() const;
double get_nobleach_fretr() const;

void   set_photobleach(double Pbleach0, double Pbleach1);

public:
  FretrRestraint(Particles pd, Particles pa,
                 double R0, double gamma, double Ida,
                 double Pbleach0, double Pbleach1,
                 double fretr, double kappa,
                 std::string name);

  IMP_RESTRAINT(FretrRestraint);

};


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_FRETR_RESTRAINT_H */
