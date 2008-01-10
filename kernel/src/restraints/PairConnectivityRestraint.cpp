/**
 *  \file PairConnectivityRestraint.cpp \brief  Pair connectivity restraint.
 *
 *  Restrict max distance between one or more pair of particles of any
 *  two sets of particles (e.g. rigid bodies).
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/PairConnectivityRestraint.h"
#include "../mystdexcept.h"

namespace IMP
{

//! Set up a restraint using the given mean for particle-particle distance.
/** \param[in] model Pointer to the model.
    \param[in] particle1_indexes Vector of indexes of particles of first body.
    \param[in] particle2_indexes Vector of indexes of particles of second body.
    \param[in] score_func_params Parameters for creating a score function.
    \param[in] num_to_apply Number of minimum restraints to apply.
    \param[in] particle_reuse Allow minimum restraints to use particle
                              more than once.
 */
PairConnectivityRestraint::PairConnectivityRestraint(Model* model,
    std::vector<int>& particle1_indexes, std::vector<int>& particle2_indexes,
    BasicScoreFuncParams* score_func_params, const int num_to_apply,
    const bool particle_reuse)
{
  particle_reuse_ = particle_reuse;
  num_to_apply_ = num_to_apply;
  set_up(model, particle1_indexes, particle2_indexes);


  // get the indexes associated with the restraints
  int idx = 0;
  // use iterator to move through each predefined position in the
  // restraint/score list
  std::list<PairConnectivityRestraint::RestraintScore>::iterator rs_iter;
  rs_iter = rsr_scores_.begin();
  for (int i = 0; i < num_particles1_; i++) {
    for (int j = num_particles1_; j < num_particles1_ + num_particles2_; j++) {
      // create the restraint
      if (rs_iter == rsr_scores_.end()) {
        IMP_failure("Reached end of rsr_scores too early.",
                    std::out_of_range("Reached end of rsr_scores too early"));
      } else {
        IMP_LOG(VERBOSE, "Adding possible restraint: " << i << " " << j);
        UnaryFunctor *sf = score_func_params->create_score_func();
        rs_iter->rsr_ = new DistanceRestraint(get_particle(i),
                                              get_particle(j),
                                              sf);

        rs_iter->part1_idx_ = i;
        rs_iter->part2_idx_ = j;
      }

      ++rs_iter;
    }
  }

  IMP_LOG(VERBOSE, idx <<
          "  num_restraints_: " << num_restraints_ << "  num_particles1_: " <<
          num_particles1_ << " num_particles2_:" << num_particles2_
          << std::endl);
}

//! Set up a restraint using the given attribute for particle-particle distance.
/** \param[in] model Pointer to the model.
    \param[in] particle1_indexes Vector of indexes of particles of first body.
    \param[in] particle2_indexes Vector of indexes of particles of second body.
    \param[in] attr_name Name to get radii to calculate the mean distance.
    \param[in] score_func_params Parameters for creating a score function.
    \param[in] num_to_apply Number of minimum restraints to apply.
    \param[in] particle_reuse Allow minimum restraints to use particle
                              more than once.
 */
PairConnectivityRestraint::PairConnectivityRestraint(Model* model,
    std::vector<int>& particle1_indexes, std::vector<int>& particle2_indexes,
    FloatKey attr_name, BasicScoreFuncParams* score_func_params,
    const int num_to_apply, const bool particle_reuse)
{
  particle_reuse_ = particle_reuse;
  num_to_apply_ = num_to_apply;
  set_up(model, particle1_indexes, particle2_indexes);

  // get the indexes associated with the restraints
  int idx = 0;
  Float actual_mean;

  // use iterator to move through each predefined position in the
  // restraint/score list
  std::list<PairConnectivityRestraint::RestraintScore>::iterator rs_iter;
  rs_iter = rsr_scores_.begin();
  // particles from list 1
  for (int i = 0; i < num_particles1_; i++) {
    // particles from list 2
    for (int j = num_particles1_; j < num_particles_; j++) {
      // Use those radii to calculate the expected distance
      Float attri, attrj;
      attri = get_particle(i)->get_value(attr_name);
      attrj = get_particle(j)->get_value(attr_name);
      actual_mean = attri + attrj;

      score_func_params->set_mean(actual_mean);

      // create the restraint
      if (rs_iter == rsr_scores_.end()) {
        IMP_failure("Reached end of rsr_scores too early.",
                    std::out_of_range("Reached end of rsr_scores too early"));
      } else {
        IMP_LOG(VERBOSE, "Adding possible restraint: " << i << " " << j);
        UnaryFunctor *sf = score_func_params->create_score_func();
        rs_iter->rsr_ = new DistanceRestraint(get_particle(i),
                                              get_particle(j),
                                              sf);

        rs_iter->part1_idx_ = i;
        rs_iter->part2_idx_ = j;
      }

      ++rs_iter;
    }
  }

  IMP_LOG(VERBOSE, idx <<
          "  num_restraints_: " << num_restraints_ << "  num_particles1_: " <<
          num_particles1_ << " num_particles2_:" << num_particles2_
          << std::endl);
}


