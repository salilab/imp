/**
 *  \file NonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/NonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/Grid3D.h"
#include "IMP/score_states/MaxChangeScoreState.h"

namespace IMP
{

NonbondedListScoreState::NonbondedListScoreState(const Particles &ps, 
                                                 float tvs):
  target_voxel_side_(tvs), grid_valid_(false)
{
  last_cutoff_=-1;
  FloatKeys fks;
  fks.push_back(FloatKey("x"));
  fks.push_back(FloatKey("y"));
  fks.push_back(FloatKey("z"));
  mc_= std::auto_ptr<MaxChangeScoreState>(new MaxChangeScoreState(fks));
  set_particles(ps);
}

NonbondedListScoreState::~NonbondedListScoreState()
{
  IMP_CONTAINER_DELETE(BondedListScoreState, bonded_list);
}

void NonbondedListScoreState::audit_particles(const Particles &ps) const
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

void NonbondedListScoreState::add_if_nonbonded(Particle *a, Particle *b)
{
  if (!a->get_is_active() || !b->get_is_active()) return;
  bool found=false;
  for (BondedListIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    if ((*bli)->are_bonded(a, b)) {
      found = true;
      break;
    }
  }
  if (!found) {
    /*IMP_LOG(VERBOSE, "Found pair " << a->get_index() 
      << " " << b->get_index() << std::endl);*/
    nbl_.push_back(std::make_pair(a, b));
  }
}

void NonbondedListScoreState::create_grid()
{
  IMP_LOG(VERBOSE, "Creating nonbonded grid..." << std::flush);
  float mn[3]= {std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()};
  float mx[3]={-std::numeric_limits<float>::max(),
               -std::numeric_limits<float>::max(),
               -std::numeric_limits<float>::max()};
  for (unsigned int i = 0; i < get_particles().size(); ++i) {
    XYZDecorator d= XYZDecorator::cast(get_particles()[i]);
    for (unsigned int j=0; j<3; ++j) {
      if (d.get_coordinate(j)< mn[j]) mn[j]= d.get_coordinate(j);
      if (d.get_coordinate(j)> mx[j]) mx[j]= d.get_coordinate(j);
    }
  }
  grid_= Grid(target_voxel_side_,
             Vector3D(mn[0], mn[1], mn[2]),
             Vector3D(mx[0], mx[1], mx[2]),
             Particles());
  for (unsigned int i = 0; i < get_particles().size(); ++i) {
    XYZDecorator d= XYZDecorator::cast(get_particles()[i]);
    Vector3D v(d.get_x(), d.get_y(), d.get_z());
    grid_.get_voxel(grid_.get_index(v)).push_back(get_particles()[i]);
  }
  grid_valid_=true;
  mc_->reset();
  invalidate_nbl();
  IMP_LOG(VERBOSE, "done." << std::endl);
}

void NonbondedListScoreState::handle_particle(Particle *p, 
                                              const Grid::VirtualIndex &center,
                                              float cut,
                                              bool skip_lower)
{
  Grid::VirtualIndex lc, uc;

  if ( cut > target_voxel_side_*1000 ) {
    // This is needed to handle overflow
    lc=Grid::VirtualIndex(0,0,0);
    uc=Grid::VirtualIndex(grid_.get_number_of_voxels(0),
                          grid_.get_number_of_voxels(1),
                          grid_.get_number_of_voxels(2));
  } else {
    // it is important that this is not unsigned
    int ncells= static_cast<int>(std::ceil(cut/target_voxel_side_));
    // to allow laziness in rebuilding
    ++ncells;
    lc=Grid::VirtualIndex(center[0]-ncells,
                          center[1]-ncells,
                          center[2]-ncells);
    uc=Grid::VirtualIndex(center[0]+ncells,
                          center[1]+ncells,
                          center[2]+ncells);
  }
  //IMP_LOG(VERBOSE, "Iteration bounds are " << lc << " and " 
  // << uc << std::endl);
  for (Grid::IndexIterator cur= grid_.indexes_begin(lc, uc);
       cur != grid_.indexes_end(lc, uc); 
       ++cur){
    if ( skip_lower && center >= *cur) continue;
    if (grid_.get_voxel(*cur).empty()) continue;
    //IMP_LOG(VERBOSE, "Paired with " << cur << std::endl);

    for (unsigned int pi= 0;
         pi< grid_.get_voxel(*cur).size(); ++pi) {
      Particle *op = grid_.get_voxel(*cur)[pi];
      add_if_nonbonded(p, op);
    }
  }
  if (skip_lower) {
    Grid::Index ri= grid_.get_index(center);
    if (ri != Grid::Index()) {
      const IMP::Particles &pis= grid_.get_voxel(ri);
      for (unsigned int pj= 0; pis[pj] != p; ++pj) {
        Particle *op= pis[pj];
        add_if_nonbonded(p, op);
      }
    }
  }
}


void NonbondedListScoreState::rescan(float cut)
{
  if (last_cutoff_ >= cut) {
    return;
  }
  IMP_LOG(VERBOSE, "Rescanning " << cut 
          << "(" << last_cutoff_ << ")" << std::endl);

  last_cutoff_=cut;

  nbl_.clear();
  IMP_LOG(VERBOSE, "Scanning for nonbonded pairs..." << std::flush);
  for (Grid::IndexIterator center= grid_.all_indexes_begin();
       center != grid_.all_indexes_end();
       ++center) {
    if (grid_.get_voxel(*center).empty()) continue;
    //IMP_LOG(VERBOSE, "Scanning from grid index " << *center << std::endl);
    for (unsigned int pi= 0; pi< grid_.get_voxel(*center).size(); ++pi) {
      Particle *p= grid_.get_voxel(*center)[pi];
      handle_particle(p, *center, cut, true);
    }
  }
  IMP_LOG(VERBOSE, "done" << std::endl);
  //IMP_LOG(VERBOSE, "Found " << nbl_.size() << " nonbonded pairs" 
  //<< std::endl);
  IMP_LOG(VERBOSE, "Found " << nbl_.size() << " nonbonded pairs" 
          << " for " << get_particles().size()
          << " particles" << std::endl);
}

void NonbondedListScoreState::set_particles(const Particles &ps)
{
  audit_particles(get_particles());
  for (BondedListScoreStateIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    (*bli)->set_particles(ps);
  }
  mc_->set_particles(ps);
  grid_valid_=false;
}

void NonbondedListScoreState::update()
{
  IMP_LOG(VERBOSE, "Updating non-bonded list" << std::endl);
  for (BondedListScoreStateIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    (*bli)->update();
  }
  if (!grid_valid_ || mc_->get_max() > target_voxel_side_) {
    create_grid();
  }
}

void NonbondedListScoreState::show(std::ostream &out) const
{
  out << "NonbondedList" << std::endl;
}

IMP_CONTAINER_IMPL(NonbondedListScoreState, BondedListScoreState,
                   bonded_list, BondedListIndex, );
} // namespace IMP
