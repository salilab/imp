/**
 *  \file RemoveRigidMOtionOptimizerState.cpp
 *  \brief Maintains temperature during molecular dynamics by velocity scaling.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/RemoveRigidMotionOptimizerState.h>
#include <IMP/atom/MolecularDynamics.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZ.h>

IMPATOM_BEGIN_NAMESPACE

RemoveRigidMotionOptimizerState::RemoveRigidMotionOptimizerState(
    Model *m, ParticleIndexesAdaptor pis)
    : OptimizerState(m, "RemoveRigidMotionOptimizerState%1%") {
  IMP_FOREACH(ParticleIndex pi, pis) {
    pis_.push_back(m->get_particle(pi));
  }
}

void RemoveRigidMotionOptimizerState::do_update(unsigned int) {
  remove_rigid_motion();
}

void RemoveRigidMotionOptimizerState::remove_rigid_motion() const {
  set_was_used(true);
  remove_linear();
  remove_angular();
}

void RemoveRigidMotionOptimizerState::remove_linear() const {

  algebra::Vector3D cm(0., 0., 0.);
  Float cm_mass = 0.;

  for (Particles::const_iterator pi = pis_.begin(); pi != pis_.end();
       ++pi) {
    Particle *p = *pi;

    Float mass = Mass(p).get_mass();
    cm_mass += mass;
    cm += mass * LinearVelocity(p).get_velocity();
  }

  for (Particles::const_iterator pi = pis_.begin(); pi != pis_.end();
       ++pi) {
    Particle *p = *pi;

    LinearVelocity v(p);
    v.set_velocity(v.get_velocity() - cm / cm_mass);
  }
}

void RemoveRigidMotionOptimizerState::remove_angular() const {

  algebra::Vector3D vl(0,0,0);
  Float inertia[3][3];

  for (unsigned i = 0; i < 3; ++i) {
    for (unsigned j = 0; j < 3; ++j) {
      inertia[i][j] = 0.;
    }
  }

  for (Particles::const_iterator pi = pis_.begin(); pi != pis_.end();
       ++pi) {
    Particle *p = *pi;

    Float mass = Mass(p).get_mass();
    LinearVelocity lv(p);

    algebra::Vector3D x = core::XYZ(p).get_coordinates();
    algebra::Vector3D vx = lv.get_velocity();

    vl += algebra::get_vector_product(x, vx) * mass;

    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j) inertia[i][j] -= mass * x[i] * x[j];
  }

  Float trace = inertia[0][0] + inertia[1][1] + inertia[2][2];
  for (unsigned i = 0; i < 3; ++i) inertia[i][i] -= trace;

  Float a = inertia[0][0];
  Float b = inertia[1][1];
  Float c = inertia[2][2];
  Float d = inertia[0][1];
  Float e = inertia[0][2];
  Float f = inertia[1][2];
  Float o = vl[0];
  Float r = vl[1];
  Float q = vl[2];

  Float af_de = a * f - d * e;
  Float aq_eo = a * q - e * o;
  Float ab_dd = a * b - d * d;
  Float ac_ee = a * c - e * e;

  // Avoid division by zero
  if (a == 0. || af_de == 0. || (af_de * af_de - ab_dd * ac_ee) == 0.) {
    return;
  }

  algebra::Vector3D oo;
  oo[2] = (af_de * (a * r - d * o) - ab_dd * aq_eo) /
          (af_de * af_de - ab_dd * ac_ee);
  oo[1] = (aq_eo - oo[2] * ac_ee) / af_de;
  oo[0] = (o - d * oo[1] - e * oo[2]) / a;

  for (Particles::const_iterator pi = pis_.begin(); pi != pis_.end();
       ++pi) {
    Particle *p = *pi;

    LinearVelocity lv(p);
    algebra::Vector3D x = core::XYZ(p).get_coordinates();

    algebra::Vector3D v = algebra::get_vector_product(oo, x);
    lv.set_velocity(lv.get_velocity() - v);
  }
}

IMPATOM_END_NAMESPACE
