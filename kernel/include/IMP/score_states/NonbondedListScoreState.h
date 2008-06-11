/**
 *  \file NonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_NONBONDED_LIST_SCORE_STATE_H

#include "../ScoreState.h"
#include "../internal/kernel_version_info.h"
#include "../decorators/XYZDecorator.h"
#include "BondedListScoreState.h"

#include <boost/iterator/filter_iterator.hpp>

#include <vector>
#include <limits>

namespace IMP
{

namespace internal
{
class NBLAddPairIfNonbonded;
class NBLAddIfNonbonded;
}

typedef std::vector<BondedListScoreState*> BondedListScoreStates;

//! An abstract base class for classes that maintain a list of non-bonded pairs.
/** \note If no value for the radius key is specified, all radii are
    considered to be zero.
 */
class IMPDLLEXPORT NonbondedListScoreState: public ScoreState
{
private:
  FloatKey rk_;
  //! the distance cutoff to count a pair as adjacent
  float cutoff_;
  /** How much to add to the size of particles to allow particles to move
      without rebuilding the list */
  Float slack_;
  bool nbl_is_valid_;
  int number_of_rebuilds_;
  int number_of_updates_;
  int number_of_overflows_;
  //! The maximum allowable size for the NBL
  /** An exception will be thrown if the list exceeds this size.
   */
  unsigned int max_nbl_size_;
  ParticlePairs nbl_;

  struct NBLTooLargeException{};

protected:

  Float get_slack() const {return slack_;}
  Float get_cutoff() const {return cutoff_;}

  unsigned int size_nbl() const {return nbl_.size();}

  //! rebuild the nonbonded list
  /** This is used by classes which inherit from this to rebuild the NBL

      \internal
   */
  virtual void rebuild_nbl()=0;



  //! Return true if two particles are bonded
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
  friend struct internal::NBLAddPairIfNonbonded;
  friend struct internal::NBLAddIfNonbonded;



  //! Add the pair to the nbl if the particles are not bonded
  void add_if_nonbonded(Particle *a, Particle *b) {
    IMP_assert(a->get_is_active() && b->get_is_active(),
               "Inactive particles should have been stripped");

    if (!are_bonded(a,b)) {
      IMP_LOG(VERBOSE, "Found pair " << a->get_index() 
        << " " << b->get_index() << std::endl);
      if (nbl_.size() <  max_nbl_size_) {
        nbl_.push_back(std::make_pair(a, b));
      } else {
        throw NBLTooLargeException();
      }
    } else {
      IMP_LOG(VERBOSE, "Pair " << a->get_index()
              << " and " << b->get_index() << " rejected on bond" 
              <<std::endl);
    }
  }

  struct GetRadius
  {
    FloatKey rk_;
    GetRadius(FloatKey rk): rk_(rk){
    }
    GetRadius(){}
    Float operator() (Particle *a) const {
      if (rk_ != FloatKey() && a->has_attribute(rk_)) {
        return a->get_value(rk_);
      } else {
        return 0;
      }
    }
  };

  //! Check if the bounding boxes of the spheres overlap
  void add_if_box_overlap(Particle *a, Particle *b) {
    BoxesOverlap bo= boxes_overlap_object(slack_+ cutoff_);
    if (!bo(a, b)) {
      IMP_LOG(VERBOSE, "Pair " << a->get_index()
              << " and " << b->get_index() << " rejected on coordinate "
              << std::endl);
    }
    IMP_LOG(VERBOSE, "Adding pair " << a->get_index()
            << " and " << b->get_index() << std::endl);
    add_if_nonbonded(a, b); 
  }

  GetRadius get_radius_object() const {
    return GetRadius(rk_);
  }

  //! \return true if the nbl was invalidated by a move of mc
  /** rebuild_cost is an estimate of the rebuilding cost used
      to try to get the slack right. The units are that of
      evaluating a distance
   */
  bool update(Float mc, Float rebuild_cost);

  //! \return a list of all the particles in in with a radius field
  Particles particles_with_radius(const Particles &in) const;


  //! \return true if the nbl is valid
  bool get_nbl_is_valid() const {return nbl_is_valid_;}

