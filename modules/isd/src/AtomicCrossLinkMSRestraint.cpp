/**
 *  \file isd/AtomicCrossLinkMSRestraint.h
 *  \brief A sigmoid shaped restraint between
 *  residues with discrete classifier
 *  and ambiguous assignment. To be used with
 *  cross-linking mass-spectrometry data.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/AtomicCrossLinkMSRestraint.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Scale.h>
#include <boost/math/special_functions/erf.hpp>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

namespace {
  const Float sq2=1.4142135623730951;
  const Float sqPi=1.7724538509055159;
  const Float sq2Pi=2.5066282746310002;
}
AtomicCrossLinkMSRestraint::AtomicCrossLinkMSRestraint(Model* m,
                                                       double xlen,
                                                       const ParticleIndex& psi,
                                                       Float slope,
                                                       bool part_of_log_score,
                                                       std::string name):
  kernel::Restraint(m,name),
  xlen_(xlen),
  psi_(psi),
  slope_(slope),
  part_of_log_score_(part_of_log_score)
  {
}

void AtomicCrossLinkMSRestraint::add_contribution(
                      const kernel::ParticleIndexPair& ppi,
                      const kernel::ParticleIndexPair& sigmas){

  ppis_.push_back(ppi);
  sigmass_.push_back(sigmas);
  default_range_.push_back((int)default_range_.size());
}

Float AtomicCrossLinkMSRestraint::evaluate_for_contributions(Ints c,
                                       DerivativeAccumulator *accum) const{
  Float score_accum = 1.0;
  algebra::Vector3Ds tmp_derivs(c.size());
  Floats tmp_scores(c.size());

  // loop over the contributions and score things
  for (Ints::const_iterator nit=c.begin();nit!=c.end();++nit){
    int n = *nit;
    core::XYZ d0(get_model(),ppis_[n][0]);
    core::XYZ d1(get_model(),ppis_[n][1]);

    Float s0 = isd::Scale(get_model(),sigmass_[n][0]).get_scale();
    Float s1 = isd::Scale(get_model(),sigmass_[n][1]).get_scale();

    Float sig  = std::sqrt(s0*s0+s1*s1);
    Float sig2 = sig*sig;
    Float dist = algebra::get_distance(d0.get_coordinates(),
                                       d1.get_coordinates());

    Float eLpR_2 = std::exp(-(dist+xlen_)*(dist+xlen_)/(2*sig2));
    //Float e2LR   = std::exp(2*xlen_*dist/sig2);

    Float log_eLpR_2 = -(dist+xlen_)*(dist+xlen_)/(2*sig2);
    Float log_e2LR   = 2*xlen_*dist/sig2;
    Float eLpR_2_e2LR = std::exp(log_eLpR_2+log_e2LR);


    Float erfLmR = boost::math::erf((xlen_-dist)/sq2/sig);
    Float erfLpR = boost::math::erf((xlen_+dist)/sq2/sig);
    Float score = -sig/(sq2Pi*dist) * (eLpR_2_e2LR - eLpR_2) + 0.5 * (erfLmR + erfLpR);

    // add the prior
    Float prior = std::exp(-slope_*dist);
    Float score_adj = 1-prior*score;
    score_accum*=score_adj;

    if (accum){
      tmp_scores[n] = score_adj;
      Float dd = 1.0/(sq2Pi*dist*dist*sig) * (eLpR_2*(-sig2 - dist*xlen_) + eLpR_2_e2LR*(sig2-dist*xlen_));
      dd = prior*(slope_*score - dd);
      tmp_derivs[n] = -dd/dist * (d1.get_coordinates()-d0.get_coordinates());
    }
  }
  Float psi  = isd::Scale(get_model(),psi_).get_scale();
  Float like = psi*score_accum + (1-psi)*(1-score_accum);

  // final derivative calculation (needs the scores already calculated)
  if (accum){
    for (Ints::const_iterator nit=c.begin();nit!=c.end();++nit){
      int n = *nit;
      core::XYZ d0(get_model(),ppis_[n][0]);
      core::XYZ d1(get_model(),ppis_[n][1]);

      algebra::Vector3D deriv = -1/like*(2*psi-1) * score_accum/tmp_scores[n] * tmp_derivs[n];
      d0.add_to_derivatives(deriv,*accum);
      d1.add_to_derivatives(-1.0*deriv,*accum);
    }
  }
  if (part_of_log_score_) return like;
  else return -log(like);
}


double AtomicCrossLinkMSRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  return evaluate_for_contributions(default_range_,accum);
}

ModelObjectsTemp AtomicCrossLinkMSRestraint::do_get_inputs() const {
    ParticlesTemp ret;
    for (unsigned int k = 0; k < get_number_of_contributions(); ++k) {
        ret.push_back(get_model()->get_particle(ppis_[k][0]));
        ret.push_back(get_model()->get_particle(ppis_[k][1]));
        ret.push_back(get_model()->get_particle(sigmass_[k][0]));
        ret.push_back(get_model()->get_particle(sigmass_[k][1]));
    }
    ret.push_back(get_model()->get_particle(psi_));
    return ret;
}

IMPISD_END_NAMESPACE
