/**
 *  \file ConnectivityRestraint.cpp  \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "../Model.h"
#include "../Particle.h"
#include "../log.h"
#include "../mystdexcept.h"
#include "ConnectivityRestraint.h"

namespace IMP
{

//! Constructor using a given mean for particle-particle expected distance.
/**
    \param[in] model Pointer to the model.
    \param[in] particle_indexes Vector of particle indices. \todo Should use
               Particle pointers instead.
    \param[in] type The attribute used to determine if particles are equivalent.
    \param[in] score_func_params Parameters for creating a score function.
 */
ConnectivityRestraint::ConnectivityRestraint(Model& model,
    std::vector<int>& particle_indexes, const std::string type,
    BasicScoreFuncParams* score_func_params)
{
  std::list<ConnectivityRestraint::RestraintScore>::iterator rs_iter;

  set_up(model, particle_indexes, type);

  // set up the restraints
  rs_iter = rsr_scores_.begin();
  for (int i = 0; i < num_particles_ - 1; i++) {
    for (int j = i + 1; j < num_particles_; j++) {
      if (particle_type_[j] != particle_type_[i]) {
        if (rs_iter == rsr_scores_.end()) {
          IMP_failure("Over ran the caculated number of restraints in ConnectivityRestraint", std::out_of_range("Over ran the calculated number of restraints"));
        } else {
          rs_iter->part1_type_ = particle_type_[i];
          rs_iter->part2_type_ = particle_type_[j];

          // create the restraint
          rs_iter->rsr_ = new DistanceRestraint(model,
                                           particles_[i],
                                           particles_[j],
                                           score_func_params);
          ++rs_iter;
        }
      }
    }
  }

  IMP_LOG(VERBOSE,
         "Number of types: " << num_types_ << "  max_type_: " << max_type_ <<
         "  num_restraints_: " << num_restraints_ << "  num_particles_: " <<
         num_particles_ << std::endl);
}


//! Constructor using a given attribute for particle-particle expected distance.
/** \param[in] model Pointer to the model.
    \param[in] particle_indexes Vector of particle indices. \todo Should use
               Particle pointers instead.
    \param[in] type The attribute used to determine if particles are equivalent.
    \param[in] attr_name Name to get radii to calculate the mean distance.
    \param[in] score_func_params Parameters for creating a score function.
 */
ConnectivityRestraint::ConnectivityRestraint(Model& model,
    std::vector<int>& particle_indexes, const std::string type,
    const std::string attr_name, BasicScoreFuncParams* score_func_params)
{
  std::list<ConnectivityRestraint::RestraintScore>::iterator rs_iter;

  IMP_LOG(VERBOSE, "ConnectivityRestraint constructor");
  set_up(model, particle_indexes, type);

  // set up the restraints
  Float actual_mean;
  rs_iter = rsr_scores_.begin();
  for (int i = 0; i < num_particles_ - 1; i++) {
    for (int j = i + 1; j < num_particles_; j++) {
      if (particle_type_[j] != particle_type_[i]) {
        if (rs_iter == rsr_scores_.end()) {
          IMP_failure("Over ran the caculated number of restraints in ConnectivityRestraint", std::out_of_range("Over ran the calculated number of restraints"));
        } else {
          rs_iter->part1_type_ = particle_type_[i];
          rs_iter->part2_type_ = particle_type_[j];

          // Use those radii to calculate the expected distance
          actual_mean = model_data_->get_float(particles_[i]->get_float_index(attr_name))
                        + model_data_->get_float(particles_[j]->get_float_index(attr_name));

          score_func_params->set_mean(actual_mean);
          
          // create the restraint
          rs_iter->rsr_ = new DistanceRestraint(model,
                                           particles_[i],
                                           particles_[j],
                                           score_func_params);
          ++rs_iter;
        }
      }
    }
  }

  IMP_LOG(VERBOSE,
         "Number of types: " << num_types_ << "  max_type_: " << max_type_ <<
         "  num_restraints_: " << num_restraints_ << "  num_particles_: " <<
         num_particles_ << std::endl);
}


//! Set up initial values for the constructors.
/** \param[in] model Pointer to the model.
    \param[in] particle_indexes Vector of particle indices. \todo Should use
               Particle pointers instead.
    \param[in] type The attribute used to determine if particles are equivalent.
 */
