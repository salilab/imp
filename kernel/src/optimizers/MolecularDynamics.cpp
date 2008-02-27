/**
 *  \file MolecularDynamics.cpp  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/optimizers/MolecularDynamics.h"
#include "IMP/decorators/XYZDecorator.h"

#include <cmath>

namespace IMP
{

//! Constructor
MolecularDynamics::MolecularDynamics() : time_step_(4.0)
{
  cs_[0] = FloatKey("x");
  cs_[1] = FloatKey("y");
  cs_[2] = FloatKey("z");
  masskey_ = FloatKey("mass");
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}


//! Destructor
MolecularDynamics::~MolecularDynamics()
{
}

IMP_LIST_IMPL(MolecularDynamics, Particle, particle, Particle*,
              {
                if (0) std::cout << index;
                for (unsigned int i=0; i< 3; ++i) {
                  if (!obj->has_attribute(vs_[i])) {
                    obj->add_attribute(vs_[i], 0.0, false);
                  }
                }
              },);


void MolecularDynamics::setup_particles()
{
  clear_particles();

  for (unsigned int i = 0; i < get_model()->number_of_particles(); ++i) {
    Particle *p = get_model()->get_particle(i);
    if (p->has_attribute(cs_[0]) && p->get_is_optimized(cs_[2])
        && p->has_attribute(cs_[1]) && p->get_is_optimized(cs_[2])
        && p->has_attribute(cs_[2]) && p->get_is_optimized(cs_[2])
        && p->has_attribute(masskey_) && !p->get_is_optimized(masskey_)) {
      add_particle(p);
    }
  }
}


//! Perform a single dynamics step.
void MolecularDynamics::step()
{
  // Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
  // and mass is in g/mol, conversion factor necessary to get accelerations
  // in angstrom/fs/fs from raw derivatives
  static const Float deriv_to_acceleration = -4.1868e-4;

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;
    Float x = p->get_value(cs_[0]);
    Float y = p->get_value(cs_[1]);
    Float z = p->get_value(cs_[2]);
    Float invmass = 1.0 / p->get_value(masskey_);
    Float dvx = p->get_derivative(cs_[0]);
    Float dvy = p->get_derivative(cs_[1]);
    Float dvz = p->get_derivative(cs_[2]);

    // calculate velocity at t+(delta t/2) from that at t-(delta t/2)
    Float vx = p->get_value(vs_[0]);
    Float vy = p->get_value(vs_[1]);
    Float vz = p->get_value(vs_[2]);
    vx += dvx * deriv_to_acceleration * invmass * time_step_;
    vy += dvy * deriv_to_acceleration * invmass * time_step_;
    vz += dvz * deriv_to_acceleration * invmass * time_step_;
    p->set_value(vs_[0], vx);
    p->set_value(vs_[1], vy);
    p->set_value(vs_[2], vz);

    // get atomic shift
    Float dx = vx * time_step_;
    Float dy = vy * time_step_;
    Float dz = vz * time_step_;

    // calculate position at t+(delta t) from that at t
    x += dx;
    y += dy;
    z += dz;
    p->set_value(cs_[0], x);
    p->set_value(cs_[1], y);
    p->set_value(cs_[2], z);
  }
}


//! Optimize the model.
/** \param[in] max_steps   Maximum number of iterations before aborting.
    \return score of the final state of the model.
 */
Float MolecularDynamics::optimize(unsigned int max_steps)
{
  Model *model = get_model();
  setup_particles();

  // get initial system score
  Float score = model->evaluate(true);

  for (unsigned int i = 0; i < max_steps; ++i) {
    update_states();
    step();
    score = model->evaluate(true);
  }
  return score;
}

} // namespace IMP
