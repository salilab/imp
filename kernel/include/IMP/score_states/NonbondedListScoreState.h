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
#include "../internal/Grid3D.h"

namespace IMP
{

class BondedListScoreState;
class MaxChangeScoreState;
typedef std::vector<BondedListScoreState*> BondedListScoreStates;

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
  typedef internal::Grid3D<Particles> Grid;
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
