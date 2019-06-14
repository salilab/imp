/**
 *  \file IMP/isd/GaussianAnchorEMRestraint.h
 *  \brief Restraint between two sets of anchor points "model" and "EM density"
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_GAUSSIAN_ANCHOR_EMRESTRAINT_H
#define IMPISD_GAUSSIAN_ANCHOR_EMRESTRAINT_H

#include "isd_config.h"
#include <IMP/isd/ISDRestraint.h>
#include <IMP/PairContainer.h>
#include <IMP/isd/Scale.h>
#include <IMP/core/XYZ.h>
#include <IMP/container/CloseBipartitePairContainer.h>
#include <map>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

//! Restraint between two sets of anchor points "model" and "EM density"
/** \note This class is obsolete - use GaussianEMRestraint instead. It exists
          solely to support older applications of IMP that used this method.
 */
class IMPISDEXPORT GaussianAnchorEMRestraint : public ISDRestraint
{
public:
  //! Constructor
  /**
      \param[in] model_ps particles for the model GMM
      \param[in] model_sigs sigmas for the model GMM
                            (currently spherically symmetric)
      \param[in] model_weights weights for the model GMM
      \param[in] density_ps particles for the density GMM
      \param[in] density_sigs sigmas for the density GMM
                            (currently spherically symmetric)
      \param[in] density_weights weights for the density GMM
      \param[in] global_sigma Particle to modulate the uncertainty
      \param[in] rigid Set to true if the model is rigid (faster)
   */
  GaussianAnchorEMRestraint(ParticlesTemp model_ps, Floats model_sigs,
                      Floats model_weights, ParticlesTemp density_ps,
                      Floats density_sigs, Floats density_weights,
                      Particle *global_sigma,Float cutoff_dist,
                      bool rigid, bool tabexp,
                      std::string name="GaussianAnchorEMRestraint%1%"):
  ISDRestraint(model_ps[0]->get_model(),name),
     model_ps_(model_ps),
    model_sigs_(model_sigs),
    model_weights_(model_weights),
    density_ps_(density_ps),
    density_sigs_(density_sigs),
    density_weights_(density_weights),
    global_sigma_(global_sigma),
    cutoff_dist_(cutoff_dist),
    rigid_(rigid),
    tabexp_(tabexp){
      msize_=model_ps.size();
      dsize_=density_ps.size();
      IMP_USAGE_CHECK(model_sigs.size()==msize_ &&
                      model_weights.size()==msize_,
                      "All model inputs must be same size");
      IMP_USAGE_CHECK(density_ps.size()==dsize_ &&
                      density_sigs.size()==dsize_ &&
                      density_weights.size()==dsize_,
                      "All input vectors must be same size");

      if (tabexp_){
         unsigned exparg_grid_size=1000001;
         argmax_=10.0;
         invdx_=double(exparg_grid_size)/argmax_;
         for(unsigned k=0;k<exparg_grid_size;++k){
            double argvalue=double(k)/invdx_;
            exp_grid_.push_back(std::exp(-argvalue));
         }
      }


      // calculating prefactors and initial model-model score
      init_mm_score_=0.0;
      for (size_t m1=0;m1<msize_;m1++){
        init_mm_score_+=calc_prefactor(model_sigs_[m1],model_weights_[m1],
                                       model_sigs[m1],model_weights_[m1]);
        for (size_t m2=m1;m2<msize_;m2++){
          Float dist=core::get_distance(core::XYZ(model_ps_[m1]),
                                        core::XYZ(model_ps_[m2]));
          ParticlePair pp1(model_ps_[m1],model_ps_[m2]);
          ParticlePair pp2(model_ps_[m2],model_ps_[m1]);
          Float pref=calc_prefactor(model_sigs_[m1],model_weights_[m1],
                                    model_sigs[m2],model_weights_[m2]);
          Float prod=calc_prod(model_sigs_[m1],model_sigs_[m2]);
          mm_prefactors_[pp1]=pref;
          mm_prefactors_[pp2]=pref;
          mm_prods_[pp1]=prod;
          mm_prods_[pp2]=prod;
          if (m2>m1){
            init_mm_score_+=2.0*pref*calc_score(dist,prod);
          }
        }

        //model-density prefactors
        for (size_t d1=0;d1<dsize_;d1++){
          ParticlePair ppd1(model_ps_[m1],density_ps_[d1]);
          md_prefactors_[ppd1]=
               calc_prefactor(model_sigs_[m1],model_weights_[m1],
                              density_sigs[d1],density_weights_[d1]);
          md_prods_[ppd1]=calc_prod(model_sigs_[m1],density_sigs[d1]);
        }
      }

      // calculating density-density score (doesn't change)
      for (size_t d1=0;d1<dsize_;d1++){
        core::XYZ d_d1(density_ps_[d1]);
        dd_score_+=calc_prefactor(density_sigs_[d1],density_weights_[d1],
                                  density_sigs_[d1],density_weights_[d1]);
        for (size_t d2=d1+1;d2<dsize_;d2++){
          Float dist=core::get_distance(core::XYZ(density_ps_[d1]),
                                        core::XYZ(density_ps_[d2]));
          Float pref=calc_prefactor(density_sigs_[d1],density_weights_[d1],
                                 density_sigs_[d2],density_weights_[d2]);
          Float prod=calc_prod(density_sigs_[d1],density_sigs_[d2]);
          dd_score_+=2.0*pref*calc_score(dist,prod);
        }
      }
      //KLUGE!
      if (rigid_){
        init_mm_score_=0.0;
        dd_score_=0.0;
      }

      //Set up md container
      md_container_ = new container::CloseBipartitePairContainer(
                              model_ps,density_ps,cutoff_dist_);
    }
  ParticlesTemp get_density_particles() const {
    ParticlesTemp ret =  density_ps_;
    return ret;
  }
  double get_probability() const {
    return exp(-unprotected_evaluate(NULL));
  }

  virtual double
    unprotected_evaluate(IMP::DerivativeAccumulator *accum) const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(GaussianAnchorEMRestraint);
 protected:
  Particles model_ps_;
  Floats model_sigs_,model_weights_;
  Particles density_ps_;
  Floats density_sigs_,density_weights_;
  Particle * global_sigma_;
  Float cutoff_dist_;
  bool rigid_;
  bool tabexp_;
  Float dd_score_,init_mm_score_;
  std::map<ParticlePair,Float> mm_prefactors_,
    md_prefactors_,mm_prods_,md_prods_;
  size_t msize_,dsize_;
  //variables needed to tabulate the exponential
  Floats exp_grid_;
  double invdx_;
  double argmax_;
  PointerMember<container::CloseBipartitePairContainer> md_container_;
  //actual scores defined here (prefactor*score)
  inline Float calc_prefactor(Float s1, Float w1, Float s2, Float w2) const{
    return w1*w2*sqrt(8*algebra::PI)*s1*s1*s2*s2/
      pow(s1*s1+s2*s2,1.5);
  }
  inline Float calc_prod(Float s1, Float s2) const{
    return 1.0/(2.0*(s1*s1+s2*s2));
  }
  inline Float calc_score (Float dist, Float prod) const{
    double argvalue=dist*dist*prod;
    double expn;
    if (tabexp_){
         double minarg=std::min(argvalue,argmax_);
         unsigned k = static_cast<unsigned>( std::floor(minarg*invdx_) );
         expn=exp_grid_[k];
    }
    else{
         expn=std::exp(-argvalue);
    }
    return expn;
  }
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_GAUSSIAN_ANCHOR_EMRESTRAINT_H */
