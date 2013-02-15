/**
 *  \file ProjectionParameters.cpp
 *  \brief Decorator for projection parameters
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/ProjectionParameters.h"
#include "IMP/Model.h" // ParticleIterator
#include "IMP/log.h"

IMPEM2D_BEGIN_NAMESPACE

bool ProjectionParameters::particle_is_instance(Particle *p) {
  // Check with only the first key
  return p->has_attribute(get_keys()[0]);
}

ProjectionParameters ProjectionParameters::setup_particle(Particle *p) {
  p->add_attribute(get_keys()[0],0.0);
  p->add_attribute(get_keys()[1],0.0);
  p->add_attribute(get_keys()[2],0.0);
  p->add_attribute(get_keys()[3],0.0);
  p->add_attribute(get_keys()[4],0.0);
  p->add_attribute(get_keys()[5],0.0);
  return ProjectionParameters(p);
}


void ProjectionParameters::show(std::ostream &os) const {
  os << "ProjectionParameters: Rotation " << get_rotation()
     << " | Tranlation (x,y,z) = " << get_translation();
}

const FloatKeys& ProjectionParameters::get_keys() {
  static FloatKeys keys(6);
  keys[0] = FloatKey("rot_q1");
  keys[1] = FloatKey("rot_q2");
  keys[2] = FloatKey("rot_q3");
  keys[3] = FloatKey("rot_q4");
  keys[4] = FloatKey("translation_x");
  keys[5] = FloatKey("translation_y");
  return keys;
}


void ProjectionParameters::set_proper_ranges_for_keys(Model *m,
                           const algebra::Vector3D &min_translation_values,
                           const algebra::Vector3D &max_translation_values) {
  // Range for the quaternion of the rotation
  IMP::FloatRange range_q(0,1);
  for (unsigned int i=0;i<4;++i) {
    m->set_range(get_keys()[i],range_q);
  }
  // Ranges for the translations
  IMP::FloatRange xrange(min_translation_values[0],max_translation_values[0]);
  IMP::FloatRange yrange(min_translation_values[1],max_translation_values[1]);
  m->set_range(get_keys()[4],xrange);
  m->set_range(get_keys()[5],yrange);
}


void ProjectionParametersScoreState::do_before_evaluate() {
  // Nothing here
}


void ProjectionParametersScoreState::do_after_evaluate(
                                                DerivativeAccumulator *) {

  IMP_LOG_VERBOSE(ProjectionParameters(proj_params_));
}

void ProjectionParametersScoreState::do_show(std::ostream& out) const {
  // Dummy line, nothing here
  out << "ProjectionParametersScoreState" << std::endl;
}

ParticlesTemp ProjectionParametersScoreState::get_input_particles() const {
  // simply return the particle with the projection parameters
  ParticlesTemp used;
  used.push_back(proj_params_);
  return used;
}

ParticlesTemp ProjectionParametersScoreState::get_output_particles() const {
  // simply return the particle with the projection parameters
  ParticlesTemp used;
  used.push_back(proj_params_);
  return used;
}

ContainersTemp ProjectionParametersScoreState::get_input_containers() const {
  ContainersTemp ot;
  return ot;
}

ContainersTemp ProjectionParametersScoreState::get_output_containers() const {
  // Nothing here in this case
  ContainersTemp ot;
  return ot;
}


IMPEM2D_END_NAMESPACE
