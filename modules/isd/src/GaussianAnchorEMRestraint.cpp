/**
 *  \file isd/GaussianAnchorEMRestraint.cpp
 *  \brief Restrain two sets of anchor points (e.g. protein and EM map)
 *  NOTE: derivitives not implemented
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/GaussianAnchorEMRestraint.h>
#include <math.h>
#include <IMP/PairContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/container_macros.h>

IMPISD_BEGIN_NAMESPACE

double
GaussianAnchorEMRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
  const {
  //score is the square difference between two GMMs
  float scale=Scale(global_sigma_).get_scale();
  FloatsList mm,md;
  double mm_score=0.0,md_score=0.0;
  if (rigid_) mm_score=init_mm_score_;
  typedef std::map<ParticlePair,Float>::const_iterator ppiter;
  IMP_CONTAINER_FOREACH(container::CloseBipartitePairContainer,
                        md_container_,{
         core::XYZ d1(get_model(),_1[0]);
         core::XYZ d2(get_model(),_1[1]);
         Float dist=core::get_distance(d1,d2);
         ParticlePair pp(get_model()->get_particle(_1[0]),
                         get_model()->get_particle(_1[1]));
         ppiter iter_pre=md_prefactors_.find(pp);
         ppiter iter_prod=md_prods_.find(pp);
         md_score+=iter_pre->second*calc_score(dist,iter_prod->second);
                        });

  double log_score=log(sqrt(2*algebra::PI)*scale)+1.0/(2*scale*scale)*
    (mm_score+dd_score_-2.0*md_score);

  IMP_LOG(VERBOSE,"md: "<<md_score
          <<" mm: "<<mm_score
          <<" dd: "<<dd_score_
          <<" scale: "<<scale
          <<" final: "<<log_score<<std::endl);

  if (accum){
  }
  return log_score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp GaussianAnchorEMRestraint::do_get_inputs() const {
  ModelObjectsTemp ret = model_ps_;
  ret.push_back(md_container_);
  return ret;
}

IMPISD_END_NAMESPACE