//! Internal set up for the constructors.
/** \param[in] model Pointer to the model.
    \param[in] particle1_indexes Vector of indexes of particles in first body
                                 of the restraint.
    \param[in] particle2_indexes Vector of indexes of particles in second body
                                 of the restraint.
 */
void PairConnectivityRestraint::set_up(Model* model,
                                       std::vector<int>& particle1_indexes,
                                       std::vector<int>& particle2_indexes)
{
  Particle* p1;

  IMP_LOG(VERBOSE, "init ConnectivityRestraint");

  //model_data_ = model.get_model_data();

  /** number of particles in the restraint */
  num_particles1_ = particle1_indexes.size();
  num_particles2_ = particle2_indexes.size();
  num_particles_ = num_particles1_ + num_particles2_;

  IMP_LOG(VERBOSE, "set up particle types");
  // set up the particles, their position indexes, and their type indexes
  for (int i = 0; i < num_particles1_; i++) {
    p1 = model->get_particle(particle1_indexes[i]);
    add_particle(p1);
  }

  for (int i = 0; i < num_particles2_; i++) {
    p1 = model->get_particle(particle2_indexes[i]);
    add_particle(p1);
  }

  // figure out how many restraints there are
  num_restraints_ = num_particles1_ * num_particles2_;

  // use number of restraints and number of types to set up some of the
  // ... main arrays
  used_.resize(num_particles_);
  rsr_scores_.resize(num_restraints_);
}

//! Destructor
PairConnectivityRestraint::~PairConnectivityRestraint()
{
  std::list<PairConnectivityRestraint::RestraintScore>::iterator rs_iter;

  for (rs_iter = rsr_scores_.begin(); rs_iter != rsr_scores_.end(); ++rs_iter) {
    delete(rs_iter->rsr_);
  }
}


//! Evaluate the restraint for the current model state.
/** Calculate the distance restraints for the given particles. Use the smallest
    restraints that will connect one particle of each type together (i.e. a
    minimum spanning tree with nodes corresponding to particle types and the
    edge weights corresponding to restraint violation scores).

    There is a potential when using this restraint for two large rigid bodies
    to maintain its own exclusion volume restraints. These could be calculated
    only for the particles in the neighborhoods of the particles involved in the
    activated restraints rather than between all particles in the two bodies.
    Since once restraints are activated, they tend to be activated over and
    over, the exclusion volume restraint sets should only be reset whenever
    on rare occasion the neighborhoods actually change.

    \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return score associated with this restraint for the given state of
            the model.
 */
Float PairConnectivityRestraint::evaluate(DerivativeAccumulator *accum)
{
  std::list<PairConnectivityRestraint::RestraintScore>::iterator rs_iter;

  IMP_LOG(VERBOSE, "evaluate PairConnectivityRestraint");

  // only use a particle at most once in set of restraints
  for (int i = 0; i < num_particles_; i++)
    used_[i] = false;

  // calculate the scores for all of the restraints
  IMP_LOG(VERBOSE, "calculate restraint scores");
  int j = 0;
  for (rs_iter = rsr_scores_.begin(); rs_iter != rsr_scores_.end(); ++rs_iter) {
    rs_iter->evaluate();
    IMP_LOG(VERBOSE, j++ << " score: " << rs_iter->score_);
  }

  // sort by the scores
  rsr_scores_.sort();

  IMP_LOG(VERBOSE, "sorted");
  j = 0;
  for (rs_iter = rsr_scores_.begin(); rs_iter != rsr_scores_.end(); ++rs_iter) {
    IMP_LOG(VERBOSE, j++ << " score: " << rs_iter->score_);
  }

  Float score = 0.0;
  rs_iter = rsr_scores_.begin();
  int num_applied = 0;
  for (int i = 0; num_applied < num_to_apply_ && rs_iter != rsr_scores_.end();
       ++i) {
    if (particle_reuse_ || (!used_[rs_iter->part1_idx_]
                            && !used_[rs_iter->part2_idx_])) {
      used_[rs_iter->part1_idx_] = true;
      used_[rs_iter->part2_idx_] = true;
      score += rs_iter->rsr_->evaluate(accum);
      num_applied++;
    }

    ++rs_iter;
  }

  return score;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void PairConnectivityRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "pair connectivity restraint (active):" << std::endl;
  } else {
    out << "pair connectivity restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  ";
  out << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  num particles1:" << num_particles1_;
  out << "  num particles2:" << num_particles2_;
  out << "  num restraints:" << num_restraints_;
  out << "  num restraints to apply:" << num_to_apply_;
}

}  // namespace IMP
