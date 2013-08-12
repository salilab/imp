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
  : base::Object(name), model_(m) {
  m->do_add_model_object(this);
}

ModelObject::~ModelObject() {
  if (get_model()) get_model()->do_remove_model_object(this);
}

bool ModelObject::get_has_dependencies() const {
  IMP_USAGE_CHECK(get_model(), "No model set yet.");
  return get_model()->do_get_has_dependencies(this);
}

void ModelObject::set_has_dependencies(bool tf) {
  IMP_USAGE_CHECK(get_model(), "Model not set");
  get_model()->do_set_has_dependencies(this, tf);
  //get_model()->check_dependency_invariants(this);
}

bool ModelObject::get_has_required_score_states() const {
  IMP_USAGE_CHECK(get_model(), "Must set model first");
  return get_model() && get_model()->do_get_has_required_score_states(this);
}

void ModelObject::set_has_required_score_states(bool tf) {
  IMP_USAGE_CHECK(tf, "Can only set them this way.");
  IMP_USAGE_CHECK(get_model(), "Must set model first");
  get_model()->do_set_has_required_score_states(this, true);
  //if (get_model()) get_model()->check_dependency_invariants(this);
}

const ScoreStatesTemp &ModelObject::get_required_score_states() const {
  IMP_USAGE_CHECK(get_has_required_score_states(),
                  "Required score states have not been computed yet.");
  return get_model()->do_get_required_score_states(this);
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
  return get_model();
}

void ModelObject::set_model(Model *m) {
  if (model_) {
    model_->do_remove_model_object(this);
  }
  model_ = m;
  if (model_) {
    model_->do_add_model_object(this);
  }
  do_set_model(m);
}

ModelObject::ModelObject(std::string name)
  : base::Object(name) {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1,
                                    "Pass the Model to the constructor.");

}


IMPKERNEL_END_NAMESPACE
