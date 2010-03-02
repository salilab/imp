/**
 *  \file FitRestraint.cpp
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/FitRestraint.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/HarmonicUpperBound.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>
#include <boost/lambda/lambda.hpp>

IMPEM_BEGIN_NAMESPACE

FitRestraint::FitRestraint(Particles ps,
                           DensityMap *em_map,
                           FloatKey radius_key,
                           FloatKey weight_key,
                           float scale)
{
  target_dens_map_ = em_map;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_USAGE_CHECK(ps[i]->has_attribute(radius_key),
                "Particle " << ps[i]->get_name()
                << " is missing the radius "<< radius_key
                << std::endl);
      IMP_USAGE_CHECK(ps[i]->has_attribute(radius_key),
                "Particle " << ps[i]->get_name()
                << " is missing the mass "<< weight_key
                << std::endl);
    }
  }
  if (scale==-1) {
    double total_mass=0;
    for (unsigned int i=0; i< ps.size(); ++i) {
      total_mass+= ps[i]->get_value(weight_key);
    }
    scalefac_= total_mass/20;
  } else {
    scalefac_ = scale;
  }
  model_dens_map_ = new SampledDensityMap(*em_map->get_header());
  model_dens_map_->set_particles(ps,radius_key,weight_key);
  add_particles(ps);
  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after setting up particles "
  //                   << endl );


   // initialize the derivatives

  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit before initializing derivatives "
  //                   << endl);
  dx_.resize(get_number_of_particles(), 0.0);
  dy_.resize(get_number_of_particles(), 0.0);
  dz_.resize(get_number_of_particles(), 0.0);

  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after initializing derivatives "
  //                   << endl);


  // normalize the target density data
  //target_dens_map->std_normalize();


  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after std norm" << std::endl);
  //  have an initial sampling of the model grid
  model_dens_map_->resample();
  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after resample " << std::endl);
}

IMP_LIST_IMPL(FitRestraint, Particle, particle,Particle*, Particles,
              {
              IMP_INTERNAL_CHECK(get_number_of_particles()==0
                         || obj->get_model()
                         == (*particles_begin())->get_model(),
                         "All particles in FitRestraint must belong to the "
                         "same Model.");
              },,);

//! Calculate the em coarse restraint score.
/** \param[in] calc_deriv If true, partial first derivatives should be
                          calculated.
    \return score associated with this restraint for the given state of
            the model.
 */
double FitRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  Float escore;
  bool calc_deriv = accum? true: false;
  escore = CoarseCC::evaluate(const_cast<DensityMap&>(*target_dens_map_),
                             const_cast<SampledDensityMap&>(*model_dens_map_),
                             const_cast<FitRestraint*>(this)->dx_,
                             const_cast<FitRestraint*>(this)->dy_,
                             const_cast<FitRestraint*>(this)->dz_,
                             scalefac_, calc_deriv);

  //In many optimization senarios particles are can be found outside of
  //the density. When all particles are outside of the density the
  //cross-correlation score is zero and the derivatives are meaningless.
  //To handle these cases we guide the particles back into the density by
  //using a simple distance restraint between the centroids of the density
  //and the particles. Once the particles are back (at least partly) in
  //the density, the CC score is back on. To smooth the score,
  //we start considering centroids distance once 80% of the particles
  //are outside of the density.
  Float percentage_outside_of_density =
    (1.*
    get_number_of_particles_outside_of_the_density(
      target_dens_map_,
      model_dens_map_->get_sampled_particles()))/
    model_dens_map_->get_sampled_particles().size();
  Float score=escore;
  if (percentage_outside_of_density>0.8) {
    IMP_LOG(IMP::TERSE,"More than 80% of the particles are outside "<<
       "of the density. Pulling the particles back "<<
       "to the density using a upper bound harmonic between the centroids "<<
       " of the density and the map"<<std::endl);
    algebra::Vector3D map_centroid=core::get_centroid(core::XYZsTemp());
    algebra::Vector3D ps_centroid=target_dens_map_->get_centroid();
    algebra::Vector3D dist_deriv;
    algebra::Vector3D centroid_delta=map_centroid-ps_centroid;
    core::HarmonicUpperBound hup(0,1.);
    Float dist_score = core::internal::compute_distance_pair_score(
        centroid_delta,
        &hup,&dist_deriv,
        boost::lambda::_1);
    score=(1.-percentage_outside_of_density)*escore+
           percentage_outside_of_density*dist_score;
    //fix derivatives
    for(unsigned int i=0;i<dx_.size();i++) {
      (const_cast<FitRestraint*>(this)->dx_)[i]=
       dx_[i]*(1.-percentage_outside_of_density)+
        dist_deriv[0]*(percentage_outside_of_density);
      (const_cast<FitRestraint*>(this)->dy_)[i]=
       dy_[i]*(1.-percentage_outside_of_density)+
         dist_deriv[1]*(percentage_outside_of_density);
      (const_cast<FitRestraint*>(this)->dz_)[i]=
       dz_[i]*(1.-percentage_outside_of_density)+
         dist_deriv[2]*(percentage_outside_of_density);
    }
  }

  // now update the derivatives
  FloatKeys xyz_keys=IMP::core::XYZR::get_xyz_keys ();
  if (calc_deriv) {
    for (unsigned int ii = 0; ii < dx_.size(); ++ii) {
      get_particle(ii)->add_to_derivative(xyz_keys[0], dx_[ii],
                                          *accum);
      get_particle(ii)->add_to_derivative(xyz_keys[1], dy_[ii],
                                          *accum);
      get_particle(ii)->add_to_derivative(xyz_keys[2], dz_[ii],
                                          *accum);
    }
  }

  IMP_LOG(VERBOSE, "Finish calculating fit restraint with emscore of : "
         << score << std::endl);
  return score;
}

ParticlesList FitRestraint::get_interacting_particles() const
{
  return ParticlesList(1, get_input_particles());
}

ParticlesTemp FitRestraint::get_input_particles() const
{
  return ParticlesTemp(particles_begin(), particles_end());
}

ContainersTemp FitRestraint::get_input_containers() const {
  return ContainersTemp();
}

void FitRestraint::do_show(std::ostream& out) const
{
  out<<"FitRestraint"<<std::endl;
}

IMPEM_END_NAMESPACE
