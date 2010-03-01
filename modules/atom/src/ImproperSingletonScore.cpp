/**
 *  \file atom/ImproperSingletonScore.h
 *  \brief A score on the deviation of an improper angle from ideality.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/atom/ImproperSingletonScore.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

ImproperSingletonScore::ImproperSingletonScore(UnaryFunction *f): f_(f){}

double ImproperSingletonScore::evaluate(Particle *b,
                                        DerivativeAccumulator *da) const
{
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    Dihedral::decorate_particle(b);
  }
  Dihedral id(b);
  Float ideal = id.get_ideal();
  Float s = id.get_stiffness();
  if (s == 0.) {
    return 0.;
  }
  core::XYZ d[4];
  for (unsigned int i = 0; i < 4; ++i) {
    d[i] = core::XYZ(id.get_particle(i));
  }
  if (da) {
    algebra::Vector3D derv[4];
    double dih = core::internal::dihedral(d[0], d[1], d[2], d[3], &derv[0],
                                          &derv[1], &derv[2], &derv[3]);
    double diff = core::internal::get_angle_difference(ideal, dih);
    DerivativePair dp = f_->evaluate_with_derivative(s * diff);
    for (unsigned int i = 0; i < 4; ++i) {
      d[i].add_to_derivatives(derv[i] * s * dp.second, *da);
    }
    return dp.first;
  } else {
    double dih = core::internal::dihedral(d[0], d[1], d[2], d[3], NULL,
                                          NULL, NULL, NULL);
    double diff = core::internal::get_angle_difference(dih, ideal);
    return f_->evaluate(s * diff);
  }
}

ParticlesList
ImproperSingletonScore::get_interacting_particles(Particle *p) const {
  ParticlesList ret(1,ParticlesTemp(4));
  Dihedral bd(p);
  ret[0][0]= bd.get_particle(0);
  ret[0][1]= bd.get_particle(1);
  ret[0][2]= bd.get_particle(2);
  ret[0][3]= bd.get_particle(3);
  return ret;
}

ContainersTemp ImproperSingletonScore::get_input_containers(Particle *p) const {
  return ContainersTemp(1,p);
}

ParticlesTemp ImproperSingletonScore::get_input_particles(Particle *p) const {
  ParticlesTemp ret(4);
  Dihedral bd(p);
  ret[0]= bd.get_particle(0);
  ret[1]= bd.get_particle(1);
  ret[2]= bd.get_particle(2);
  ret[3]= bd.get_particle(3);
  return ret;
}

bool ImproperSingletonScore::get_is_changed(Particle *p) const {
  Dihedral b(p);
  return b.get_particle(0)->get_is_changed()
    || b.get_particle(1)->get_is_changed()
    || b.get_particle(2)->get_is_changed()
    || b.get_particle(3)->get_is_changed();
}

void ImproperSingletonScore::do_show(std::ostream &out) const
{
  out << "function " << *f_ << std::endl;
}

IMPATOM_END_NAMESPACE
