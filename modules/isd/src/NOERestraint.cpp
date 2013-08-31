/**
 *  \file isd/NOERestraint.cpp
 *  \brief Restrain a list of particle pairs with a lognormal+ISPA.
 *  NOTE: for now, the derivatives are written to all variables.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/NOERestraint.h>
#include <IMP/isd/FNormal.h>
#include <IMP/isd/Scale.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

NOERestraint::NOERestraint(kernel::Particle *p0, kernel::Particle *p1, kernel::Particle *sigma,
                           kernel::Particle *gamma, double Vexp)
    : p0_(p0), p1_(p1), sigma_(sigma), gamma_(gamma), Vexp_(Vexp) {}

/* Apply the restraint to two atoms, two Scales, one experimental value.
 */
double
NOERestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  core::XYZ d0(p0_);
  core::XYZ d1(p1_);
  Scale sigma_nuis(sigma_);
  Scale gamma_nuis(gamma_);
  /* compute Icalc */
  algebra::Vector3D c0 = d0.get_coordinates();
  algebra::Vector3D c1 = d1.get_coordinates();
  double diff = (c0-c1).get_magnitude();
  double gamma_val=gamma_nuis.get_scale();
  double sigma_val=sigma_nuis.get_scale();
  double Icalc = gamma_val*pow(diff,-6);
  /* compute all arguments to FNormal */
  double FA = log(Vexp_);
  double FM = log(Icalc);
  double JA = 1.0/Vexp_;
  IMP_NEW(FNormal, lognormal, (FA,JA,FM,sigma_val));
  //lognormal->set_was_used(true); // get rid of warning
  /* get score */
  double score= lognormal->evaluate();
  const_cast<NOERestraint *>(this)->set_chi(FA-FM);

  if (accum)
  {
      /* derivative for coordinates */
      double DFM = lognormal->evaluate_derivative_FM();
      double factor = -6/diff; /* d(log(gamma*pow(diff,-6)))/d(diff) */
      algebra::Vector3D deriv = DFM*factor*(c0-c1)/diff;
      d0.add_to_derivatives(deriv, *accum);
      d1.add_to_derivatives( -deriv, *accum);
      /* derivative for sigma */
      sigma_nuis.add_to_scale_derivative(
              lognormal-> evaluate_derivative_sigma(), *accum);
      /* derivative for gamma */
      gamma_nuis.add_to_scale_derivative(DFM/gamma_val, *accum);
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp NOERestraint::do_get_inputs() const
{
  kernel::ParticlesTemp ret;
  ret.push_back(p0_);
  ret.push_back(p1_);
  ret.push_back(sigma_);
  ret.push_back(gamma_);
  return ret;
}

IMPISD_END_NAMESPACE
