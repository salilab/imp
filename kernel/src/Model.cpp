/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/RestraintSet.h"



IMP_BEGIN_NAMESPACE


//! Constructor
Model::Model(std::string name): Object(name)
{
  cur_stage_=NOT_EVALUATING;
  incremental_update_=false;
  first_incremental_=true;
  gather_statistics_=false;
  eval_count_=0;
  set_was_used(true);
}


//! Destructor
Model::~Model()
{
  IMP_CHECK_OBJECT(this);
  for (ParticleIterator it= particles_begin();
       it != particles_end(); ++it) {
    (*it)->m_ = NULL;
    internal::unref(*it);
  }
}


IMP_LIST_IMPL(Model, Restraint, restraint, Restraint*,
              Restraints,
              {IMP_INTERNAL_CHECK(cur_stage_== NOT_EVALUATING,
          "The set of restraints cannot be changed during evaluation.");
                obj->set_model(this);
                obj->set_was_used(true);
                first_incremental_=true;}, reset_dependencies();,
              {
                obj->set_model(NULL);
                if(container) container->reset_dependencies();
              });

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
                  std::set<ScoreState*> in(score_states_begin(),
                                           score_states_end());
                  IMP_USAGE_CHECK(in.size() == get_number_of_score_states(),
                                  "Score state already in model "
                                  << obj->get_name());
                }
              },{reset_dependencies();},
              {Model::set_score_state_model(obj, NULL);
               if(container) container->reset_dependencies(); });

void Model::set_score_state_model(ScoreState *ss, Model *model) {
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
  return restraint_weights_[restraint_index_[r]];
}




double Model::evaluate(bool calc_derivs) {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  return do_evaluate(ordered_restraints_,
                     restraint_weights_,
                     ordered_score_states_,
                     calc_derivs);
}

double Model::evaluate(const RestraintsTemp &inrestraints, bool calc_derivs)
{
  IMP_CHECK_OBJECT(this);
  IMP_OBJECT_LOG;
  RestraintsTemp restraints;
  std::vector<double> weights;
  boost::tie(restraints, weights)=
    get_restraints(inrestraints.begin(), inrestraints.end());
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< restraints.size(); ++i) {
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(restraints[i]),
                      "Cannot pass restraint sets to model to evaluate");
      IMP_USAGE_CHECK(restraint_index_.find(restraints[i])
                      != restraint_index_.end(),
                      "You must add restraints to model before "
                      << "asking it to evaluate them");
    }
  }
  boost::dynamic_bitset<> bs(ordered_score_states_.size(), false);
  for (unsigned int i=0; i< restraints.size(); ++i) {
    int index=restraint_index_[restraints[i]];
    bs|= restraint_dependencies_[index];
  }
  ScoreStatesTemp ss;
  for (unsigned int i=0; i< ordered_score_states_.size(); ++i) {
    if (bs[i]) ss.push_back(ordered_score_states_[i]);
  }
  return do_evaluate(restraints, weights, ss, calc_derivs);
}



void Model::update() {
  /*SetIt<Stage, NOT_EVALUATING> reset(&cur_stage_);
  if (!score_states_ordered_) {
    order_score_states();
  } else {
    ScoreStatesTemp st(score_states_begin(), score_states_end());
    before_evaluate(st);
  }
  ++eval_count_;*/
  evaluate(false);
}

void Model::set_is_incremental(bool tf) {
  DerivativeAccumulator da;
  if (tf && !get_is_incremental()) {
    first_incremental_=true;
    for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
      (*it)->setup_incremental();
    }
  } else if (!tf && get_is_incremental()) {
    for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
      (*it)->teardown_incremental();
    }
  }
  incremental_update_=tf;
}

void Model::do_show(std::ostream& out) const
{
  out << get_number_of_particles() << " particles" << std::endl;
  out << get_number_of_restraints() << " restraints" << std::endl;
  out << get_number_of_score_states() << " score states" << std::endl;

  out << std::endl;
  IMP_CHECK_OBJECT(this);
}



IMP_END_NAMESPACE
