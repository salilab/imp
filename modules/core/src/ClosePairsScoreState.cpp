/**
 *  \file ClosePairsScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */


#include <IMP/core/ClosePairsScoreState.h>


IMPCORE_BEGIN_NAMESPACE



ClosePairsScoreState::ClosePairsScoreState(SingletonContainer *pc):
  ScoreState("ClosePairsScoreState %1%"),
  in_(pc)
{
}

void ClosePairsScoreState::set_distance(Float d) {
  IMP_USAGE_CHECK(!out_, "Can only set the slack before adding"
                  << " to the model.", ValueException);
  distance_=d;
}

void ClosePairsScoreState::set_slack(Float d) {
  IMP_USAGE_CHECK(!out_, "Can only set the slack before adding"
                  << " to the model.", ValueException);
  slack_=d;
}

void ClosePairsScoreState::set_close_pairs_finder(ClosePairsFinder *f) {
  IMP_USAGE_CHECK(!out_,
                  "Can only set the close pairs finder before adding"
                  << " to the model.", ValueException);
  f_=f;
}

void ClosePairsScoreState::do_before_evaluate()
{
}
void ClosePairsScoreState::do_after_evaluate(DerivativeAccumulator*){
}

ParticlesList ClosePairsScoreState::get_interacting_particles() const {
  return ParticlesList();
}


ObjectsTemp ClosePairsScoreState::get_input_objects() const {
  return ObjectsTemp();
}

ObjectsTemp ClosePairsScoreState::get_output_objects() const {
  return ObjectsTemp();
}

ParticlesTemp ClosePairsScoreState::get_input_particles() const {
  return ParticlesTemp();
}

ParticlesTemp ClosePairsScoreState::get_output_particles() const {
  return ParticlesTemp();
}


void ClosePairsScoreState::show(std::ostream &out) const
{
  out << "ClosePairsScoreState" << std::endl;
}



void ClosePairsScoreState::set_model(Model *m) {
  ScoreState::set_model(m);
  if (!f_) {
    out_= new ClosePairContainer(in_, m, distance_, slack_);
  } else {
    out_= new ClosePairContainer(in_, m, distance_, f_, slack_);
  }
}


IMPCORE_END_NAMESPACE
