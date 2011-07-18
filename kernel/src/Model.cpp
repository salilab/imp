/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/RestraintSet.h"
#include "IMP/dependency_graph.h"
#include "IMP/compatibility/set.h"


IMP_BEGIN_NAMESPACE


//! Constructor
Model::Model(std::string name): Object(name),
                                rs_(new RestraintSet())
{
  cur_stage_=internal::NOT_EVALUATING;
  gather_statistics_=false;
  eval_count_=0;
  set_was_used(true);
  rs_->set_model(this);
  first_call_=true;
  max_score_ =std::numeric_limits<double>::max();
  has_good_score_=false;

  next_particle_=0;
}


//! Destructor
Model::~Model()
{
  IMP_CHECK_OBJECT(this);
  rs_->set_model(NULL);
  for (unsigned int i=0; i< particle_index_.size(); ++i) {
    if (particle_index_[i]) {
      Object* op=particle_index_[i];
      dynamic_cast<Particle*>(op)->m_=NULL;
    }
  }
}

void Model::set_maximum_score(double d) {
  max_score_=d;
  reset_dependencies();
}

void Model::add_restraint(Restraint *r) {
  IMP_USAGE_CHECK(r, "Cannot add null restraint.");
  rs_->add_restraint(r);
}
void Model::remove_restraint(Restraint *r) {
  IMP_USAGE_CHECK(r, "Cannot remove null restraint.");
  rs_->remove_restraint(r);
}
Model::RestraintIterator Model::restraints_begin() {
  return rs_->restraints_begin();
}
Model::RestraintIterator Model::restraints_end() {
  return rs_->restraints_end();
}
Model::RestraintConstIterator Model::restraints_begin() const {
  return rs_->restraints_begin();
}
Model::RestraintConstIterator Model::restraints_end() const {
  return rs_->restraints_end();
}

IMP_LIST_IMPL(Model, ScoreState, score_state, ScoreState*,
              ScoreStates,
              {IMP_INTERNAL_CHECK(cur_stage_== internal::NOT_EVALUATING,
                     "The set of score states cannot be changed during"
                                  << "evaluation.");
                Model::set_score_state_model(obj, this);
                obj->set_was_used(true);
                IMP_LOG(VERBOSE, "Added score state " << obj->get_name()
                        << std::endl);
                IMP_IF_CHECK(USAGE) {
                  compatibility::set<ScoreState*> in(score_states_begin(),
                                           score_states_end());
                  IMP_USAGE_CHECK(in.size() == get_number_of_score_states(),
                                  "Score state already in model "
                                  << obj->get_name());
                }
              },{reset_dependencies();},
              {Model::set_score_state_model(obj, NULL);
               if(container) container->reset_dependencies(); });

void Model::set_score_state_model(ScoreState *ss, Model *model) {
  IMP_CHECK_OBJECT(ss);
  if (model) {
    IMP_CHECK_OBJECT(model);
  }
  ss->set_model(model);
}


double Model::get_weight(Restraint *r) const {
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  if (restraint_index_.find(r) == restraint_index_.end()) return 0;
  else return restraint_weights_[restraint_index_.find(r)->second];
}


ParticlesTemp Model::get_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< particle_index_.size(); ++i) {
    if (particle_index_[i]) {
      ret.push_back(dynamic_cast<Particle*>(particle_index_[i]));
    }
  }
  return ret;
}


void Model::add_particle_internal(Particle *p) {
    IMP_CHECK_OBJECT(this);
    IMP_CHECK_OBJECT(p);
    p->set_was_used(true);
    int id;
    if (free_particles_.empty()){
      id= next_particle_;
      ++next_particle_;
    } else {
      id= free_particles_.back();
      free_particles_.pop_back();
    }
    p->id_=id;
    int maxp= std::max(particle_index_.size(),
                       static_cast<size_t>(p->id_+1));
    particle_index_.resize(maxp);
    particle_index_[p->id_]=p;
  }



void Model::update() {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  Floats ret= do_evaluate(RestraintsTemp(),
                          Floats(),
                          ordered_score_states_,
                          false, true, false);
}

void Model::do_show(std::ostream& out) const
{
  out << get_particles().size() << " particles" << std::endl;
  out << get_number_of_restraints() << " restraints" << std::endl;
  out << get_number_of_score_states() << " score states" << std::endl;

  out << std::endl;
  IMP_CHECK_OBJECT(this);
}

void Model::remove_particle(Particle *p) {
  int pi= p->get_index();
  free_particles_.push_back(pi);
  particle_index_[pi]=NULL;
  p->m_=NULL;
  FloatAttributeTable::clear_attributes(pi);
  StringAttributeTable::clear_attributes(pi);
  IntAttributeTable::clear_attributes(pi);
  ObjectAttributeTable::clear_attributes(pi);
  IntsAttributeTable::clear_attributes(pi);
  ObjectsAttributeTable::clear_attributes(pi);
  ParticleAttributeTable::clear_attributes(pi);
  ParticlesAttributeTable::clear_attributes(pi);
}


bool Model::get_has_good_score() const {
  return has_good_score_;
}

IMP_END_NAMESPACE
