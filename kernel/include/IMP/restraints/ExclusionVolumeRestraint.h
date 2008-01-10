/**
 *  \file ExclusionVolumeRestraint.h    \brief Excluded volume restraint.
 *
 *  Prevent particles from getting too close together, by restricting the
 *  minimum distance between all pairs of particles formed from one or two
 *  sets of particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_EXCLUSION_VOLUME_RESTRAINT_H
#define __IMP_EXCLUSION_VOLUME_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../ScoreFuncParams.h"
#include "DistanceRestraint.h"

namespace IMP
{

//! Apply restraints that prevent particles from getting too close together.
/** Restrict min distance between all pairs of particles formed from one or
    two sets of particles. If two sets of particles are passed, it assumes
    that the two sets of particles have no overlap (otherwise, you will get
    repeats).

    Calculate the distance restraints for the given particles. Use the
    smallest restraints that will connect one particle of each type
    together (i.e. a minimum spanning tree with nodes corresponding to
    particle types and the edge weights corresponding to restraint
    violation scores).
 */
class IMPDLLEXPORT ExclusionVolumeRestraint : public Restraint
{
public:
  ExclusionVolumeRestraint(Model* model, std::vector<int>& particle1_indexes,
                           std::vector<int>& particle2_indexes,
                           FloatKey attr_name,
                           BasicScoreFuncParams* score_func_params);

  ExclusionVolumeRestraint(Model* model, std::vector<int>& particle_indexes,
                           FloatKey attr_name,
                           BasicScoreFuncParams* score_func_params);

  virtual ~ExclusionVolumeRestraint();

  IMP_RESTRAINT("0.5", "Daniel Russel")

protected:
  //! number of particles all together
  int num_particles_;
  //! number of particles in vector 1
  int num_particles1_;
  //! number of particles in vector 2
  int num_particles2_;

  //! total number of restraints
  int num_restraints_;
  //! restraints and their scores
  std::vector<DistanceRestraint*> dist_rsrs_;
};

} // namespace IMP

#endif /* __IMP_EXCLUSION_VOLUME_RESTRAINT_H */
