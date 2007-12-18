/**
 *  \file ProximityRestraint.cpp \brief Proximity restraint.
 *
 *  Restrict maximum distance between any two particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/ProximityRestraint.h"

namespace IMP
{

//! Constructor - set up the restraint for point-like particles.
/** \param[in] model Pointer to the model.
    \param[in] particle_indexes Vector of indexes of particles
    \param[in] distance Maximum length allowable between any two particles.
    \param[in] score_func_params Parameters for creating a score function.
 */
ProximityRestraint::ProximityRestraint(Model* model,
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

      ScoreFunc *sf = score_func_params->create_score_func();
      dist_rsrs_[idx] = new DistanceRestraint(get_particle(i),
                                              get_particle(j),
                                              sf);
      idx++;
    }
  }

  IMP_LOG(VERBOSE,
          "Number of restraints: " << num_restraints_
          << "  number of particles: " << num_particles_ << std::endl);
}


//! Constructor - set up the restraint for particles with radii.
/** \param[in] model Pointer to the model.
    \param[in] particle_indexes Vector of indexes of particles
    \param[in] attr_name Name to get radii to calculate the mean distance.
    \param[in] distance Maximum length allowable between any two particles.
    \param[in] score_func_params Parameters for creating a score function.
 */
ProximityRestraint::ProximityRestraint(Model* model,
                                       std::vector<int>& particle_indexes,
                                       FloatKey attr_name,
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
      Float attri, attrj;
      attri = get_particle(i)->get_value(attr_name);
      attrj = get_particle(j)->get_value(attr_name);
      actual_mean = distance - attri - attrj;

      // create the restraint
      IMP_LOG(VERBOSE, i << " " << j << " add distance: " << actual_mean);
      score_func_params->set_mean(actual_mean);
      ScoreFunc *sf = score_func_params->create_score_func();
      dist_rsrs_[idx] = new DistanceRestraint(get_particle(i),
                                              get_particle(j),
                                              sf);
      idx++;
    }
  }
}


//! Internal set up for the constructors.
/** \param[in] model Pointer to the model.
    \param[in] particle_indexes Vector of indexes of particles in the restraint.
 */
void ProximityRestraint::set_up(Model* model,
                                std::vector<int>& particle_indexes)
{
  IMP_LOG(VERBOSE, "init ConnectivityRestraint");

  /** number of particles in the restraint */
  num_particles_ = particle_indexes.size();

  // set up the particles, their position indexes, and their type indexes
  Particle* p1;
  for (int i = 0; i < num_particles_; i++) {
    p1 = model->get_particle(particle_indexes[i]);
    add_particle(p1);
  }

  // figure out how many restraints there are
  num_restraints_ = num_particles_ * (num_particles_ - 1) / 2;

  // use number of restraints and number of types to set up some of the
  // ... main arrays
  dist_rsrs_.resize(num_restraints_);
  scores_.resize(num_restraints_);
  rsr_idx_.resize(num_restraints_);
}


//! Destructor
ProximityRestraint::~ProximityRestraint()
{
  for (int i = 0; i < num_restraints_; i++) {
    delete dist_rsrs_[i];
  }
}


//! Evaluate the score for the model.
/** Calculate the distance restraints for the given particles. Use the smallest
    restraints that will connect one particle of each type together (i.e. a
    minimum spanning tree with nodes corresponding to particle types and the
    edge weights corresponding to restraint violation score values).

    \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return score associated with this restraint for the given state of
            the model.
 */
Float ProximityRestraint::evaluate(DerivativeAccumulator *accum)
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

    score += dist_rsrs_[idx]->evaluate(accum);
    IMP_LOG(VERBOSE,
            " " << i << " Applying Restraint: score: "
            << score << std::endl);
  }

  return score;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void ProximityRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "proximity restraint (active):" << std::endl;
  } else {
    out << "proximity restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  ";
  out << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  num particles:" << num_particles_;
}

}  // namespace IMP
