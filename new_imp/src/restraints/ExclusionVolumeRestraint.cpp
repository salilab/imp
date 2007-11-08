/**
 *  \file ExclusionVolumeRestraint.cpp  \brief Excluded volume restraint.
 *
 *  Prevent particles from getting too close together, by restricting the
 *  minimum distance between all pairs of particles formed from one or two
 *  sets of particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/ExclusionVolumeRestraint.h"

namespace IMP
{

//! Set up restraint using two sets of particles.
/** Uses the attr_name to access the radii for the minimum distance between
    any two particles. Assumes that there is no overlap between the two
    particle lists. Create restraints for all possible pairs between the
    two lists.

    \param[in] model Pointer to the model.
    \param[in] particle1_indexes Vector of indexes of particles of the
                                 first body.
    \param[in] particle2_indexes Vector of indexes of particles of the
                                 second body.
    \param[in] attr_name The attribute used to determine the radius of
                         each particle.
    \param[in] score_func_params Scoring function parameters.
 */
ExclusionVolumeRestraint::ExclusionVolumeRestraint(Model& model,
    std::vector<int>& particle1_indexes, std::vector<int>& particle2_indexes,
    const std::string attr_name, BasicScoreFuncParams* score_func_params)
{
  Particle* p1;

  model_data_ = model.get_model_data();

  num_particles1_ = particle1_indexes.size();
  num_particles2_ = particle2_indexes.size();
  num_particles_ = num_particles1_ + num_particles2_;
  for (int i = 0; i < num_particles1_; i++) {
    p1 = model.get_particle(particle1_indexes[i]);
    particles_.push_back(p1);
  }

  for (int i = 0; i < num_particles2_; i++) {
    p1 = model.get_particle(particle2_indexes[i]);
    particles_.push_back(p1);
  }

  num_restraints_ = num_particles1_ * num_particles2_;

  // get the indexes associated with the restraints
  Float actual_mean;
  IMP_LOG(VERBOSE, "Add inter-body exclusion volume restraints "
          << num_restraints_);

  // particle 1 indexes
  for (int i = 0; i < num_particles1_; i++) {
    // particle 2 indexes
    for (int j = num_particles1_; j < num_particles_; j++) {
      // Use those radii to calculate the expected distance
      Float attri, attrj;
      attri = model_data_->get_float(particles_[i]->get_float_index(attr_name));
      attrj = model_data_->get_float(particles_[j]->get_float_index(attr_name));
      actual_mean = attri + attrj;

      score_func_params->set_mean(actual_mean);

      // create the restraint
      dist_rsrs_.push_back(new DistanceRestraint(model,
                                            particles_[i],
                                            particles_[j],
                                            score_func_params));
    }
  }
}


//! Set up restraint using one set of particles.
/** Uses the attr_name to access the radii for the minimum distance between
    any two particles. Create restraints for all possible pairs of particles
    in the list.

    \param[in] model Pointer to the model.
    \param[in] particle_indexes Vector of indexes of particles
    \param[in] attr_name The attribute used to determine the radius of
                         each particle.
    \param[in] score_func_params Scoring function parameters.
 */
ExclusionVolumeRestraint::ExclusionVolumeRestraint(Model& model,
    std::vector<int>& particle_indexes, const std::string attr_name,
    BasicScoreFuncParams* score_func_params)
{
  Particle* p1;

  model_data_ = model.get_model_data();

  num_particles_ = particle_indexes.size();
  for (int i = 0; i < num_particles_; i++) {
    p1 = model.get_particle(particle_indexes[i]);
    particles_.push_back(p1);
  }

  num_restraints_ = num_particles_ * (num_particles_ - 1) / 2;

  // get the indexes associated with the restraints
  int idx = 0;
  Float actual_mean;
  IMP_LOG(VERBOSE, "Add intra-body exclusion volume restraints "
          << num_restraints_);

  // particle 1 indexes
  for (int i = 0; i < num_particles_ - 1; i++) {
    // particle 2 indexes (avoid duplicates and particle with itself)
    for (int j = i+1; j < num_particles_; j++) {
      // Use those radii to calculate the expected distance
      Float attri, attrj;
      attri = model_data_->get_float(particles_[i]->get_float_index(attr_name));
      attrj = model_data_->get_float(particles_[j]->get_float_index(attr_name));
      actual_mean = attri + attrj;

      score_func_params->set_mean(actual_mean);

      // create the restraint
      dist_rsrs_.push_back(new DistanceRestraint(model,
                                            particles_[i],
                                            particles_[j],
                                            score_func_params));
      idx++;
    }
  }
}


//! Destructor
ExclusionVolumeRestraint::~ExclusionVolumeRestraint ()
{
  std::vector<DistanceRestraint*>::iterator rsr_iter;

  for (rsr_iter = dist_rsrs_.begin(); rsr_iter != dist_rsrs_.end();
       ++rsr_iter) {
    delete(*rsr_iter);
  }
}


//! Get the score of the restraint for the model.
/** Calculate the distance restraints for the given particles. Use the smallest
    restraints that will connect one particle of each type together (i.e. a
    minimum spanning tree with nodes corresponding to particle types and the
    edge weights corresponding to restraint violation scores).

    \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return score associated with this restraint for the given state of
            the model.
 */
Float ExclusionVolumeRestraint::evaluate(DerivativeAccumulator *accum)
{
  std::vector<DistanceRestraint*>::iterator rsr_iter;

  Float score = 0.0;

  // until this is smarter, just calculate them all
  for (rsr_iter = dist_rsrs_.begin(); rsr_iter != dist_rsrs_.end();
       ++rsr_iter) {
    score += (*rsr_iter)->evaluate(accum);
  }

  return score;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void ExclusionVolumeRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "exclusion volume restraint (active):" << std::endl;
  } else {
    out << "exclusion volume  restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  " << "last_modified_by: "
      << last_modified_by() << std::endl;
  out << "  num particles:" << num_particles_;
}

}  // namespace IMP
