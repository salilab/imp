/*
 *  \file ProximityRestraint.h     Restrict maximum distance between any
 *                                 two particles.
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

namespace imp
{

//! Restrict maximum distance between any two particles
class IMPDLLEXPORT ProximityRestraint : public Restraint
{
public:
  ProximityRestraint(Model& model,
                // couldn't get Swig to work with std::vector<Particle*>&
                std::vector<int>& particle_indexes,
                const Float distance,
                BasicScoreFuncParams* score_func_params);

  ProximityRestraint(Model& model,
                // couldn't get Swig to work with std::vector<Particle*>&
                std::vector<int>& particle_indexes,
                const std::string attr_name,
                const Float distance,
                BasicScoreFuncParams* score_func_params);

  virtual ~ProximityRestraint();

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
  void set_up(Model& model,
              // couldn't get Swig to work with std::vector<Particle*>&
              std::vector<int>& particle_indexes);

  /** number of particles in the restraint */
  int num_particles_;

  /** total number of restraints being tested */
  int num_restraints_;
  /** calculated restraint scores */
  std::vector<Float> scores_;
  /** indexes of local indexes sorted by energies */
  std::vector<int> rsr_idx_;
  /** the distance restraints */
  std::vector<DistanceRestraint*> dist_rsrs_;
};

} // namespace imp

#endif /* __IMP_PROXIMITY_RESTRAINT_H */
