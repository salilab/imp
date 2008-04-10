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
#include "../internal/kernel_version_info.h"
#include "BondedListScoreState.h"

#include <vector>
#include <limits>

namespace IMP
{

typedef std::vector<BondedListScoreState*> BondedListScoreStates;

//! A base class for classes that maintain a list of non-bonded pairs.
/** 
 */
class IMPDLLEXPORT NonbondedListScoreState: public ScoreState
{
private:
  FloatKey rk_;

protected:
  // made protected for debugging code, do not use otherwise
  typedef std::vector<std::pair<Particle*, Particle*> > NBL;
  NBL nbl_;
  float last_cutoff_;

  unsigned int size_nbl() const {return nbl_.size();}

  //! rebuild the nonbonded list
  /** \internal
   */
  virtual void rebuild_nbl(float cut)=0;

  void clear_nbl() {
    last_cutoff_=-1;
    nbl_.clear();
  }

  bool are_bonded(Particle *a, Particle *b) const {
    IMP_assert(a->get_is_active() && b->get_is_active(),
               "Inactive particles should have been removed"
               << a << " and " << b);
    for (BondedListConstIterator bli= bonded_lists_begin();
         bli != bonded_lists_end(); ++bli) {
      if ((*bli)->are_bonded(a, b)) {
        return true;
      }
    }
    return false;
  }

  struct AddToNBL;
  friend struct AddToNBL;

  // these should be two separate classes, but they are not
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
    IMP_assert(a->get_is_active() && b->get_is_active(),
               "Inactive particles should have been stripped");

    if (!are_bonded(a,b)) {
      IMP_LOG(VERBOSE, "Found pair " << a->get_index() 
        << " " << b->get_index() << std::endl);
      nbl_.push_back(std::make_pair(a, b));
    } else {
      IMP_LOG(VERBOSE, "Pair " << a->get_index()
              << " and " << b->get_index() << " rejected on bond" 
              <<std::endl);
    }
  }

  Float get_radius(Particle *a) const {
    if (rk_ != FloatKey() && a->has_attribute(rk_)) {
      return a->get_value(rk_);
    } else {
      return 0;
    }
  }

public:
  /**
   */
  NonbondedListScoreState(FloatKey rk);

  FloatKey get_radius_key() const {return rk_;}
  void set_radius_key(FloatKey rk) {rk_=rk;} 

  IMP_CONTAINER(BondedListScoreState, bonded_list,
                BondedListIndex);
  // kind of evil hack to make the names better
  // perhaps the macro should be made more flexible
  typedef BondedListScoreStateIterator BondedListIterator;
  typedef BondedListScoreStateConstIterator BondedListConstIterator;

  IMP_SCORE_STATE(internal::kernel_version_info)

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
