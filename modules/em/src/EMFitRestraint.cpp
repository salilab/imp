/**
 *  \file EMFitRestraint.cpp
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/EMFitRestraint.h>
#include "IMP/container_macros.h"
#include <IMP/log.h>

IMPEM_BEGIN_NAMESPACE

EMFitRestraint::EMFitRestraint(const Particles &ps,
                               DensityMap &em_map,
                               FloatKey radius_key,
                               FloatKey weight_key,
                               float scale)
{
  target_dens_map_ = &em_map;
  scalefac_ = scale;
  model_dens_map_ = new SampledDensityMap(*em_map.get_header());
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


  //     IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after std norm" << endl);
  //  have an initial sampling of the model grid
  std::cout<<"RSR_EM_Fit::RSR_EM_Fit after std norm:"<<std::endl;
  std::cout<<model_dens_map_<<std::endl;
  model_dens_map_->resample(access_p_);
  std::cout<<"RSR_EM_Fit::RSR_EM_Fit after resample " <<std::endl;
  // IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after resample " << endl);
}


//! Destructor
EMFitRestraint::~EMFitRestraint()
{
}

IMP_LIST_IMPL(EMFitRestraint, Particle, particle,Particle*,  {
              IMP_assert(get_number_of_particles()==0
                         || obj->get_model()
                         == (*particles_begin())->get_model(),
                         "All particles in EMFitRestraint must belong to the "
                         "same Model.");
              },,);

//! Calculate the em coarse restraint score.
/** \param[in] calc_deriv If true, partial first derivatives should be
                          calculated.
    \return score associated with this restraint for the given state of
            the model.
 */
Float EMFitRestraint::evaluate(DerivativeAccumulator *accum)
{
  //  IMP_LOG(VERBOSE, "in RSR_EM_Fit::evaluate calc_deriv: " << calc_deriv
  //                   << endl);

  Float score;
  bool calc_deriv = accum? true: false;
  score = CoarseCC::evaluate(*target_dens_map_, *model_dens_map_,
                             access_p_, dx_, dy_, dz_, scalefac_, calc_deriv);

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

void EMFitRestraint::show(std::ostream& out) const
{
  // TODO - add implementation
}

IMPEM_END_NAMESPACE
