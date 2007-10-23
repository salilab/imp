#include "IMP/log.h"
#include "EMFitRestraint.h"

namespace IMP
{


EMFitRestraint::EMFitRestraint(Model& model_,
                               std::vector<int>& particle_indexes_,
                               DensityMap &em_map_,
                               std::string radius_str_,
                               std::string weight_str_,
                               float scale_)
{

  target_dens_map = &em_map_;
  model_data = model_.get_model_data();
  scalefac = scale_;
  model_dens_map =   new SampledDensityMap(em_map_.get_header());



  /*  number of particles in the restraint */
  num_particles = particle_indexes_.size();

  //  IMP_LOG(VERBOSE, " RSR_EM_Fit::RSR_EM_Fit set up particles");

  // set up the particles, their position indexes, and their type indexes
  Particle* p1;
  for (int i = 0; i < num_particles; i++) {
    p1 = model_.get_particle(particle_indexes_[i]);
    particles_.push_back(p1);
  }

  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after setting up particles " << endl );

  // init the access_p
  access_p = IMPParticlesAccessPoint(
               model_,
               particle_indexes_,
               radius_str_,
               weight_str_);


  // initialize the derivatives

  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit before initializing derivatives " << endl);
  dx.insert(dx.begin(),particle_indexes_.size(),0.0);
  dy.insert(dy.begin(),particle_indexes_.size(),0.0);
  dz.insert(dz.begin(),particle_indexes_.size(),0.0);


  //  IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after initializing derivatives " << endl);

  // normalize the target density data
  target_dens_map->stdNormalize();

  //     IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after std norm" << endl);
  //  have an initial sampling of the model grid

  int ierr;
  model_dens_map->resample(access_p,
                           ierr);
  // IMP_LOG(VERBOSE, "RSR_EM_Fit::RSR_EM_Fit after resample " << endl);
  cout << 5 << endl;
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
Float EMFitRestraint::evaluate(bool calc_deriv)
{

  return 0.0;
  calc_deriv = true;
  //  IMP_LOG(VERBOSE, "in RSR_EM_Fit::evaluate calc_deriv: " << calc_deriv << endl);
  // TODO - should we resample again?
  int ierr;
  model_dens_map->resample( access_p,
                            ierr);
  Float score;
  score =CoarseCC::evaluate(*target_dens_map,
                            *model_dens_map,
                            access_p,
                            dx,dy,dz,
                            scalefac,
                            calc_deriv);



  // now update the derivatives
  for (int ii=0;ii<access_p.size();ii++) {
    model_data->add_to_deriv(access_p.x_float_ind(ii),dx[ii]);
    model_data->add_to_deriv(access_p.y_float_ind(ii),dy[ii]);
    model_data->add_to_deriv(access_p.z_float_ind(ii),dz[ii]);
  }

  //  IMP_LOG(VERBOSE, "after emscore: " << score << " calc_deriv" << calc_deriv);
  return score;
}


} // namespace IMP
