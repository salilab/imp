/**
 *  \file BrownianDynamics.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/random.h"
#include "IMP/internal/constants.h"
#include "IMP/internal/units.h"
#include "IMP/optimizers/BrownianDynamics.h"
#include "IMP/decorators/XYZDecorator.h"
#include <boost/random/normal_distribution.hpp>

#include <cmath>

namespace IMP
{
typedef internal::MKSUnit<-3, -1, 1, 0, -1> MilliPascalSeconds;

static MilliPascalSeconds eta(internal::Kelvin T)
{
  const std::pair<internal::Kelvin, MilliPascalSeconds> points[]
    ={ std::make_pair(internal::Kelvin(273+10.0),
                      MilliPascalSeconds(1.308)),
       std::make_pair(internal::Kelvin(273+20.0),
                      MilliPascalSeconds(1.003)),
       std::make_pair(internal::Kelvin(273+30.0),
                      MilliPascalSeconds(0.7978)),
       std::make_pair(internal::Kelvin(273+40.0),
                      MilliPascalSeconds(0.6531)),
       std::make_pair(internal::Kelvin(273+50.0),
                      MilliPascalSeconds(0.5471)),
       std::make_pair(internal::Kelvin(273+60.0),
                      MilliPascalSeconds(0.4668)),
       std::make_pair(internal::Kelvin(273+70.0),
                      MilliPascalSeconds(0.4044)),
       std::make_pair(internal::Kelvin(273+80.0),
                      MilliPascalSeconds(0.3550)),
       std::make_pair(internal::Kelvin(273+90.0),
                      MilliPascalSeconds(0.3150)),
       std::make_pair(internal::Kelvin(273+100.0),
                      MilliPascalSeconds(0.2822))};

  const unsigned int npoints= sizeof(points)/sizeof(std::pair<float,float>);
  if (T < points[0].first) {
    return points[0].second;
  } else {
    for (unsigned int i=1; i< npoints; ++i) {
      if (points[i].first > T) {
        internal::Scalar f= (T - points[i-1].first)/(points[i].first
                                                     - points[i-1].first);
        MilliPascalSeconds ret= 
          (internal::Scalar(1.0)-f) *points[i-1].second + f*points[i].second;
        return ret;
      }
    }
  }
  return points[npoints-1].second;
}

BrownianDynamics::BrownianDynamics(FloatKey dkey) :
  max_change_(10), max_dt_(1e7), cur_dt_(max_dt_), cur_time_(0),
  T_(300.0), dkey_(dkey)
{
  IMP_check(dkey_ != FloatKey(), "BrownianDynamics needs a valid key for the "
            << "diffusion coefficient",
            ValueException("Bad diffusion coef key"));
}


BrownianDynamics::~BrownianDynamics()
{
}

IMP_LIST_IMPL(BrownianDynamics, Particle, particle, Particle*,
              {if (0) std::cout << obj << index;},);


void BrownianDynamics::set_time_step(internal::FemtoSecond t)
{
  time_steps_.clear();
  max_dt_ = t;
  cur_dt_= max_dt_;
}

void BrownianDynamics::setup_particles()
{
  clear_particles();
  FloatKeys xyzk=XYZDecorator::get_xyz_keys();
  for (unsigned int i = 0; i < get_model()->number_of_particles(); ++i) {
    Particle *p = get_model()->get_particle(i);
    if (p->has_attribute(xyzk[0]) && p->get_is_optimized(xyzk[0])
        && p->has_attribute(xyzk[1]) && p->get_is_optimized(xyzk[1])
        && p->has_attribute(xyzk[2]) && p->get_is_optimized(xyzk[2])
        && p->has_attribute(dkey_) && !p->get_is_optimized(dkey_)) {
      add_particle(p);
    }
  }
}

// rt_dt= rt+Fdt/psi + sqrt(2kTdt/psi)omega
// omega has variance 6 phi kT


/*
  radius
  if step is xi*radius^2/(3pi E) for some E=1
  then the motion per step should be sqrt(2kT/(pi E))

  T* is 
 */


bool BrownianDynamics::step()
{
  std::vector<Vector3D> new_pos(number_of_particles());
  bool noshrink=true;
  while (!propose_step(new_pos)) {
    cur_dt_= cur_dt_/internal::Scalar(2.0);
    noshrink=false;
  }

  for (unsigned int i=0; i< number_of_particles(); ++i) {
    XYZDecorator d(get_particle(i));
    for (unsigned int j=0; j< 3; ++j) {
      d.set_coordinate(j, new_pos[i][j]);
    }
  }
  return noshrink;
}


