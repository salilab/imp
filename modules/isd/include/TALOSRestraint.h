/**
 *  \file TALOSRestraint.h  \brief TALOS dihedral restraint between four particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_TALOS_RESTRAINT_H
#define IMPISD_TALOS_RESTRAINT_H

#include "isd_config.h"
#include "Scale.h"
#include "vonMisesSufficient.h"

#include <IMP/Restraint.h>
#include <IMP/Particle.h>

IMPISD_BEGIN_NAMESPACE

//! phi/psi dihedral restraint between four particles, using data from TALOS.
class IMPISDEXPORT TALOSRestraint : public Restraint
{
public:
  //! Create the von Mises dihedral restraint on one dihedral angle given a number of observations.
  /** The likelihood is
       \f[ 
       f(\chi_{exp}^1,\cdots,\chi_{exp)^N|\chi(X),\kappa,I) 
        = \frac{\exp \left(N \kappa 
           \left( \cos \chi_{exp} \cos \chi(X) 
                + \sin \chi_{exp} \sin \chi(X)
           \right)\right)}
               {2\pi I_0(\kappa)^N} 
      \f]
      where the sufficient statistics are \f$N\f$ the number of observations, and
      \f[
        \cos\chi_{exp} = \frac{1}{N} \sum_{i=1}^N \cos\chi_{exp}^i
        \quad
        \sin\chi_{exp} = \frac{1}{N} \sum_{i=1}^N \sin\chi_{exp}^i
      \f]

      \param[in] p1 Pointer to first particle in dihedral restraint.
      \param[in] p2 Pointer to second particle in dihedral restraint.
      \param[in] p3 Pointer to third particle in dihedral restraint.
      \param[in] p4 Pointer to fourth particle in dihedral restraint.
      \param[in] kappa Pointer to the \f$\kappa\f$ concentration particle.
      \param[in] N Number of observations
      \param[in] cosbar Average of the cosines of the observations
      \param[in] sinbar Average of the sines of the observations

      \note For now, does not use vonMises.h for efficiency reasons.
   */
  TALOSRestraint(Particle* p1, Particle* p2, Particle* p3, Particle *p4, Particle *kappa, 
          unsigned N, double cosbar, double sinbar);


  //! compute sufficient statistics from a list of "observations"
  /** returns the number of observations, the average of cosines and the average of
      sines of the input values.
  */
  static Floats get_sufficient_statistics(Floats data);

  /* call for probability */
  double get_probability() const
  {
    return exp(-unprotected_evaluate(NULL));
  }

  IMP_RESTRAINT(TALOSRestraint);

private:
  IMP::Pointer<Particle> p_[4];
  IMP::Pointer<Particle> kappa_;
  IMP::Pointer<vonMisesSufficient> mises_;
  unsigned int N_;
  double cosbar_, sinbar_;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_TALOS_RESTRAINT_H */
