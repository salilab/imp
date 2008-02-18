/**
 *  \file AllNonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/AllNonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/internal/Grid3D.h"
#include "IMP/score_states/MaxChangeScoreState.h"

namespace IMP
{

AllNonbondedListScoreState::AllNonbondedListScoreState(const Particles &ps, 
                                                       float tvs):
  grid_(tvs)
{
  set_particles(ps);
}

void AllNonbondedListScoreState::rebuild_nbl(float cut)
{
  IMP_LOG(VERBOSE, "Scanning for nonbonded pairs..." << std::flush);
  internal::ParticleGrid::Index last_index;
  for (internal::ParticleGrid::ParticleVoxelIterator it
         = grid_.particle_voxels_begin();
         it != grid_.particle_voxels_end(); ++it) {
    IMP_LOG(VERBOSE, "Searching with particle " << it->first->get_index()
            << std::endl);
    NonbondedListScoreState::AddToNBL f(this, it->first);
    grid_.apply_to_nearby(f, it->second, cut, true);
    if (it->second != last_index) {
      grid_.apply_to_cell_pairs(f, it->second);
    }
    last_index=it->second;
  }

  IMP_LOG(VERBOSE, "done" << std::endl);
  IMP_LOG(VERBOSE, "Found " << NonbondedListScoreState::size_nbl()
          << " nonbonded pairs" 
          << std::endl);
}

void AllNonbondedListScoreState::set_particles(const Particles &ps)
{
  grid_.clear_particles();
  grid_.add_particles(ps);
  NonbondedListScoreState::propagate_particles(ps);
}

void AllNonbondedListScoreState::update()
{
  NonbondedListScoreState::update();
  if (grid_.update()) {
    NonbondedListScoreState::clear_nbl();
  }
}

void AllNonbondedListScoreState::show(std::ostream &out) const
{
  out << "NonbondedList" << std::endl;
}

} // namespace IMP
