/**
 *  \file isd/GaussianEMRestraint.cpp
 *  \brief Restrain two sets of Gaussians (model and GMM derived from EM map)
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/GaussianEMRestraint.h>
#include <IMP/math.h>
#include <IMP/atom/Atom.h>
#include <Eigen/LU>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/isd/em_utilities.h>

IMPISD_BEGIN_NAMESPACE

namespace {

struct KahanAccumulation{
  double sum;
  double correction;
  KahanAccumulation(): sum(0.0),correction(0.0) {}
};
struct KahanVectorAccumulation{
  Eigen::Vector3d sum;
  Eigen::Vector3d correction;
  KahanVectorAccumulation(): sum(Eigen::Vector3d(0,0,0)),
                             correction(Eigen::Vector3d(0,0,0)) {}
};
inline KahanAccumulation kahan_sum(KahanAccumulation accumulation,
                                   double value) {
  KahanAccumulation result;
  double y = value - accumulation.correction;
  double t = accumulation.sum + y;
  result.correction = (t - accumulation.sum) - y;
  result.sum = t;
  return result;
}
inline KahanVectorAccumulation
kahan_vector_sum(KahanVectorAccumulation accumulation,
                 Eigen::Vector3d value) {
  KahanVectorAccumulation result;
  Eigen::Vector3d y = value - accumulation.correction;
  Eigen::Vector3d t = accumulation.sum + y;
  result.correction = (t - accumulation.sum) - y;
  result.sum = t;
  return result;
}

} // anonymous namespace

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
  model_cutoff_dist_(model_cutoff_dist),
  density_cutoff_dist_(density_cutoff_dist),
  model_ps_(model_ps),
  density_ps_(density_ps),
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
      IMP_LOG_TERSE("Limited slope to " << slope_ps_.size()
                    << " particles out of " << model_ps_.size() << std::endl);
    }
    else slope_ps_ = model_ps_;
  }

void GaussianEMRestraint::compute_initial_scores() {

  // precalculate DD score
  Eigen::Vector3d deriv;
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
}

double GaussianEMRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
  const {
  //score is the square difference between two GMMs
  KahanAccumulation md_score,mm_score;
  mm_score = kahan_sum(mm_score,self_mm_score_);
  Eigen::Vector3d deriv;
  boost::unordered_map<ParticleIndex,KahanVectorAccumulation> derivs_mm,derivs_md,slope_md;
  std::set<ParticleIndex> local_dens;

  Float slope_score=0.0;

  if (slope_>0.0){
    for (ParticleIndexes::const_iterator mit=slope_ps_.begin();
         mit!=slope_ps_.end();++mit){
      for (ParticleIndexes::const_iterator dit=density_ps_.begin();
           dit!=density_ps_.end();++dit){
        Eigen::Vector3d v = Eigen::Vector3d(core::XYZ(get_model(),*mit).
                                            get_coordinates().get_data()) -
                            Eigen::Vector3d(core::XYZ(get_model(),*dit).
                                            get_coordinates().get_data());
        Float sd = v.norm();
        slope_md[*mit] = kahan_vector_sum(slope_md[*mit],v*slope_/sd);
        slope_score+=slope_*sd;
      }
    }
  }

  IMP_CONTAINER_FOREACH(container::ClosePairContainer,
                          mm_container_,{
      Float score = score_gaussian_overlap(get_model(),_1,&deriv);
      mm_score = kahan_sum(mm_score,2*score);
      if (accum) {
        //multiply by 2 because...
        derivs_mm[_1[0]] = kahan_vector_sum(derivs_mm[_1[0]],-2.0*deriv);
        derivs_mm[_1[1]] = kahan_vector_sum(derivs_mm[_1[1]],2.0*deriv);
      }
  });

  IMP_CONTAINER_FOREACH(container::CloseBipartitePairContainer,
                        md_container_,{
    Float score = score_gaussian_overlap(get_model(),_1,&deriv);
    md_score = kahan_sum(md_score,score);
    if (local_) local_dens.insert(_1[1]);
    if (accum) {
      derivs_md[_1[0]] = kahan_vector_sum(derivs_md[_1[0]],-deriv);
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
  cross_correlation_ = 2*md_score.sum/(mm_score.sum+dd_score);
  double log_score=-std::log(cross_correlation_) + slope_score;

  /* energy calculation */

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
        algebra::Vector3D d = -2.0 / cross_correlation_
                              * (mmdd*d_md - md_score.sum*d_mm) / (mmdd * mmdd);
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
  ret.push_back(md_container_);
  ret.push_back(mm_container_);
  return ret;
}

RestraintInfo *GaussianEMRestraint::get_static_info() const {
  IMP_NEW(RestraintInfo, ri, ());
  ri->add_string("type", "IMP.isd.GaussianEMRestraint");
  if (!density_fn_.empty()) {
    ri->add_filename("filename", density_fn_);
  }
  ri->add_float("model cutoff", model_cutoff_dist_);
  ri->add_float("density cutoff", density_cutoff_dist_);
  ri->add_float("slope", slope_);
  // Typically the density doesn't move during sampling (i.e. static)
  ri->add_particle_indexes("density", density_ps_);
  return ri.release();
}

RestraintInfo *GaussianEMRestraint::get_dynamic_info() const {
  IMP_NEW(RestraintInfo, ri, ());
  ri->add_float("cross correlation", get_cross_correlation_coefficient());
  return ri.release();
}

IMPISD_END_NAMESPACE
