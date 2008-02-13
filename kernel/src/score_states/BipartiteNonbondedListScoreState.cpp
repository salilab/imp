/**
 *  \file BipartiteNonbondedListScoreState.cpp  
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/BipartiteNonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/Grid3D.h"

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
  NonbondedListScoreState(ps0, target)
{
  FloatKeys fks;
  fks.push_back(FloatKey("x"));
  fks.push_back(FloatKey("y"));
  fks.push_back(FloatKey("z"));
  mc_= std::auto_ptr<MaxChangeScoreState>(new MaxChangeScoreState(fks));
  set_particles(ps0, ps1);
}

BipartiteNonbondedListScoreState::~BipartiteNonbondedListScoreState()
{
}

void BipartiteNonbondedListScoreState::rescan(float cut)
{
  if (P::last_cutoff_ >= cut) {
    return;
  }
  P::last_cutoff_=cut;

  /*for (Grid::IndexIterator it= P::grid_.all_indexes_begin(); 
       it != P::grid_.all_indexes_end(); ++it) {
    if (!P::grid_.get_voxel(*it).empty()) {
      IMP_LOG(VERBOSE, "Voxel " << *it << ":");
      for (unsigned int i=0; i< P::grid_.get_voxel(*it).size(); ++i) {
        IMP_LOG(VERBOSE, " " << P::grid_.get_voxel(*it)[i]->get_index());
      }
      IMP_LOG(VERBOSE, std::endl);
    }
    }*/

  nbl_.clear();
  for (unsigned int i=0; i< get_particles().size(); ++i) {
    Particle *p= get_particles()[i];
    XYZDecorator d= XYZDecorator::cast(p);
    Grid::VirtualIndex index
      = P::grid_.get_virtual_index(Vector3D(d.get_x(), d.get_y(), d.get_z()));
    //IMP_LOG(VERBOSE, "Index is " << index << std::endl);
    P::handle_particle(p, index, cut, false);
  }
  IMP_LOG(VERBOSE, "Found " << nbl_.size() << " bipartite nonbonded pairs" 
          << " for " << P::get_particles().size() << " and " 
          << get_particles().size() << " sized sets" << std::endl);
}

void BipartiteNonbondedListScoreState::set_particles(const Particles &ps0,
                                                     const Particles &ps1) 
{
  IMP_LOG(VERBOSE, "Setting bipartite particles " << ps0.size()
          << " and " << ps1.size() << std::endl);
  mc_->set_particles(ps1);
  P::set_particles(ps0);
  P::audit_particles(ps1);
  IMP_assert(ps0.size() == P::get_particles().size(),
             "Where did they go?");
  IMP_assert(ps1.size() == get_particles().size(),
             "Where did we go?");
}

void BipartiteNonbondedListScoreState::update()
{
  P::update();
  if (mc_->get_max() > P::target_voxel_side_) {
    P::invalidate_nbl();
  }
}

void BipartiteNonbondedListScoreState::show(std::ostream &out) const
{
  out << "BipartiteNonbondedList" << std::endl;
}

} // namespace IMP
