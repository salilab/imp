/**
 *  \file FitRestraint.cpp
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/FitRestraint.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>

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
                << std::endl, ValueException);
      IMP_USAGE_CHECK(ps[i]->has_attribute(radius_key),
                "Particle " << ps[i]->get_name()
                << " is missing the mass "<< weight_key
                << std::endl, ValueException);
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
  Float score;
  bool calc_deriv = accum? true: false;
  score = CoarseCC::evaluate(const_cast<DensityMap&>(*target_dens_map_),
                             const_cast<SampledDensityMap&>(*model_dens_map_),
                             const_cast<FitRestraint*>(this)->dx_,
                             const_cast<FitRestraint*>(this)->dy_,
                             const_cast<FitRestraint*>(this)->dz_,
                             scalefac_, calc_deriv);
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
  //  IMP_LOG(VERBOSE, "after emscore: " << score << " calc_deriv"
  //                   << calc_deriv);
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
