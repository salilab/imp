/**
 *  \file FitRestraint.cpp
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/FitRestraint.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>

IMPEM_BEGIN_NAMESPACE

FitRestraint::FitRestraint(const Particles &ps,
                           DensityMap *em_map,
                           FloatKey radius_key,
                           FloatKey weight_key,
                           float scale)
{
  target_dens_map_ = em_map;
  for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_USAGE_CHECK(ps[i]->has_attribute(radius_key),
                "Particle " << ps[i]->get_name()
                << " is missing the radius "<< radius_key
                << std::endl, ValueException);
      IMP_USAGE_CHECK(ps[i]->has_attribute(radius_key),
                "Particle " << ps[i]->get_name()
                << " is missing the mass "<< weight_key
                << std::endl, ValueException);
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
  add_particles(ps);
  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after setting up particles "
  //                   << endl );


  // init the access_p
  access_p_ = IMPParticlesAccessPoint(ps, radius_key,weight_key);
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
  model_dens_map_->resample(access_p_);
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
  //  IMP_LOG(VERBOSE, "in RSR_EM_Fit::evaluate calc_deriv: " << calc_deriv
  //                   << endl);

  Float score;
  bool calc_deriv = accum? true: false;
  score = CoarseCC::evaluate(const_cast<DensityMap&>(*target_dens_map_),
                             const_cast<SampledDensityMap&>(*model_dens_map_),
                             access_p_,
                             const_cast<FitRestraint*>(this)->dx_,
                             const_cast<FitRestraint*>(this)->dy_,
                             const_cast<FitRestraint*>(this)->dz_,
                             scalefac_, calc_deriv);

  // now update the derivatives
  if (calc_deriv) {
    for (int ii = 0; ii < access_p_.get_size(); ++ii) {
      get_particle(ii)->add_to_derivative(access_p_.get_x_key(), dx_[ii],
                                          *accum);
      get_particle(ii)->add_to_derivative(access_p_.get_y_key(), dy_[ii],
                                          *accum);
      get_particle(ii)->add_to_derivative(access_p_.get_z_key(), dz_[ii],
                                          *accum);
    }
  }
  //  IMP_LOG(VERBOSE, "after emscore: " << score << " calc_deriv"
  //                   << calc_deriv);
  return score;
}

ParticlesList FitRestraint::get_interacting_particles() const
{
  return ParticlesList(1, get_used_particles());
}

ParticlesTemp FitRestraint::get_used_particles() const
{
  return ParticlesTemp(particles_begin(), particles_end());
}

void FitRestraint::show(std::ostream& out) const
{
  // TODO - add implementation
}

IMPEM_END_NAMESPACE
