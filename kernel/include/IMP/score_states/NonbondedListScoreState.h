/**
 *  \file NonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_NONBONDED_LIST_SCORE_STATE_H

#include <vector>
#include <limits>
#include "BondedListScoreState.h"
#include "MaxChangeScoreState.h"
#include "../ScoreState.h"
#include "../Grid3D.h"

namespace IMP
{

class BondedListScoreState;
class MaxChangeScoreState;

//! This class maintains a list of non-bonded pairs.
/** \ingroup restraint
 */
class IMPDLLEXPORT NonbondedListScoreState: public ScoreState
{
protected:
  float target_voxel_side_;
  typedef std::vector<std::pair<Particle*, Particle*> > NBL;
  NBL nbl_;
  float last_cutoff_;
  bool grid_valid_;
  typedef Grid3D<Particles> Grid;
  Grid grid_;
  std::auto_ptr<MaxChangeScoreState> mc_;


  virtual void rescan(float cut);
  void audit_particles(const Particles &ps) const;
  void add_if_nonbonded(Particle *a, Particle *b);
  void invalidate_nbl() {
    last_cutoff_=-1;
  }
  void create_grid();

  // Handle the particle against all the cells other than 
  // the one containing it.
  void handle_particle(Particle *p, 
                       const Grid::VirtualIndex& v,
                       float cut,
                       bool skip_lower);
  /*class SingeParticleIterator {
    void advance() {
      if (curv_== Grid::IndexIterator()) return;
      if (curp_== nbl_->get_grid().get_voxel(*curv_).end()) {
        ++curv_;
        if (curv_ != Grid::IndexIterator()) {
          curp_= nbl_->get_grid().get_voxel(*curv_).begin();
        }
      }
    }
    void find() {
      bool ok=false;
      do {
        ok=true;
        if ( sl_ && i_ >= *curv_) {
          ok=false;
        } else if (curp_== nbl_->get_grid().get_voxel(*curv_).end()) {
          ok=false;
        } else if (nbl_->are_bonded(p_, *curp_)) {
          ok=false;
        }
        if (!ok) advance();
      } while (!ok);
      if (nbl_->are_bonded(p_, 
      return true;

    }
  public:
    SingleParticleIterator(Particle *p, 
                           const Grid::VirtualIndex& v,
                           int ncells,
                           bool skip_lower,
                           NonbondedListScoreState *nbl): p_(p),
                                                          i_(v),
                                                          sl_(skip_lower),
                                                          nbl_(nbl) {
      Grid::VirtualIndex lc(i_[0]-ncells,
                            i_[1]-ncells,
                            i_[2]-ncells);
      Grid::VirtualIndex uc(i_[0]+ncells,
                            i_[1]+ncells,
                            i_[2]+ncells);
      curv_= nbl_->get_grid().indexes_begin(lc, uc);
      endv_= nbl_->get_grid().indices_end(lc, uc);
      curp_= nbl_->get_grid().get_voxel(*curv_);
      find_next();
    }
    };*/

public:
  /**
     \param[in] ps A list of particles to use.
     \param[in] tvs A suggested size for the voxel side.
   */
  NonbondedListScoreState(const Particles &ps, float tvs=1);
  virtual ~NonbondedListScoreState();
  IMP_CONTAINER(BondedListScoreState, bonded_list,
                BondedListIndex);
  // kind of evil hack to make the names better
  // perhaps the macro should be made more flexible
  typedef BondedListScoreStateIterator BondedListIterator;
public:
  IMP_SCORE_STATE("0.5", "Daniel Russel");

  void set_particles(const Particles &ps);

  typedef NBL::const_iterator NonbondedIterator;

  //! This iterates through the pairs of non-bonded particles
  /** \param[in] cutoff The state may ignore pairs which are futher
      apart than the cutoff.
      \note that this is highly unsafe and iteration can only be
      done once at a time. I will fix that eventually.
  */
  NonbondedIterator nonbonded_begin(Float cutoff
                                    =std::numeric_limits<Float>::max()) {
    IMP_assert(last_cutoff_== cutoff || last_cutoff_==-1,
               "Bad things are happening with the iterators in "
               << "NonbondedListScoreState");
    rescan(cutoff);
    return nbl_.begin();
  }
  NonbondedIterator nonbonded_end(Float cutoff
                                  =std::numeric_limits<Float>::max()) {
    rescan(cutoff);
    return nbl_.end();
  }

  const Particles &get_particles() const {
    return mc_->get_particles();
  }
};

} // namespace IMP

#endif  /* __IMP_NONBONDED_LIST_SCORE_STATE_H */
