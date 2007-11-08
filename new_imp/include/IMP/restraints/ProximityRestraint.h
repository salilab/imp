/**
 *  \file ProximityRestraint.h   \brief Proximity restraint.
 *
 *  Restrict maximum distance between any two particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PROXIMITY_RESTRAINT_H
#define __IMP_PROXIMITY_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "Restraint.h"
#include "DistanceRestraint.h"
#include <vector>

namespace IMP
{

//! Restrict maximum distance between any two particles
/** Given a list of particles, this restraint calculates the distance
    restraints between all pairs of particles, and then applies the
    one restraint with the greatest score.
 */
class IMPDLLEXPORT ProximityRestraint : public Restraint
{
public:
  ProximityRestraint(Model& model, std::vector<int>& particle_indexes,
                     const Float distance,
                     BasicScoreFuncParams* score_func_params);

  ProximityRestraint(Model& model, std::vector<int>& particle_indexes,
                     const std::string attr_name, const Float distance,
                     BasicScoreFuncParams* score_func_params);

  virtual ~ProximityRestraint();

  //! Evaluate the score for the model.
  /** Calculate the distance restraints for the given particles. Use the
      smallest restraints that will connect one particle of each type together
      (i.e. a minimum spanning tree with nodes corresponding to particle
      types and the edge weights corresponding to restraint violation score
      values).

      \param[in] accum If not NULL, use this object to accumulate partial first
                       derivatives.
      \return score associated with this restraint for the given state of
              the model.
   */
  virtual Float evaluate(DerivativeAccumulator *accum);

  //! Show the current restraint.
  /** \param[in] out Stream to send restraint description to.
   */
  virtual void show(std::ostream& out = std::cout) const;

  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  //! Internal set up for the constructors.
  /** \param[in] model Pointer to the model.
      \param[in] particle_indexes Vector of indexes of particles in the
                 restraint.
   */
  void set_up(Model& model, std::vector<int>& particle_indexes);

  //! number of particles in the restraint
  int num_particles_;

  //! total number of restraints being tested
  int num_restraints_;
  //! calculated restraint scores
  std::vector<Float> scores_;
  //! indexes of local indexes sorted by energies
  std::vector<int> rsr_idx_;
  //! the distance restraints
  std::vector<DistanceRestraint*> dist_rsrs_;
};

} // namespace IMP

#endif /* __IMP_PROXIMITY_RESTRAINT_H */
