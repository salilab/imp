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
#include <IMP/container_macros.h>

IMPISD_BEGIN_NAMESPACE

AmbiguousNOERestraint::AmbiguousNOERestraint(PairContainer *pc,
                           kernel::Particle *sigma, kernel::Particle *gamma,
                           double Vexp) : pc_(pc), sigma_(sigma),
                                          gamma_(gamma), Vexp_(Vexp) {}

/* Apply the restraint to two atoms, two Scales, one experimental value.
 */
double
AmbiguousNOERestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(get_model(),
                  "You must at least register the restraint with the model"
                  << " before calling evaluate.");

  /* compute Icalc = 1/(gamma*d^6) where d = (sum d_i^-6)^(-1/6) */
  double vol = 0;
  Floats vols;
  IMP_CONTAINER_FOREACH(PairContainer, pc_,
                        {
            core::XYZ d0(get_model(), _1[0]);
            core::XYZ d1(get_model(), _1[1]);
            algebra::Vector3D c0 = d0.get_coordinates();
            algebra::Vector3D c1 = d1.get_coordinates();
            //will raise an error if c0 == c1
            double tmp = 1.0/(c0-c1).get_squared_magnitude();
            vols.push_back(IMP::cube(tmp)); // store di^-6
            vol += vols.back();
                        });
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
      IMP_CONTAINER_FOREACH(PairContainer, pc_,
                            {
            core::XYZ d0(get_model(), _1[0]);
            core::XYZ d1(get_model(), _1[1]);
            algebra::Vector3D c0 = d0.get_coordinates();
            algebra::Vector3D c1 = d1.get_coordinates();
            algebra::Vector3D deriv = DFM*factor*(c0-c1)*vols[_2]
                /(c0-c1).get_squared_magnitude();
            d0.add_to_derivatives(deriv, *accum);
            d1.add_to_derivatives( -deriv, *accum);
                            });
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp AmbiguousNOERestraint::do_get_inputs() const
{
  ModelObjectsTemp ret;
  ret+= IMP::get_particles(get_model(),
                           pc_->get_all_possible_indexes());
  ret.push_back(sigma_);
  ret.push_back(gamma_);

  ret.push_back(pc_);
  return ret;
}

IMPISD_END_NAMESPACE
