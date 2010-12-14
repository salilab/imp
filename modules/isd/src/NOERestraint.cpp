/**
 *  \file isd/NOERestraint.cpp
 *  \brief Restrain a list of particle pairs with a lognormal+ISPA.
 *  NOTE: for now, the derivatives are written to all variables.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/NOERestraint.h>
#include <IMP/isd/FNormal.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

NOERestraint::NOERestraint(Particle *p0, Particle *p1, 
			   Particle *sigma, Particle *gamma,
			   double Iexp) : p0_(p0), p1_(p1),
					  sigma_(sigma),
					  gamma_(gamma),
					  Iexp_(Iexp) {}
                                          
/* Apply the restraint to two atoms, two nuisances, one experimental value.
 */
double
NOERestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  core::XYZ d0(p0_);
  core::XYZ d1(p1_);
  Nuisance sigma_nuis(sigma_);
  Nuisance gamma_nuis(gamma_);
  /* compute Icalc */
  algebra::Vector3D c0 = d0.get_coordinates();
  algebra::Vector3D c1 = d1.get_coordinates();
  double diff = (c0-c1).get_magnitude();
  double gamma_val=gamma_nuis.get_nuisance();
  double sigma_val=sigma_nuis.get_nuisance();
  double Icalc = gamma_val*pow(diff,-6);
  /* compute all arguments to FNormal */
  double FA = log(Iexp_);
  double FM = log(Icalc);
  double JA = 1.0/Iexp_;
  IMP_NEW(FNormal, lognormal, (FA,JA,FM,sigma_val));
  /* get score */
  double score= lognormal->evaluate();
  if (accum)
  {
      /* derivative for coordinates */
      double DFM = lognormal->evaluate_derivative_FM();  
      double factor = -6/diff; /* d(log(gamma*pow(diff,-6)))/d(diff) */
      algebra::Vector3D deriv = DFM*factor*(c0-c1)/diff;
      d0.add_to_derivatives(deriv, *accum);
      d1.add_to_derivatives( -deriv, *accum);
      /* derivative for sigma */
      sigma_nuis.add_to_nuisance_derivative(
              lognormal-> evaluate_derivative_sigma(), *accum);
      /* derivative for gamma */
      gamma_nuis.add_to_nuisance_derivative(DFM*gamma_val, *accum);
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ParticlesTemp NOERestraint::get_input_particles() const
{
  ParticlesTemp ret;
  ret.push_back(p0_);
  ret.push_back(p1_);
  ret.push_back(sigma_);
  ret.push_back(gamma_);
  return ret;
}

/* The only container used is pc_. */
ContainersTemp NOERestraint::get_input_containers() const
{
  return ContainersTemp();
}

void NOERestraint::do_show(std::ostream& out) const
{
  out << "particle0= " << p0_->get_name() << std::endl;
  out << "particle1= " << p1_->get_name() << std::endl;
  out << "sigma= " << sigma_->get_name() << std::endl;
  out << "gamma= " << gamma_->get_name() << std::endl;
  out << "Iexp= " << Iexp_ << std::endl;
}

IMPISD_END_NAMESPACE
