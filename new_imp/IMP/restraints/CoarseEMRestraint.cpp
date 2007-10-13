/**
 *  \file CoarseEMRestraint.cpp  Calculate score based on fit to EM map.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "../Model.h"
#include "../Particle.h"
#include "../log.h"
#include "../emscore.h"
#include "CoarseEMRestraint.h"

namespace IMP
{

//######### CoarseEMRestraint Restraint #########

/**
  Constructor - set up the values and indexes for this EM coarse restraint. 
  
*/

CoarseEMRestraint::CoarseEMRestraint(Model& model,
                             std::vector<int>& particle_indexes,
                             EM_Density* emdens,
                             int nx,
                             int ny,
                             int nz,
                             float pixelsize,
                             float resolution,
                             std::string radius_str,
                             std::string weight_str,
                             EM_Gridcoord* gridcd,
                             float scalefac
                            )
{
  Particle* p1;

  /** number of particles in the restraint */
  num_particles_ = particle_indexes.size();

  IMP_LOG(VERBOSE, "set up particles");
  // set up the particles, their position indexes, and their type indexes
  for (int i = 0; i < num_particles_; i++) {
    p1 = model.get_particle(particle_indexes[i]);
    particles_.push_back(p1);
  }

  x_ = new FloatIndex[num_particles_];
  y_ = new FloatIndex[num_particles_];
  z_ = new FloatIndex[num_particles_];

  cdx_ = new float[num_particles_];
  cdy_ = new float[num_particles_];
  cdz_ = new float[num_particles_];

  dvx_ = new float[num_particles_];
  dvy_ = new float[num_particles_];
  dvz_ = new float[num_particles_];

  weight_ = new float[num_particles_];
  radius_ = new float[num_particles_];

  model_data_ = model.get_model_data();

  for (int i = 0; i < num_particles_; i++) {
    p1 = particles_[i];
    x_[i] = p1->get_float_index(std::string("x"));
    y_[i] = p1->get_float_index(std::string("y"));
    z_[i] = p1->get_float_index(std::string("z"));
    weight_[i] = model_data_->get_float(p1->get_float_index(weight_str));
    radius_[i] = model_data_->get_float(p1->get_float_index(radius_str));
    IMP_LOG (VERBOSE, "radius " << radius_str << radius_[i]);
  }

  emdens_ = emdens;
  nx_ = nx;
  ny_ = ny;
  nz_ = nz;
  pixelsize_ = pixelsize;
  resolution_ = resolution;
  gridcd_ = gridcd;
  scalefac_ = scalefac;
}


/**
  Destructor
 */

CoarseEMRestraint::~CoarseEMRestraint ()
{
  delete x_;
  delete y_;
  delete z_;

  delete cdx_;
  delete cdy_;
  delete cdz_;

  delete dvx_;
  delete dvy_;
  delete dvz_;
}

/**
  Calculate the em coarse restraint score.

 \param[in] calc_deriv If true, partial first derivatives should be calculated.
 \return score associated with this restraint for the given state of the model.
  */

Float CoarseEMRestraint::evaluate(bool calc_deriv)
{
  int lderiv = (int) calc_deriv;
  int ierr;

  for (int i = 0; i < num_particles_; i++) {
    cdx_[i] = model_data_->get_float(x_[i]);
    cdy_[i] = model_data_->get_float(y_[i]);
    cdz_[i] = model_data_->get_float(z_[i]);

    dvx_[i] = 0.0;
    dvy_[i] = 0.0;
    dvz_[i] = 0.0;

    IMP_LOG(VERBOSE, i << " x: " << cdx_[i] << " y: " << cdy_[i] << " z: " << cdz_[i] << " weight_: " << weight_[i] << " radius: " << radius_[i]);
  }

  IMP_LOG(VERBOSE, "before emscore: " << emdens_ << " nx_" << nx_ << " ny_" << ny_ << " nz_" << nz_ << " pixelsize_" << pixelsize_ << " resolution_" << resolution_ << " " <<
         cdx_ << " " << cdy_ << " " << cdz_ << " " << dvx_ << " " << dvy_ << " " << dvz_ << " num_particles_" << num_particles_ << " " <<
         radius_ << " " << weight_ << " gridcd_" << gridcd_ << " scalefac_" << scalefac_);

  Float score;
  score = emscore(emdens_, nx_, ny_, nz_, pixelsize_, resolution_,
                  cdx_, cdy_, cdz_, dvx_, dvy_, dvz_, num_particles_,
                  radius_, weight_, gridcd_, scalefac_, lderiv, &ierr);

  IMP_LOG(VERBOSE, "after emscore: " << score << " calc_deriv" << calc_deriv);
  score = 0.0;
  if (calc_deriv) {
    for (int i = 0; i < num_particles_; i++) {
      model_data_->add_to_deriv(x_[i], dvx_[i]);
      model_data_->add_to_deriv(y_[i], dvy_[i]);
      model_data_->add_to_deriv(z_[i], dvz_[i]);
    }
  }

  return score;
}


/**
  Show the current restraint.

 \param[in] out Stream to send restraint description to.
 */

void CoarseEMRestraint::show(std::ostream& out) const
{
  if (is_active()) {
    out << "em coarse restraint (active):" << std::endl;
  } else {
    out << "em coarse  restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  " << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  num particles:" << num_particles_;
}

}  // namespace IMP
