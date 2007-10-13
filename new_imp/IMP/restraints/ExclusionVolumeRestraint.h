/**
 *  \file ExclusionVolumeRestraint.h     Restrict min distance between all
 *                                       pairs of particles of formed from
 *                                       one or two sets of particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_EXCLUSION_VOLUME_RESTRAINT_H
#define __IMP_EXCLUSION_VOLUME_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "Restraint.h"
#include "DistanceRestraint.h"

namespace IMP
{

// Restrict min distance between all pairs of particles of formed from one or two sets of particles.
// If two sets of particles are passed, it assumes that the two sets of particles have no overlap
// (otherwise, you will get repeats).
class IMPDLLEXPORT ExclusionVolumeRestraint : public Restraint
{
public:
  ExclusionVolumeRestraint(Model& model,
                       // couldn't get Swig to work with std::vector<Particle*>&
                       std::vector<int>& particle1_indexes,
                       std::vector<int>& particle2_indexes,
                       const std::string attr_name,
                       BasicScoreFuncParams* score_func_params
                      );

  ExclusionVolumeRestraint(Model& model,
                       // couldn't get Swig to work with std::vector<Particle*>&
                       std::vector<int>& particle_indexes,
                       const std::string attr_name,
                       BasicScoreFuncParams* score_func_params
                      );

  virtual ~ExclusionVolumeRestraint();

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
  /** number of particles all together */
  int num_particles_;
  /** number of particles in vector 1 */
  int num_particles1_;
  /** number of particles in vector 2 */
  int num_particles2_;

  /** total number of restraints */
  int num_restraints_;
  /** restraints and their scores */
  std::vector<DistanceRestraint*> dist_rsrs_;
};

} // namespace IMP

#endif /* __IMP_EXCLUSION_VOLUME_RESTRAINT_H */
