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
void ModelObject::update_dependencies() {
  return do_update_dependencies();
}
ModelObject::ModelObject(Model *m, std::string name):
  Tracked(this, m, name) {
}

ModelObject::ModelObject(std::string name): Tracked(name) {
}

void ModelObject::set_model(Model *m) {
  Tracked::set_tracker(this, m);
}
ModelObjectsTemp ModelObject::get_inputs() const {
  return do_get_inputs();
}
ModelObjectsTemp ModelObject::get_outputs() const {
  return do_get_outputs();
}


ModelObjectsTemps ModelObject::do_get_interactions() const {
  return ModelObjectsTemps(1,do_get_inputs()+do_get_outputs());
}

ModelObjectsTemps ModelObject::get_interactions() const {
  return do_get_interactions();
}


ParticlesTemp get_input_particles(const ModelObjectsTemp &mo) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< mo.size(); ++i) {
    ModelObject *o= mo[i];
    Particle *p= dynamic_cast<Particle*>(o);
    if (p) ret.push_back(p);
    else {
      ret+= get_input_particles(o->get_inputs());
    }
  }
  return ret;
}

ContainersTemp get_input_containers(const ModelObjectsTemp &mo) {
  ContainersTemp ret;
  for (unsigned int i=0; i< mo.size(); ++i) {
    ModelObject *o= mo[i];
    Container *p= dynamic_cast<Container*>(o);
    if (p) ret.push_back(p);
    else {
      ret+= get_input_containers(o->get_inputs());
    }
  }
  return ret;
}


ParticlesTemp get_output_particles(const ModelObjectsTemp &mo) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< mo.size(); ++i) {
    ModelObject *o= mo[i];
    Particle *p= dynamic_cast<Particle*>(o);
    if (p) ret.push_back(p);
    else {
      ret+= get_output_particles(o->get_inputs());
    }
  }
  return ret;
}

ContainersTemp get_output_containers(const ModelObjectsTemp &mo) {
  ContainersTemp ret;
  for (unsigned int i=0; i< mo.size(); ++i) {
    ModelObject *o= mo[i];
    Container *p= dynamic_cast<Container*>(o);
    if (p) ret.push_back(p);
    else {
      ret+= get_output_containers(o->get_inputs());
    }
  }
  return ret;
}

IMPKERNEL_END_NAMESPACE