  void set_nbl_is_valid(bool tf);

  struct BoxesOverlap
  {
    GetRadius gr_;
    Float slack_;
    BoxesOverlap(GetRadius rk, Float sl): gr_(rk), slack_(sl){}
    BoxesOverlap(){}
    bool operator()(Particle *a, Particle *b) const {
      XYZDecorator da(a);
      XYZDecorator db(b);
      float ra= gr_(a);
      float rb= gr_(b);
      for (unsigned int i=0; i< 3; ++i) {
        float delta=std::abs(da.get_coordinate(i) - db.get_coordinate(i));
        if (delta -ra -rb > slack_) {
          return false;
        }
      }
      return true;
    }

    bool operator()(std::pair<Particle *, Particle *> p) const {
      return operator()(p.first, p.second);
    }
  };

  BoxesOverlap boxes_overlap_object(float cut) const {
    return BoxesOverlap(get_radius_object(), cut);
  }
public:
  NonbondedListScoreState(Float cutoff, FloatKey rk);
  ~NonbondedListScoreState();

  FloatKey get_radius_key() const {return rk_;}
  void set_radius_key(FloatKey rk) {rk_=rk;} 

  //! Set the maximum allowable size for the NBL
  /** The NBL will keep reducing the slack and trying to
      rebuild until it can make the list smaller than this.
   */
  void set_max_size(unsigned int mx) {
    max_nbl_size_= mx;
  }


  //! Set the slack used when generating the NBL
  /** The slack allows the the NBL to non be rebuilt every step
      making the process more efficient. However, too large
      a value can result in the NBL being excessively large.

      A good guideline is that it should be the maximum amount
      a particle coordinate would change in 20 steps or so.
   */
  void set_slack(float slack) {
    IMP_check(slack>= 0, "Slack must be nonnegative",
              ValueException);
    slack_=slack;
  }

  IMP_CONTAINER(BondedListScoreState, bonded_list,
                BondedListIndex);

  // kind of evil hack to make the names better
  // perhaps the macro should be made more flexible
  typedef BondedListScoreStateIterator BondedListIterator;
  typedef BondedListScoreStateConstIterator BondedListConstIterator;

  virtual void show(std::ostream &out=std::cout) const;
  virtual IMP::VersionInfo get_version_info() const {
    return internal::kernel_version_info;
  }

  //! An iterator through nonbonded particles
  /** The value type is an ParticlePair. 
   */
  typedef boost::filter_iterator<BoxesOverlap,
    ParticlePairs::const_iterator> NonbondedIterator;

  //! Iterates through the pairs of non-bonded particles
  NonbondedIterator nonbonded_begin() const {
    IMP_check(get_nbl_is_valid(), "Must call update first",
              ValueException);
    return NonbondedIterator(boxes_overlap_object(cutoff_),
                             nbl_.begin(), nbl_.end());
  }
  NonbondedIterator nonbonded_end() const {
    IMP_check(get_nbl_is_valid(), "Must call update first",
              ValueException);
    return NonbondedIterator(boxes_overlap_object(cutoff_),
                             nbl_.end(), nbl_.end());
  }

  const ParticlePairs get_nonbonded() const {
    return nbl_;
  }

  unsigned int number_of_nonbonded() const {
    IMP_check(get_nbl_is_valid(), "Must call update first",
              ValueException);
    return nbl_.size();
  }

  void show_statistics(std::ostream &out=std::cout) const;

};


namespace internal
{

struct NBLAddPairIfNonbonded
{
  NonbondedListScoreState *state_;
  NBLAddPairIfNonbonded(NonbondedListScoreState *s): state_(s){}
  void operator()(Particle *a, Particle *b) {
    state_->add_if_nonbonded(a,b);
  }
};

struct NBLAddIfNonbonded
{
  NonbondedListScoreState *state_;
  Particle *p_;
  NBLAddIfNonbonded(NonbondedListScoreState *s, Particle *p): state_(s),
                                                           p_(p){}
  void operator()(Particle *p) {
    state_->add_if_nonbonded(p_, p);
  }
};

} // namespace internal

} // namespace IMP

#endif  /* __IMP_NONBONDED_LIST_SCORE_STATE_H */
