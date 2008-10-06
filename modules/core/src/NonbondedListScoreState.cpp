/**
 *  \file NonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/core/NonbondedListScoreState.h>
#include <IMP/core/MaxChangeScoreState.h>
#include <IMP/core/internal/Grid3D.h>
#include <IMP/core/XYZDecorator.h>

#include <algorithm>

IMPCORE_BEGIN_NAMESPACE

namespace internal
{

struct HasInactive
{
  bool operator()(ParticlePair pp) const {
    return !pp.first->get_is_active() || !pp.second->get_is_active();
  }
};

} // namespace internal




NonbondedListScoreState
::NonbondedListScoreState(Float cut,
                          FloatKey rk): rk_(rk),
                                        cutoff_(cut),
                                        nbl_is_valid_(false)
{
  slack_=cutoff_;
  number_of_updates_=1;
  number_of_rebuilds_=0;
  number_of_overflows_=0;
  max_nbl_size_= std::numeric_limits<unsigned int>::max();
}




NonbondedListScoreState::~NonbondedListScoreState()
{
}

void NonbondedListScoreState::show_statistics(std::ostream &out) const
{
  out << "Nonbonded list averaged "
      << static_cast<Float>(number_of_updates_)
      / number_of_rebuilds_ << " steps between rebuilds"
      << " and overflowed " <<  number_of_overflows_
      << " times" << std::endl;
}



Particles NonbondedListScoreState
::particles_with_radius(const Particles &in) const
{
  Particles ret;
  if (rk_== FloatKey()) return ret;
  ret.reserve(in.size());
  for (unsigned int i=0; i< in.size(); ++i) {
    if (in[i]->has_attribute(rk_)) {
      ret.push_back(in[i]);
    }
  }
  return ret;
}


/** Want to minimize rebuild_cost*rebuild_frequence+ 2*nbl_size.

    To do this assume 0 slack has nbl of zero. So the nbl_size
    goes as (rate*time_steps)^3= current_size, so
    rate= current_size^(1/3)/time_steps

    rebuild_cost/time_steps + 2*(rate*timesteps)^3

    Min is at 3^(-1/3) (cost)^(1/4)*r^(-3/4);

    Then the slack should be something or another XXXXX
 */
bool NonbondedListScoreState::update(Float mc, Float rebuild_cost)
{
  ++number_of_updates_;
  IMP_LOG(VERBOSE, "Updating non-bonded list" << std::endl);
  for (BondedListScoreStateIterator bli= bonded_lists_begin();
       bli != bonded_lists_end(); ++bli) {
    (*bli)->before_evaluate(ScoreState::get_before_evaluate_iteration());
  }

  bool rebuilt=false;
  if (mc > slack_ || !get_nbl_is_valid()) {
    unsigned int rebuild_attempts=0;
    do {
      try {
        ++rebuild_attempts;
        ++number_of_rebuilds_;
        set_nbl_is_valid(false);
        rebuild_nbl();
        rebuilt=true;
      } catch (NBLTooLargeException &) {
        slack_= slack_/2.0;
        ++number_of_overflows_;
        if (number_of_rebuilds_==100) {
          IMP_WARN("Can't rebuild NBL with given max NBL size of "
                   << max_nbl_size_ << std::endl);
          throw ValueException("Bad NBL max size");
        }
      }
    } while (!rebuilt);
  } else {
    nbl_.erase(std::remove_if(nbl_.begin(), nbl_.end(),
                              internal::HasInactive()),
               nbl_.end());
  }
  return rebuilt;
}

void NonbondedListScoreState::show(std::ostream &out) const
{
  out << "NonbondedList" << std::endl;
}

void NonbondedListScoreState::set_nbl_is_valid(bool tf)
{
  nbl_is_valid_= tf;
  if (!nbl_is_valid_) {
    ParticlePairs empty;
    // free memory to make sure it shrinks
    std::swap(empty, nbl_);
  }
}

IMP_CONTAINER_IMPL(NonbondedListScoreState, BondedListScoreState,
                   bonded_list, BondedListIndex, {
                     if (0) std::cout <<*obj;
                   },,);

IMPCORE_END_NAMESPACE
