/**
 *  \file ParticleGrid.cpp
 *  \brief A grid for keeping track of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/MaxChangeScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/internal/ParticleGrid.h"

namespace IMP
{

namespace internal
{


ParticleGrid::ParticleGrid(float tvs): target_voxel_side_(tvs),
                                       grid_valid_(false)
{
  FloatKeys fks;
  fks.push_back(FloatKey("x"));
  fks.push_back(FloatKey("y"));
  fks.push_back(FloatKey("z"));
  mc_= std::auto_ptr<MaxChangeScoreState>(new MaxChangeScoreState(fks));
}

ParticleGrid::ParticleGrid(): target_voxel_side_(0), grid_valid_(0)
{
}

void ParticleGrid::build_grid()
{
  IMP_LOG(VERBOSE, "Creating nonbonded grid..." << std::flush);
  float mn[3]= {std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()};
  float mx[3]={-std::numeric_limits<float>::max(),
               -std::numeric_limits<float>::max(),
               -std::numeric_limits<float>::max()};
  for (unsigned int i = 0; i < mc_->get_particles().size(); ++i) {
    XYZDecorator d= XYZDecorator::cast(mc_->get_particles()[i]);
    for (unsigned int j=0; j<3; ++j) {
      if (d.get_coordinate(j)< mn[j]) mn[j]= d.get_coordinate(j);
      if (d.get_coordinate(j)> mx[j]) mx[j]= d.get_coordinate(j);
    }
  }
  if (!mc_->get_particles().empty()) {
    // keep the grid size sane if things blow up
    float maxdim= std::max(mx[0]-mn[0], std::max(mx[1]-mn[1], mx[2]-mn[2]));
    float vx= std::pow(static_cast<float>(10.0*(maxdim*maxdim*maxdim
                                                /mc_->get_particles().size())),
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
  for (unsigned int i = 0; i < mc_->get_particles().size(); ++i) {
    XYZDecorator d= XYZDecorator::cast(mc_->get_particles()[i]);
    Vector3D v(d.get_x(), d.get_y(), d.get_z());
    grid_.get_voxel(grid_.get_index(v)).push_back(mc_->get_particles()[i]);
  }
  grid_valid_=true;
  mc_->reset();
  IMP_LOG(VERBOSE, "done." << std::endl);
}


void ParticleGrid::add_particles(const Particles &ps)
{
  audit_particles(ps);
  mc_->add_particles(ps);
}

void ParticleGrid::clear_particles()
{
  mc_->clear_particles();
}

bool ParticleGrid::update()
{
  if (!grid_valid_ || mc_->get_max() > target_voxel_side_) {
    build_grid();
    return true;
  }
  return false;
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

} // namespace internal

} // namespace IMP
