/**
 *  \file ProjectionParameters.cpp
 *  \brief Decorator for projection parameters
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
**/

#include "IMP/em2D/ProjectionParameters.h"
#include "IMP/Model.h" // ParticleIterator

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
  os << "(Phi,Theta,Psi) = ( " << get_Phi() << " , " << get_Theta() << " , "
     << get_Psi() << " ) | Tranlation (x,y,z) = " << get_translation();
}

FloatKey* ProjectionParameters::get_keys() {
  static FloatKey keys[]= {FloatKey("projection_Phi"),
                           FloatKey("projection_Theta"),
                           FloatKey("projection_Psi"),
                           FloatKey("projection_translation_x"),
                           FloatKey("projection_translation_y"),
                           FloatKey("projection_translation_z")};
  return keys;
}

void ProjectionParameters::set_proper_ranges_for_keys(Model *m,
                                algebra::Vector3D &min_translation_values,
                                algebra::Vector3D &max_translation_values) {
  IMP::FloatRange range_phi(0,2*PI);
  IMP::FloatRange range_theta(0,PI/2);
  IMP::FloatRange range_psi(0,2*PI);
  m->set_range(get_keys()[0],range_phi);
  m->set_range(get_keys()[1],range_theta);
  m->set_range(get_keys()[2],range_psi);
  IMP::FloatRange xrange(min_translation_values[0],max_translation_values[0]);
  IMP::FloatRange yrange(min_translation_values[1],max_translation_values[1]);
  IMP::FloatRange zrange(min_translation_values[2],max_translation_values[2]);
  m->set_range(get_keys()[3],xrange);
  m->set_range(get_keys()[4],yrange);
  m->set_range(get_keys()[5],zrange);
}


void ProjectionParametersSS::do_before_evaluate() {
  // Nothing here
}


void ProjectionParametersSS::do_after_evaluate(DerivativeAccumulator *accpt) {
  // Nothing here yet
}

void ProjectionParametersSS::do_show(std::ostream& out) const {
  // Dummy line, nothing here
  out << "ProjectionParametersSS" << std::endl;
}

ParticlesTemp ProjectionParametersSS::get_input_particles() const {
  // simply return the particle with the projection parameters
  ParticlesTemp used;
  used.push_back(proj_params_);
  return used;
}

ParticlesTemp ProjectionParametersSS::get_output_particles() const {
  // simply return the particle with the projection parameters
  ParticlesTemp used;
  used.push_back(proj_params_);
  return used;
}

ContainersTemp ProjectionParametersSS::get_input_containers() const {
  ContainersTemp ot;
  return ot;
}

ContainersTemp ProjectionParametersSS::get_output_containers() const {
  // Nothing here in this case
  ContainersTemp ot;
  return ot;
}

IMPEM2D_END_NAMESPACE
