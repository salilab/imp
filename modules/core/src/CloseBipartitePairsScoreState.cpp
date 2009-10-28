/**
 *  \file CloseBipartitePairsScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */


#include <IMP/core/CloseBipartitePairsScoreState.h>


IMPCORE_BEGIN_NAMESPACE


CloseBipartitePairsScoreState
::CloseBipartitePairsScoreState(SingletonContainer *a,
                                SingletonContainer *b):
  ScoreState("CloseBipartitePairsScoreState %1%"),
  a_(a), b_(b)
{
}

void CloseBipartitePairsScoreState::set_distance(Float d) {
  IMP_USAGE_CHECK(!out_, "Can only set the slack before adding"
                  << " to the model.", ValueException);
  distance_=d;
}

void CloseBipartitePairsScoreState::set_slack(Float d) {
  IMP_USAGE_CHECK(!out_, "Can only set the slack before adding"
                  << " to the model.", ValueException);
  slack_=d;
}

void CloseBipartitePairsScoreState
::set_close_pairs_finder(ClosePairsFinder *f) {
  IMP_USAGE_CHECK(!out_,
                  "Can only set the close pairs finder before adding"
                  << " to the model.", ValueException);
  f_=f;
}

void CloseBipartitePairsScoreState::do_before_evaluate()
{
}
void CloseBipartitePairsScoreState
::do_after_evaluate(DerivativeAccumulator*){
}

ParticlesList CloseBipartitePairsScoreState
::get_interacting_particles() const {
  return ParticlesList();
}


ObjectsTemp CloseBipartitePairsScoreState::get_input_objects() const {
  return ObjectsTemp();
}

ObjectsTemp CloseBipartitePairsScoreState::get_output_objects() const {
  return ObjectsTemp();
}

ParticlesTemp CloseBipartitePairsScoreState::get_input_particles() const {
  return ParticlesTemp();
}

ParticlesTemp CloseBipartitePairsScoreState::get_output_particles() const {
  return ParticlesTemp();
}


void CloseBipartitePairsScoreState::show(std::ostream &out) const
{
  out << "CloseBipartitePairsScoreState" << std::endl;
}



void CloseBipartitePairsScoreState::set_model(Model *m) {
  ScoreState::set_model(m);
  if (!f_) {
    out_= new CloseBipartitePairContainer(a_,
                                               b_,
                                               m, distance_, slack_);
  } else {
    out_= new CloseBipartitePairContainer(a_,
                                               b_,
                                               m, distance_, f_, slack_);
  }
}


IMPCORE_END_NAMESPACE
