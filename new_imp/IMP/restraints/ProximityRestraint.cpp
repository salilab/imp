/**
 *  \file ProximityRestraint.cpp   Restrict maximum distance between any
 *                                 two particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "../Model.h"
#include "../Particle.h"
#include "../log.h"
#include "ProximityRestraint.h"

namespace imp
{

//######### ProximityRestraint Restraint #########
// Given a list of particles, this restraint calculates the distance
// restraints between all pairs of particles, and then applies the
// one restraint with the greatest score.

/**
  Constructor - set up the values and indexes for this connectivity restraint.

  \param[in] model Pointer to the model.
  \param[in] particles Vector of indexes of particles of first body.
  \param[in] distance Maximum length allowable between any two particles.
  \param[in] score_func_params Parameters for creating a score function.
 */

ProximityRestraint::ProximityRestraint(Model& model,
                             std::vector<int>& particle_indexes,
                             const Float distance,
                             BasicScoreFuncParams* score_func_params)
{
  set_up(model, particle_indexes);

  // get the indexes associated with the restraints
  int idx = 0;
  score_func_params->set_mean(distance);
  score_func_params->set_score_func_type("harmonic_upper_bound");
  for (int i = 0; i < num_particles_ - 1; i++) {
    for (int j = i + 1; j < num_particles_; j++) {
      // create the restraint
      
      dist_rsrs_[idx] = new DistanceRestraint(model,
                                         particles_[i],
                                         particles_[j],
                                         score_func_params);
      idx++;
    }
  }

  IMP_LOG(VERBOSE,
         "Number of restraints: " << num_restraints_ << "  number of particles: " <<
         num_particles_ << std::endl);
}

/**
  Constructor - set up the values and indexes for this connectivity restraint.

  \param[in] model Pointer to the model.
  \param[in] particles Vector of pointers  to particle of the restraint.
  \param[in] attr_name Name to get radii to calculate the mean distance.
  \param[in] distance Maximum length allowable between any two particles.
  \param[in] score_func_params Parameters for creating a score function.
 */

ProximityRestraint::ProximityRestraint(Model& model,
                             std::vector<int>& particle_indexes,
                             const std::string attr_name,
                             const Float distance,
                             BasicScoreFuncParams* score_func_params)
{
  // Use those radii to calculate the expected distance
  set_up(model, particle_indexes);

  // get the indexes associated with the restraints
  int idx = 0;
  Float actual_mean;
  score_func_params->set_score_func_type("harmonic_upper_bound");
  for (int i = 0; i < num_particles_ - 1; i++) {
    for (int j = i + 1; j < num_particles_; j++) {
      // Use those radii to calculate the expected distance between centers
      actual_mean = distance - model_data_->get_float(particles_[i]->float_index(attr_name))
                    - model_data_->get_float(particles_[j]->float_index(attr_name));

      // create the restraint
      IMP_LOG(VERBOSE, i << " " << j << " add distance: " << actual_mean);
      score_func_params->set_mean(actual_mean);
      dist_rsrs_[idx] = new DistanceRestraint(model,
                                         particles_[i],
                                         particles_[j],
                                         score_func_params);
      idx++;
    }
  }
}

/**
  Set up the values and indexes for this connectivity restraint for the constructors.

  \param[in] model Pointer to the model.
  \param[in] particles Vector of indexes of particles in the restraint.
 */

void ProximityRestraint::set_up(Model& model,
                           std::vector<int>& particle_indexes)
{
  IMP_LOG(VERBOSE, "init ConnectivityRestraint");

  model_data_ = model.get_model_data();

  /** number of particles in the restraint */
  num_particles_ = particle_indexes.size();

  // set up the particles, their position indexes, and their type indexes
  Particle* p1;
  for (int i = 0; i < num_particles_; i++) {
    p1 = model.get_particle(particle_indexes[i]);
    particles_.push_back(p1);
  }

  // figure out how many restraints there are
  num_restraints_ = num_particles_ * (num_particles_ - 1) / 2;

  // use number of restraints and number of types to set up some of the
  // ... main arrays
  dist_rsrs_.resize(num_restraints_);
  scores_.resize(num_restraints_);
  rsr_idx_.resize(num_restraints_);
}

/**
  Destructor
 */

ProximityRestraint::~ProximityRestraint ()
{
  for (int i = 0; i < num_restraints_; i++) {
    delete dist_rsrs_[i];
  }
}

/**
  Calculate the distance restraints for the given particles. Use the smallest
  restraints that will connect one particle of each type together (i.e. a
  minimum spanning tree with nodes corresponding to particle types and the
  edge weights corresponding to restraint violation score values).

 \param[in] calc_deriv If true, partial first derivatives should be calculated.
 \resturn score associated with this restraint for the given state of the model.
  */

Float ProximityRestraint::evaluate(bool calc_deriv)
{
  int idx;

  IMP_LOG(VERBOSE, "evaluate ProximityRestraint");

  // calculate the scores for all of the restraints
  /*
  There is a problem with using just the largest violation restraint
  in that it causes an inherent unsmoothness in the optimization process.
  I.e. when one pair becomes more distant in another, there is a complete
  switch to an orthogonal direction.

  LogMsg(VERBOSE, "calculate restraint scores");
  for (int i = 0; i < num_restraints_; i++)
   {
   scores_[i] = dist_rsrs_[i]->evaluate(false);
   }
  */

  // dumb bubble sort of indexes by scores.
  // /rsr_idx[]/ holds the sorted indexes so that all other arrays
  // associated with the particles can be left as they are.

  /*
  LogMsg(VERBOSE, "sort scores");
  for (int i = 0; i < num_restraints_; i++)
   {
   rsr_idx_[i] = i;
   }

  for (int i = 0; i < num_restraints_ - 1; i++)
   {
   // move the next highest score index to the left most position of the
   // remaining unsorted list.
   for (int j = num_restraints_ - 1; j > i; j--)
    {
    if (scores_[rsr_idx_[j - 1]] < scores_[rsr_idx_[j]])
     {
     idx = rsr_idx_[j - 1];
     rsr_idx_[j - 1] = rsr_idx_[j];
     rsr_idx_[j] = idx;
     }
    }
   }
  */

  // apply the restraints corresponding the highest score restraints
  int num_restraints_to_use = 1; // mod later if it should be more than one

  // switching back and forth seems to be less smooth
  // just apply all of the restraints every time (also faster)
  num_restraints_to_use = num_restraints_;
  Float score = 0.0;
  for (int i = 0; i < num_restraints_to_use; i++) {
    // idx = rsr_idx_[i];
    idx = i;

    score += dist_rsrs_[idx]->evaluate(calc_deriv);
    IMP_LOG(VERBOSE,
           " " << i << " Applying Restraint: score: "
           << score << std::endl);
  }

  return score;
}


/**
  Show the current restraint.

 \param[in] out Stream to send restraint description to.
 */

void ProximityRestraint::show(std::ostream& out) const
{
  if (is_active()) {
    out << "proximity restraint (active):" << std::endl;
  } else {
    out << "proximity restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  " << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  num particles:" << num_particles_;
}

}  // namespace imp
