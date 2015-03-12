/**
 *  \file IMP/isd/LognormalAmbiguousRestraint.cpp
 *  \brief A sigmoid shaped restraint between
 *  residues with discrete classifier
 *  and ambiguous assignment. To be used with
 *  cross-linking mass-spectrometry data.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/LognormalAmbiguousRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Scale.h>
#include <IMP/constants.h>
#include <math.h>
#include <IMP/PairContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <boost/random/uniform_real.hpp>

IMPISD_BEGIN_NAMESPACE

LognormalAmbiguousRestraint::LognormalAmbiguousRestraint(
   Particle *p1, Particle *p2, Particle *ki, Particle *sigmaG):
   Restraint(p1->get_model(), "LognormalAmbiguousRestraint%1%"),
   p1_(p1), p2_(p2),  ki_(ki), sigmaG_(sigmaG) {}


// LognormalAmbiguousRestraint:: LognormalAmbiguousRestraint(double
//                                                        smin, double smax)
//                                             : smin_(smin), smax_(smax) {}

// add a contribution: simple case



void LognormalAmbiguousRestraint::add_contribution(double lexp,
                                                     Particle *omega)
{
  lexp_.push_back(lexp);
  omegas_.push_back(omega);
}


void LognormalAmbiguousRestraint::draw_k_from_posterior(double kt)
{
  Floats scores, xi;
  double sumi=0.0;
  for(unsigned k=0; k<get_number_of_contributions();++k){
     Scale(ki_).set_scale((double)k);
     scores.push_back(unprotected_evaluate(NULL));
     xi.push_back(exp(-(scores[k]-scores[0])/kt));
     sumi+=xi[k];
  }

  Floats pi;
  for(unsigned k=0; k<get_number_of_contributions();++k){
    pi.push_back(xi[k]/sumi);
  }

  boost::uniform_real<> rand(0.0,1.0);
  double a =rand(IMP::base::random_number_generator);

  sumi=0.0;
  for(unsigned k=0; k<get_number_of_contributions();++k){
    if(a>=sumi && a<sumi+pi[k]){
      Scale(ki_).set_scale((double)k);
      break;
    } else {
      sumi+=pi[k];
    }
  }
}

double LognormalAmbiguousRestraint::
                 unprotected_evaluate(DerivativeAccumulator *accum) const
{
    unsigned ki = unsigned(std::floor(Scale(ki_).get_scale()+0.5));
    IMP_USAGE_CHECK(ki < get_number_of_contributions(),
            "assignment number out of bounds " );


    core::XYZ d1(p1_),d2(p2_);
    double dist = (d1.get_coordinates() -
           d2.get_coordinates()).get_magnitude();

    double sigma_val =
             Scale(omegas_[ki]).get_scale()*Scale(sigmaG_).get_scale();

    double eps = dist/lexp_[ki];

    double bcf = log(sqrt(IMP::PI*2.0)*sigma_val*lexp_[ki]);

    double log_eps=log(eps);

    double ccf = log_eps*log_eps/2.0/sigma_val/sigma_val;

    double score=bcf+ccf;

   if (accum)
   {
     algebra::Vector3D dv=d1.get_coordinates() -
       d2.get_coordinates();

     double ef = log_eps*2.0/sigma_val/sigma_val/dist/dist;
     FloatKeys xyz_keys=IMP::core::XYZ::get_xyz_keys();
     p1_->add_to_derivative(xyz_keys[0], ef*dv[0],
                                         *accum);
     p1_->add_to_derivative(xyz_keys[1], ef*dv[1],
                                         *accum);
     p1_->add_to_derivative(xyz_keys[2], ef*dv[2],
                                         *accum);
     p2_->add_to_derivative(xyz_keys[0], -ef*dv[0],
                                         *accum);
     p2_->add_to_derivative(xyz_keys[1], -ef*dv[1],
                                         *accum);
     p2_->add_to_derivative(xyz_keys[2], -ef*dv[2],
                                         *accum);
   }
    return score;
}


/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp LognormalAmbiguousRestraint::do_get_inputs() const
{
  ParticlesTemp ret;
  for (unsigned i=0; i<get_number_of_contributions(); ++i)
  {
      ret.push_back(omegas_[i]);
  }
  ret.push_back(sigmaG_);
  ret.push_back(p1_);
  ret.push_back(p2_);
  ret.push_back(ki_);
  return ret;
}

IMPISD_END_NAMESPACE
