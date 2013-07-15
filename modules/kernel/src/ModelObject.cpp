/**
 *  \file ModelObject.cpp  \brief Single variable function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/kernel/ModelObject.h>
#include <IMP/kernel/container_base.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/kernel/container_base.h>
// should move to own .cpp
#include <IMP/kernel/input_output.h>
#include <boost/foreach.hpp>
#include <algorithm>

IMPKERNEL_BEGIN_NAMESPACE

ModelObject::ModelObject(Model *m, std::string name)
  : Tracked(this, m, name),
    has_required_score_states_(false),
    computing_required_score_states_(false) {
  m->do_set_has_all_dependencies(false);
}

bool ModelObject::get_has_dependencies() const {
  IMP_USAGE_CHECK(get_model(), "No model set yet.");
  return get_model()->do_get_has_dependencies(this);
}

void ModelObject::set_has_dependencies(bool tf) {
  IMP_USAGE_CHECK(get_model(), "Model not set");
  get_model()->do_set_has_dependencies(this, tf);
  get_model()->check_dependency_invariants();
}

void ModelObject::do_set_has_required_score_states(bool tf) {
  IMP_USAGE_CHECK(!computing_required_score_states_,
                "Already computing score states, probably a dependency loop.");

  if (tf) {
    // make sure everything is there as we invalidate lazily
    get_model()->do_set_has_all_dependencies(true);
  }
  if (!tf && !has_required_score_states_) return;
  else if (tf && get_has_required_score_states()) return;
  computing_required_score_states_ = true;
  IMP_OBJECT_LOG;
  if (tf) {
    has_required_score_states_ = true;
    ScoreStates all;
    BOOST_FOREACH(ModelObject *input,
                  get_model()->get_dependency_graph_inputs(this)) {
      input->do_set_has_required_score_states(true);
      all += input->get_required_score_states();
      ScoreState *ss = dynamic_cast<ScoreState*>(input);
      if (ss) all.push_back(ss);
    }
    required_score_states_ = get_update_order(all);
    IMP_LOG_VERBOSE("Score states for " << get_name()
                    << " are " << required_score_states_ << std::endl);
    handle_set_has_required_score_states(true);
  } else {
    has_required_score_states_ = false;
    handle_set_has_required_score_states(false);
  }
  computing_required_score_states_ = false;
}

void ModelObject::set_has_required_score_states(bool tf) {
  IMP_USAGE_CHECK(tf, "Can only set them this way.");
  do_set_has_required_score_states(tf);
  if (get_model()) get_model()->check_dependency_invariants();
}

void ModelObject::validate_inputs() const {
  if (!get_has_dependencies()) return;
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    ModelObjectsTemp ret = do_get_inputs();
    std::sort(ret.begin(), ret.end());
    ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
    // since I/O nodes are considered outputs
    ModelObjectsTemp saved = get_model()->get_dependency_graph_inputs(this)
      + get_model()->get_dependency_graph_outputs(this);
    std::sort(saved.begin(), saved.end());
    ModelObjectsTemp intersection;
    std::set_intersection(saved.begin(), saved.end(),
                          ret.begin(), ret.end(),
                          std::back_inserter(intersection));
    IMP_USAGE_CHECK(intersection.size() == ret.size(),
                    "Dependencies changed without invalidating dependencies."
                    << " Make sure you call set_has_dependencies(false) any "
                    << "time the list of dependencies changed. Object is "
                    << get_name() << " of type " << get_type_name());
  }
}

void ModelObject::validate_outputs() const {
  if (!get_has_dependencies()) return;
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    ModelObjectsTemp ret = do_get_outputs();
    std::sort(ret.begin(), ret.end());
    ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
    ModelObjectsTemp saved = get_model()->get_dependency_graph_outputs(this);
    std::sort(saved.begin(), saved.end());
    ModelObjectsTemp intersection;
    std::set_intersection(saved.begin(), saved.end(),
                          ret.begin(), ret.end(),
                          std::back_inserter(intersection));
    IMP_USAGE_CHECK(intersection.size() == ret.size(),
                    "Dependencies changed without invalidating dependencies."
                    << " Make sure you call set_has_dependencies(false) any "
                    << "time the list of dependencies changed. Object is "
                    << get_name() << " of type " << get_type_name());
  }
}

ModelObjectsTemp ModelObject::get_inputs() const {
  IMP_OBJECT_LOG;
  validate_inputs();
  return do_get_inputs();
}

ModelObjectsTemp ModelObject::get_outputs() const {
  IMP_OBJECT_LOG;
  validate_outputs();
  return do_get_outputs();
}

ModelObjectsTemps ModelObject::do_get_interactions() const {
  return ModelObjectsTemps(1, get_inputs() + get_outputs());
}

ModelObjectsTemps ModelObject::get_interactions() const {
  return do_get_interactions();
}

///////////////////////////////////////// DEPRECATED

ParticlesTemp get_input_particles(const ModelObjectsTemp &mo) {
  ParticlesTemp ret;
  for (unsigned int i = 0; i < mo.size(); ++i) {
    ModelObject *o = mo[i];
    Particle *p = dynamic_cast<Particle *>(o);
    if (p)
      ret.push_back(p);
    else {
      ret += get_input_particles(o->get_inputs());
    }
  }
  return ret;
}

ContainersTemp get_input_containers(const ModelObjectsTemp &mo) {
  ContainersTemp ret;
  for (unsigned int i = 0; i < mo.size(); ++i) {
    ModelObject *o = mo[i];
    Container *p = dynamic_cast<Container *>(o);
    if (p)
      ret.push_back(p);
    else {
      ret += get_input_containers(o->get_inputs());
    }
  }
  return ret;
}

ParticlesTemp get_output_particles(const ModelObjectsTemp &mo) {
  ParticlesTemp ret;
  for (unsigned int i = 0; i < mo.size(); ++i) {
    ModelObject *o = mo[i];
    Particle *p = dynamic_cast<Particle *>(o);
    if (p)
      ret.push_back(p);
    else {
      ret += get_output_particles(o->get_inputs());
    }
  }
  return ret;
}

ContainersTemp get_output_containers(const ModelObjectsTemp &mo) {
  ContainersTemp ret;
  for (unsigned int i = 0; i < mo.size(); ++i) {
    ModelObject *o = mo[i];
    Container *p = dynamic_cast<Container *>(o);
    if (p)
      ret.push_back(p);
    else {
      ret += get_output_containers(o->get_inputs());
    }
  }
  return ret;
}

bool ModelObject::get_is_part_of_model() const {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1,
                                    "Should always be true.");
  return Tracked::get_is_tracked();
}

void ModelObject::set_model(Model *m) {
  Tracked::set_tracker(this, m);
  do_set_model(m);
}

ModelObject::ModelObject(std::string name)
  : Tracked(name),
    has_required_score_states_(false),
    computing_required_score_states_(false) {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1,
                                    "Pass the Model to the constructor.");

}


IMPKERNEL_END_NAMESPACE
