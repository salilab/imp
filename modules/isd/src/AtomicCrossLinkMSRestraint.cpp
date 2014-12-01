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

  struct XLIntermed {
    Float dist,prior,sig;
    Float e2LR,eLpR_2,erfLmR,erfLpR;
    Float score,like;
    XLIntermed(Float dist_,Float prior_,Float sig_,
               Float e2LR_,Float eLpR_2_,Float erfLmR_,Float erfLpR_,
               Float score_,Float like_):
      dist(dist_),
      prior(prior_),
      sig(sig_),
      e2LR(e2LR_),
      eLpR_2(eLpR_2_),
      erfLmR(erfLmR_),
      erfLpR(erfLpR_),
      score(score_),
      like(like_)
      {}
  };
}

AtomicCrossLinkMSRestraint::AtomicCrossLinkMSRestraint(Model* m,
                                                       double xlen,
                                                       Float slope,
                                                       bool i_am_part_of_log_score,
                                                       std::string name):
  kernel::Restraint(m,name),
  xlen_(xlen),
  slope_(slope),
  i_am_part_of_log_score_(i_am_part_of_log_score)
  {
}

void AtomicCrossLinkMSRestraint::add_contribution(
                      const kernel::ParticleIndexPair& ppi,
                      const kernel::ParticleIndexPair& sigmas,
                      const ParticleIndex& psi){

  ppis_.push_back(ppi);
  sigmass_.push_back(sigmas);
  psis_.push_back(psi);
  default_range_.push_back((int)default_range_.size());
}

Floats AtomicCrossLinkMSRestraint::get_contribution_scores(int idx) const{
  Floats ret;
  ret.push_back(core::get_distance(core::XYZ(get_model(),ppis_[idx][0]),
                                   core::XYZ(get_model(),ppis_[idx][1])));
  ret.push_back(isd::Scale(get_model(),sigmass_[idx][0]).get_scale());
  ret.push_back(isd::Scale(get_model(),sigmass_[idx][1]).get_scale());
  ret.push_back(isd::Scale(get_model(),psis_[idx]).get_scale());
  return ret;
}

Float AtomicCrossLinkMSRestraint::evaluate_for_contributions(Ints c,
                                       DerivativeAccumulator *accum) const{
  Float like=1.0;
  std::vector<XLIntermed> stuff;

  // loop over the contributions and score things
  for (Ints::const_iterator nit=c.begin();nit!=c.end();++nit){
    int n = *nit;
    core::XYZ d0(get_model(),ppis_[n][0]);
    core::XYZ d1(get_model(),ppis_[n][1]);

    Float s0 = isd::Scale(get_model(),sigmass_[n][0]).get_scale();
    Float s1 = isd::Scale(get_model(),sigmass_[n][1]).get_scale();
    Float psi  = isd::Scale(get_model(),psis_[n]).get_scale();

    Float sig  = std::sqrt(s0*s0+s1*s1);
    Float sig2 = sig*sig;
    Float dist = algebra::get_distance(d0.get_coordinates(),
                                       d1.get_coordinates());
    Float eLpR_2 = std::exp(-(dist+xlen_)*(dist+xlen_)/(2*sig2));
    Float e2LR   = std::exp(2*xlen_*dist/sig2);
    Float erfLmR = boost::math::erf((xlen_-dist)/sq2/sig);
    Float erfLpR = boost::math::erf((xlen_+dist)/sq2/sig);
    Float score = -sig/(sq2Pi*dist) * eLpR_2 * (e2LR-1) + 0.5 * (erfLmR + erfLpR);

    // add the prior and psi
    Float prior = std::exp(-slope_*dist);
    Float f = psi*(1-score) + (1-psi)*score;
    Float cur_like = 1-f*prior;
    like*=cur_like;
    if (accum){
      stuff.push_back(XLIntermed(dist,prior,sig,e2LR,eLpR_2,erfLmR,erfLpR,score,cur_like));
    }
  }

  if (accum){
    for (Ints::const_iterator nit=c.begin();nit!=c.end();++nit){
      int n = *nit;
      core::XYZ d0(get_model(),ppis_[n][0]);
      core::XYZ d1(get_model(),ppis_[n][1]);
      Float psi  = isd::Scale(get_model(),psis_[n]).get_scale();
      XLIntermed &X = stuff[n];

      Float C = like/X.like;
      Float dscore = X.eLpR_2*(-xlen_*X.dist-X.sig*X.sig+X.e2LR*(-xlen_*X.dist+X.sig*X.sig)) /
        (sq2Pi*X.dist*X.dist*X.sig);
      Float dlike = C*X.prior*(-slope_*psi+(2*psi-1)*(slope_*X.score-dscore));

      if (i_am_part_of_log_score_) dlike/=(like-1);
      algebra::Vector3D deriv = -dlike/X.dist*(d1.get_coordinates()-d0.get_coordinates());

      d0.add_to_derivatives(deriv,*accum);
      d1.add_to_derivatives(-1.0*deriv,*accum);
    }
  }
  return 1-like;
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
        ret.push_back(get_model()->get_particle(psis_[k]));
    }
    return ret;
}

IMPISD_END_NAMESPACE
