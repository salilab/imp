/**
 *  \file MolecularDynamics.cpp  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/MolecularDynamics.h>
#include <IMP/core/XYZ.h>

#include <IMP/log.h>
#include <IMP/random.h>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

IMPATOM_BEGIN_NAMESPACE

MolecularDynamics::MolecularDynamics()
{
  initialize();
}

MolecularDynamics::MolecularDynamics(Model *m)
{
  initialize();
  set_model(m);
}


void MolecularDynamics::initialize() {
  time_step_=4.0;
  therm_type_=0;
  degrees_of_freedom_=0;
  velocity_cap_=std::numeric_limits<Float>::max();
  cs_[0] = FloatKey("x");
  cs_[1] = FloatKey("y");
  cs_[2] = FloatKey("z");
  masskey_ = FloatKey("mass");
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}

void MolecularDynamics::do_show(std::ostream &) const {
}

IMP_LIST_IMPL(MolecularDynamics, Particle, particle, Particle*,
              Particles,
              {
                if (0) std::cout << index;
                for (unsigned int i=0; i< 3; ++i) {
                  if (!obj->has_attribute(vs_[i])) {
                    obj->add_attribute(vs_[i], 0.0, false);
                  }
                }
              },{},{});


void MolecularDynamics::setup_particles()
{
  degrees_of_freedom_ = 0;
  clear_particles();

  for (Model::ParticleIterator it= get_model()->particles_begin();
       it != get_model()->particles_end(); ++it) {
    Particle *p= *it;
    if (p->has_attribute(cs_[0]) && p->get_is_optimized(cs_[0])
        && p->has_attribute(cs_[1]) && p->get_is_optimized(cs_[1])
        && p->has_attribute(cs_[2]) && p->get_is_optimized(cs_[2])
        && p->has_attribute(masskey_) && !p->get_is_optimized(masskey_)) {
      add_particle(p);
      degrees_of_freedom_ += 3;
    }
  }
  degrees_of_freedom_ -= 6;
}

void MolecularDynamics::set_thermostat(unsigned int type,
                                       Float temperature,
                                       Float time_friction)
{

 therm_type_ = type;

 if(type==0) return;
 if(type==1) therm_temp_ = temperature;
 if(type==2 || type==3) {therm_temp_ = temperature; therm_tf_ = time_friction;}

}

void MolecularDynamics::do_therm()
{

 double rescale, c1=0, c2=0;
 static const Float gas_constant = 8.31441e-7;

 // NVE
 if(therm_type_==0) return;
 // scale velocities
 if(therm_type_==1)
  rescale = sqrt(therm_temp_/get_kinetic_temperature(get_kinetic_energy()));
 // Berendsen
 if(therm_type_==2)
  rescale = sqrt(1.0-(time_step_/therm_tf_)*(1.0-(therm_temp_/
            get_kinetic_temperature(get_kinetic_energy()))));
 // Langevin
 if(therm_type_==3){
  c1 = exp(-therm_tf_*time_step_);
  c2 = sqrt((1.0-c1)*gas_constant*therm_temp_);
 }

 boost::normal_distribution<Float> mrng(0., 1.);
 boost::variate_generator<RandomNumberGenerator&,
                          boost::normal_distribution<Float> >
     sampler(random_number_generator, mrng);

 for (ParticleIterator iter = particles_begin();
      iter != particles_end(); ++iter) {
   Particle *p = *iter;

   double mass = p->get_value(masskey_);

   for (int i = 0; i < 3; ++i) {
     double velocity = p->get_value(vs_[i]);

     if(therm_type_==1 || therm_type_==2) velocity *= rescale;
     if(therm_type_==3)
      velocity = c1*velocity+c2*sqrt((c1+1.0)/mass)*sampler();

     p->set_value(vs_[i], velocity);
   }
 }

}

//! First part of velocity verlet
void MolecularDynamics::step_1()
{
  // Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
  // and mass is in g/mol, conversion factor necessary to get accelerations
  // in angstrom/fs/fs from raw derivatives
  static const Float deriv_to_acceleration = -4.1868e-4;

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;
    Float invmass = 1.0 / p->get_value(masskey_);
    for (unsigned i = 0; i < 3; ++i) {
      Float coord = p->get_value(cs_[i]);
      Float dcoord = p->get_derivative(cs_[i]);
      Float velocity = p->get_value(vs_[i]);

//    calculate position at t+(delta t) from that at t
      coord += time_step_ * velocity +
               0.5 * time_step_ * time_step_ * dcoord
               * deriv_to_acceleration * invmass;
      p->set_value(cs_[i], coord);

//    calculate velocity at t+(delta t/2) from that at t
      velocity += 0.5 * time_step_ * dcoord * deriv_to_acceleration * invmass;

      //cap_velocity_component(velocity);
      p->set_value(vs_[i], velocity);

    }
  }
}


//! Second part of velocity verlet
void MolecularDynamics::step_2()
{
  // Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
  // and mass is in g/mol, conversion factor necessary to get accelerations
  // in angstrom/fs/fs from raw derivatives
  static const Float deriv_to_acceleration = -4.1868e-4;

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;
    Float invmass = 1.0 / p->get_value(masskey_);
    for (unsigned i = 0; i < 3; ++i) {
      Float dcoord = p->get_derivative(cs_[i]);
      Float velocity = p->get_value(vs_[i]);


      // calculate velocity at t+delta t from that at t+(delta t/2)
      velocity += 0.5 * time_step_ * dcoord * deriv_to_acceleration * invmass;

      //cap_velocity_component(velocity);
      p->set_value(vs_[i], velocity);

    }
  }
}


double MolecularDynamics::do_optimize(unsigned int max_steps)
{
  setup_particles();

  // get initial system score
  Float score = evaluate(true);

  for (unsigned int i = 0; i < max_steps; ++i) {
    update_states();
    step_1();
    score = evaluate(true);
    step_2();
    //remove_linear();
    //remove_angular();
    do_therm();
  }
  return score;
}

Float MolecularDynamics::get_kinetic_energy() const
{
  // Conversion factor to get energy in kcal/mol from velocities in A/fs and
  // mass in g/mol
  static const Float conversion = 1.0 / 4.1868e-4;

  Float ekinetic = 0.;
  for (ParticleConstIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;
    Float vx = p->get_value(vs_[0]);
    Float vy = p->get_value(vs_[1]);
    Float vz = p->get_value(vs_[2]);
    Float mass = p->get_value(masskey_);

    ekinetic += mass * (vx * vx + vy * vy + vz * vz);
  }
  return 0.5 * ekinetic * conversion;
}

Float MolecularDynamics::get_kinetic_temperature(Float ekinetic) const
{
  if (degrees_of_freedom_ == 0) {
    return 0.;
  } else {
    // E = (n/2)kT  n=degrees of freedom, k = Boltzmann constant
    // Boltzmann constant, in kcal/mol
    const Float boltzmann = 8.31441 / 4186.8;
    return 2.0 * ekinetic / (degrees_of_freedom_ * boltzmann);
  }
}

void MolecularDynamics::remove_linear()
{

  Float cm[3];
  Float cm_mass = 0.;

  for (unsigned i = 0; i < 3; ++i) cm[i] = 0.;

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;

    Float mass =  p->get_value(masskey_);
    cm_mass +=mass;

    for (unsigned i = 0; i < 3; ++i) {
      Float velocity = p->get_value(vs_[i]);
      cm[i] += mass * velocity;
    }
  }

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;

    for (unsigned i = 0; i < 3; ++i) {
      Float velocity = p->get_value(vs_[i]);

      velocity -= cm[i]/cm_mass;
      p->set_value(vs_[i], velocity);
    }
  }
}

void MolecularDynamics::remove_angular()
{

  Float x[3],vx[3],v[3],vl[3],oo[3];
  Float inertia[3][3];

  for (unsigned i = 0; i < 3; ++i) {
   vl[i] = 0.;
   for (unsigned j = 0; j < 3; ++j) {
    inertia[i][j] = 0.;
   }
  }

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;

    Float mass =  p->get_value(masskey_);

    for (unsigned i = 0; i < 3; ++i) {
      x[i]   = p->get_value(cs_[i]);
      vx[i]  = p->get_value(vs_[i]);
    }

    v[0] = x[1]* vx[2] - x[2]*vx[1];
    v[1] = x[2]* vx[0] - x[0]*vx[2];
    v[2] = x[0]* vx[1] - x[1]*vx[0];

    for (unsigned i = 0; i < 3; ++i) vl[i] += v[i] * mass;

    for (unsigned i = 0; i < 3; ++i)
     for (unsigned j = 0; j < 3; ++j)
      inertia[i][j] -= mass * x[i] * x[j];
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

   Float af_de = a*f-d*e;
   Float aq_eo = a*q-e*o;
   Float ab_dd = a*b-d*d;
   Float ac_ee = a*c-e*e;

   oo[2] = (af_de*(a*r-d*o)-ab_dd*aq_eo) / (af_de*af_de-ab_dd*ac_ee);
   oo[1] = (aq_eo - oo[2]*ac_ee)/af_de;
   oo[0] = (o - d*oo[1] - e*oo[2])/a;

   for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;

    for (unsigned i = 0; i < 3; ++i) {
      x[i]   = p->get_value(cs_[i]);
      vx[i]  = p->get_value(vs_[i]);
    }

    v[0] = oo[1]* x[2] - oo[2]*x[1];
    v[1] = oo[2]* x[0] - oo[0]*x[2];
    v[2] = oo[0]* x[1] - oo[1]*x[0];

    for (int i = 0; i < 3; ++i) {
      vx[i]  -= v[i];
      p->set_value(vs_[i], vx[i]);
    }
   }
}

void MolecularDynamics::assign_velocities(Float temperature)
{

  setup_particles();

  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<Float> >
      sampler(random_number_generator, mrng);

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      p->set_value(vs_[i], sampler());
    }
  }

  //remove_linear();
  //remove_angular();

  Float rescale = sqrt(temperature/
                  get_kinetic_temperature(get_kinetic_energy()));

  for (ParticleIterator iter = particles_begin();
       iter != particles_end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      Float velocity = p->get_value(vs_[i]);
      velocity *= rescale;
      p->set_value(vs_[i], velocity);
    }

  }
}


IMPATOM_END_NAMESPACE
