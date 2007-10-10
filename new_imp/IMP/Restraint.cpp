/*
 *  Restraint.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "Restraint.h"
#include "Particle.h"
#include "Model.h"
#include "log.h"

namespace imp
{

// static constants
const Float RSR_Distance::MIN_DISTANCE = 0.0000001;

//######### Restraint #########
// Abstract class for representing restraints

/**
  Constructor
 */

Restraint::Restraint ()
{
  LogMsg(VERBOSE, "Restraint constructed");
  is_active_ = true; // active by default
  are_particles_active_ = true; // active by default
}


/**
Destructor
 */

Restraint::~Restraint ()
{
  LogMsg(VERBOSE, "Restraint deleted");
}



/**
Give accesss to model particle data.

\param[in] model_data All particle data in the model.
*/

void Restraint::set_model_data(Model_Data* model_data)
{
  model_data_ = model_data;
}


/**
  Set whether the restraint is active. I.e. if it should be
evaluated.

 \param[in] is_active If true, the restraint is active.
 */

void Restraint::set_is_active(const bool active)
{
  is_active_ = active;
}


/**
  Get whether the restraint is active. I.e. if it should be
evaluated.

 \return true it the restraint is active.
 */

bool Restraint::is_active(void) const
{
  return is_active_  && are_particles_active_;
}


/**
  Show the current restraint.

 \param[in] out Stream to send restraint description to.
 */

