#include "IMP/log.h"
#include "EMFitRestraint.h"

namespace IMP
{


EMFitRestraint::EMFitRestraint(Model& model,
                               std::vector<int>& particle_indexes,
                               DensityMap &em_map,
                               std::string radius_str,
                               std::string weight_str,
                               float scale)
{

  target_dens_map_ = &em_map;
  scalefac_ = scale;
  model_dens_map_ = new SampledDensityMap(*em_map.get_header());



  /*  number of particles in the restraint */
  num_particles_ = particle_indexes.size();


  // set up the particles, their position indexes, and their type indexes
  Particle* p1;
  for (int i = 0; i < num_particles_; ++i) {
    p1 = model.get_particle(particle_indexes[i]);
    add_particle(p1);
  }

  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after setting up particles "
  //                   << endl );


  // init the access_p
  access_p_ = IMPParticlesAccessPoint(model, particle_indexes, radius_str,
                                      weight_str);


   // initialize the derivatives

  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit before initializing derivatives "
  //                   << endl);
  dx_.insert(dx_.begin(), particle_indexes.size(), 0.0);
  dy_.insert(dy_.begin(), particle_indexes.size(), 0.0);
  dz_.insert(dz_.begin(), particle_indexes.size(), 0.0);



  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after initializing derivatives "
  //                   << endl);


  // normalize the target density data
  //target_dens_map->std_normalize();


  //     IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after std norm" << endl);
  //  have an initial sampling of the model grid

  model_dens_map_->resample(access_p_);
  // IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after resample " << endl);
}


//! Destructor
EMFitRestraint::~EMFitRestraint()
{
}

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

} // namespace IMP