//! Perform a single dynamics step.
bool BrownianDynamics::propose_step(std::vector<Vector3D>& new_pos)
{
  // Assuming score is in kcal/mol, its derivatives in kcal/mol/angstrom,
  // and mass is in g/mol, conversion factor necessary to get accelerations
  // in angstrom/fs/fs from raw derivatives
  // we want 
  //const double eta= 10;// kg/(m s) from ~1 cg/(cm s)
  //const double pi=M_PI;
  //const double kTnu= 1.3806504*T_.get_value();
  FloatKeys xyzk=XYZDecorator::get_xyz_keys();

  IMP_LOG(VERBOSE, "dt is " << cur_dt_ << std::endl);


  for (unsigned int i=0; i< number_of_particles(); ++i) {
    Particle *p= get_particle(i);
    XYZDecorator d(p);
    //double xi= 6*pi*eta*radius; // kg/s
    internal::Cm2PerSecond D(p->get_value(dkey_));
    internal::Angstrom sigma(compute_sigma_from_D(D));
#ifndef NDEBUG
    internal::Angstrom osigma(sqrt(internal::Scalar(2.0)*D*cur_dt_));
#endif
    IMP_assert(sigma
               - osigma
               <= internal::Scalar(.01)* sigma,
               "Sigma computations don't match " << sigma 
               << " " 
               << sqrt(internal::Scalar(2.0)*D*cur_dt_));
    IMP_LOG(VERBOSE, p->get_index() << ": sigma is " 
            << sigma << std::endl);
    boost::normal_distribution<double> mrng(0, sigma.get_value());
    boost::variate_generator<RandomNumberGenerator&,
      boost::normal_distribution<double> >
      sampler(random_number_generator, mrng);

    //std::cout << p->get_index() << std::endl;

    internal::Angstrom delta[3];

    for (unsigned j = 0; j < 3; ++j) {
      // force originally in kcal/mol/A
      // derive* 2.390e-4 gives it in J/(mol A)
      // that * e10 gives it in J/(mol m)
      // that / NA gives it in J/m
      internal::KCalPerAMol force(-d.get_coordinate_derivative(j));
      //*4.1868e+13/NA; old conversion
      internal::FemtoNewton nforce= convert_to_mks(force);
      internal::Angstrom R(sampler());
      internal::Angstrom force_term=nforce*cur_dt_*D/kt();
      //std::cout << "Force term is " << force_term << " and R is "
      //<< R << std::endl;
      internal::Angstrom dr= force_term +  R; //std::sqrt(2*kb*T_*ldt/xi) *
      // get back from meters
      delta[j]=dr;
    }
    //internal::Angstrom max_motion= internal::Scalar(4)*sigma;
    /*std::cout << "delta is " << delta << " mag is " 
      << delta.get_magnitude() << " sigma " << sigma << std::endl;*/

    IMP_LOG(VERBOSE, "For particle " << p->get_index()
            << " delta is " << delta[0] << " " << delta[1] << " " << delta[2]
            << " from a force of " 
            << "[" << d.get_coordinate_derivative(0)
            << ", " << d.get_coordinate_derivative(1)
            << ", " << d.get_coordinate_derivative(2) << "]" << std::endl);

    internal::SquaredAngstrom t= square(delta[0]);

    internal::SquaredAngstrom magnitude2=square(delta[0])+square(delta[1])
      +square(delta[2]);

    //internal::SquaredAngstrom max_motion2= square(max_motion);
    if (magnitude2 > square(max_change_)) {
      return false;
    }
    for (unsigned int j=0; j< 3; ++j) {
      new_pos[i][j]= d.get_coordinate(j) + delta[j].get_value();
    }
  }
  return true;
}


//! Optimize the model.
/** \param[in] max_steps   Maximum number of iterations before aborting.
    \return score of the final state of the model.
 */
Float BrownianDynamics::optimize(unsigned int max_steps)
{
  setup_particles();
  IMP_LOG(SILENT, "Running brownian dynamics on " << get_particles().size() 
          << " particles with a step of " << cur_dt_ << std::endl);
  unsigned int num_const_dt=0;
  for (unsigned int i = 0; i < max_steps; ++i) {
    update_states();
    get_model()->evaluate(true);
    if (step()) {
      ++num_const_dt;
    }

    cur_time_= cur_time_+cur_dt_;
    if (num_const_dt == 10) {
      num_const_dt=0;
      cur_dt_= std::min(cur_dt_*internal::Scalar(2), max_dt_);
    }
    {
      internal::FemtoSecond lb(max_dt_/internal::Scalar(2.0));
      std::vector<int>::size_type d=0;
      while (lb > cur_dt_) {
        lb = lb/internal::Scalar(2.0);
        ++d;
      }
      time_steps_.resize(std::max(time_steps_.size(), d+1), 0);
      ++time_steps_[d];
    }
  }
  return get_model()->evaluate(false);
}



internal::FemtoJoule BrownianDynamics::kt() const
{
  return internal::KB*T_;
}


internal::Angstrom
BrownianDynamics::estimate_radius_from_mass_units(Float mass_in_kd) const
{
  /* Data points used:
     thyroglobulin 669kD 85A
     catalase 232kD 52A
     aldolase 158kD 48A
   */

  internal::Angstrom r(std::pow(mass_in_kd, .3333f)*10);
  return r;
}

internal::Cm2PerSecond
BrownianDynamics::estimate_D_from_radius(internal::Angstrom r) const
{
  MilliPascalSeconds e=eta(T_);
  internal::MKSUnit<-13, 0, 1, 0, -1> etar( e*r);
  /*std::cout << e << " " << etar << " " << kt << std::endl;
  std::cout << "scalar etar " << (internal::Scalar(6*internal::PI)*etar) 
            << std::endl;
  std::cout << "ret pre conv " << (kt/(internal::Scalar(6* internal::PI)*etar))
  << std::endl;*/
  internal::Cm2PerSecond ret( kt()/(internal::Scalar(6* M_PI)*etar));
  //std::cout << "ret " << ret << std::endl;
  return ret;
}

internal::Angstrom
BrownianDynamics::compute_sigma_from_D(internal::Cm2PerSecond D) const
{
  return sqrt(internal::Scalar(2.0)*D*cur_dt_); //6*xi*kb*T_;
}

internal::KCalPerAMol BrownianDynamics
::get_force_scale_from_D(internal::Cm2PerSecond D) const
{
  // force motion is f*cur_dt_*D/kT
  // sigma*kT/(cur_dt_*D)
  return convert_to_kcal(internal::Angstrom(compute_sigma_from_D(D))*kt()
                         /(max_dt_*D));
}


} // namespace IMP