void Restraint::show(std::ostream& out) const
{
  if (is_active()) {
    out << "unknown restraint (active):" << std::endl;
  } else {
    out << "unknown restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << std::endl;
  out << "last_modified_by: " << last_modified_by() << std::endl;
}


/**
  Check if all necessary particles are still active. If not, inactivate self.
 */

void Restraint::check_particles_active(void)
{
  are_particles_active_ = true;
  for (size_t i = 0; i < particles_.size(); i++) {
    if (!particles_[i]->is_active()) {
      are_particles_active_ = false;
      return;
    }
  }
}

//######### Distance Restraint #########
// Distance restraint between two particles

/**
  Constructor - set up the values and indexes for this distant restraint.

  \param[in] model Pointer to the model.
  \param[in] p1 Pointer to first particle in distance restraint.
  \param[in] p2 Pointer to second particle in distance restraint.
  \param[in] mean Mean associated with the score function for the restraint.
  \param[in] sd Standard deviation associated with the score function for the restraint.
  \param[in] score_func Score function for the restraint.
 */

RSR_Distance::RSR_Distance(Model& model,
                           Particle* p1,
                           Particle* p2,
                           Basic_Score_Func_Params* score_func_params)
{
  model_data_ = model.get_model_data();
  set_up(model, p1, p2, score_func_params);
}

/**
  Constructor - set up the values and indexes for this distant restraint.

  \param[in] model Pointer to the model.
  \param[in] p1 Pointer to first particle in distance restraint.
  \param[in] p2 Pointer to second particle in distance restraint.
  \param[in] attr_name Name of attribute to be used to determine the expected distance (e.g. "radius").
  \param[in] sd Standard deviation associated with the score function for the restraint.
  \param[in] score_func Score function for the restraint.
 */

RSR_Distance::RSR_Distance(Model& model,
                           Particle* p1,
                           Particle* p2,
                           const std::string attr_name,
                           Basic_Score_Func_Params* score_func_params)
{
  model_data_ = model.get_model_data();

  // LogMsg(VERBOSE, "Construct distance restraint: " << attr_name);
  Float mean = model_data_->get_float(p1->float_index(attr_name))
               + model_data_->get_float(p2->float_index(attr_name));
               
  score_func_params->set_mean(mean);
  set_up(model, p1, p2, score_func_params);
}


/**
  Set up for the distant restraint constructors.

  \param[in] model Pointer to the model.
  \param[in] p1 Pointer to first particle in distance restraint.
  \param[in] p2 Pointer to second particle in distance restraint.
  \param[in] mean Mean associated with the score function for the restraint.
  \param[in] sd Standard deviation associated with the score function for the restraint.
  \param[in] score_func Score function for the restraint.
 */

void RSR_Distance::set_up(Model& model,
                          Particle* p1,
                          Particle* p2,
                          Basic_Score_Func_Params* score_func_params)
{
  // LogMsg(VERBOSE, "Set up distance restraint.");
  particles_.push_back(p1);
  x1_ = p1->float_index(std::string("X"));
  y1_ = p1->float_index(std::string("Y"));
  z1_ = p1->float_index(std::string("Z"));

  particles_.push_back(p2);
  x2_ = p2->float_index(std::string("X"));
  y2_ = p2->float_index(std::string("Y"));
  z2_ = p2->float_index(std::string("Z"));

  score_func_ = score_func_params->create_score_func();
}


/**
  Destructor
 */

RSR_Distance::~RSR_Distance ()
{
}

/**
  Calculate the score for this distance restraint based on the current
  state of the model.

 \param[in] calc_deriv If true, partial first derivatives should be calculated.
  */

Float RSR_Distance::evaluate(bool calc_deriv)
{
  Float distance;
  Float delta_x, delta_y, delta_z;
  Float score;

  // we need deltas for calculing the distance and the derivatives
  delta_x = model_data_->get_float(x1_) - model_data_->get_float(x2_);
  delta_y = model_data_->get_float(y1_) - model_data_->get_float(y2_);
  delta_z = model_data_->get_float(z1_) - model_data_->get_float(z2_);

  // calculate the distance feature
  distance = sqrt(delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);

  // if distance is too close to zero, set it to some non-zero value
  if (distance < RSR_Distance::MIN_DISTANCE) {
    delta_x = std::rand(); // arbitrary move
    delta_y = std::rand(); // arbitrary move
    delta_z = std::rand(); // arbitrary move
    distance = sqrt(delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);

    // normalize the random move, to the min disance
    delta_x = RSR_Distance::MIN_DISTANCE * delta_x / distance;
    delta_y = RSR_Distance::MIN_DISTANCE * delta_y / distance;
    delta_z = RSR_Distance::MIN_DISTANCE * delta_z / distance;
    distance = RSR_Distance::MIN_DISTANCE;
  }

  // if needed, calculate the partial derivatives of the scores with respect to the particle attributes
  if (calc_deriv) {
    Float dx, dy, dz;
    Float deriv;

    // calculate the score and feature derivative based on the distance feature
    score = (*score_func_)(distance, deriv);

    dx = delta_x / distance * deriv;
    model_data_->add_to_deriv(x1_, dx);
    model_data_->add_to_deriv(x2_, -dx);

    dy = delta_y / distance * deriv;
    model_data_->add_to_deriv(y1_, dy);
    model_data_->add_to_deriv(y2_, -dy);

    dz = delta_z / distance * deriv;
    model_data_->add_to_deriv(z1_, dz);
    model_data_->add_to_deriv(z2_, -dz);

    /*
     LogMsg(VERBOSE, "distance restraint dX1 (" << model_data_->get_float(x1_) << "): " << model_data_->get_deriv(x1_) << std::endl);
     LogMsg(VERBOSE, "distance restraint dX2 (" << model_data_->get_float(x2_) << "): " << model_data_->get_deriv(x2_) << std::endl);

     LogMsg(VERBOSE, "distance restraint dY1 (" << model_data_->get_float(y1_) << "): " << model_data_->get_deriv(y1_) << std::endl);
     LogMsg(VERBOSE, "distance restraint dY2 (" << model_data_->get_float(y2_) << "): " << model_data_->get_deriv(y2_) << std::endl);

     LogMsg(VERBOSE, "distance restraint dZ1 (" << model_data_->get_float(z1_) << "): " << model_data_->get_deriv(z1_) << std::endl);
     LogMsg(VERBOSE, "distance restraint dZ2 (" << model_data_->get_float(z2_) << "): " << model_data_->get_deriv(z2_) << std::endl);
    */
  }

  else {
    // calculate the score based on the distance feature
    score = (*score_func_)(distance);
  }

  LogMsg(VERBOSE, "distance: " << distance << "   score: " << score);
  return score;
}


/**
  Show the current restraint.

 \param[in] out Stream to send restraint description to.
 */

void RSR_Distance::show(std::ostream& out) const
{
  if (is_active()) {
    out << "distance restraint (active):" << std::endl;
  } else {
    out << "distance restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  " << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  x1:" << model_data_->get_float(x1_);
  out << "  y1:" << model_data_->get_float(y1_);
  out << "  z1:" << model_data_->get_float(z1_) << std::endl;

  out << "  dx1:" << model_data_->get_deriv(x1_);
  out << "  dy1:" << model_data_->get_deriv(y1_);
  out << "  dz1:" << model_data_->get_deriv(z1_) << std::endl;

  out << "  x2:" << model_data_->get_float(x2_);
  out << "  y2:" << model_data_->get_float(y2_);
  out << "  z2:" << model_data_->get_float(z2_) << std::endl;

  out << "  dx2:" << model_data_->get_deriv(x2_);
  out << "  dy2:" << model_data_->get_deriv(y2_);
  out << "  dz2:" << model_data_->get_deriv(z2_) << std::endl;

  out << "  mean:" << mean_;
  out << "  sd:" << sd_ << std::endl;
}




}  // namespace imp
