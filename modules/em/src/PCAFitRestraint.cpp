/**
 *  \file PCAFitRestraint.cpp
 *  \brief Calculate match between density and particle pca
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/PCAFitRestraint.h>
#include <IMP/log.h>
#include <IMP/em/converters.h>

IMPEM_BEGIN_NAMESPACE

namespace {
double get_angle(const algebra::Vector3D &v1,
                 const algebra::Vector3D &v2) {
  return std::acos((v1*v2)/(v1.get_magnitude()*v2.get_magnitude()));
}

algebra::PrincipalComponentAnalysis
      get_pca_from_particles(const core::XYZs &ps_xyz) {
  //find the pca of the protein
  algebra::Vector3Ds ps_vecs;
  for (core::XYZs::const_iterator it = ps_xyz.begin();
        it != ps_xyz.end(); it++) {
    ps_vecs.push_back(it->get_coordinates());
  }
  return algebra::get_principal_components(ps_vecs);
}

algebra::PrincipalComponentAnalysis get_pca_from_density(
   DensityMap *dmap, float threshold) {
  //find the pca of the density map
  algebra::Vector3Ds vecs=density2vectors(dmap,threshold);
  return algebra::get_principal_components(vecs);
}
}

PCAFitRestraint::PCAFitRestraint(
   ParticlesTemp ps,
   DensityMap *em_map, float threshold,
   float max_pca_size_diff,float max_angle_diff,
   float max_centroid_diff,
   FloatKey weight_key): Restraint(IMP::internal::get_model(ps),
      "Fit restraint%1%"),
      max_angle_diff_(algebra::PI*max_angle_diff/180.),
      max_pca_size_diff_(max_pca_size_diff*max_pca_size_diff),
      max_centroid_diff_(max_centroid_diff){
  target_dens_map_ = em_map;
  threshold_=threshold;
  weight_key_=weight_key;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_USAGE_CHECK(core::XYZR::particle_is_instance(ps[i]),
                      "Particle " << ps[i]->get_name()
                      << " is not XYZR"
                      << std::endl);
      IMP_USAGE_CHECK(ps[i]->has_attribute(weight_key),
                "Particle " << ps[i]->get_name()
                << " is missing the mass "<< weight_key
                << std::endl);
    }
  }
  store_particles(ps);
  dens_pca_ = get_pca_from_density(target_dens_map_,threshold_);
  IMP_LOG_TERSE( "Finish initialization" << std::endl);
}

IMP_LIST_IMPL(PCAFitRestraint, Particle, particle,Particle*, Particles);

double PCAFitRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  Float escore;//(1) 0  means that the pcas are (not) matching
  bool calc_deriv = accum? true: false;
  algebra::PrincipalComponentAnalysis ps_pca=
    get_pca_from_particles(core::XYZs(all_ps_));
  /*
  std::cout<<"density PCA:"<<std::endl;
  dens_pca_.show();
  std::cout<<"particles PCA:"<<std::endl;
  ps_pca.show();
  */
  escore=0;
  for(int i=0;i<3;i++) {
    if (std::abs(ps_pca.get_principal_value(i)-dens_pca_.get_principal_value(i))
        > max_pca_size_diff_) {
      IMP_LOG_VERBOSE(
     "Principal value "<<i<< " do not match "<< ps_pca.get_principal_value(i)
     <<" "<< dens_pca_.get_principal_value(i)
     << " "<< max_pca_size_diff_<<std::endl);
      escore=1;
    }
    double angle=
     get_angle(ps_pca.get_principal_component(i),
               dens_pca_.get_principal_component(i));
    if (!((angle<max_angle_diff_)||(angle>(algebra::PI-max_angle_diff_)))) {
      IMP_LOG_VERBOSE(
      "Principal angle "<<i<< " do not match "<<
      180./algebra::PI*angle<< " "<<
      180.*max_angle_diff_/algebra::PI<<std::endl);
      escore=1;
    }
  }
  if (algebra::get_distance(ps_pca.get_centroid(),dens_pca_.get_centroid())>
        max_centroid_diff_) {
    IMP_LOG_VERBOSE(
      "Pricipal cnetroid distance does not match: "
      << algebra::get_distance(ps_pca.get_centroid(),dens_pca_.get_centroid())
      <<" "<< max_centroid_diff_<<std::endl);
    escore=1;
  }
  if (calc_deriv) {
    IMP_WARN("not derivative calculation");
  }
  return escore;
}

ParticlesTemp PCAFitRestraint::get_input_particles() const
{
  ParticlesTemp pt(all_ps_.begin(), all_ps_.end());
  return pt;
}

ContainersTemp PCAFitRestraint::get_input_containers() const {
  return ContainersTemp();
}

void PCAFitRestraint::do_show(std::ostream& out) const
{
  out<<"FitRestraint"<<std::endl;
}
void PCAFitRestraint::store_particles(ParticlesTemp ps) {
  all_ps_=get_as<Particles>(ps);
  add_particles(ps);
}
IMPEM_END_NAMESPACE
