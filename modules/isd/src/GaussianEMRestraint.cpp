/**
 *  \file isd/GaussianEMRestraint.cpp
 *  \brief Restraint two sets of gaussians (model and gmm derived from EM map)
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/GaussianEMRestraint.h>
#include <IMP/math.h>
#include <IMP/atom/Atom.h>
#include <IMP/algebra/eigen3/Eigen/LU>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/isd/em_utilities.h>

IMPISD_BEGIN_NAMESPACE

GaussianEMRestraint::GaussianEMRestraint(
                         Model *mdl,
                         ParticleIndexes model_ps, ParticleIndexes density_ps,
                         ParticleIndex global_sigma,
                         Float model_cutoff_dist,Float density_cutoff_dist,
                         Float slope,
                         bool update_model, bool backbone_slope,
                         bool local,
                         std::string name):
  Restraint(mdl,name),
  model_ps_(model_ps),
  density_ps_(density_ps),
  global_sigma_(global_sigma),
  slope_(slope),
  update_model_(update_model),
  local_(local){

    msize_=model_ps.size();
    dsize_=density_ps.size();

    // check to make sure all particles are Gaussian and Mass
    for (int i=0;i<msize_;i++){
      IMP_USAGE_CHECK(core::Gaussian::get_is_setup(mdl, model_ps_[i]),
                      "Model particles must be Gaussian");
      IMP_USAGE_CHECK(atom::Mass::get_is_setup(mdl, model_ps_[i]),
                      "Model particles must have Mass");
    }
    for (int j=0;j<dsize_;j++){
      IMP_USAGE_CHECK(core::Gaussian::get_is_setup(mdl, density_ps_[j]),
                      "Density particles must be Gaussian");
      IMP_USAGE_CHECK(atom::Mass::get_is_setup(mdl, density_ps_[j]),
                      "Density particles must have Mass");
    }

    //Set up md container
    md_container_ = new container::CloseBipartitePairContainer(
         new container::ListSingletonContainer(mdl,model_ps),
         new container::ListSingletonContainer(mdl,density_ps),density_cutoff_dist);

    mm_container_ = new container::ClosePairContainer(
         new container::ListSingletonContainer(mdl,model_ps),model_cutoff_dist);

    compute_initial_scores();

    if (backbone_slope){
      for (size_t nm=0;nm<model_ps_.size();nm++){
        atom::AtomType a = atom::Atom(mdl,model_ps[nm]).get_atom_type();
        if (a == atom::AtomType("CA") ||
            a == atom::AtomType("C") ||
            a == atom::AtomType("N")) {
          slope_ps_.push_back(model_ps_[nm]);
        }
      }
      std::cout<<"limited slope to "<<slope_ps_.size()<<" ps out of "
               <<model_ps_.size()<<std::endl;
    }
    else slope_ps_ = model_ps_;
  }

void GaussianEMRestraint::compute_initial_scores() {

  // precalculate DD score
  IMP_Eigen::Vector3d deriv;
  dd_score_=0.0;
  self_mm_score_=0.0;
  for (int i1=0;i1<dsize_;i1++){
    for (int i2=0;i2<dsize_;i2++){
      Float score = score_gaussian_overlap(get_model(),
                    ParticleIndexPair(density_ps_[i1],density_ps_[i2]),
                    &deriv);
      dd_score_+=score;
    }
  }

  // precalculate the self-mm score and initialize
  for (int i=0;i<msize_;i++){
    Float score = score_gaussian_overlap(get_model(),
                    ParticleIndexPair(model_ps_[i],model_ps_[i]),
                    &deriv);
    self_mm_score_+=score;
  }
  //std::cout<<"init dd: "<<dd_score_<<" init mm "<<self_mm_score_<<std::endl;
}

double GaussianEMRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
  const {
  //score is the square difference between two GMMs
  //Float scale=isd::Scale(get_model(),global_sigma_).get_scale();
  KahanAccumulation md_score,mm_score;
  mm_score = KahanSum(mm_score,self_mm_score_);
  IMP_Eigen::Vector3d deriv;
  //std::cout<<"\neval"<<std::endl;
  boost::unordered_map<ParticleIndex,KahanVectorAccumulation> derivs_mm,derivs_md,slope_md;
  std::set<ParticleIndex> local_dens;
  //boost::unordered_map<ParticleIndexPair,Float> md_dists(msize_*dsize_);

  Float slope_score=0.0;

  if (slope_>0.0){
    for (ParticleIndexes::const_iterator mit=slope_ps_.begin();
         mit!=slope_ps_.end();++mit){
      for (ParticleIndexes::const_iterator dit=density_ps_.begin();
           dit!=density_ps_.end();++dit){
        IMP_Eigen::Vector3d v = IMP_Eigen::Vector3d(core::XYZ(get_model(),*mit).
                                                get_coordinates().get_data()) -
                                IMP_Eigen::Vector3d(core::XYZ(get_model(),*dit).
                                                get_coordinates().get_data());
        Float sd = v.norm();
        //md_dists[ParticleIndexPair(*mit,*dit)]=dist;
        slope_md[*mit] = KahanVectorSum(slope_md[*mit],v*slope_/sd);
        slope_score+=slope_*sd;
      }
    }
  }

  IMP_CONTAINER_FOREACH(container::ClosePairContainer,
                          mm_container_,{
      Float score = score_gaussian_overlap(get_model(),_1,&deriv);
      mm_score = KahanSum(mm_score,2*score);
      if (accum) {
        //multiply by 2 because...
        derivs_mm[_1[0]] = KahanVectorSum(derivs_mm[_1[0]],-2.0*deriv);
        derivs_mm[_1[1]] = KahanVectorSum(derivs_mm[_1[1]],2.0*deriv);
      }
  });

  //std::cout<<"calculating MD score"<<std::endl;
  IMP_CONTAINER_FOREACH(container::CloseBipartitePairContainer,
                        md_container_,{
    Float score = score_gaussian_overlap(get_model(),_1,&deriv);
    md_score = KahanSum(md_score,score);
    if (local_) local_dens.insert(_1[1]);
    if (accum) {
      derivs_md[_1[0]] = KahanVectorSum(derivs_md[_1[0]],-deriv);
    }
  });


  //local gets new DD score each time
  Float dd_score = 0.0;
  if (local_){
    for(std::set<ParticleIndex>::const_iterator it1=local_dens.begin();
        it1!=local_dens.end();++it1){
      for(std::set<ParticleIndex>::const_iterator it2=local_dens.begin();
          it2!=local_dens.end();++it2){
        Float score = score_gaussian_overlap(get_model(),
                                             ParticleIndexPair(*it1,*it2),
                                             &deriv);
        dd_score+=score;
      }
    }
  }
  else dd_score = dd_score_;

  /* distance calculation */
  double cc=2*md_score.sum/(mm_score.sum+dd_score);
  //double cc=2*md_score/(dd_score_);
  double log_score=-std::log(cc) + slope_score;
  //double dist=-std::log(cc+std::numeric_limits<float>::min()*(1-cc));
  //double dist=mm_score+dd_score_-2*md_score;

  /* energy calculation */
  //double log_score=std::log(2)+std::log(dist);
  //double log_score=dist;

  //std::cout<<"\ndd: "<<dd_score_<<" mm: "<<mm_score.sum<<" md: "<<md_score.sum<<" dist: "<<dist<<" slope score: "<<slope_score<<std::endl;

  if (accum){
    for (ParticleIndexes::const_iterator it=model_ps_.begin();
         it!=model_ps_.end();++it){
      if (IMP::isinf(log_score) || log_score==0.0) {
        core::XYZ(get_model(),*it).add_to_derivatives(algebra::Vector3D(0,0,0),
                                                      *accum);
      }
      else{
        algebra::Vector3D d_mm(derivs_mm[*it].sum[0],derivs_mm[*it].sum[1],derivs_mm[*it].sum[2]);
        algebra::Vector3D d_md(derivs_md[*it].sum[0],derivs_md[*it].sum[1],derivs_md[*it].sum[2]);
        Float mmdd=mm_score.sum+dd_score;
        algebra::Vector3D d = -2.0 / cc * (mmdd*d_md - md_score.sum*d_mm) / (mmdd * mmdd);
        //std::cout<<"deriv calc-> d_mm: "<<d_mm<<" d_md: "<<d_md<<" mmdd: "<<mmdd<<" d: "<<d<<std::endl;
        d += algebra::Vector3D(slope_md[*it].sum[0],slope_md[*it].sum[1],slope_md[*it].sum[2]);
        core::XYZ(get_model(),*it).add_to_derivatives(d,*accum);
      }
    }
  }
  return log_score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp GaussianEMRestraint::do_get_inputs() const {
  ModelObjectsTemp ret;
  for (int i=0;i<msize_;i++){
    ret.push_back(get_model()->get_particle(model_ps_[i]));
  }
  for (int j=0;j<dsize_;j++){
    ret.push_back(get_model()->get_particle(density_ps_[j]));
  }
  ret.push_back(get_model()->get_particle(global_sigma_));
  ret.push_back(md_container_);
  ret.push_back(mm_container_);
  return ret;
}

IMPISD_END_NAMESPACE
