/**
 *  \file ConnectivityRestraint.h     Restrict max distance between at least
 *                                    one pair of particles of any two
 *                                    distinct types.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONNECTIVITY_RESTRAINT_H
#define __IMP_CONNECTIVITY_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "../emfile.h"
#include "Restraint.h"
#include "DistanceRestraint.h"

namespace IMP
{

// Restrict max distance between at least one pair of particles of any two distinct types
class IMPDLLEXPORT ConnectivityRestraint : public Restraint
{
public:
  ConnectivityRestraint(Model& model,
                   // couldn't get Swig to work with std::vector<Particle*>&
                   std::vector<int>& particle_indexes,
                   const std::string type,
                   BasicScoreFuncParams* score_func_params);

  ConnectivityRestraint(Model& model,
                   // couldn't get Swig to work with std::vector<Particle*>&
                   std::vector<int>& particle_indexes,
                   const std::string type,
                   const std::string attr_name,
                   BasicScoreFuncParams* score_func_params);

  virtual ~ConnectivityRestraint();

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
  // switch to using rsr_scores to allow STL sorting
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

    int part1_type_;
    int part2_type_;
    DistanceRestraint* rsr_;
    Float score_;
  };

  /** restraints and their scores */
  std::list<RestraintScore> rsr_scores_;

  void set_up(Model& model,
              // couldn't get Swig to work with std::vector<Particle*>&
              std::vector<int>& particle_indexes,
              const std::string type);

  // variables to determine the particle type
  std::vector<IntIndex> type_;

  /** number of particles in the restraint */
  int num_particles_;

  /** maximum type (type can be from 0 to max_type-1) */
  int max_type_;
  /** number of particle types */
  int num_types_;
  /** particle types */
  std::vector<int> particle_type_;

  /** total number of restraints being tested */
  int num_restraints_;

  /** each unconnected tree has a non-zero id */
  std::vector<int> tree_id_;
};

} // namespace IMP

#endif /* __IMP_CONNECTIVITY_RESTRAINT_H */
