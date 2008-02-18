/**
 *  \file BipartiteNonbondedListScoreState.cpp  
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/BipartiteNonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/score_states/MaxChangeScoreState.h"

namespace IMP
{

  /*
    Iterator needs:
    iterator into ps_, end iterator into ps_
    cell for ps_[i]
    -> actually has value current iterator around cell
    -put in logic for current
    number of bins to search


    nonbipartite:
    iterator through all cells
    index in current cell
    -> actual value in iterator around cell
    num bins
   */

BipartiteNonbondedListScoreState
::BipartiteNonbondedListScoreState(const Particles &ps0,
                                   const Particles &ps1, float target):
  grid_(target)
{
  FloatKeys fks;
  fks.push_back(FloatKey("x"));
  fks.push_back(FloatKey("y"));
  fks.push_back(FloatKey("z"));
  mc_= std::auto_ptr<MaxChangeScoreState>(new MaxChangeScoreState(fks));
  set_particles(ps0, ps1);
}

void BipartiteNonbondedListScoreState::rebuild_nbl(float cut)
{
  for (unsigned int i=0; i< mc_->get_particles().size(); ++i) {
    Particle *p= mc_->get_particles()[i];
    NonbondedListScoreState::AddToNBL f(this, p);
    XYZDecorator d= XYZDecorator::cast(p);
    internal::ParticleGrid::VirtualIndex index
      = grid_.get_virtual_index(Vector3D(d.get_x(), d.get_y(), d.get_z()));
    IMP_LOG(VERBOSE, "Searching for " << p->get_index()
            << " from cell " << index << std::endl);
    grid_.apply_to_nearby(f, index, cut, false);
  }
  IMP_LOG(VERBOSE, "Found " << P::size_nbl()
          << " bipartite nonbonded pairs" 
          << " for " << mc_->get_particles().size() 
          << " nongridded particles" << std::endl);
}

void BipartiteNonbondedListScoreState::set_particles(const Particles &ps0,
                                                     const Particles &ps1) 
{
  IMP_LOG(VERBOSE, "Setting bipartite particles " << ps0.size()
          << " and " << ps1.size() << std::endl);
  mc_->clear_particles();
  mc_->add_particles(ps1);
  grid_.clear_particles();
  grid_.add_particles(ps0);
  Particles all(ps0);
  all.insert(all.end(), ps1.begin(), ps1.end());
  NonbondedListScoreState::propagate_particles(all);
  /*IMP_assert(ps0.size() == P::get_particles().size(),
    "Where did they go?");*/
  IMP_assert(ps1.size() == mc_->get_particles().size(),
             "Where did we go?");
}

void BipartiteNonbondedListScoreState::update()
{
  NonbondedListScoreState::update();
  if (mc_->get_max() > grid_.get_voxel_size() 
      || grid_.update()) {
    NonbondedListScoreState::clear_nbl();
  }
}

void BipartiteNonbondedListScoreState::show(std::ostream &out) const
{
  out << "BipartiteNonbondedList" << std::endl;
}

} // namespace IMP
