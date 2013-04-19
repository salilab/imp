/**
 *  \file isd/AmbiguousNOERestraint.cpp
 *  \brief Restrain a list of particle pairs with a lognormal+ISPA.
 *  NOTE: for now, the derivatives are written to all variables.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/AmbiguousNOERestraint.h>
#include <IMP/isd/FNormal.h>
#include <IMP/isd/Scale.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>
#include <math.h>
#include <IMP/PairContainer.h>

IMPISD_BEGIN_NAMESPACE

AmbiguousNOERestraint::AmbiguousNOERestraint(PairContainer *pc,
                           Particle *sigma, Particle *gamma,
                           double Vexp) : pc_(pc), sigma_(sigma),
                                          gamma_(gamma), Vexp_(Vexp) {}

/* Apply the restraint to two atoms, two Scales, one experimental value.
 */
double
AmbiguousNOERestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  /* compute Icalc = 1/(gamma*d^6) where d = (sum d_i^-6)^(-1/6) */
  double vol = 0;
  Floats vols;
  int num = pc_->get_number_of_particle_pairs();
  for (int i=0; i<num; ++i)
  {
        ParticlePair it=pc_->get_particle_pair(i);
        IMP::core::XYZ d0=IMP::core::XYZ(it[0]);
        IMP::core::XYZ d1=IMP::core::XYZ(it[1]);
        algebra::Vector3D c0 = d0.get_coordinates();
        algebra::Vector3D c1 = d1.get_coordinates();
        //will raise an error if c0 == c1
        double tmp = 1.0/(c0-c1).get_squared_magnitude();
        vols.push_back(IMP::cube(tmp)); // store di^-6
        vol += vols.back();
  }
  Scale gamma_scale(gamma_);
  Scale sigma_scale(sigma_);
  double gamma_val=gamma_scale.get_scale();
  double sigma_val=sigma_scale.get_scale();
  double Icalc = gamma_val*vol;
  /* compute all arguments to FNormal */
  double FA = log(Vexp_);
  double FM = log(Icalc);
  double JA = 1.0/Vexp_;
  IMP_NEW(FNormal, lognormal, (FA,JA,FM,sigma_val));
  //lognormal->set_was_used(true); // get rid of warning
  /* get score */
  double score= lognormal->evaluate();
  const_cast<AmbiguousNOERestraint *>(this)->set_chi(FA-FM);
  if (accum)
  {
      /* derivative for gamma */
      double DFM = lognormal->evaluate_derivative_FM();
      gamma_scale.add_to_scale_derivative(DFM/gamma_val, *accum);
      /* derivative for sigma */
      sigma_scale.add_to_scale_derivative(
              lognormal->evaluate_derivative_sigma(), *accum);
      /* derivative for coordinates */
      double factor = -6/vol;
      for (int i=0; i < num; i++)
      {
        ParticlePair it=pc_->get_particle_pair(i);
        IMP::core::XYZ d0=IMP::core::XYZ(it[0]);
        IMP::core::XYZ d1=IMP::core::XYZ(it[1]);
        algebra::Vector3D c0 = d0.get_coordinates();
        algebra::Vector3D c1 = d1.get_coordinates();
        algebra::Vector3D deriv = DFM*factor*(c0-c1)*vols[i]
                                  /(c0-c1).get_squared_magnitude();
        d0.add_to_derivatives(deriv, *accum);
        d1.add_to_derivatives( -deriv, *accum);
      }
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ParticlesTemp AmbiguousNOERestraint::get_input_particles() const
{
  ParticlesTemp ret;
  int num = pc_->get_number_of_particle_pairs();
  for (int i=0; i<num; ++i)
  {
    ParticlePair it=pc_->get_particle_pair(i);
    ret.push_back(it[0]);
    ret.push_back(it[1]);
  }
  ret.push_back(sigma_);
  ret.push_back(gamma_);
  return ret;
}

/* The only container used is pc_. */
ContainersTemp AmbiguousNOERestraint::get_input_containers() const
{
  ContainersTemp ret;
  ret.push_back(pc_);
  return ret;
}

void AmbiguousNOERestraint::do_show(std::ostream& out) const
{
  int num = pc_->get_number_of_particle_pairs();
  for (int i=0; i<num; ++i)
  {
    ParticlePair it=pc_->get_particle_pair(i);
    out << "pair " << i+1 << " particle0= " << it[0]->get_name() << std::endl;
    out << "pair " << i+1 << " particle1= " << it[1]->get_name() << std::endl;
  }
  out << "sigma= " << sigma_->get_name() << std::endl;
  out << "gamma= " << gamma_->get_name() << std::endl;
  out << "Vexp= " << Vexp_ << std::endl;
}

IMPISD_END_NAMESPACE
