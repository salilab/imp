/**
 *  \file NonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/NonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/internal/Grid3D.h"
#include "IMP/score_states/MaxChangeScoreState.h"

#include <algorithm>

namespace IMP
{


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
  slack_=20;
  next_slack_=slack_;
  num_steps_=1;
  number_of_updates_=1;
  number_of_rebuilds_=0;
}




NonbondedListScoreState::~NonbondedListScoreState()
{
}

void NonbondedListScoreState::show_statistics(std::ostream &out) const
{
  out << "Nonbonded list averaged " 
      << static_cast<Float>(number_of_updates_)
      / number_of_rebuilds_ << " steps between rebuilds" << std::endl;
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

  if (nbl_is_valid_) {
    /*std::cout << "Rate is " << rate << " target is " << target_steps
              << " so slack is " << target_slack << " mc " << mc
              << " nbl " << nbl_.size() << " cost " 
              << rebuild_cost << std::endl;*/
    if (mc > slack_) {
      /*    Float rate= std::pow(static_cast<Float>(nbl_.size()),
            .333f)/ num_steps_;
            Float target_steps= .6*std::pow(rebuild_cost, .25f)
            *std::pow(rate, -.75f);
            Float target_slack= (target_steps+1)*mc/num_steps_;
            next_slack_= target_slack*.5 + .5*next_slack_;
      */

      /*std::cout << "Killing nbl because " << mc << " 
        << slack_ << " " << next_slack_ 
        << " " << num_steps_ << std::endl;*/
      if (num_steps_ < 50) {
        //slack_= next_slack_;
      }
      num_steps_=1;
      //next_slack_= std::max(2.0*mc, 2.0*slack_);
      set_nbl_is_valid(false);
      ++number_of_rebuilds_;
      slack_=next_slack_;
    /*} else if (num_steps_ > 100) {
      //next_slack_=slack_/2.0;
      slack_=next_slack_;
      num_steps_=1;
      set_nbl_is_valid(false);
      ++number_of_rebuilds_;*/
    } else {
      ++num_steps_;
      //next_slack_= next_slack_*.98;
    }
  }

  bool rebuilt=false;
  if (!get_nbl_is_valid()) {
    rebuild_nbl();
    rebuilt=true;
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
    NBL empty;
    // free memory to make sure it shrinks
    std::swap(empty, nbl_);
  }
}

IMP_CONTAINER_IMPL(NonbondedListScoreState, BondedListScoreState,
                   bonded_list, BondedListIndex, {
                     if (0) std::cout <<*obj;
                   },,);

} // namespace IMP
