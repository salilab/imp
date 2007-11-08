/**
 *  \file ConnectivityRestraint.h    \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONNECTIVITY_RESTRAINT_H
#define __IMP_CONNECTIVITY_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "Restraint.h"
#include "DistanceRestraint.h"

namespace IMP
{

//! Connectivity restraint.
/** Restrict max distance between at least one pair of particles of any
    two distinct types.
 */
class IMPDLLEXPORT ConnectivityRestraint : public Restraint
{
public:
  ConnectivityRestraint(Model& model,
                        std::vector<int>& particle_indexes,
                        const std::string type,
                        BasicScoreFuncParams* score_func_params);

  ConnectivityRestraint(Model& model,
                        std::vector<int>& particle_indexes,
                        const std::string type,
                        const std::string attr_name,
                        BasicScoreFuncParams* score_func_params);

  virtual ~ConnectivityRestraint();

  //! Evaluate this restraint and return the score.
  /** Calculate the distance restraints for the given particles. Use the
      smallest restraints that will connect one particle of each type
      together (i.e. a minimum spanning tree with nodes corresponding to
      particle types and the edge weights corresponding to restraint
      violation scores).

      \param[in] accum If not NULL, use this object to accumulate partial first
                       derivatives.
      \return score associated with this restraint for the given state of
              the model.
   */
  virtual Float evaluate(DerivativeAccumulator *accum);

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

  //! restraints and their scores
  std::list<RestraintScore> rsr_scores_;

  //! Set up initial values for the constructors.
  /** \param[in] model Pointer to the model.
      \param[in] particle_indexes Vector of particle indices. \todo Should use
                                  Particle pointers instead.
      \param[in] type The attribute used to determine if particles
                      are equivalent.
   */
  void set_up(Model& model, std::vector<int>& particle_indexes,
              const std::string type);

  //! variables to determine the particle type
  std::vector<IntIndex> type_;

  //! number of particles in the restraint
  int num_particles_;

  //! maximum type (type can be from 0 to max_type-1)
  int max_type_;
  //! number of particle types
  int num_types_;
  //! particle types
  std::vector<int> particle_type_;

  //! total number of restraints being tested
  int num_restraints_;

  //! each unconnected tree has a non-zero id
  std::vector<int> tree_id_;
};

} // namespace IMP

#endif /* __IMP_CONNECTIVITY_RESTRAINT_H */