void ConnectivityRestraint::set_up(Model& model,
                                   std::vector<int>& particle_indexes,
                                   const std::string type)
{
  Particle* p1;

  IMP_LOG(VERBOSE, "init ConnectivityRestraint");

  model_data_ = model.get_model_data();

  IMP_LOG(VERBOSE, "got model data " << model_data_);

  /** number of particles in the restraint */
  num_particles_ = particle_indexes.size();
  particle_type_.resize(num_particles_);

  IMP_LOG(VERBOSE, "set up particle types");
  // set up the particles, their position indexes, and their type indexes
  for (int i = 0; i < num_particles_; i++) {
    p1 = model.get_particle(particle_indexes[i]);
    particles_.push_back(p1);
    type_.push_back(p1->get_int_index(type));
  }

  IMP_LOG(VERBOSE, "Size of particles: " << particles_.size() << "  size of types:" << type_.size() << "  num_particles_: " << num_particles_ << "  particle_type_: " << particle_type_.size());
  IMP_LOG(VERBOSE, "Figure out number of types");
  // figure out how many types there are
  int next_type;
  max_type_ = 0;
  num_types_ = 0;
  for (int i = 0; i < num_particles_; i++) {
    next_type = model_data_->get_int(type_[i]);
    if (max_type_ < next_type) {
      max_type_ = next_type;
    }

    num_types_++;
    particle_type_[i] = next_type;
    // if this type already was used, decrement numbe of types
    for (int j = 0; j < i; j++) {
      if (particle_type_[j] == next_type) {
        num_types_--;
        break;
      }
    }
  }

  // types can range from 0 to max_type_ - 1
  max_type_++;
  IMP_LOG(VERBOSE, "Max types: " << max_type_);

  IMP_LOG(VERBOSE, "Figure out number of restraints");
  // figure out how many restraints there are
  // Could use num_restraints = sum((S-si)si)) where is total number of particles
  // ... and si is number of particles of type i (summed over all types)
  num_restraints_ = num_particles_ * (num_particles_ - 1) / 2;
  for (int i = 0; i < num_particles_ - 1; i++) {
    for (int j = i + 1; j < num_particles_; j++) {
      if (particle_type_[j] == particle_type_[i]) {
        num_restraints_--;
      }
    }
  }

  IMP_LOG(VERBOSE, "Num restraints: " << num_restraints_);
  // use number of restraints and number of types to set up some of the
  // ... main arrays
  tree_id_.resize(max_type_);
  rsr_scores_.resize(num_restraints_);
}

//! Destructor
ConnectivityRestraint::~ConnectivityRestraint ()
{
  std::list<ConnectivityRestraint::RestraintScore>::iterator rs_iter;

  for (rs_iter = rsr_scores_.begin(); rs_iter != rsr_scores_.end(); ++rs_iter) {
    delete(rs_iter->rsr_);
  }
}


//! Evaluate this restraint and return the score.
/** Calculate the distance restraints for the given particles. Use the smallest
    restraints that will connect one particle of each type together (i.e. a
    minimum spanning tree with nodes corresponding to particle types and the
    edge weights corresponding to restraint violation scores).

    \param[in] calc_deriv If true, partial first derivatives should be
                          calculated.
    \return score associated with this restraint for the given state of
            the model.
 */
Float ConnectivityRestraint::evaluate(bool calc_deriv)
{
  IMP_LOG(VERBOSE, "evaluate ConnectivityRestraint");

  std::list<ConnectivityRestraint::RestraintScore>::iterator rs_iter;

  // calculate the scores for all of the restraints
  IMP_LOG(VERBOSE, "calculate restraint scores");
  for (rs_iter = rsr_scores_.begin(); rs_iter != rsr_scores_.end(); ++rs_iter) {
    rs_iter->evaluate();
  }

  // sort by the scores
  rsr_scores_.sort();

  // calculate the minmimum spanning tree
  IMP_LOG(VERBOSE, "calc spanning tree");
  int max_tree_id = 0;

  // initially there are no trees
  for (int i = 0; i < max_type_; i++) {
    tree_id_[i] = 0;
  }

  // start with the lowest score_ edges and work our way up
  Float score = 0.0;
  int num_edges = 0;
  int type1, type2;
  for (rs_iter = rsr_scores_.begin(); (rs_iter != rsr_scores_.end()) && (num_edges < num_types_ - 1); ++rs_iter) {
    type1 = rs_iter->part1_type_;
    type2 = rs_iter->part2_type_;

    IMP_LOG(VERBOSE, "Test for " << type1 << " " << type2);
    // if neither particle is in a tree, create a new tree
    if (!tree_id_[type1] && !tree_id_[type2]) {
      max_tree_id++;
      tree_id_[type1] = max_tree_id;
      tree_id_[type2] = max_tree_id;

      num_edges++;
      IMP_LOG(VERBOSE, "Evaluate for " << type1 << " " << type2);
      score += rs_iter->rsr_->evaluate(calc_deriv);
    }

    // if only one particle is already in a tree, add the other particle
    // to that tree
    else if (!tree_id_[type1] || !tree_id_[type2]) {
      if (tree_id_[type1]) {
        tree_id_[type2] = tree_id_[type1];
      } else {
        tree_id_[type1] = tree_id_[type2];
      }

      num_edges++;
      IMP_LOG(VERBOSE, "Evaluate for " << type1 << " " << type2);
      score += rs_iter->rsr_->evaluate(calc_deriv);
    }

    // both particles are already in trees
    // if they are already in the same tree, do nothing
    // otherwise, merge the two trees
    else {
      if (tree_id_[type1] != tree_id_[type2]) {
        int old_tree_num = tree_id_[type1];
        int new_tree_num = tree_id_[type2];

        for (int j = 0; j < max_type_; j++) {
          if (tree_id_[j] == old_tree_num) {
            tree_id_[j] = new_tree_num;
          }
        }

        num_edges++;
        IMP_LOG(VERBOSE, "Evaluate for " << type1 << " " << type2);
        score += rs_iter->rsr_->evaluate(calc_deriv);
      }
    }
  }

  return score;
}


/**
  Show the current restraint.

 \param[in] out Stream to send restraint description to.
 */

void ConnectivityRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "connectivity restraint (active):" << std::endl;
  } else {
    out << "connectivity restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  " << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  num particles:" << num_particles_;
}

}  // namespace IMP
