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
#include <IMP/restraint_macros.h>
#include <IMP/Particle.h>
#include <boost/random/uniform_real.hpp>

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
double Pbleach0_;
double Pbleach1_;
unsigned Na_;
unsigned mcsteps_;
bool photobleach_;
mutable Floats power6_;

double get_bleach_fretr() const;
double get_nobleach_fretr() const;
double get_sumFi(const Floats& power6, double Pbleach) const;

public:
  FretrRestraint(Particles pd, Particles pa,
                 double R0, double gamma, double Ida,
                 double Pbleach0, double Pbleach1,
                 double fretr, double kappa,
                 std::string name, double multi=1.0);

  IMP_RESTRAINT(FretrRestraint);

};


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_FRETR_RESTRAINT_H */
