/**
 *  \file PairConnectivityRestraint.h      Restrict max distance between one
 *                                         or more pair of particles of any
 *                                         two sets of particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PAIR_CONNECTIVITY_RESTRAINT_H
#define __IMP_PAIR_CONNECTIVITY_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "../emfile.h"
#include "Restraint.h"
#include "DistanceRestraint.h"

namespace imp
{

/// Restrict max distance between at one or more pair of particles of any two sets of particles
class IMPDLLEXPORT PairConnectivityRestraint : public Restraint
{
public:
  PairConnectivityRestraint(Model& model,
                        // couldn't get Swig to work with std::vector<Particle*>&
                        std::vector<int>& particle1_indexes,
                        std::vector<int>& particle2_indexes,
                        BasicScoreFuncParams* score_func_params,
                        const int num_to_apply = 1,
                        const bool particle_reuse = false);

  PairConnectivityRestraint(Model& model,
                        // couldn't get Swig to work with std::vector<Particle*>&
                        std::vector<int>& particle1_indexes,
                        std::vector<int>& particle2_indexes,
                        const std::string attr_name,
                        BasicScoreFuncParams* score_func_params,
                        const int num_to_apply,
                        const bool particle_reuse = false);

  virtual ~PairConnectivityRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  class RestraintScore
  {
  public:
    RestraintScore() {}
    ~RestraintScore() {}
    void evaluate(void) {
      score_ = rsr_->evaluate(false);
    }
    bool operator<(const RestraintScore& rs) const {
      return score_ < rs.score_;
    }

    int part1_idx_;
    int part2_idx_;
    DistanceRestraint* rsr_;
    Float score_;
  };

  void set_up(Model& model,
              // couldn't get Swig to work with std::vector<Particle*>&
              std::vector<int>& particle1_indexes,
              std::vector<int>& particle2_indexes);

  // variables to determine the particle type
  std::vector<IntIndex> type_;

  /** number of particles all together */
  int num_particles_;
  /** number of particles in vector 1 */
  int num_particles1_;
  /** number of particles in vector 2 */
  int num_particles2_;
  /** total number of restraints to apply */
  int num_to_apply_;
  /** true if a particle can be involved in more than one restraint that is applied */
  int particle_reuse_;
  /** which of particles have already been used */
  std::vector<bool> used_;

  /** total number of restraints being tested */
  int num_restraints_;
  /** restraints and their scores */
  std::list<RestraintScore> rsr_scores_;
};

} // namespace imp

#endif /* __IMP_PAIR_CONNECTIVITY_RESTRAINT_H */
