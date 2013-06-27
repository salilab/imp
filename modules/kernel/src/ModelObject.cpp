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

IMPKERNEL_BEGIN_NAMESPACE

ModelObject::ModelObject(Model *m, std::string name)
    : Tracked(this, m, name), has_dependencies_(false) {}

void ModelObject::set_has_dependencies(bool tf, const ScoreStatesTemp &ss) {
  has_dependencies_ = tf;
  IMP_USAGE_CHECK(
      std::find(ss.begin(), ss.end(), dynamic_cast<ScoreState *>(this)) ==
          ss.end(),
      "An object can't depend on itself");
  IMP_LOG_VERBOSE(get_name() << " depends on " << ss << std::endl);
  required_score_states_ = ss;
}

void ModelObject::set_has_dependencies(bool tf) {
  IMP_OBJECT_LOG;
  if (tf == has_dependencies_) return;
  if (!tf) {
    has_dependencies_ = false;
  } else {
    if (!get_model()) {
      has_dependencies_ = true;
      return;
    }
    if (!get_model()->get_has_dependencies()) {
      get_model()->set_has_dependencies(true);
    }
    ScoreStatesTemp ss;
    ModelObjectsTemp inputs = get_inputs();
    for (unsigned int i = 0; i < inputs.size(); ++i) {
      ss += inputs[i]->get_required_score_states();
      ScoreState *css = dynamic_cast<ScoreState *>(inputs[i].get());
      if (css) {
        ss.push_back(css);
      }
    }
    set_has_dependencies(true, get_update_order(ss));
  }
  do_set_has_dependencies(tf);
}

ModelObjectsTemp ModelObject::get_inputs() const { return do_get_inputs(); }
ModelObjectsTemp ModelObject::get_outputs() const { return do_get_outputs(); }

ModelObjectsTemps ModelObject::do_get_interactions() const {
  return ModelObjectsTemps(1, do_get_inputs() + do_get_outputs());
}

ModelObjectsTemps ModelObject::get_interactions() const {
  return do_get_interactions();
}

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


///////////////////////////////////////// DEPRECATED

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
    : Tracked(name), has_dependencies_(false) {
  IMPKERNEL_DEPRECATED_METHOD_DEF(2.1,
                                    "Pass the Model to the constructor.");

}


IMPKERNEL_END_NAMESPACE
