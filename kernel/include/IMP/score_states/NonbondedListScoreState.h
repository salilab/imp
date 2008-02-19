/**
 *  \file NonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_NONBONDED_LIST_SCORE_STATE_H

#include "../ScoreState.h"
#include "../internal/ParticleGrid.h"
#include "BondedListScoreState.h"

#include <vector>
#include <limits>

namespace IMP
{

typedef std::vector<BondedListScoreState*> BondedListScoreStates;

//! A base class for classes that maintain a list of non-bonded pairs.
/** \ingroup restraint
 */
class IMPDLLEXPORT NonbondedListScoreState: public ScoreState
{
private:
  typedef std::vector<std::pair<Particle*, Particle*> > NBL;
  NBL nbl_;
  float last_cutoff_;
protected:

  unsigned int size_nbl() const {return nbl_.size();}

  //! rebuild the nonbonded list
  /** \internal
   */
  virtual void rebuild_nbl(float cut)=0;

  void clear_nbl() {
    last_cutoff_=-1;
    nbl_.clear();
  }

  struct AddToNBL;
  friend struct AddToNBL;

  struct AddToNBL{
    NonbondedListScoreState *state_;
    Particle *p_;
    AddToNBL(NonbondedListScoreState *s, Particle *p): state_(s),
                                                             p_(p){}
    void operator()(Particle *p) {
      operator()(p_, p);
    }
    void operator()(Particle *a, Particle *b) {
      state_->add_to_nbl(a,b);
    }
  };

  //! tell the bonded lists what particles to pay attention to
  void propagate_particles(const Particles &ps);

  void add_to_nbl(Particle *a, Particle *b) {
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
      IMP_LOG(VERBOSE, "Found pair " << a->get_index() 
        << " " << b->get_index() << std::endl);
      nbl_.push_back(std::make_pair(a, b));
    }
  }

public:
  /**
   */
  NonbondedListScoreState();

  IMP_CONTAINER(BondedListScoreState, bonded_list,
                BondedListIndex);
  // kind of evil hack to make the names better
  // perhaps the macro should be made more flexible
  typedef BondedListScoreStateIterator BondedListIterator;

  IMP_SCORE_STATE("0.5", "Daniel Russel");

  //! An iterator through nonbonded particles
  /** The value type is an std::pair<Particle*, Particle*> 
   */
  typedef NBL::const_iterator NonbondedIterator;

  //! This iterates through the pairs of non-bonded particles
  /** \param[in] cutoff The state may ignore pairs which are futher
      apart than the cutoff.
      \note that this is highly unsafe and iteration can only be
      done once at a time. I will fix that eventually.

      \note The distance cutoff is the l2 norm between the 3D coordinates
      of the Particles. It ignore any size that may be associated with
      the particles.
  */
  NonbondedIterator nonbonded_begin(Float cutoff
                                    =std::numeric_limits<Float>::max()) {
    IMP_assert(last_cutoff_== cutoff || last_cutoff_==-1,
               "Bad things are happening with the iterators in "
               << "NonbondedListScoreState");
    if (last_cutoff_ < cutoff) {
      IMP_LOG(VERBOSE, "Rebuilding NBL cutoff " << cutoff << std::endl);
      clear_nbl();
      rebuild_nbl(cutoff);
      last_cutoff_=cutoff;
    }
    return nbl_.begin();
  }
  NonbondedIterator nonbonded_end(Float cutoff
                                  =std::numeric_limits<Float>::max()) {
    if (last_cutoff_ < cutoff) {
      IMP_LOG(VERBOSE, "Rebuilding NBL cutoff " << cutoff << std::endl);
      clear_nbl();
      rebuild_nbl(cutoff);
      last_cutoff_=cutoff;
    }
    return nbl_.end();
  }
};

} // namespace IMP

#endif  /* __IMP_NONBONDED_LIST_SCORE_STATE_H */
