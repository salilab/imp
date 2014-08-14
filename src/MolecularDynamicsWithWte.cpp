/**
 *  \file MolecularDynamicsWithWte.cpp \brief Molecular dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/membrane/MolecularDynamicsWithWte.h>
#include <IMP/atom/RemoveRigidMotionOptimizerState.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Simulator.h>
#include <IMP/base/log.h>
#include <IMP/base/random.h>
#include <boost/random/normal_distribution.hpp>
#include <cmath>

IMPMEMBRANE_BEGIN_NAMESPACE

MolecularDynamicsWithWte::MolecularDynamicsWithWte
 (Model *m, double emin, double emax, double sigma, double gamma, double w0):
atom::Simulator(m, "MD %1%"){
  initialize();
  min_   = emin;
  max_   = emax;
  sigma_ = sigma;
  gamma_ = gamma;
  w0_    = w0;
  dx_    = sigma / 4.0;
  nbin_  = floor((emax-emin)/dx_)+1;
  bias_.reset(new double[2*nbin_]);
  for(int i=0;i<2*nbin_;++i) {bias_[i]=0.0;}
  deriv_to_acceleration_ = -4.1868e-4;
}

void MolecularDynamicsWithWte::initialize() {
  set_maximum_time_step(4.0);
  degrees_of_freedom_=0;
  velocity_cap_=std::numeric_limits<Float>::max();
  vs_[0] = FloatKey("vx");
  vs_[1] = FloatKey("vy");
  vs_[2] = FloatKey("vz");
}

double MolecularDynamicsWithWte::get_bias(double score) const
{
  int index=floor((score-min_)/dx_);
  index=std::max(0,std::min(nbin_-1,index));
  return bias_[index];
}

double MolecularDynamicsWithWte::get_derivative(double score) const
{
  int index=floor((score-min_)/dx_);
  double der=0.0;
  if(index >= 0 && index < nbin_){der=bias_[index+nbin_];}
  return der;
}

void MolecularDynamicsWithWte::update_bias(double score)
{
  if(score < min_ || score >= max_) return;
  double vbias=get_bias(score);
  double ww=w0_*exp(-vbias/(get_kt()*(gamma_-1.0)));
  int i0=floor((score-4.0*sigma_-min_)/dx_);
  int i1=floor((score+4.0*sigma_-min_)/dx_)+1;
  for (int i=std::max(0,i0);i<=std::min(i1,nbin_-1);++i){
   double xx=min_ + ((double) i)*dx_;
   double dp=(xx-score)/sigma_;
   double newbias = ww*exp(-0.5*dp*dp);
   bias_[i] += newbias;
   bias_[i+nbin_] += -newbias * dp / sigma_;
  }
}

bool MolecularDynamicsWithWte::get_is_simulation_particle
 (ParticleIndex pi) const {
  Particle *p=get_model()->get_particle(pi);
  bool ret=IMP::core::XYZ::particle_is_instance(p)
    && IMP::core::XYZ(p).get_coordinates_are_optimized()
    && IMP::atom::Mass::particle_is_instance(p);
  if (ret) {
    for (unsigned int i=0; i< 3; ++i) {
      if (!p->has_attribute(vs_[i])) {
        p->add_attribute(vs_[i], 0.0, false);
      }
    }
  }
  return ret;
}


void MolecularDynamicsWithWte::setup(const ParticleIndexes &ps)
{
  // Get starting score and derivatives, for first dynamics step velocities
  currentscore_=get_scoring_function()->evaluate(true);

  setup_degrees_of_freedom(ps);
}

void MolecularDynamicsWithWte::setup_degrees_of_freedom
 (const ParticleIndexes &ps)
{
  degrees_of_freedom_ = 3*ps.size();

  // If global rotation and translation have been removed, reduce degrees
  // of freedom accordingly (kind of ugly...)
  for (OptimizerStateIterator o = optimizer_states_begin();
       o != optimizer_states_end(); ++o) {
    OptimizerState *os=*o;
    if (dynamic_cast<atom::RemoveRigidMotionOptimizerState *>(os)) {
      degrees_of_freedom_ -= 6;
      break;
    }
  }
}

//! Perform a single dynamics step.
double MolecularDynamicsWithWte::do_step(const ParticleIndexes &ps,
                                  double ts)
{
  IMP_OBJECT_LOG;
  // Get coordinates at t+(delta t) and velocities at t+(delta t/2)
  propagate_coordinates(ps, ts);

  // Get derivatives at t+(delta t)
  currentscore_=get_scoring_function()->evaluate(true);

// update bias
  update_bias(currentscore_);

  // Get velocities at t+(delta t)
  propagate_velocities(ps, ts);

  return ts;
}

void MolecularDynamicsWithWte::propagate_coordinates(const ParticleIndexes &ps,
                                              double ts)
{

  // store the derivative of the bias with respect to the score
  double dbias = get_derivative(currentscore_);

  for (unsigned int i=0; i< ps.size(); ++i) {
    Float invmass = 1.0 / atom::Mass(get_model(), ps[i]).get_mass();
    for (unsigned j = 0; j < 3; ++j) {
      core::XYZ d(get_model(), ps[i]);

      Float coord = d.get_coordinate(j);
      Float dcoord = d.get_derivative(j);

      // add contribution from bias potential
      dcoord *= ( 1.0 + dbias );

      // calculate velocity at t+(delta t/2) from that at t
      Float velocity = get_model()->get_attribute(vs_[j], ps[i]);
      velocity += 0.5 * dcoord * deriv_to_acceleration_ * invmass * ts;

      cap_velocity_component(velocity);
      get_model()->set_attribute(vs_[j], ps[i], velocity);

      // calculate position at t+(delta t) from that at t
      coord += velocity * ts;
      d.set_coordinate(j, coord);
    }
  }
}

void MolecularDynamicsWithWte::propagate_velocities(const ParticleIndexes &ps,
                                             double ts)
{

  // store the derivative of the bias with respect to the score
  double dbias = get_derivative(currentscore_);

  for (unsigned int i=0; i< ps.size(); ++i) {
    Float invmass = 1.0 / atom::Mass(get_model(), ps[i]).get_mass();
    for (unsigned j = 0; j < 3; ++j) {
      core::XYZ d(get_model(), ps[i]);
      Float dcoord = d.get_derivative(j);

      // add contribution from bias potential
      dcoord *= ( 1.0 + dbias );

      // calculate velocity at t+(delta t) from that at t+(delta t/2)
      Float velocity = get_model()->get_attribute(vs_[j], ps[i]);
      velocity += 0.5 * dcoord * deriv_to_acceleration_ * invmass * ts;

      get_model()->set_attribute(vs_[j], ps[i], velocity);
    }
  }
}

Float MolecularDynamicsWithWte::get_kinetic_energy() const
{
  // Conversion factor to get energy in kcal/mol from velocities in A/fs and
  // mass in g/mol
  static const Float conversion = 1.0 / 4.1868e-4;

  Float ekinetic = 0.;
  ParticlesTemp ps=get_simulation_particles();
  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;
    Float vx = p->get_value(vs_[0]);
    Float vy = p->get_value(vs_[1]);
    Float vz = p->get_value(vs_[2]);
    Float mass = atom::Mass(p).get_mass();

    ekinetic += mass * (vx * vx + vy * vy + vz * vz);
  }
  return 0.5 * ekinetic * conversion;
}

Float MolecularDynamicsWithWte::get_kinetic_temperature(Float ekinetic) const
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


void MolecularDynamicsWithWte::assign_velocities(Float temperature)
{
  ParticleIndexes ips=get_simulation_particle_indexes();
  setup_degrees_of_freedom(ips);
  ParticlesTemp ps= IMP::internal::get_particle(get_model(), ips);

  boost::normal_distribution<Float> mrng(0., 1.);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<Float> >
      sampler(random_number_generator, mrng);

  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      p->set_value(vs_[i], sampler());
    }
  }

  Float rescale = sqrt(temperature/
                  get_kinetic_temperature(get_kinetic_energy()));

  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      Float velocity = p->get_value(vs_[i]);
      velocity *= rescale;
      p->set_value(vs_[i], velocity);
    }

  }
}

void MolecularDynamicsWithWte::rescale_velocities(Float rescale)
{
   ParticlesTemp ps=get_simulation_particles();

  for (ParticlesTemp::iterator iter = ps.begin();
       iter != ps.end(); ++iter) {
    Particle *p = *iter;

    for (int i = 0; i < 3; ++i) {
      Float velocity = p->get_value(vs_[i]);
      velocity *= rescale;
      p->set_value(vs_[i], velocity);
    }

  }
}


IMPMEMBRANE_END_NAMESPACE
