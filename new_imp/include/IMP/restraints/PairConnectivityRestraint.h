/**
 *  \file PairConnectivityRestraint.h   \brief  Pair connectivity restraint.
 *
 *  Restrict max distance between one or more pair of particles of any
 *  two sets of particles (e.g. rigid bodies).
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PAIR_CONNECTIVITY_RESTRAINT_H
#define __IMP_PAIR_CONNECTIVITY_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "Restraint.h"
#include "DistanceRestraint.h"

namespace IMP
{

//! Pair connectivity restraint.
/** Restrict max distance between one or more pair of particles of any
    two sets of particles (e.g. rigid bodies).
 */
class IMPDLLEXPORT PairConnectivityRestraint : public Restraint
{
public:
  PairConnectivityRestraint(Model& model, std::vector<int>& particle1_indexes,
                            std::vector<int>& particle2_indexes,
                            BasicScoreFuncParams* score_func_params,
                            const int num_to_apply = 1,
                            const bool particle_reuse = false);

  PairConnectivityRestraint(Model& model, std::vector<int>& particle1_indexes,
                            std::vector<int>& particle2_indexes,
                            const std::string attr_name,
                            BasicScoreFuncParams* score_func_params,
                            const int num_to_apply,
                            const bool particle_reuse = false);

  virtual ~PairConnectivityRestraint();

  //! Evaluate the restraint for the current model state.
  /** Calculate the distance restraints for the given particles. Use the
      smallest restraints that will connect one particle of each type together
      (i.e. a minimum spanning tree with nodes corresponding to particle
      types and the edge weights corresponding to restraint violation scores).

      There is a potential when using this restraint for two large rigid bodies
      to maintain its own exclusion volume restraints. These could be calculated
      only for the particles in the neighborhoods of the particles involved in
      the activated restraints rather than between all particles in the two
      bodies. Since once restraints are activated, they tend to be activated
      over and over, the exclusion volume restraint sets should only be reset
      whenever on rare occasion the neighborhoods actually change.
    
      \param[in] calc_deriv If true, partial first derivatives should be
                            calculated.
      \return score associated with this restraint for the given state of
              the model.
   */
  virtual Float evaluate(bool calc_deriv);

  //! Show the current restraint.
  /** \param[in] out Stream to send restraint description to.
   */
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

  //! Internal set up for the constructors.
  /** \param[in] model Pointer to the model.
      \param[in] particle1_indexes Vector of indexes of particles in first body
                                   of the restraint.
      \param[in] particle2_indexes Vector of indexes of particles in second body
                                   of the restraint.
   */
  void set_up(Model& model, std::vector<int>& particle1_indexes,
              std::vector<int>& particle2_indexes);

  //! variables to determine the particle type
  std::vector<IntIndex> type_;

  //! number of particles all together
  int num_particles_;
  //! number of particles in vector 1
  int num_particles1_;
  //! number of particles in vector 2
  int num_particles2_;
  //! total number of restraints to apply
  int num_to_apply_;
  //! true if a particle can be involved in more than one restraint
  int particle_reuse_;
  //! which of particles have already been used
  std::vector<bool> used_;

  //! total number of restraints being tested
  int num_restraints_;
  //! restraints and their scores
  std::list<RestraintScore> rsr_scores_;
};

} // namespace IMP

#endif /* __IMP_PAIR_CONNECTIVITY_RESTRAINT_H */
