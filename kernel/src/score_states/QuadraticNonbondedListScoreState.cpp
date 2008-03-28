/**
 *  \file QuadraticNonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of s.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/QuadraticNonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"

#include <algorithm>

namespace IMP
{

QuadraticNonbondedListScoreState
::QuadraticNonbondedListScoreState(FloatKey radius):
  P(radius), slack_(.1)
{
  mc_= new MaxChangeScoreState(XYZDecorator::get_xyz_keys());
}

QuadraticNonbondedListScoreState::~QuadraticNonbondedListScoreState()
{
}


void QuadraticNonbondedListScoreState::update()
{
  // placeholder to do tuning of slack
  NonbondedListScoreState::update();
  if (mc_->get_max() > slack_) {
    NonbondedListScoreState::clear_nbl();    
    mc_->reset();
  }
}

void QuadraticNonbondedListScoreState
::handle_nbl_pair( Particle *a,  Particle *b,
                   float d)
{
  XYZDecorator da= XYZDecorator::cast(a);
  XYZDecorator db= XYZDecorator::cast(b);
  float ra= P::get_radius(a);
  float rb= P::get_radius(b);
  for (unsigned int i=0; i< 3; ++i) {
    float delta=std::abs(da.get_coordinate(i) - db.get_coordinate(i));
    if (delta -ra -rb > d-slack_) {
      IMP_LOG(VERBOSE, "Pair " << a->get_index()
              << " and " << b->get_index() << " rejected on coordinate "
              << i << std::endl);
      return ;
    }
  }
  IMP_LOG(VERBOSE, "Adding pair " << a->get_index()
            << " and " << b->get_index() << std::endl);
  P::add_to_nbl(a, b); 
}

} // namespace IMP
