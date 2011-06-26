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
  cur_stage_=NOT_EVALUATING;
  gather_statistics_=false;
  eval_count_=0;
  set_was_used(true);
  rs_->set_model(this);
  first_call_=true;
  max_score_ =std::numeric_limits<double>::max();
  has_good_score_=false;
}


//! Destructor
Model::~Model()
{
  IMP_CHECK_OBJECT(this);
  rs_->set_model(NULL);
  for (ParticleIterator it= particles_begin();
       it != particles_end(); ++it) {
    (*it)->m_ = NULL;
    internal::unref(*it);
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
              {IMP_INTERNAL_CHECK(cur_stage_== NOT_EVALUATING,
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


FloatRange Model::get_range(FloatKey k) const {
  IMP_CHECK_OBJECT(this);
  if (ranges_.find(k) != ranges_.end()) {
    return ranges_.find(k)->second;
  } else {
    FloatRange r(std::numeric_limits<Float>::max(),
                 -std::numeric_limits<Float>::max());
    for (ParticleConstIterator it= particles_begin();
         it != particles_end(); ++it) {
      if ((*it)->has_attribute(k)) {
        Float v= (*it)->get_value(k);
        r.first = std::min(r.first, v);
        r.second= std::max(r.second, v);
      }
    }
    return r;
  }
}

double Model::get_weight(Restraint *r) const {
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  if (restraint_index_.find(r) == restraint_index_.end()) return 0;
  else return restraint_weights_[restraint_index_.find(r)->second];
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
                          false);
}

void Model::do_show(std::ostream& out) const
{
  out << get_number_of_particles() << " particles" << std::endl;
  out << get_number_of_restraints() << " restraints" << std::endl;
  out << get_number_of_score_states() << " score states" << std::endl;

  out << std::endl;
  IMP_CHECK_OBJECT(this);
}



bool Model::get_has_good_score() const {
  return has_good_score_;
}


IMP_END_NAMESPACE
