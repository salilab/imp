/**
 *  \file ParticleGrid.cpp
 *  \brief A grid for keeping track of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/MaxChangeScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/internal/ParticleGrid.h"
#include "IMP/internal/utility.h"

namespace IMP
{

namespace internal
{

static const int target_cell_occupancy=10;

ParticleGrid::ParticleGrid(float tvs,
                           const Particles &ps): target_voxel_side_(tvs)
{
  IMP_assert(tvs >0, "Target voxel edge size must be positive");
  build_grid(ps);
}

ParticleGrid::ParticleGrid(): target_voxel_side_(0)
{
}

void ParticleGrid::build_grid(const Particles &ps)
{
  audit_particles(ps);
  IMP_LOG(TERSE, "Creating nonbonded grid..." << std::flush);
  float mn[3]= {std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()};
  float mx[3]={-std::numeric_limits<float>::max(),
               -std::numeric_limits<float>::max(),
               -std::numeric_limits<float>::max()};
  for (unsigned int i = 0; i < ps.size(); ++i) {
    XYZDecorator d(ps[i]);
    for (unsigned int j=0; j<3; ++j) {
      if (d.get_coordinate(j)< mn[j]) mn[j]= d.get_coordinate(j);
      if (d.get_coordinate(j)> mx[j]) mx[j]= d.get_coordinate(j);
    }
  }
  if (!ps.empty()) {
    // keep the grid size sane if things blow up
    float maxdim= std::max(mx[0]-mn[0], std::max(mx[1]-mn[1], mx[2]-mn[2]));
    float vx= std::pow(static_cast<float>(target_cell_occupancy
                                          *(maxdim*maxdim*maxdim
                                            /ps.size())),
                       .3333f);
    if (vx > target_voxel_side_) {
      IMP_LOG(VERBOSE, "Overroade target side of " << target_voxel_side_ 
              << " with " << vx << std::endl);
    }
    target_voxel_side_= std::max(vx, target_voxel_side_);
  }
  grid_= Grid(target_voxel_side_,
             Vector3D(mn[0], mn[1], mn[2]),
             Vector3D(mx[0], mx[1], mx[2]),
             Particles());
  for (unsigned int i = 0; i < ps.size(); ++i) {
    XYZDecorator d(ps[i]);
    Vector3D v(d.get_x(), d.get_y(), d.get_z());
    grid_.get_voxel(grid_.get_index(v)).push_back(ps[i]);
  }
  IMP_LOG(TERSE, "done." << std::endl);
}



void ParticleGrid::audit_particles(const Particles &ps) const
{
  for (unsigned int i=0; i< ps.size(); ++i) {
    try {
      XYZDecorator d= XYZDecorator::cast(ps[i]);
    } catch (...) {
      IMP_WARN("Particle " << ps[i]->get_index() 
               << " does not have x,y,z coordinates " 
               << " but was passed to the NonbondedListScoreState.\n");
    }
  }
}


void ParticleGrid::show(std::ostream &out) const
{
  for (Grid::IndexIterator it= grid_.all_indexes_begin();
       it != grid_.all_indexes_end(); ++it) {
    out << *it << ": ";
    //Grid::Index 
    for (unsigned int i=0; i< grid_.get_voxel(*it).size(); ++i) {
      out << grid_.get_voxel(*it)[i]->get_index() << " ";
    }
    out << std::endl;
  }
}

} // namespace internal

} // namespace IMP
